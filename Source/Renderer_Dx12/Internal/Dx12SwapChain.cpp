#include "Dx12Private.h"

#if REN_DX12

static void REN_Dx12GetSwapChainDimensions(REN_Dx12SwapChain* swapChain, REN_SwapChainCfg cfg, u32* outWidth, u32* outHeight)
{
    *outWidth = (u32) cfg.width;
    *outHeight = (u32) cfg.height;

    if (*outWidth && *outHeight)
        return;

    RECT clientRect = { };
    if (GetClientRect(WND_FromHandle(swapChain->window), &clientRect))
    {
        u32 clientWidth = (u32) (clientRect.right - clientRect.left);
        u32 clientHeight = (u32) (clientRect.bottom - clientRect.top);

        if (!(*outWidth)) *outWidth = clientWidth;
        if (!(*outHeight)) *outHeight = clientHeight;
    }

    if (!(*outWidth)) *outWidth = 8;
    if (!(*outHeight)) *outHeight = 8;
}

static void REN_DestroyDx12SwapChainImages(REN_Dx12SwapChain* swapChain)
{
    for (isize i = 0; i < REN_FRAMES_IN_FLIGHT; i++)
    {
        if (swapChain->buffers.swapchainRTs[i])
        {
            swapChain->buffers.swapchainRTs[i]->Release();
            swapChain->buffers.swapchainRTs[i] = nil;
        }
    }
}

static void REN_CreateDx12SwapChain(REN_Dx12SwapChain* swapChain, REN_SwapChainCfg cfg)
{
    u32 width = 0, height = 0;
    REN_Dx12GetSwapChainDimensions(swapChain, cfg, &width, &height);

    if (swapChain->actual) // already has one
    {
        REN_DestroyDx12SwapChainImages(swapChain);
        REN_DX12_CHECKED_CALL(swapChain->actual->ResizeBuffers(
            REN_FRAMES_IN_FLIGHT,
            width, height,
            swapChain->swapChainFormat,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
        ));
    }
    else
    {
        // swapchain & its properties
        swapChain->swapChainFormat = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most widely supported swapchain format, even though we render to a different format internally

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
        {
            .Width = width,
            .Height = height,
            .Format = swapChain->swapChainFormat,
            .Stereo = FALSE,
            .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = REN_FRAMES_IN_FLIGHT,
            .Scaling = DXGI_SCALING_NONE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        };

        IDXGISwapChain1* dxgiSwapChain = nil;
        REN_DX12_CHECKED_CALL(swapChain->renderer->dxgiFactory->CreateSwapChainForHwnd(
            swapChain->renderer->cmdQueue,
            WND_FromHandle(swapChain->window),
            &swapChainDesc,
            nil,
            nil,
            &dxgiSwapChain
        ));

        REN_DX12_CHECKED_CALL(dxgiSwapChain->QueryInterface(IID_PPV_ARGS(&(swapChain->actual))));
        dxgiSwapChain->Release();

        REN_DX12_CHECKED_CALL(swapChain->renderer->dxgiFactory->MakeWindowAssociation(
            WND_FromHandle(swapChain->window),
            DXGI_MWA_NO_ALT_ENTER
        ));
    }

    // get width/height
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        swapChain->actual->GetDesc1(&swapChainDesc);
        swapChain->swapChainWidth = swapChainDesc.Width;
        swapChain->swapChainHeight = swapChainDesc.Height;
    }

    // initialise new render targets
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain->swapchainRtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (isize i = 0; i < REN_FRAMES_IN_FLIGHT; i++)
        {
            ID3D12Resource* backBuffer = nil;
            REN_DX12_CHECKED_CALL(swapChain->actual->GetBuffer((u32) i, IID_PPV_ARGS(&backBuffer)));

            swapChain->renderer->device->CreateRenderTargetView(backBuffer, nil, rtvHandle);
            rtvHandle.ptr += swapChain->swapchainRtvDescriptorSize;

            swapChain->buffers.swapchainRTs[i] = backBuffer;

            REN_DX12_SET_OBJ_DEBUG_NAME(
                swapChain->renderer,
                backBuffer,
                "%.swpch_%.rt_%",
                FMT(swapChain->renderer->appName),
                FMT(cfg.objectName),
                FMT(i)
            );
        }
    }

    swapChain->vSync = cfg.vSync;
    swapChain->curFrame = swapChain->actual->GetCurrentBackBufferIndex();
}

