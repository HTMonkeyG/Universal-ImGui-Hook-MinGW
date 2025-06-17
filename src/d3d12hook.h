#pragma once

#include "aliases.h"

namespace D3D12Functions {
  struct FrameContext {
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12Resource* mainRenderTargetResource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE mainRenderTargetDescriptor;
  };

  typedef long (STDMETHODCALLTYPE *PresentFnD3D12)(IDXGISwapChain *, UINT, UINT);
  typedef void (STDMETHODCALLTYPE *DrawInstancedFnD3D12)(ID3D12GraphicsCommandList *, UINT, UINT, UINT, UINT);
  typedef void (STDMETHODCALLTYPE *DrawIndexedInstancedFnD3D12)(ID3D12GraphicsCommandList *, UINT, UINT, UINT, INT);
  typedef ULONG (STDMETHODCALLTYPE *ReleaseFnD3D12)(IDXGISwapChain3 *);
  
  extern PresentFnD3D12 oPresentD3D12;
  extern DrawInstancedFnD3D12 oDrawInstancedD3D12;
  extern DrawIndexedInstancedFnD3D12 oDrawIndexedInstancedD3D12;
  extern ReleaseFnD3D12 oReleaseD3D12;
  extern void (*oExecuteCommandListsD3D12)(ID3D12CommandQueue *, UINT, ID3D12CommandList *);
  extern HRESULT (*oSignalD3D12)(ID3D12CommandQueue *, ID3D12Fence *, UINT64);

  extern HRESULT STDMETHODCALLTYPE hookPresentD3D12(
    IDXGISwapChain3 *pSwapChain,
    UINT SyncInterval,
    UINT Flags);
  extern void STDMETHODCALLTYPE hookkDrawInstancedD3D12(
    ID3D12GraphicsCommandList *dCommandList,
    UINT VertexCountPerInstance,
    UINT InstanceCount,
    UINT StartVertexLocation,
    UINT StartInstanceLocation);
  extern void STDMETHODCALLTYPE hookDrawIndexedInstancedD3D12(
    ID3D12GraphicsCommandList *dCommandList,
    UINT IndexCount,
    UINT InstanceCount,
    UINT StartIndex,
    INT BaseVertex);
  extern void STDMETHODCALLTYPE hookExecuteCommandListsD3D12(
    ID3D12CommandQueue *queue,
    UINT NumCommandLists,
    ID3D12CommandList *ppCommandLists);
  extern HRESULT STDMETHODCALLTYPE hookSignalD3D12(
    ID3D12CommandQueue *queue,
    ID3D12Fence *fence,
    UINT64 value);
  extern ULONG hookReleaseD3D12(IDXGISwapChain3 *pSwapChain);
  extern void STDMETHODCALLTYPE release();
}

namespace D3D12Hooks {
  void Init();
  bool Deinit();
}

template<typename T> void SafeRelease(T **obj) {
  if (*obj) {
    (*obj)->Release();
    *obj = nullptr;
  }
}
