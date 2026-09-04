#include "VkPrivate.h"

#if GPU_VK

void GPU_VkCmdsBegin(GPU_CmdBuffer* cb)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VK_CHECKED_CALL(vkBeginCommandBuffer(cmdBuffer->cmdBuffer, &(VkCommandBufferBeginInfo)
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    }));
}

void GPU_VkCmdsEnd(GPU_CmdBuffer* cb)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VK_CHECKED_CALL(vkEndCommandBuffer(cmdBuffer->cmdBuffer));
}

void GPU_VkCmdBeginPass(GPU_CmdBuffer* cb, GPU_PassCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    MSR_ASSERT(cfg.drawTargets.count > 0 && "must provide at least one draw target");

    Slice_(VkRenderingAttachmentInfo) colorAttachments = COL_NewSlice(VkRenderingAttachmentInfo, cfg.drawTargets.count, true, MEM_temp);

    // we'll store the width/height from the first valid draw target texture
    u16 rpW = 0, rpH = 0;
    for (isize i = 0; i < cfg.drawTargets.count; i++)
    {
        GPU_PassDrawTargetCfg* tgt = &(cfg.drawTargets.data[i]);

        GPU_VkTexture* tex = GPU_ToVkTexture(tgt->target);
        MSR_ASSERT(tex && "draw target texture must not be null");

        // store width & height
        if (!rpW || !rpH) { rpW = tex->width; rpH = tex->height; }

        colorAttachments.data[i] = (VkRenderingAttachmentInfo)
        {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = tex->view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = GPU_BreakVkLoadOp(tgt->loadOp),
            .storeOp     = GPU_BreakVkStoreOp(tgt->storeOp),
            .clearValue  = {.color = {.float32 = {tgt->clearColor[0], tgt->clearColor[1],
                                                  tgt->clearColor[2], tgt->clearColor[3]}}},
        };
    }

    b8 hasDs = (cfg.depthStencilTarget.target != nil);
    VkRenderingAttachmentInfo depthAttachment = {0};
    if (hasDs)
    {
        GPU_VkTexture* dsTex = GPU_ToVkTexture(cfg.depthStencilTarget.target);
        MSR_ASSERT(dsTex && "depth-stencil target texture must not be null");

        depthAttachment = (VkRenderingAttachmentInfo)
        {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = dsTex->view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp      = GPU_BreakVkLoadOp(cfg.depthStencilTarget.loadOp),
            .storeOp     = GPU_BreakVkStoreOp(cfg.depthStencilTarget.storeOp),
            .clearValue  = {.depthStencil = {.depth = cfg.depthStencilTarget.clearDepth,
                            .stencil = cfg.depthStencilTarget.clearStencil}},
        };
    }

    vkCmdBeginRendering(cmdBuffer->cmdBuffer, &(VkRenderingInfo)
    {
        .sType      = VK_STRUCTURE_TYPE_RENDERING_INFO,

        .renderArea =
        {
            .offset = {.x = 0, .y = 0},
            .extent = {.width = rpW, .height = rpH},
        },
        .layerCount = 1,

        // colour attachments
        .colorAttachmentCount = colorAttachments.count,
        .pColorAttachments    = colorAttachments.data,

        // depth stencil
        .pDepthAttachment   = hasDs ? &depthAttachment : nil,
        .pStencilAttachment = hasDs ? &depthAttachment : nil,
    });

    vkCmdSetViewport(cmdBuffer->cmdBuffer, 0, 1, &(VkViewport)
    {
        .x        = cfg.viewport.x,
        .y        = cfg.viewport.y,
        .width    = cfg.viewport.width,
        .height   = cfg.viewport.height,
        .minDepth = cfg.viewport.minDepth,
        .maxDepth = cfg.viewport.maxDepth,
    });

    vkCmdSetScissor(cmdBuffer->cmdBuffer, 0, 1, &(VkRect2D)
    {
        .offset = {.x = cfg.scissor.offsetX, .y = cfg.scissor.offsetY},
        .extent = {.width = cfg.scissor.width, .height = cfg.scissor.height},
    });
}

void GPU_VkCmdEndPass(GPU_CmdBuffer* cb)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");
    vkCmdEndRendering(cmdBuffer->cmdBuffer);
}

