#ifndef __UGLH_D3D11HOOK_H__
#define __UGLH_D3D11HOOK_H__

#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>

namespace D3D11Hooks {
  typedef void (__fastcall *InitCB)(const DXGI_SWAP_CHAIN_DESC *, void *);
  typedef void (__fastcall *PresentCB)(void *);
  typedef void (__fastcall *DeinitCB)(void *);

  extern IDXGISwapChain *gSavedSwapChain;
  extern ID3D11Device *gDevice;
  extern ID3D11RenderTargetView *gRenderTargetView;
  extern ID3D11DeviceContext *gDeviceContext;
  extern bool gInit;

  /**
   * Install hooks with given callback functions.
   * 
   * Workflow: 
   * Inject and install hooks.
   * -> Wait for the first call of IDXGISwapChain::Present.
   * -> Get all dx11 data and call InitCB (Initialize overlay renderer).
   * -> Call PresentCB every time IDXGISwapChain::Present is called when the
   *    initialize is done (Render overlay).
   * -> Call DeinitCB when IDXGISwapChain::Release is called (Destroy overlay
   *    renderer and wait for next possible reinitialize).
   * 
   * The resize event is automatically processed.
   * 
   * NOTE: The InitCB may be called multiple times due to possible release 
   * operations on the swap chain of the injected process. DO NOT include
   * operations must be executed only once.
   * 
   * @param init Callback of the initialize of device and swap chain.
   * @param present Function to be call when the Present function is called.
   * @param deinit Function to be call when the Release function is called.
   * @param lpUser User data pointer.
   */
  extern bool init(InitCB init, PresentCB present, DeinitCB deinit, void *lpUser);

  /**
   * Remove all hooks and release dx12 objects created.
   */
  extern bool deinit();
}

#endif
