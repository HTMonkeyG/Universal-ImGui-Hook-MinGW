#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include "kiero.h"

#include "d3d11hook.h"
#include "globals.h"
#include "utils.h"

#include <stdio.h>

namespace D3D11Hooks {
  typedef long (STDMETHODCALLTYPE *PresentFnD3D11)(IDXGISwapChain *, UINT, UINT);
  typedef ULONG (STDMETHODCALLTYPE *ReleaseFnD3D11)(IDXGISwapChain *);

  // Trampoline function pointers.
  static PresentFnD3D11 oPresentD3D11;
  static ReleaseFnD3D11 oReleaseD3D11;

  // Callbacks.
  static InitCB cbInit = nullptr;
  static PresentCB cbPresent = nullptr;
  static DeinitCB cbDeinit = nullptr;

  // User specified data.
  static void *pUser = nullptr;

  // Data.
  IDXGISwapChain *gSavedSwapChain = nullptr;
  ID3D11Device *gDevice = nullptr;
  ID3D11RenderTargetView *gRenderTargetView = nullptr;
  ID3D11DeviceContext *gDeviceContext = nullptr;
  bool gInit = false;

  // Functions.
  bool createAllFrom(IDXGISwapChain *pSwapChain) {
    DXGI_SWAP_CHAIN_DESC sdesc;

    if (!SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&gDevice)))
      return false;
    if (!SUCCEEDED(pSwapChain->GetDesc(&sdesc)))
      return false;

    if (!UniHookGlobals::mainWindow)
      UniHookGlobals::mainWindow = sdesc.OutputWindow;
    if (!UniHookGlobals::mainWindow)
      UniHookGlobals::mainWindow = GetForegroundWindow();

    ID3D11Texture2D *pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    gDevice->CreateRenderTargetView(pBackBuffer, nullptr, &gRenderTargetView);
    gDevice->GetImmediateContext(&gDeviceContext);
    pBackBuffer->Release();

    if (cbInit)
      cbInit(&sdesc, pUser);

    return true;
  }

  void clearAll() {
    if (cbDeinit)
      cbDeinit(pUser);

    SafeRelease(&gRenderTargetView);
    SafeRelease(&gDeviceContext);
    SafeRelease(&gDevice);
  }

  HRESULT STDMETHODCALLTYPE hookPresentD3D11(
    IDXGISwapChain *pSwapChain,
    UINT SyncInterval,
    UINT Flags
  ) {
    if (!gInit) {
      // Initialize from Present.
      gSavedSwapChain = pSwapChain;
      gInit = createAllFrom(pSwapChain);
    }

    if (gInit) {
      if (!gDevice || !gRenderTargetView)
        return oPresentD3D11(pSwapChain, SyncInterval, Flags);

      if (cbPresent)
        cbPresent(pUser);
    }

    return oPresentD3D11(pSwapChain, SyncInterval, Flags);
  }

  ULONG hookReleaseD3D11(IDXGISwapChain *pSwapChain) {
    if (gInit && pSwapChain == gSavedSwapChain) {
      gInit = false;
      clearAll();
    }
    return oReleaseD3D11(pSwapChain);
  }

  bool init(InitCB init, PresentCB present, DeinitCB deinit, void *lpUser) {
    if (kiero::init(kiero::RenderType::D3D11) != kiero::Status::Success)
      return false;

    cbInit = init;
    cbPresent = present;
    cbDeinit = deinit;
    pUser = lpUser;

    kiero::bind(2, (void **)&oReleaseD3D11, (void *)hookReleaseD3D11);
    kiero::bind(8, (void **)&oPresentD3D11, (void *)hookPresentD3D11);

    return true;
  }

  bool deinit() {
    clearAll();
    kiero::shutdown();
    return true;
  }
}
