#include "stdafx.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

namespace D3D12Functions {
  // Function pointers.
  PresentFnD3D12 oPresentD3D12;
  DrawInstancedFnD3D12 oDrawInstancedD3D12;
  DrawIndexedInstancedFnD3D12 oDrawIndexedInstancedD3D12;
  ReleaseFnD3D12 oReleaseD3D12;
  void (*oExecuteCommandListsD3D12)(ID3D12CommandQueue*, UINT, ID3D12CommandList*);
  HRESULT (*oSignalD3D12)(ID3D12CommandQueue*, ID3D12Fence*, UINT64);

  // Variables.
  IDXGISwapChain3 *gSavedSwapChain = nullptr;
  ID3D12Device *gDevice = nullptr;
  ID3D12DescriptorHeap *gHeapRTV = nullptr;
  ID3D12DescriptorHeap *gHeapSRV = nullptr;
  ID3D12GraphicsCommandList *gCommandList = nullptr;
  ID3D12Fence *gFence = nullptr;
  UINT64 gFenceValue = 0;
  ID3D12CommandQueue* gCommandQueue = nullptr;
  UINT gBufferCount = -1;
  FrameContext* gFrameContext;
  bool shutdown = false
    , gInit = false;

  // Functions.
  bool createAllFrom(IDXGISwapChain3* pSwapChain) {
    if (!SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&gDevice)))
      return false;

    if (!UniHookGlobals::mainWindow)
      pSwapChain->GetHwnd(&UniHookGlobals::mainWindow);
    if (!UniHookGlobals::mainWindow)
      UniHookGlobals::mainWindow = GetForegroundWindow();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();
    io.IniFilename = NULL;

    DXGI_SWAP_CHAIN_DESC sdesc;
    pSwapChain->GetDesc(&sdesc);
    sdesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sdesc.OutputWindow = UniHookGlobals::mainWindow;
    sdesc.Windowed = ((GetWindowLongPtr(UniHookGlobals::mainWindow, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

    gBufferCount = sdesc.BufferCount;
    gFrameContext = new FrameContext[gBufferCount];

    D3D12_DESCRIPTOR_HEAP_DESC descriptorImGuiRender = {};
    descriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorImGuiRender.NumDescriptors = gBufferCount;
    descriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (gDevice->CreateDescriptorHeap(&descriptorImGuiRender, IID_PPV_ARGS(&gHeapSRV)) != S_OK)
      return false;

    ID3D12CommandAllocator* allocator;
    if (gDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)) != S_OK)
      return false;

    for (size_t i = 0; i < gBufferCount; i++)
      gFrameContext[i].commandAllocator = allocator;

    if (
      gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL, IID_PPV_ARGS(&gCommandList)) != S_OK
      || gCommandList->Close() != S_OK
    )
      return false;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorBackBuffers;
    descriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descriptorBackBuffers.NumDescriptors = gBufferCount;
    descriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descriptorBackBuffers.NodeMask = 1;

    if (gDevice->CreateDescriptorHeap(&descriptorBackBuffers, IID_PPV_ARGS(&gHeapRTV)) != S_OK)
      return false;

    const auto rtvDescriptorSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = gHeapRTV->GetCPUDescriptorHandleForHeapStart();

    for (size_t i = 0; i < gBufferCount; i++) {
      ID3D12Resource* pBackBuffer = nullptr;

      gFrameContext[i].mainRenderTargetDescriptor = rtvHandle;
      pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
      gDevice->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
      gFrameContext[i].mainRenderTargetResource = pBackBuffer;
      rtvHandle.ptr += rtvDescriptorSize;
    }

    ImGui_ImplWin32_Init(UniHookGlobals::mainWindow);
    ImGui_ImplDX12_Init(
      gDevice,
      gBufferCount,
      DXGI_FORMAT_R8G8B8A8_UNORM,
      gHeapSRV,
      gHeapSRV->GetCPUDescriptorHandleForHeapStart(),
      gHeapSRV->GetGPUDescriptorHandleForHeapStart()
    );
    ImGui_ImplDX12_CreateDeviceObjects();

    InputHandler::Init(UniHookGlobals::mainWindow);

    return true;
  }

  void clearAll() {
    InputHandler::Remove(UniHookGlobals::mainWindow);
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    for (UINT i = 0; i < gBufferCount; i++) {
      SafeRelease(&gFrameContext[i].mainRenderTargetResource);
      SafeRelease(&gFrameContext[i].commandAllocator);
    }
    SafeRelease(&gCommandList);
    SafeRelease(&gHeapRTV);
    SafeRelease(&gHeapSRV);
    SafeRelease(&gDevice);
    gCommandQueue = nullptr;
    gFenceValue = 0;
    gFence = nullptr;
  }


  HRESULT STDMETHODCALLTYPE hookPresentD3D12(
    IDXGISwapChain3* pSwapChain,
    UINT SyncInterval,
    UINT Flags
  ) {
    if (GetAsyncKeyState(UniHookGlobals::openMenuKey) & 0x1)
      menu::isOpen ? menu::isOpen = false : menu::isOpen = true;

    if (!gInit) {
      gSavedSwapChain = pSwapChain;
      gInit = createAllFrom(pSwapChain);
    }

    if (gInit) {
      if (gCommandQueue == nullptr)
        return oPresentD3D12(pSwapChain, SyncInterval, Flags);

      ImGui_ImplDX12_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      menu::Init();

      FrameContext& currentFrameContext = gFrameContext[pSwapChain->GetCurrentBackBufferIndex()];
      currentFrameContext.commandAllocator->Reset();

      D3D12_RESOURCE_BARRIER barrier;
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      barrier.Transition.pResource = currentFrameContext.mainRenderTargetResource;
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

      gCommandList->Reset(currentFrameContext.commandAllocator, nullptr);
      gCommandList->ResourceBarrier(1, &barrier);
      gCommandList->OMSetRenderTargets(1, &currentFrameContext.mainRenderTargetDescriptor, FALSE, nullptr);
      gCommandList->SetDescriptorHeaps(1, &gHeapSRV);

      ImGui::Render();
      ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

      gCommandList->ResourceBarrier(1, &barrier);
      gCommandList->Close();

      gCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&gCommandList));
    }

    return oPresentD3D12(pSwapChain, SyncInterval, Flags);
  }

  void STDMETHODCALLTYPE hookkDrawInstancedD3D12(
    ID3D12GraphicsCommandList* dCommandList,
    UINT VertexCountPerInstance,
    UINT InstanceCount,
    UINT StartVertexLocation,
    UINT StartInstanceLocation
  ) {
    return oDrawInstancedD3D12(
      dCommandList,
      VertexCountPerInstance,
      InstanceCount,
      StartVertexLocation,
      StartInstanceLocation);
  }

  void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(
    ID3D12GraphicsCommandList* dCommandList,
    UINT IndexCount,
    UINT InstanceCount,
    UINT StartIndex,
    INT BaseVertex
  ) {
    return oDrawIndexedInstancedD3D12(
      dCommandList,
      IndexCount,
      InstanceCount,
      StartIndex,
      BaseVertex);
  }

  void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(
    ID3D12CommandQueue* queue,
    UINT NumCommandLists,
    ID3D12CommandList* ppCommandLists
  ) {
    if (!gCommandQueue)
      gCommandQueue = queue;

    oExecuteCommandListsD3D12(queue, NumCommandLists, ppCommandLists);
  }

  HRESULT STDMETHODCALLTYPE hookSignalD3D12(
    ID3D12CommandQueue* queue,
    ID3D12Fence* fence,
    UINT64 value
  ) {
    if (gCommandQueue != nullptr && queue == gCommandQueue) {
      gFence = fence;
      gFenceValue = value;
    }

    return oSignalD3D12(queue, fence, value);
  }

  ULONG hookReleaseD3D12(IDXGISwapChain3 *pSwapChain) {
    if (gInit && pSwapChain == gSavedSwapChain) {
      gInit = false;
      clearAll();
    }
    return oReleaseD3D12(pSwapChain);
  }

  void release() {
    shutdown = true;
    gDevice->Release();
    gHeapRTV->Release();
    gHeapSRV->Release();
    gCommandList->Release();
    gFence->Release();
    gCommandQueue->Release();
  }
}

