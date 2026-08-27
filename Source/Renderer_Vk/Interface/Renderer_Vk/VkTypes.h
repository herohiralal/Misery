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
    APP_Handle    appHandle;

    VkInstance       instance;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;

    u32     gfxQueueFamilyIndex;
    VkQueue gfxQueue;

    u32     presQueueFamilyIndex;
    VkQueue presQueue;

    u32     dedicatedTransferQueueFamilyIndex;
    VkQueue dedicatedTransferQueue;

    u32     asyncComputeQueueFamilyIndex;
    VkQueue asyncComputeQueue;

    b8 meshShadersSupported;
    b8 taskShadersSupported;
    b8 descriptorBufferSupported;

    VkDebugUtilsMessengerEXT debugMessenger;
    utf8str                  appName;

    VmaAllocator vmaAllocator;

    struct
    {
        u8 appName[32];
    } buffers;
);

REN_EXTEND_OBJECT(Vk, CmdBuffer,
    const REN_VkInstance* renderer;
    VkCommandPool         cmdPool;
    VkCommandBuffer       cmdBuffer;
);

REN_EXTEND_OBJECT(Vk, SwapChain,
    REN_VkInstance* renderer;
    WND_Handle      window;
    VkSwapchainKHR  actual;

    // surface info
    VkSurfaceKHR       surface;
    VkSurfaceFormatKHR surfaceFmt;
    VkExtent2D         surfaceSize;

    // cfg
    b8 vSync;

    // syncing
    b8 allowCmdBuff;
    u32 acquiredSwpchImgIdx;
    u64 frameIdx, nextSignalValue;
    VkSemaphore timelineSem;
    List_(VkImage) imgs;
    List_(VkImageView) imgViews;
    List_(VkSemaphore) renderCompleteSems;

    struct
    {
        VkSemaphore imgAcquiredSem;
        REN_CmdBuffer cmdBuffer;
    } perFrameInFlight[REN_FRAMES_IN_FLIGHT];

    struct
    {
        VkImage imgs[4];
        VkImageView imgViews[4];
        VkSemaphore renderCompleteSems[4];
    } buffers;
);

EXTERN_C_END
#endif
