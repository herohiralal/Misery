#include "Dx12Private.h"

#if GPU_DX12

void GPU_Dx12CmdsBegin(GPU_CmdBuffer* cb)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    cmdBuffer->cmdList->Reset(cmdBuffer->cmdAllocator, nil);
}

void GPU_Dx12CmdsEnd(GPU_CmdBuffer* cb)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    cmdBuffer->cmdList->Close();
}

void GPU_Dx12CmdBeginPass(GPU_CmdBuffer* cb, GPU_PassCfg cfg)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    MSR_ASSERT(cfg.drawTargets.count > 0 && "must provide at least one draw target");

    Slice_(D3D12_CPU_DESCRIPTOR_HANDLE) rtvHandles = COL_NewSlice(D3D12_CPU_DESCRIPTOR_HANDLE, cfg.drawTargets.count, true, MEM_temp);

    for (isize i = 0; i < cfg.drawTargets.count; i++)
    {
        GPU_PassDrawTarget* tgt = &(cfg.drawTargets.data[i]);

        GPU_Dx12Texture* tex = GPU_ToDx12Texture(tgt->target);
        MSR_ASSERT(tex && "draw target texture must not be null");

        rtvHandles.data[i] = tex->asRtv.data.cpuHandle;
    }

    b8 hasDs = (cfg.depthStencilTarget.target != nil);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = { };
    if (hasDs)
    {
        GPU_Dx12Texture* dsTex = GPU_ToDx12Texture(cfg.depthStencilTarget.target);
        MSR_ASSERT(dsTex && "depth-stencil target texture must not be null");

        dsvHandle = dsTex->asDsv.data.cpuHandle;
    }

    cmdBuffer->cmdList->OMSetRenderTargets((u32) rtvHandles.count, rtvHandles.data,
        false, hasDs ? &dsvHandle : nil);

    // clear the ones that need clearing
    for (isize i = 0; i < cfg.drawTargets.count; i++)
    {
        GPU_PassDrawTarget* tgt = &(cfg.drawTargets.data[i]);
        if (tgt->loadOp != GPU_LoadOp_Clear) continue;

        GPU_Dx12Texture* tex = GPU_ToDx12Texture(tgt->target);
        MSR_ASSERT(tex && "draw target texture must not be null");

        cmdBuffer->cmdList->ClearRenderTargetView(tex->asRtv.data.cpuHandle,
            tgt->clearColor, 0, nil);
    }

    if (hasDs && cfg.depthStencilTarget.loadOp == GPU_LoadOp_Clear)
    {
        GPU_Dx12Texture* dsTex = GPU_ToDx12Texture(cfg.depthStencilTarget.target);
        MSR_ASSERT(dsTex && "depth-stencil target texture must not be null");

        cmdBuffer->cmdList->ClearDepthStencilView(
            dsTex->asDsv.data.cpuHandle,
            D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
            cfg.depthStencilTarget.clearDepth,
            cfg.depthStencilTarget.clearStencil,
            0, nil);
    }
}

void GPU_Dx12CmdEndPass(GPU_CmdBuffer* cb)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");
}