namespace D3D12Hooks {
  void Init() {
    if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
      kiero::bind(54, (void**)&D3D12Functions::oExecuteCommandListsD3D12, (void *)D3D12Functions::hookExecuteCommandListsD3D12);
      kiero::bind(58, (void**)&D3D12Functions::oSignalD3D12, (void *)D3D12Functions::hookSignalD3D12);
      kiero::bind(140, (void**)&D3D12Functions::oPresentD3D12, (void *)D3D12Functions::hookPresentD3D12);
      kiero::bind(84, (void**)&D3D12Functions::oDrawInstancedD3D12, (void *)D3D12Functions::hookkDrawInstancedD3D12);
      kiero::bind(85, (void**)&D3D12Functions::oDrawIndexedInstancedD3D12, (void *)D3D12Functions::hookDrawIndexedInstancedD3D12);
      kiero::bind(134, (void **)&D3D12Functions::oReleaseD3D12, (void *)D3D12Functions::hookReleaseD3D12);

      do {
        Sleep(100);
      } while (!(GetAsyncKeyState(UniHookGlobals::uninjectKey) & 0x1));

      D3D12Functions::release();
      kiero::shutdown();
      InputHandler::Remove(UniHookGlobals::mainWindow);

      Beep(220, 100);

      FreeLibraryAndExitThread(UniHookGlobals::mainModule, 0);
    }
  }
}
