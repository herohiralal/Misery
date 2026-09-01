#pragma once
#include "VkTypes.h"

#if GPU_VK
EXTERN_C_BEGIN

void GPU_VkCreate(GPU_Instance*, GPU_InstanceCfg);
void GPU_VkWaitTillIdle(GPU_Instance*);
void GPU_VkDestroy(GPU_Instance*);

void GPU_VkCreateSwapChainFromWindow(GPU_SwapChain*, GPU_Instance*, WND_Handle, GPU_SwapChainCfg);
void GPU_VkReconfigureSwapChain(GPU_SwapChain*, GPU_SwapChainCfg);
void GPU_VkDestroySwapChain(GPU_SwapChain*);
GPU_TextureFormat GPU_VkGetSwapChainTextureFormat(GPU_SwapChain*);
void GPU_VkIterateSwapChain(GPU_SwapChain*);
GPU_SwapChainFrameContext GPU_VkBeginSwapChainFrame(GPU_SwapChain*);
void GPU_VkEndSwapChainFrame(GPU_SwapChain*);

void GPU_VkNewBuffer(GPU_Buffer*, GPU_Instance*, GPU_BufferCfg);
void GPU_VkDeleteBuffer(GPU_Buffer*);

void GPU_VkNewTexture(GPU_Texture*, GPU_Instance*, GPU_TextureCfg);
void GPU_VkDeleteTexture(GPU_Texture*);

void GPU_VkCmdsBegin(GPU_CmdBuffer*);
void GPU_VkCmdsEnd(GPU_CmdBuffer*);
void GPU_VkCmdBeginPass(GPU_CmdBuffer*, GPU_PassCfg);
void GPU_VkCmdEndPass(GPU_CmdBuffer*);
void GPU_VkCmdBarrier(GPU_CmdBuffer*, GPU_BarrierCfg);

EXTERN_C_END
#endif
