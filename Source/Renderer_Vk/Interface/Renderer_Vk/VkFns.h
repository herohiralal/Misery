#pragma once
#include "VkTypes.h"

#if REN_VK
EXTERN_C_BEGIN

void REN_VkCreateRenderer(REN_Instance*, REN_InstanceCfg);
void REN_VkWaitTillRendererIdle(const REN_Instance*);
void REN_VkDestroyRenderer(REN_Instance*);

void REN_VkCreateSwapChainFromWindow(REN_SwapChain*, REN_Instance*, WND_Handle, REN_SwapChainCfg);
void REN_VkReconfigureSwapChain(REN_SwapChain*, REN_SwapChainCfg);
void REN_VkDestroySwapChain(REN_SwapChain*);
REN_TextureFormat REN_VkGetSwapChainTextureFormat(const REN_SwapChain*);
void REN_VkIterateSwapChain(REN_SwapChain*);
REN_CmdBuffer* REN_VkGetSwapChainCommandBuffer(const REN_SwapChain*, u8* outImgIdx);
void REN_VkPresentSwapChain(const REN_SwapChain*);

EXTERN_C_END
#endif
