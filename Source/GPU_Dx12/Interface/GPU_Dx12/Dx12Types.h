#pragma once
#include <__init.h>
#include <ExtDeps_GPU.h>
#include <GPU_Base/GPU_Base.h>

#if GPU_DX12
EXTERN_C_BEGIN

typedef ID3D12Resource* ID3D12ResourcePtr;

#ifdef __cplusplus
typedef class D3D12MA::Allocator* DxAllocator;
#else
typedef struct DxAllocator* DxAllocator;
#endif

GPU_EXTEND_OBJECT(Dx12, Instance,
    APP_Handle appHandle;

    IDXGIFactory6*      dxgiFactory;
    IDXGIAdapter1*      adapter;
    ID3D12Device2*      device;
    ID3D12CommandQueue* cmdQueue;

    ID3D12Debug* dbgController;
    u32          dbgCallbackCookie;

    DxAllocator d3d12maAllocator;

    struct
    {
        u32 cbvSrvUav, sampler, rtv, dsv;
    } descriptorStrides;

    utf8str appName;

    struct
    {
        u8 appName[32];
    } buffers;
);

GPU_EXTEND_OBJECT(Dx12, CmdBuffer,
    const GPU_Dx12Instance*   renderer;
    ID3D12CommandAllocator*    cmdAllocator;
    ID3D12GraphicsCommandList7* cmdList;
);

GPU_EXTEND_OBJECT(Dx12, SwapChain,
    GPU_Dx12Instance* renderer;
    WND_Handle        window;
    IDXGISwapChain4*  actual;

    // surface info
    DXGI_FORMAT swapChainFormat;
    u32         swapChainWidth;
    u32         swapChainHeight;

    // cfg
    b8 vSync;

    // syncing
    b8           allowCmdBuff;
    u32          curFrame;
    ID3D12Fence* fence;
    u64          nextFenceValue;
    HANDLE       fenceEvt;

    // resources
    ID3D12DescriptorHeap* swapchainRtvHeap;

    struct
    {
        GPU_CmdBuffer     cmdBuffers[GPU_FRAMES_IN_FLIGHT];
        u64               frameFenceValues[GPU_FRAMES_IN_FLIGHT];
        ID3D12ResourcePtr swapchainRTs[GPU_FRAMES_IN_FLIGHT];
    } buffers;
);

GPU_EXTEND_OBJECT(Dx12, Buffer,
    const GPU_Dx12Instance* renderer;
    usize                   size, align;
    GPU_MemType             memType;
    GPU_BufferUsage         usages;
    ID3D12Resource*         actual;
    void*                   mappedPtr;
);

GPU_EXTEND_OBJECT(Dx12, Texture,
    const GPU_Dx12Instance* renderer;
    u16                     width, height;
    GPU_MemType             memType;
    GPU_TextureUsage        usages;
    GPU_TextureFormat       format;
    ID3D12Resource*         actual;

    struct
    {
        b8 valid;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu;
        D3D12_GPU_DESCRIPTOR_HANDLE gpu;
    } asRT;
);

EXTERN_C_END
#endif
