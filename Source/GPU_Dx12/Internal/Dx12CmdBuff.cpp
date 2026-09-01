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