void GPU_Dx12CmdBarrier(GPU_CmdBuffer* cb, GPU_BarrierCfg cfg)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    Slice_(D3D12_GLOBAL_BARRIER) glbBarriers = { };
    Slice_(D3D12_BUFFER_BARRIER) bufBarriers = { };
    Slice_(D3D12_TEXTURE_BARRIER) texBarriers = { };
    List_(D3D12_BARRIER_GROUP) barrierGroups = COL_NewList(D3D12_BARRIER_GROUP, 3, MEM_temp);

    if (cfg.globalBarriers.count > 0)
    {
        glbBarriers = COL_NewSlice(D3D12_GLOBAL_BARRIER, cfg.globalBarriers.count, true, MEM_temp);

        for (isize i = 0; i < cfg.globalBarriers.count; i++)
        {
            GPU_GlobalBarrierCfg* gCfg = &(cfg.globalBarriers.data[i]);
            glbBarriers.data[i] = D3D12_GLOBAL_BARRIER
            {
                .SyncBefore = GPU_BreakDx12BarrierStage(gCfg->src.stage),
                .SyncAfter = GPU_BreakDx12BarrierStage(gCfg->dst.stage),
                .AccessBefore = GPU_BreakDx12BarrierAccess(gCfg->src.access),
                .AccessAfter = GPU_BreakDx12BarrierAccess(gCfg->dst.access),
            };
        }

        D3D12_BARRIER_GROUP barrierGroup =
        {
            .Type = D3D12_BARRIER_TYPE_GLOBAL,
            .NumBarriers = (u32) glbBarriers.count,
            .pGlobalBarriers = glbBarriers.data,
        };

        COL_AppendToList(&barrierGroups, barrierGroup);
    }

    if (cfg.bufferBarriers.count > 0)
    {
        bufBarriers = COL_NewSlice(D3D12_BUFFER_BARRIER, cfg.bufferBarriers.count, true, MEM_temp);

        for (isize i = 0; i < cfg.bufferBarriers.count; i++)
        {
            GPU_BufferBarrierCfg* bCfg = &(cfg.bufferBarriers.data[i]);
            bufBarriers.data[i] = D3D12_BUFFER_BARRIER
            {
                .SyncBefore = GPU_BreakDx12BarrierStage(bCfg->src.stage),
                .SyncAfter = GPU_BreakDx12BarrierStage(bCfg->dst.stage),
                .AccessBefore = GPU_BreakDx12BarrierAccess(bCfg->src.access),
                .AccessAfter = GPU_BreakDx12BarrierAccess(bCfg->dst.access),
                .pResource = GPU_ToDx12Buffer(bCfg->buffer)->actual,
                .Offset = bCfg->offset,
                .Size = bCfg->size,
            };
        }

        D3D12_BARRIER_GROUP barrierGroup =
        {
            .Type = D3D12_BARRIER_TYPE_BUFFER,
            .NumBarriers = (u32) bufBarriers.count,
            .pBufferBarriers = bufBarriers.data,
        };

        COL_AppendToList(&barrierGroups, barrierGroup);
    }

    if (cfg.textureBarriers.count > 0)
    {
        texBarriers = COL_NewSlice(D3D12_TEXTURE_BARRIER, cfg.textureBarriers.count, true, MEM_temp);

        for (isize i = 0; i < cfg.textureBarriers.count; i++)
        {
            GPU_TextureBarrierCfg* tCfg = &(cfg.textureBarriers.data[i]);
            texBarriers.data[i] = D3D12_TEXTURE_BARRIER
            {
                .SyncBefore = GPU_BreakDx12BarrierStage(tCfg->src.stage),
                .SyncAfter = GPU_BreakDx12BarrierStage(tCfg->dst.stage),
                .AccessBefore = GPU_BreakDx12BarrierAccess(tCfg->src.access),
                .AccessAfter = GPU_BreakDx12BarrierAccess(tCfg->dst.access),
                .LayoutBefore = GPU_BreakDx12TextureLayout(tCfg->srcLayout),
                .LayoutAfter = GPU_BreakDx12TextureLayout(tCfg->dstLayout),
                .pResource = GPU_ToDx12Texture(tCfg->texture)->actual,
                .Subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(U32_MAX), // 'all subresources'
            };
        }

        D3D12_BARRIER_GROUP barrierGroup =
        {
            .Type = D3D12_BARRIER_TYPE_TEXTURE,
            .NumBarriers = (u32) texBarriers.count,
            .pTextureBarriers = texBarriers.data,
        };

        COL_AppendToList(&barrierGroups, barrierGroup);
    }

    cmdBuffer->cmdList->Barrier((u32) barrierGroups.count, barrierGroups.data);
}

#endif
