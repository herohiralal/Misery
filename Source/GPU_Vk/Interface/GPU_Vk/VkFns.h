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
GPU_CmdBuffer* GPU_VkGetSwapChainCommandBuffer(GPU_SwapChain*, u8* outImgIdx);
void GPU_VkPresentSwapChain(GPU_SwapChain*);

EXTERN_C_END
#endif
