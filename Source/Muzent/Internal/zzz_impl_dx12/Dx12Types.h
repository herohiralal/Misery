#ifndef MZNT_DX12_TYPES_H
#define MZNT_DX12_TYPES_H
#include "Muzent/__Prelude.h"
#include "Muzent/Renderer.h"
#if defined(MZNT_IMPLEMENTATION) && MZNT_DX12
EXTERN_C_BEGIN

typedef ID3D12Resource* ID3D12ResourcePtr;
PNSLR_DECLARE_ARRAY_SLICE(ID3D12ResourcePtr);

// d3d12ma struct ptr fwds
#ifdef __cplusplus
typedef class D3D12MA::Allocator* DxAllocator;
typedef class D3D12MA::Allocation* DxAllocation;
#else
typedef struct DxAllocator* DxAllocator;
typedef struct DxAllocation* DxAllocation;
#endif

// renderer
typedef struct MZNT_DirectX12Renderer
{
    MZNT_Renderer       parent;
    IDXGIFactory6*      dxgiFactory;
    IDXGIAdapter1*      adapter;
    ID3D12Device2*      device;
    ID3D12CommandQueue* cmdQueue;

    ID3D12Debug* dbgController;
    u32          dbgCallbackCookie;

    DxAllocator d3d12maAllocator;

    utf8str appName;
} MZNT_DirectX12Renderer;

typedef struct MZNT_DirectX12RendererCommandBuffer MZNT_DirectX12RendererCommandBuffer;
PNSLR_DECLARE_ARRAY_SLICE(MZNT_DirectX12RendererCommandBuffer);

// cmd buffer
typedef struct MZNT_DirectX12RendererCommandBuffer
{
    MZNT_RendererCommandBuffer    parent;
    const MZNT_DirectX12Renderer* renderer;
    ID3D12CommandAllocator*       cmdAllocator;
    ID3D12GraphicsCommandList7*   cmdList;
} MZNT_DirectX12RendererCommandBuffer;

// swap chain
typedef struct MZNT_DirectX12SwapChain
{
    MZNT_SwapChain                parent;
    const MZNT_DirectX12Renderer* renderer;
    IDXGISwapChain4*              actual;

    // surface info
    MZNT_WindowHandle windowHandle;
    DXGI_FORMAT       swapChainFormat;
    u32               swapChainWidth, swapChainHeight;

    // cfg
    b8 vSync;
    u8 framesInFlight;

    // syncing
    b8                    allowCmdBuff;
    u32                   curFrame;
    ID3D12Fence*          fence;
    PNSLR_ArraySlice(u64) frameFenceValues;
    u64                   nextFenceValue;
    HANDLE                fenceEvt;

    // images
    ID3D12DescriptorHeap*               swapchainRtvHeap;
    u32                                 swapchainRtvDescriptorSize;
    PNSLR_ArraySlice(ID3D12ResourcePtr) swapchainRTs;

    // command buffers
    PNSLR_ArraySlice(MZNT_DirectX12RendererCommandBuffer) cmdBuffers;
} MZNT_DirectX12SwapChain;

EXTERN_C_END
#endif
#endif