void GPU_VkCmdBarrier(GPU_CmdBuffer* cb, GPU_BarrierCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    Slice_(VkMemoryBarrier2) memBarriers = {0};
    Slice_(VkBufferMemoryBarrier2) bufBarriers = {0};
    Slice_(VkImageMemoryBarrier2) imgBarriers = {0};

    if (cfg.globalBarriers.count > 0)
    {
        memBarriers = COL_NewSlice(VkMemoryBarrier2, cfg.globalBarriers.count, true, MEM_temp);

        for (isize i = 0; i < cfg.globalBarriers.count; i++)
        {
            GPU_GlobalBarrierCfg* gCfg = &(cfg.globalBarriers.data[i]);
            memBarriers.data[i] = (VkMemoryBarrier2)
            {
                .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
                .pNext = nil,
                .srcStageMask = GPU_BreakVkBarrierStage(gCfg->src.stage),
                .srcAccessMask = GPU_BreakVkBarrierAccess(gCfg->src.access),
                .dstStageMask = GPU_BreakVkBarrierStage(gCfg->dst.stage),
                .dstAccessMask = GPU_BreakVkBarrierAccess(gCfg->dst.access),
            };
        }
    }

    if (cfg.bufferBarriers.count > 0)
    {
        bufBarriers = COL_NewSlice(VkBufferMemoryBarrier2, cfg.bufferBarriers.count, true, MEM_temp);

        for (isize i = 0; i < cfg.bufferBarriers.count; i++)
        {
            GPU_BufferBarrierCfg* bCfg = &(cfg.bufferBarriers.data[i]);
            bufBarriers.data[i] = (VkBufferMemoryBarrier2)
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nil,
                .srcStageMask = GPU_BreakVkBarrierStage(bCfg->src.stage),
                .srcAccessMask = GPU_BreakVkBarrierAccess(bCfg->src.access),
                .dstStageMask = GPU_BreakVkBarrierStage(bCfg->dst.stage),
                .dstAccessMask = GPU_BreakVkBarrierAccess(bCfg->dst.access),
                .buffer = GPU_ToVkBuffer(bCfg->buffer)->actual,
                .offset = bCfg->offset,
                .size = bCfg->size,
            };
        }
    }

    if (cfg.textureBarriers.count > 0)
    {
        imgBarriers = COL_NewSlice(VkImageMemoryBarrier2, cfg.textureBarriers.count, true, MEM_temp);

        for (isize i = 0; i < cfg.textureBarriers.count; i++)
        {
            GPU_TextureBarrierCfg* tCfg = &(cfg.textureBarriers.data[i]);

            GPU_VkTexture* tex = GPU_ToVkTexture(tCfg->texture);
            MSR_ASSERT(tex && "barrierCfg->texture must not be null");

            // if has depth-stencil usage, then use depth/stencil aspect mask
            VkImageAspectFlags aspectMask = !!(tex->usages & GPU_TexUsg_DepthStencil)
                ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                : VK_IMAGE_ASPECT_COLOR_BIT;

            imgBarriers.data[i] = (VkImageMemoryBarrier2)
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nil,
                .srcStageMask = GPU_BreakVkBarrierStage(tCfg->src.stage),
                .srcAccessMask = GPU_BreakVkBarrierAccess(tCfg->src.access),
                .dstStageMask = GPU_BreakVkBarrierStage(tCfg->dst.stage),
                .dstAccessMask = GPU_BreakVkBarrierAccess(tCfg->dst.access),
                .oldLayout = GPU_BreakVkTextureLayout(tCfg->srcLayout),
                .newLayout = GPU_BreakVkTextureLayout(tCfg->dstLayout),
                .image = tex->actual,
                .subresourceRange =
                {
                    .aspectMask = aspectMask,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                },
            };
        }
    }

    vkCmdPipelineBarrier2(cmdBuffer->cmdBuffer, &(VkDependencyInfo)
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nil,
        .dependencyFlags = 0,
        .memoryBarrierCount = memBarriers.count,
        .pMemoryBarriers = memBarriers.data,
        .bufferMemoryBarrierCount = bufBarriers.count,
        .pBufferMemoryBarriers = bufBarriers.data,
        .imageMemoryBarrierCount = imgBarriers.count,
        .pImageMemoryBarriers = imgBarriers.data,
    });
}

