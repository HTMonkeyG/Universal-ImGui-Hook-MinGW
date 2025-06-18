#include "dllmain.h"

#if USE_GL == 0
using namespace D3D12Hooks;
#elif USE_GL == 1
using namespace D3D11Hooks;
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
  HWND hWnd,
  UINT msg,
  WPARAM wParam,
  LPARAM lParam);

void CALLBACK WndProcEx(
  HWND hWnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  UINT *uBlock,
  void *lpUser
) {
  ImGuiIO &io = ImGui::GetIO();

  ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

  if (
    uMsg == WM_LBUTTONDOWN
    || uMsg == WM_LBUTTONDBLCLK
    || uMsg == WM_LBUTTONUP
  )
    *uBlock = io.WantCaptureMouse;
  if (
    uMsg == WM_KEYDOWN
    || uMsg == WM_CHAR
  )
    *uBlock = io.WantCaptureKeyboard;
}

unsigned long __stdcall onAttach() {
#if USE_GL == 0
  D3D12Hooks::init(
    [](const DXGI_SWAP_CHAIN_DESC *desc, void *lpUser) -> void {
      if (useGL)
        return;
      useGL = 1;

      ImGui::CreateContext();
      ImGui::StyleColorsDark();

      ImGuiIO &io = ImGui::GetIO(); (void)io;
      io.Fonts->AddFontDefault();
      io.IniFilename = NULL;

      ImGui_ImplWin32_Init(desc->OutputWindow);
      ImGui_ImplDX12_Init(
        D3D12Hooks::gDevice,
        D3D12Hooks::gBufferCount,
        desc->BufferDesc.Format,
        D3D12Hooks::gHeapSRV,
        D3D12Hooks::gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
        D3D12Hooks::gHeapSRV->GetGPUDescriptorHandleForHeapStart()
      );
      ImGui_ImplDX12_CreateDeviceObjects();

      InputHandler::init(
        UniHookGlobals::mainWindow,
        nullptr,
        nullptr
      );
    },
    [](void *lpUser) -> void {
      if (useGL != 1)
        return;

      ImGui_ImplDX12_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      ImGui::ShowDemoWindow();

      FrameContext& currentFrameContext
        = gFrameContext[gSavedSwapChain->GetCurrentBackBufferIndex()];
      currentFrameContext.commandAllocator->Reset();

      D3D12_RESOURCE_BARRIER barrier;
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      barrier.Transition.pResource = currentFrameContext.mainRenderTargetResource;
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

      gCommandList->Reset(
        currentFrameContext.commandAllocator,
        nullptr);
      gCommandList->ResourceBarrier(1, &barrier);
      gCommandList->OMSetRenderTargets(
        1,
        &currentFrameContext.mainRenderTargetDescriptor,
        FALSE,
        nullptr);
      gCommandList->SetDescriptorHeaps(1, &gHeapSRV);

      ImGui::Render();
      ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

      gCommandList->ResourceBarrier(1, &barrier);
      gCommandList->Close();

      gCommandQueue->ExecuteCommandLists(
        1,
        reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));
    },
    [](void *lpUser) -> void {
      InputHandler::deinit(UniHookGlobals::mainWindow);
      ImGui_ImplDX12_Shutdown();
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
    },
    nullptr
  );
#elif USE_GL == 1
  D3D11Hooks::init(
    [](const DXGI_SWAP_CHAIN_DESC *desc, void *lpUser) -> void {
      ImGui::CreateContext();
      ImGui::StyleColorsDark();

      ImGuiIO &io = ImGui::GetIO(); (void)io;
      io.Fonts->AddFontDefault();
      io.IniFilename = NULL;

      ImGui_ImplWin32_Init(desc->OutputWindow);
      ImGui_ImplDX11_Init(
        D3D11Hooks::gDevice,
        D3D11Hooks::gDeviceContext
      );

      InputHandler::init(
        UniHookGlobals::mainWindow,
        WndProcEx,
        nullptr
      );
    },
    [](void *lpUser) -> void {
      ImGui_ImplDX11_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      ImGui::ShowDemoWindow();

      ImGui::Render();
      gDeviceContext->OMSetRenderTargets(1, &gRenderTargetView, NULL);
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    },
    [](void *lpUser) -> void {
      InputHandler::deinit(UniHookGlobals::mainWindow);
      ImGui_ImplDX11_Shutdown();
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
    },
    nullptr
  );
#endif

  return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    //UniHookGlobals::mainWindow = (HWND)FindWindowW(0, L"Renderer: [DirectX12], Input: [Raw input], 64 bits"); // Main window of the GFXTest Application: https://bitbucket.org/learn_more/gfxtest/src/master/ 
      MessageBoxA(NULL, "aaa", "1", 0);
    UniHookGlobals::mainModule = hModule;
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)onAttach, hModule, 0, 0);
  }
  return 1;
}