void REN_Dx12CreateSwapChainFromWindow(REN_SwapChain* outBaseSwapChain, REN_Instance* baseRenderer, WND_Handle windowHandle, REN_SwapChainCfg cfg)
{
    REN_Dx12Instance* renderer = REN_ToDx12Instance(baseRenderer);
    if (!renderer)
        return;

    MSR_ASSERT(!!outBaseSwapChain && "outBaseSwapChain can't be null");
    outBaseSwapChain->base.type = REN_GfxAPIType_Dx12;

    REN_Dx12SwapChain* output = REN_ToDx12SwapChain(outBaseSwapChain);
    MSR_ASSERT(output && "output must not be null");

    output->renderer = renderer;
    output->window = windowHandle;

    // swapchain rtv heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
        {
            .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = REN_FRAMES_IN_FLIGHT,
            .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        };
        REN_DX12_CHECKED_CALL(renderer->device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&(output->swapchainRtvHeap))));

        output->swapchainRtvDescriptorSize = renderer->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // fence evt
    {
        output->fenceEvt = CreateEventA(nil, FALSE, FALSE, nil);
        MSR_ASSERT(output->fenceEvt && "Failed to create fence event");
    }

    // fence itself
    REN_DX12_CHECKED_CALL(renderer->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&(output->fence))));
    REN_DX12_SET_OBJ_DEBUG_NAME(renderer, output->fence, "%.swpch_%.fence",
        FMT(renderer->appName), FMT(cfg.objectName));

    output->nextFenceValue = U64_MAX;

    // initialise new cmd buffers
    for (isize i = 0; i < REN_FRAMES_IN_FLIGHT; i++)
    {
        output->buffers.frameFenceValues[i] = 0;

        REN_CmdBuffer* baseCmdBuffer = &(output->buffers.cmdBuffers[i]);
        baseCmdBuffer->base.type = REN_GfxAPIType_Dx12;

        REN_Dx12CmdBuffer* cmdBuffer = REN_ToDx12CmdBuffer(baseCmdBuffer);
        cmdBuffer->renderer = renderer;

        REN_DX12_CHECKED_CALL(renderer->device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&(cmdBuffer->cmdAllocator))
        ));

        REN_DX12_SET_OBJ_DEBUG_NAME(renderer, cmdBuffer->cmdAllocator, "%.swpch_%.cmdalloc_%",
            FMT(renderer->appName), FMT(cfg.objectName), FMT(i));

        REN_DX12_CHECKED_CALL(renderer->device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdBuffer->cmdAllocator,
            nil,
            IID_PPV_ARGS(&(cmdBuffer->cmdList))
        ));

        // it starts in the recording state...
        REN_DX12_CHECKED_CALL(cmdBuffer->cmdList->Close());
        REN_DX12_SET_OBJ_DEBUG_NAME(renderer, cmdBuffer->cmdList, "%.swpch_%.cmdbuf_%",
            FMT(renderer->appName), FMT(cfg.objectName), FMT(i));
    }

    REN_CreateDx12SwapChain(output, cfg);
}

void REN_Dx12ReconfigureSwapChain(REN_SwapChain* baseSwapChain, REN_SwapChainCfg cfg)
{
    REN_Dx12SwapChain* swapChain = REN_ToDx12SwapChain(baseSwapChain);
    MSR_ASSERT(swapChain && "swapChain must not be null");

    REN_Dx12WaitTillRendererIdle(REN_FromDx12Instance(swapChain->renderer));
    REN_CreateDx12SwapChain(swapChain, cfg);
}

