#pragma once
#include <__init.h>
#include <ExtDeps_Renderer.h>
#include <Renderer_Base/Renderer_Base.h>

#if REN_VK
EXTERN_C_BEGIN

COL_DECLARE_FOR(VkLayerProperties);
COL_DECLARE_FOR(VkExtensionProperties);
COL_DECLARE_FOR(VkPhysicalDevice);
COL_DECLARE_FOR(VkQueueFamilyProperties);
COL_DECLARE_FOR(VkDeviceQueueCreateInfo);
COL_DECLARE_FOR(VkSurfaceFormatKHR);
COL_DECLARE_FOR(VkImage);
COL_DECLARE_FOR(VkImageView);
COL_DECLARE_FOR(VkFramebuffer);
COL_DECLARE_FOR(VkPresentModeKHR);
COL_DECLARE_FOR(VkFence);
COL_DECLARE_FOR(VkSemaphore);
COL_DECLARE_FOR(VmaAllocation);

REN_EXTEND_OBJECT(Vk, Instance,
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
);

REN_EXTEND_OBJECT(Vk, CmdBuffer,
    const REN_VkInstance* renderer;
    VkCommandPool         cmdPool;
    VkCommandBuffer       cmdBuffer;
);

COL_DECLARE_FOR(REN_VkCmdBuffer);

REN_EXTEND_OBJECT(Vk, SwapChain,
    const REN_VkInstance* renderer;
    VkSwapchainKHR        actual;

    // surface info
    VkSurfaceKHR         surface;
    VkSurfaceFormatKHR   surfaceFmt;
    VkExtent2D           surfaceSize;

    // cfg
    b8 vSync;
    u8 framesInFlight;

    // syncing
    b8                  allowCmdBuff;
    u32                 curFrame, semIdx, curImgIdx;
    Slice_(VkSemaphore) presentCompleteSemaphores;
    Slice_(VkSemaphore) renderFinishedSemaphores;
    Slice_(VkFence)     inFlightFences;

    // images
    Slice_(VkImage)     imgs;
    Slice_(VkImageView) imgViews;

    // command buffers
    Slice_(REN_VkCmdBuffer) cmdBuffers;
);

EXTERN_C_END
#endif
