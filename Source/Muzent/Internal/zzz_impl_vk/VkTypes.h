#ifndef MZNT_VK_TYPES_H
#define MZNT_VK_TYPES_H
#include <Muzent/__Prelude.h>
#include <Muzent/RendererFns.h>
#if defined(MZNT_IMPLEMENTATION) && MZNT_VULKAN
EXTERN_C_BEGIN

// vulkan array slices
PNSLR_DECLARE_ARRAY_SLICE(VkLayerProperties);
PNSLR_DECLARE_ARRAY_SLICE(VkExtensionProperties);
PNSLR_DECLARE_ARRAY_SLICE(VkPhysicalDevice);
PNSLR_DECLARE_ARRAY_SLICE(VkQueueFamilyProperties);
PNSLR_DECLARE_ARRAY_SLICE(VkDeviceQueueCreateInfo);
PNSLR_DECLARE_ARRAY_SLICE(VkSurfaceFormatKHR);
PNSLR_DECLARE_ARRAY_SLICE(VkImage);
PNSLR_DECLARE_ARRAY_SLICE(VkImageView);
PNSLR_DECLARE_ARRAY_SLICE(VkFramebuffer);
PNSLR_DECLARE_ARRAY_SLICE(VkPresentModeKHR);
PNSLR_DECLARE_ARRAY_SLICE(VkFence);
PNSLR_DECLARE_ARRAY_SLICE(VkSemaphore);

// vma array slices
PNSLR_DECLARE_ARRAY_SLICE(VmaAllocation);

// renderer
typedef struct alignas(64) MZNT_VulkanRenderer
{
    MZNT_Renderer    parent;
    VkInstance       instance;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;
    u32              gfxQueueFamilyIndex;
    u32              presQueueFamilyIndex;
    VkQueue          gfxQueue;
    VkQueue          presQueue;

    VkDebugUtilsMessengerEXT debugMessenger;
    utf8str                  appName;

    VmaAllocator     vmaAllocator;
} MZNT_VulkanRenderer;

typedef struct MZNT_VulkanRendererCommandBuffer MZNT_VulkanRendererCommandBuffer;
PNSLR_DECLARE_ARRAY_SLICE(MZNT_VulkanRendererCommandBuffer);

// swap chain
typedef struct alignas(64) MZNT_VulkanSwapChain
{
    MZNT_SwapChain             parent;
    const MZNT_VulkanRenderer* renderer;
    VkSwapchainKHR             actual;

    // surface info
    VkSurfaceKHR         surface;
    VkSurfaceFormatKHR   surfaceFmt;
    VkExtent2D           surfaceSize;

    // cfg
    b8 vSync;
    u8 framesInFlight;

    // syncing
    b8                            allowCmdBuff;
    u32                           curFrame, semIdx, curImgIdx;
    PNSLR_ArraySlice(VkSemaphore) presentCompleteSemaphores;
    PNSLR_ArraySlice(VkSemaphore) renderFinishedSemaphores;
    PNSLR_ArraySlice(VkFence)     inFlightFences;

    // images
    PNSLR_ArraySlice(VkImage)     imgs;
    PNSLR_ArraySlice(VkImageView) imgViews;

    // command buffers
    PNSLR_ArraySlice(MZNT_VulkanRendererCommandBuffer) cmdBuffers;
} MZNT_VulkanSwapChain;

// cmd buffer
struct MZNT_VulkanRendererCommandBuffer
{
    MZNT_RendererCommandBuffer parent;
    const MZNT_VulkanRenderer* renderer;
    VkCommandPool              cmdPool;
    VkCommandBuffer            cmdBuffer;
};

EXTERN_C_END
#endif
#endif