void GPU_VkCmdBindProgram(GPU_CmdBuffer* cb, GPU_BindProgramCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VkProgram* program = GPU_ToVkProgram(cfg.program);
    MSR_ASSERT(program && "program must not be null");

    vkCmdBindPipeline(cmdBuffer->cmdBuffer,
        program->type == GPU_ProgramType_Compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
        program->actual);

    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    switch ((enum GPU_CullModes) cfg.cullMode)
    {
        // we always mark clockwise as front face
        case GPU_Cull_CounterClockwise: cullMode = VK_CULL_MODE_BACK_BIT; break;
        case GPU_Cull_Clockwise:        cullMode = VK_CULL_MODE_FRONT_BIT; break;
        case GPU_Cull_None:             cullMode = VK_CULL_MODE_NONE; break;
        default:                        MSR_ASSERT(false && "invalid cull mode"); break;
    }

    if (program->type != GPU_ProgramType_Compute)
        vkCmdSetCullMode(cmdBuffer->cmdBuffer, cullMode);

    VkPrimitiveTopology topology = 0;
    switch ((enum GPU_PrimitiveTopologies) cfg.topology)
    {
        case GPU_Topo_TriangleList: topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        case GPU_Topo_LineList:     topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case GPU_Topo_PointList:    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        default:                    MSR_ASSERT(false && "invalid primitive topology"); break;
    }

    if (program->type == GPU_ProgramType_VertexFragment)
        vkCmdSetPrimitiveTopology(cmdBuffer->cmdBuffer, topology);
}

void GPU_VkCmdBindProgramArgsGroup(GPU_CmdBuffer* cb, GPU_ProgramArgsGroupBindingCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VkProgramArgsLayout* argsLayout = GPU_ToVkProgramArgsLayout(cfg.layout);
    MSR_ASSERT(argsLayout && "layout must not be null");

    VkPipelineBindPoint bindPoint = cfg.programType == GPU_ProgramType_Compute
        ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;

    switch ((enum GPU_ProgramArgsGroupTypes) cfg.groupType)
    {
        case GPU_PgmArgsGrpTy_Baked:
        {
            GPU_VkProgramArgsBuffer* argsBuf = GPU_ToVkProgramArgsBuffer(cfg.value.baked);
            MSR_ASSERT(argsBuf && "argsBuffer must not be null");
            vkCmdBindDescriptorSets(cmdBuffer->cmdBuffer, bindPoint, argsLayout->actual, cfg.groupIdx, 1, &(argsBuf->actual), 0, nil);
            break;
        }
        case GPU_PgmArgsGrpTy_Direct:
        {
            GPU_VkWriteDescriptorSets writes = GPU_BreakVkProgramArgsBindings(VK_NULL_HANDLE, cfg.value.direct, MEM_temp);
            vkCmdPushDescriptorSet(cmdBuffer->cmdBuffer, bindPoint, argsLayout->actual, cfg.groupIdx, (u32) writes.writes.count, writes.writes.data);
            break;
        }
        default:
            MSR_ASSERT(false && "invalid program args group binding type");
            break;
    }
}

void GPU_VkCmdBindProgramInlineConstants(GPU_CmdBuffer* cb, GPU_ProgramInlineConstantArgBindingCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VkProgramArgsLayout* argsLayout = GPU_ToVkProgramArgsLayout(cfg.layout);
    MSR_ASSERT(argsLayout && "layout must not be null");

    vkCmdPushConstants(cmdBuffer->cmdBuffer, argsLayout->actual, argsLayout->pushConstantVisibility, 0, (u32) cfg.data.count, cfg.data.data);
}

void GPU_VkCmdDrawBasic(GPU_CmdBuffer* cb, GPU_DrawBasicCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    vkCmdDraw(cmdBuffer->cmdBuffer, cfg.vertCount, cfg.primitivesCount, cfg.firstVertIdx, cfg.firstPrimitiveComponentIdx);
}

void GPU_VkCmdDrawMeshlets(GPU_CmdBuffer* cb, GPU_DrawMeshletsCfg cfg)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    vkCmdDrawMeshTasksEXT(cmdBuffer->cmdBuffer, cfg.groupCountX, cfg.groupCountY, cfg.groupCountZ);
}

#endif
