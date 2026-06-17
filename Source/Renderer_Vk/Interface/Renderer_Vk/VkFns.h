#pragma once
#include "VkTypes.h"

#if REN_VK
EXTERN_C_BEGIN

PFN_vkDebugUtilsMessengerCallbackEXT REN_GetVkDebugCallback(void);
void REN_LogVkResultOnFailure(VkResult result, utf8str fnCall, SrcLoc loc);
void REN_SetVkObjDebugName(const REN_VkInstance* renderer, void* obj, VkObjectType objTy, utf8str fmtStr, FMT_Args fmtArgs, MEM_Allocator tempAllocator);

#define REN_VK_CHECKED_CALL(call) \
    REN_LogVkResultOnFailure((call), PNSLR_StringLiteral(#call), SRC_LOC())

inline VkFormat REN_BreakVkTextureFormat(REN_TextureFormat fmt);
inline REN_TextureFormat REN_MakeVkTextureFormat(VkFormat fmt);

REN_VkInstance* MZNT_CreateRenderer_Vulkan(REN_InstanceCfg cfg);
b8 MZNT_WaitTillRendererIdle_Vulkan(const REN_VkInstance* renderer);
b8 MZNT_DestroyRenderer_Vulkan(REN_VkInstance* renderer);

REN_VkSwapChain* MZNT_CreateSwapChainFromWindow_Vulkan(REN_VkInstance* renderer, WND_Handle windowHandle, REN_SwapChainCfg cfg);
b8 MZNT_ReconfigureSwapChain_Vulkan(REN_VkSwapChain* swapChain, REN_SwapChainCfg cfg);
b8 MZNT_DestroySwapChain_Vulkan(REN_VkSwapChain* swapChain);
REN_TextureFormat MZNT_GetSwapChainTextureFormat_Vulkan(const REN_VkSwapChain* swapChain);
b8 MZNT_IterateSwapChain_Vulkan(REN_VkSwapChain* swapChain);
REN_VkCmdBuffer* MZNT_GetSwapChainCommandBuffer_Vulkan(const REN_VkSwapChain* swapChain, u8* outImgIdx);
b8 MZNT_PresentSwapChain_Vulkan(const REN_VkSwapChain* swapChain);

EXTERN_C_END
#endif
