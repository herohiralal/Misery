#pragma once
#include <__init.h>
#include <ExtDeps_GPU.h>
#include <GPU_Base/GPU_Base.h>

#if GPU_DX12
EXTERN_C_BEGIN

COL_DECLARE_FOR(D3D12_GLOBAL_BARRIER);
COL_DECLARE_FOR(D3D12_BUFFER_BARRIER);
COL_DECLARE_FOR(D3D12_TEXTURE_BARRIER);
COL_DECLARE_FOR(D3D12_BARRIER_GROUP);

typedef ID3D12Resource* ID3D12ResourcePtr;

#ifdef __cplusplus
typedef class D3D12MA::Allocator* DxAllocator;
#else
typedef struct DxAllocator* DxAllocator;
#endif

// doing this to simplify the whole descriptor api
// this heap can be used to allocate descriptors from
//
// note that this is only for shader non-visible heaps
// in case of cbv/srv/uav
//
// because of how bindless works, we'll need a staging
// heap for shader-visible descriptors, but that will be
// handled separately, and not under this object
typedef struct
{
    SYN_Mutex                  mutex;
    MEM_Allocator              allocator;
    ID3D12DescriptorHeap*      actual;
    D3D12_DESCRIPTOR_HEAP_TYPE type;
    u32                        capacity;
    u32                        count;
    u32                        stride;
    List_(u32)                 holes;
} GPU_Dx12DescriptorHeap;

// this is the data returned when allocating a descriptor
// from the heap
typedef struct
{
    GPU_Dx12DescriptorHeap*     owningHeap;
    u32                         index;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
} GPU_Dx12DescriptorData;

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
        GPU_Dx12DescriptorHeap* cbvSrvUav;
        GPU_Dx12DescriptorHeap* rtv;
        GPU_Dx12DescriptorHeap* dsv;
    } heaps;

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

    struct
    {
        GPU_CmdBuffer     cmdBuffers[GPU_FRAMES_IN_FLIGHT];
        u64               frameFenceValues[GPU_FRAMES_IN_FLIGHT];
        GPU_Texture       imgs[GPU_FRAMES_IN_FLIGHT];
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
        GPU_Dx12DescriptorData data;
    } asSrvUav;

    struct
    {
        b8 valid;
        GPU_Dx12DescriptorData data;
    } asRtv;

    struct
    {
        b8 valid;
        GPU_Dx12DescriptorData data;
    } asDsv;
);

EXTERN_C_END
#endif
