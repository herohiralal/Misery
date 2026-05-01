#include "Dx12Fns.h"
#if MZNT_DX12

static void MZNT_Internal_CreateDx12SwapChain(MZNT_DirectX12SwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    if (swapChain->actual) // already has one
    {
        for (i32 i = 0; i < swapChain->framesInFlight; i++)
        {
            swapChain->swapchainRTs.data[i]->Release();
            swapChain->swapchainRTs.data[i] = nil;
        }

        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->actual->ResizeBuffers(cfg.framesInFlight, cfg.width, cfg.height, swapChain->swapChainFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));
    }
    else
    {
        // swapchain & its properties
        swapChain->swapChainFormat = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most widely supported swapchain format, even though we render to a different format internally

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
        swapChainDesc.Width            = cfg.width;
        swapChainDesc.Height           = cfg.height;
        swapChainDesc.Format           = swapChain->swapChainFormat;
        swapChainDesc.Stereo           = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount      = cfg.framesInFlight;
        swapChainDesc.Scaling          = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode        = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags            = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        HWND wnd;
        #if PNSLR_WINDOWS
        {
            wnd = (HWND) (uintptr_t) (swapChain->windowHandle.handle);
        }
        #else
            #error "Unsupported platform"
        #endif

        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->renderer->dxgiFactory->CreateSwapChainForHwnd(swapChain->renderer->cmdQueue, wnd, &swapChainDesc, nil, nil, (IDXGISwapChain1**) &(swapChain->actual)));

        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->renderer->dxgiFactory->MakeWindowAssociation(wnd, DXGI_MWA_NO_ALT_ENTER)); // disable automatic fullscreen transitions on alt+enter
    }

    // get width/height
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
        swapChain->actual->GetDesc1(&swapChainDesc);
        swapChain->swapChainWidth = swapChainDesc.Width;
        swapChain->swapChainHeight = swapChainDesc.Height;
    }

    // resize render targets
    PNSLR_ResizeSlice(
        ID3D12ResourcePtr,
        &(swapChain->swapchainRTs),
        cfg.framesInFlight,
        true,
        swapChain->renderer->parent.allocator,
        PNSLR_GET_LOC(),
        nil
    );

    // initialise new render targets
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain->swapchainRtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (i32 i = 0; i < cfg.framesInFlight; i++)
        {
            ID3D12Resource* backBuffer;
            MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->actual->GetBuffer((u32) i, IID_PPV_ARGS(&backBuffer)));
            swapChain->renderer->device->CreateRenderTargetView(backBuffer, nil, rtvHandle);
            rtvHandle.ptr += swapChain->swapchainRtvDescriptorSize;

            swapChain->swapchainRTs.data[i] = backBuffer;

            MZNT_Internal_SetDx12ObjDebugName(swapChain->renderer, swapChain->swapchainRTs.data[i],
                PNSLR_StringLiteral("$.swpch_$.rt_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
                tempAllocator);
        }
    }

    swapChain->vSync = cfg.vSync;

    if (swapChain->framesInFlight != cfg.framesInFlight)
    {
        // resize fence values
        PNSLR_ResizeSlice(
            u64,
            &(swapChain->frameFenceValues),
            cfg.framesInFlight,
            false,
            swapChain->renderer->parent.allocator,
            PNSLR_GET_LOC(),
            nil
        );

        // expected fence values
        swapChain->nextFenceValue = U64_MAX;
        for (i32 i = 0; i < cfg.framesInFlight; i++)
        {
            swapChain->frameFenceValues.data[i] = 0;
        }

        // fence itself
        if (swapChain->fence) swapChain->fence->Release();
        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->renderer->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&(swapChain->fence))));

        // destroy unneeded command buffers
        for (i32 i = cfg.framesInFlight; i < swapChain->framesInFlight; i++)
        {
            swapChain->cmdBuffers.data[i].cmdList->Release();
            swapChain->cmdBuffers.data[i].cmdAllocator->Release();
        }

        // resize cmd buffers
        PNSLR_ResizeSlice(
            MZNT_DirectX12RendererCommandBuffer,
            &(swapChain->cmdBuffers),
            cfg.framesInFlight,
            false,
            swapChain->renderer->parent.allocator,
            PNSLR_GET_LOC(),
            nil
        );

        // initialise new cmd buffers
        for (i32 i = swapChain->framesInFlight; i < cfg.framesInFlight; i++)
        {
            MZNT_DirectX12RendererCommandBuffer& cmdBuffer = swapChain->cmdBuffers.data[i];

            cmdBuffer.parent.type = MZNT_RendererType_DirectX12;
            cmdBuffer.renderer    = swapChain->renderer;

            MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->renderer->device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&(cmdBuffer.cmdAllocator))
            ));

            MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->renderer->device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                cmdBuffer.cmdAllocator,
                nil,
                IID_PPV_ARGS(&(cmdBuffer.cmdList))
            ));

        // it starts in the recording state...
            MZNT_INTERNAL_DX12_CHECKED_CALL(cmdBuffer.cmdList->Close());
        }

        // update count
        swapChain->framesInFlight = cfg.framesInFlight;
    }

    MZNT_Internal_SetDx12ObjDebugName(swapChain->renderer, swapChain->fence,
        PNSLR_StringLiteral("$.swpch_$.fence"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName)),
        tempAllocator);

    for (i32 i = 0; i < cfg.framesInFlight; i++)
    {
        MZNT_Internal_SetDx12ObjDebugName(swapChain->renderer, swapChain->cmdBuffers.data[i].cmdAllocator,
            PNSLR_StringLiteral("$.swpch_$.cmdPool_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);

        MZNT_Internal_SetDx12ObjDebugName(swapChain->renderer, swapChain->cmdBuffers.data[i].cmdList,
            PNSLR_StringLiteral("$.swpch_$.cmdbuf_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);
    }

    swapChain->curFrame = swapChain->actual->GetCurrentBackBufferIndex();
}

static void MZNT_Internal_DestroyDx12SwapChain(MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    for (i32 i = 0; i < swapChain->framesInFlight; i++)
    {
        swapChain->swapchainRTs.data[i]->Release();
    }

    PNSLR_FreeSlice(&(swapChain->swapchainRTs), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    for (i32 i = 0; i < swapChain->framesInFlight; i++)
    {
        swapChain->cmdBuffers.data[i].cmdList->Release();
        swapChain->cmdBuffers.data[i].cmdAllocator->Release();
    }
    PNSLR_FreeSlice(&(swapChain->cmdBuffers), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    PNSLR_FreeSlice(&(swapChain->frameFenceValues), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    swapChain->fence->Release();

    swapChain->actual->Release();
}

MZNT_DirectX12SwapChain* MZNT_CreateSwapChainFromWindow_DirectX12(MZNT_DirectX12Renderer* renderer, MZNT_WindowHandle windowHandle, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    MZNT_DirectX12SwapChain* output = PNSLR_New(MZNT_DirectX12SwapChain, renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    if (!output) FORCE_DBG_TRAP;

    output->parent.type    = MZNT_RendererType_DirectX12;
    output->renderer       = renderer;
    output->windowHandle   = windowHandle;

    // swapchain rtv heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { };
        heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.NumDescriptors = cfg.framesInFlight;
        heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        MZNT_INTERNAL_DX12_CHECKED_CALL(renderer->device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&(output->swapchainRtvHeap))));

        output->swapchainRtvDescriptorSize = renderer->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // fence evt
    {
        output->fenceEvt = CreateEventA(nil, FALSE, FALSE, nil);
        if (!output->fenceEvt) FORCE_DBG_TRAP;
    }

    MZNT_Internal_CreateDx12SwapChain(output, cfg, tempAllocator);

    return output;
}

b8 MZNT_ReconfigureSwapChain_DirectX12(MZNT_DirectX12SwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    MZNT_WaitTillRendererIdle_DirectX12(swapChain->renderer);
    MZNT_Internal_CreateDx12SwapChain(swapChain, cfg, tempAllocator);

    return true;
}

b8 MZNT_DestroySwapChain_DirectX12(MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    MZNT_WaitTillRendererIdle_DirectX12(swapChain->renderer);
    MZNT_Internal_DestroyDx12SwapChain(swapChain, tempAllocator);
    CloseHandle(swapChain->fenceEvt);
    swapChain->swapchainRtvHeap->Release();
    PNSLR_Delete(swapChain, swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    return false;
}

MZNT_TextureFormat MZNT_GetSwapChainTextureFormat_DirectX12(const MZNT_DirectX12SwapChain* swapChain)
{
    if (!swapChain) return MZNT_TextureFormat_Unknown;

    MZNT_TextureFormat output = MZNT_Internal_MakeDx12TextureFormat(swapChain->swapChainFormat);
    if (output == MZNT_TextureFormat_Unknown) { FORCE_DBG_TRAP; }

    return output;
}

b8 MZNT_IterateSwapChain_DirectX12(MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    swapChain->allowCmdBuff = false;

    swapChain->nextFenceValue++;
    swapChain->frameFenceValues.data[swapChain->curFrame] = swapChain->nextFenceValue;
    swapChain->curFrame = swapChain->actual->GetCurrentBackBufferIndex();

    if (swapChain->fence->GetCompletedValue() < swapChain->frameFenceValues.data[swapChain->curFrame])
    {
        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->fence->SetEventOnCompletion(swapChain->frameFenceValues.data[swapChain->curFrame], swapChain->fenceEvt));
        WaitForSingleObject(swapChain->fenceEvt, INFINITE);
    }

    swapChain->allowCmdBuff = true;
    return true;
}

MZNT_DirectX12RendererCommandBuffer* MZNT_GetSwapChainCommandBuffer_DirectX12(const MZNT_DirectX12SwapChain* swapChain, u8* outImgIdx, PNSLR_Allocator tempAllocator)
{
    u8 outImgIdxThrowaway = 0;
    outImgIdx = outImgIdx ? outImgIdx : &outImgIdxThrowaway;
    *outImgIdx = U8_MAX;

    if (!swapChain->allowCmdBuff) return nil;

    MZNT_DirectX12RendererCommandBuffer& cmdBuf = swapChain->cmdBuffers.data[swapChain->curFrame];
    MZNT_INTERNAL_DX12_CHECKED_CALL(cmdBuf.cmdAllocator->Reset());

    *outImgIdx = (u8) swapChain->curFrame;
    return &cmdBuf;
}

b8 MZNT_PresentSwapChain_DirectX12(const MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    if (!swapChain->allowCmdBuff) return false;

    MZNT_DirectX12RendererCommandBuffer& cmdBuf = swapChain->cmdBuffers.data[swapChain->curFrame];

    // TODO: REMOVEEEE - command buffer reset
    MZNT_INTERNAL_DX12_CHECKED_CALL(cmdBuf.cmdList->Reset(cmdBuf.cmdAllocator, nil));

    // TODO: REMOVEEEE - swapchain: common -> rt
    {
        D3D12_TEXTURE_BARRIER textureBarrier = { };
        textureBarrier.SyncBefore   = D3D12_BARRIER_SYNC_NONE;
        textureBarrier.SyncAfter    = D3D12_BARRIER_SYNC_RENDER_TARGET;
        textureBarrier.AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS;
        textureBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_RENDER_TARGET;
        textureBarrier.LayoutBefore = D3D12_BARRIER_LAYOUT_COMMON;
        textureBarrier.LayoutAfter  = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        textureBarrier.pResource    = swapChain->swapchainRTs.data[swapChain->curFrame];
        textureBarrier.Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(0xffffffff);

        D3D12_BARRIER_GROUP barrier = { };
        barrier.Type                = D3D12_BARRIER_TYPE_TEXTURE;
        barrier.NumBarriers         = 1;
        barrier.pTextureBarriers    = &textureBarrier;

        cmdBuf.cmdList->Barrier(1, &barrier);
    }

    // TODO: REMOVEEEE - bind swapchain to output
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain->swapchainRtvHeap->GetCPUDescriptorHandleForHeapStart(); // if you store the RTVs
        rtv.ptr += swapChain->curFrame * swapChain->swapchainRtvDescriptorSize;

        cmdBuf.cmdList->OMSetRenderTargets(1, &rtv, FALSE, nil);

        float clearColor[4] = {1.0, 0.0, 1.0, 1.0};
        cmdBuf.cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    // TODO: REMOVEEEE - swapchain: rt -> present
    {
        D3D12_TEXTURE_BARRIER textureBarrier = { };
        textureBarrier.SyncBefore   = D3D12_BARRIER_SYNC_RENDER_TARGET;
        textureBarrier.SyncAfter    = D3D12_BARRIER_SYNC_NONE;
        textureBarrier.AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET;
        textureBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_NO_ACCESS;
        textureBarrier.LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        textureBarrier.LayoutAfter  = D3D12_BARRIER_LAYOUT_PRESENT;
        textureBarrier.pResource    = swapChain->swapchainRTs.data[swapChain->curFrame];
        textureBarrier.Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(U32_MAX);

        D3D12_BARRIER_GROUP barrier = { };
        barrier.Type                = D3D12_BARRIER_TYPE_TEXTURE;
        barrier.NumBarriers         = 1;
        barrier.pTextureBarriers    = &textureBarrier;

        cmdBuf.cmdList->Barrier(1, &barrier);
    }

    // TODO: REMOVEEEE - command buffer over
    MZNT_INTERNAL_DX12_CHECKED_CALL(cmdBuf.cmdList->Close());

    // submit
    ID3D12CommandList* cmdLists[] = { cmdBuf.cmdList };
    swapChain->renderer->cmdQueue->ExecuteCommandLists(1, cmdLists);

    // present
    if (swapChain->vSync)
    {
        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->actual->Present(1, 0));
    }
    else
    {
        MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->actual->Present(0, DXGI_PRESENT_ALLOW_TEARING));
    }

    MZNT_INTERNAL_DX12_CHECKED_CALL(swapChain->renderer->cmdQueue->Signal(swapChain->fence, swapChain->nextFenceValue + 1));
    return true;
}

#endif