void REN_Dx12DestroySwapChain(REN_SwapChain* baseSwapChain)
{
    REN_Dx12SwapChain* swapChain = REN_ToDx12SwapChain(baseSwapChain);
    if (!swapChain || !(swapChain->renderer))
        return;

    REN_Dx12WaitTillRendererIdle(REN_FromDx12Instance(swapChain->renderer));

    REN_DestroyDx12SwapChainImages(swapChain);

    for (isize i = 0; i < REN_FRAMES_IN_FLIGHT; i++)
    {
        REN_Dx12CmdBuffer* cmdBuffer = REN_ToDx12CmdBuffer(&(swapChain->buffers.cmdBuffers[i]));
        if (cmdBuffer->cmdList)
        {
            cmdBuffer->cmdList->Release();
            cmdBuffer->cmdList = nil;
        }

        if (cmdBuffer->cmdAllocator)
        {
            cmdBuffer->cmdAllocator->Release();
            cmdBuffer->cmdAllocator = nil;
        }
    }

    if (swapChain->fence)
    {
        swapChain->fence->Release();
        swapChain->fence = nil;
    }

    if (swapChain->fenceEvt)
    {
        CloseHandle(swapChain->fenceEvt);
        swapChain->fenceEvt = nil;
    }

    if (swapChain->swapchainRtvHeap)
    {
        swapChain->swapchainRtvHeap->Release();
        swapChain->swapchainRtvHeap = nil;
    }

    if (swapChain->actual)
    {
        swapChain->actual->Release();
        swapChain->actual = nil;
    }
}

REN_TextureFormat REN_Dx12GetSwapChainTextureFormat(REN_SwapChain* baseSwapChain)
{
    REN_Dx12SwapChain* swapChain = REN_ToDx12SwapChain(baseSwapChain);
    if (!swapChain) return REN_TexFmt_Unknown;

    REN_TextureFormat output = REN_MakeDx12TextureFormat(swapChain->swapChainFormat);
    MSR_ASSERT(output != REN_TexFmt_Unknown && "Failed to convert DXGI_FORMAT to REN_TextureFormat");
    return output;
}

void REN_Dx12IterateSwapChain(REN_SwapChain* baseSwapChain)
{
    REN_Dx12SwapChain* swapChain = REN_ToDx12SwapChain(baseSwapChain);
    if (!swapChain || !(swapChain->renderer))
        return;

    swapChain->allowCmdBuff = false;

    swapChain->nextFenceValue++;
    swapChain->buffers.frameFenceValues[swapChain->curFrame] = swapChain->nextFenceValue;
    swapChain->curFrame = swapChain->actual->GetCurrentBackBufferIndex();

    u64 completedFenceValue = swapChain->fence->GetCompletedValue();
    u64 requiredFenceValue = swapChain->buffers.frameFenceValues[swapChain->curFrame];
    if (completedFenceValue < requiredFenceValue)
    {
        REN_DX12_CHECKED_CALL(swapChain->fence->SetEventOnCompletion(requiredFenceValue, swapChain->fenceEvt));
        WaitForSingleObject(swapChain->fenceEvt, INFINITE);
    }

    swapChain->allowCmdBuff = true;
}

REN_CmdBuffer* REN_Dx12GetSwapChainCommandBuffer(REN_SwapChain* baseSwapChain, u8* outImgIdx)
{
    u8 outImgIdxThrowaway = 0;
    outImgIdx = outImgIdx ? outImgIdx : &outImgIdxThrowaway;
    *outImgIdx = U8_MAX;

    REN_Dx12SwapChain* swapChain = REN_ToDx12SwapChain(baseSwapChain);
    if (!swapChain || !swapChain->allowCmdBuff)
        return nil;

    REN_CmdBuffer* baseCmdBuffer = &(swapChain->buffers.cmdBuffers[swapChain->curFrame]);
    REN_Dx12CmdBuffer* cmdBuffer = REN_ToDx12CmdBuffer(baseCmdBuffer);

    REN_DX12_CHECKED_CALL(cmdBuffer->cmdAllocator->Reset());

    *outImgIdx = (u8) swapChain->curFrame;
    return baseCmdBuffer;
}

