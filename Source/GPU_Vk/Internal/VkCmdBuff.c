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
    MSR_ASSERT(false && "Not implemented yet");
}

void GPU_VkCmdEndPass(GPU_CmdBuffer* cb)
{
    MSR_ASSERT(false && "Not implemented yet");
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

#endif
