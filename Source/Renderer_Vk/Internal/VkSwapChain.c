#include "VkPrivate.h"

MSR_SUPPRESS_WARN
#if REN_VK

void REN_VkCreateSwapChainFromWindow(REN_SwapChain* s, REN_Instance* i, WND_Handle h, REN_SwapChainCfg c) { }
void REN_VkReconfigureSwapChain(REN_SwapChain* s, REN_SwapChainCfg c) { }
void REN_VkDestroySwapChain(REN_SwapChain* s) { }
REN_TextureFormat REN_VkGetSwapChainTextureFormat(REN_SwapChain* s) { return REN_TexFmt_Unknown; }
void REN_VkIterateSwapChain(REN_SwapChain* s) { }
REN_CmdBuffer* REN_VkGetSwapChainCommandBuffer(REN_SwapChain* s, u8* outImgIdx) { return nil; }
void REN_VkPresentSwapChain(REN_SwapChain* s) { }

#endif
MSR_UNSUPPRESS_WARN