void REN_Dx12PresentSwapChain(REN_SwapChain* baseSwapChain)
{
    REN_Dx12SwapChain* swapChain = REN_ToDx12SwapChain(baseSwapChain);
    if (!swapChain || !(swapChain->renderer) || !swapChain->allowCmdBuff)
        return;

    REN_Dx12CmdBuffer* cmdBuffer = REN_ToDx12CmdBuffer(&(swapChain->buffers.cmdBuffers[swapChain->curFrame]));

    // TODO: REMOVEEEE - command buffer reset
    REN_DX12_CHECKED_CALL(cmdBuffer->cmdList->Reset(cmdBuffer->cmdAllocator, nil));

    // TODO: REMOVEEEE - swapchain: common -> rt
    {
        D3D12_TEXTURE_BARRIER textureBarrier =
        {
            .SyncBefore = D3D12_BARRIER_SYNC_NONE,
            .SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET,
            .AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
            .AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET,
            .LayoutBefore = D3D12_BARRIER_LAYOUT_COMMON,
            .LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET,
            .pResource = swapChain->buffers.swapchainRTs[swapChain->curFrame],
            .Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(U32_MAX),
        };

        D3D12_BARRIER_GROUP barrier =
        {
            .Type = D3D12_BARRIER_TYPE_TEXTURE,
            .NumBarriers = 1,
            .pTextureBarriers = &textureBarrier,
        };

        cmdBuffer->cmdList->Barrier(1, &barrier);
    }

    // TODO: REMOVEEEE - bind swapchain to output
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain->swapchainRtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtv.ptr += swapChain->curFrame * swapChain->swapchainRtvDescriptorSize;

        cmdBuffer->cmdList->OMSetRenderTargets(1, &rtv, FALSE, nil);

        float clearColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};
        cmdBuffer->cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    // TODO: REMOVEEEE - swapchain: rt -> present
    {
        D3D12_TEXTURE_BARRIER textureBarrier =
        {
            .SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET,
            .SyncAfter = D3D12_BARRIER_SYNC_NONE,
            .AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET,
            .AccessAfter = D3D12_BARRIER_ACCESS_NO_ACCESS,
            .LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET,
            .LayoutAfter = D3D12_BARRIER_LAYOUT_PRESENT,
            .pResource = swapChain->buffers.swapchainRTs[swapChain->curFrame],
            .Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(U32_MAX),
        };

        D3D12_BARRIER_GROUP barrier =
        {
            .Type = D3D12_BARRIER_TYPE_TEXTURE,
            .NumBarriers = 1,
            .pTextureBarriers = &textureBarrier,
        };

        cmdBuffer->cmdList->Barrier(1, &barrier);
    }

    // TODO: REMOVEEEE - command buffer over
    REN_DX12_CHECKED_CALL(cmdBuffer->cmdList->Close());

    // submit
    {
        ID3D12CommandList* commandLists[] = {cmdBuffer->cmdList};
        swapChain->renderer->cmdQueue->ExecuteCommandLists(1, commandLists);
    }

    // present
    if (swapChain->vSync)
    {
        REN_DX12_CHECKED_CALL(swapChain->actual->Present(1, 0));
    }
    else
    {
        REN_DX12_CHECKED_CALL(swapChain->actual->Present(0, DXGI_PRESENT_ALLOW_TEARING));
    }

    REN_DX12_CHECKED_CALL(swapChain->renderer->cmdQueue->Signal(swapChain->fence, swapChain->nextFenceValue + 1));
}

#endif
