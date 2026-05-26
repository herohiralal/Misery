#ifndef MZNT_VK_FNS_H
#define MZNT_VK_FNS_H
#include <Muzent/__Prelude.h>
#include <Muzent/RendererFns.h>
#include "VkTypes.h"
#include "VkDbgReflect.h"
EXTERN_C_BEGIN
#if defined(MZNT_IMPLEMENTATION) && MZNT_VULKAN

PFN_vkDebugUtilsMessengerCallbackEXT MZNT_Internal_GetVkDebugCallback(void);
void MZNT_Internal_LogVkResultOnFailure(VkResult result, utf8str fnCall, PNSLR_SourceCodeLocation loc);
void MZNT_Internal_SetVkObjDebugName(const MZNT_VulkanRenderer* renderer, void* obj, VkObjectType objTy, utf8str fmtStr, PNSLR_ArraySlice(PNSLR_PrimitiveFmtOptions) fmtArgs, PNSLR_Allocator tempAllocator);

#define MZNT_INTERNAL_VK_CHECKED_CALL(call) \
    MZNT_Internal_LogVkResultOnFailure((call), PNSLR_StringLiteral(#call), PNSLR_GET_LOC())

inline VkFormat MZNT_Internal_BreakVkTextureFormat(MZNT_TextureFormat fmt);
inline MZNT_TextureFormat MZNT_Internal_MakeVkTextureFormat(VkFormat fmt);

MZNT_VulkanRenderer* MZNT_CreateRenderer_Vulkan(MZNT_RendererConfiguration config, PNSLR_Allocator tempAllocator);
b8 MZNT_WaitTillRendererIdle_Vulkan(const MZNT_VulkanRenderer* renderer);
b8 MZNT_DestroyRenderer_Vulkan(MZNT_VulkanRenderer* renderer, PNSLR_Allocator tempAllocator);

MZNT_VulkanSwapChain* MZNT_CreateSwapChainFromWindow_Vulkan(MZNT_VulkanRenderer* renderer, MZNT_WindowHandle windowHandle, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator);
b8 MZNT_ReconfigureSwapChain_Vulkan(MZNT_VulkanSwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator);
b8 MZNT_DestroySwapChain_Vulkan(MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator);
MZNT_TextureFormat MZNT_GetSwapChainTextureFormat_Vulkan(const MZNT_VulkanSwapChain* swapChain);
b8 MZNT_IterateSwapChain_Vulkan(MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator);
MZNT_VulkanRendererCommandBuffer* MZNT_GetSwapChainCommandBuffer_Vulkan(const MZNT_VulkanSwapChain* swapChain, u8* outImgIdx, PNSLR_Allocator tempAllocator);
b8 MZNT_PresentSwapChain_Vulkan(const MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator);

#endif
EXTERN_C_END
#endif
