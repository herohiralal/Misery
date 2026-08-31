#include "VkPrivate.h"

#if GPU_VK

void GPU_VkCmdBuffBegin(GPU_CmdBuffer* cb)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VK_CHECKED_CALL(vkBeginCommandBuffer(cmdBuffer->cmdBuffer, &(VkCommandBufferBeginInfo)
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    }));
}

void GPU_VkCmdBuffEnd(GPU_CmdBuffer* cb)
{
    GPU_VkCmdBuffer* cmdBuffer = GPU_ToVkCmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    GPU_VK_CHECKED_CALL(vkEndCommandBuffer(cmdBuffer->cmdBuffer));
}

#endif
