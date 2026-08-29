#pragma once
#include <__init.h>
#include <ExtDeps_GPU.h>
#include <GPU_Base/GPU_Base.h>

#if GPU_VK
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

GPU_EXTEND_OBJECT(Vk, Instance,
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

GPU_EXTEND_OBJECT(Vk, CmdBuffer,
    const GPU_VkInstance* renderer;
    VkCommandPool         cmdPool;
    VkCommandBuffer       cmdBuffer;
);

GPU_EXTEND_OBJECT(Vk, SwapChain,
    GPU_VkInstance* renderer;
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
        GPU_CmdBuffer cmdBuffer;
    } perFrameInFlight[GPU_FRAMES_IN_FLIGHT];

    struct
    {
        VkImage imgs[4];
        VkImageView imgViews[4];
        VkSemaphore renderCompleteSems[4];
    } buffers;
);

GPU_EXTEND_OBJECT(Vk, Buffer,
    const GPU_VkInstance* renderer;
    usize                 size, align;
    GPU_MemType           memType;
    GPU_BufferUsage       usages;
    VkBuffer              actual;
    VmaAllocation         allocation;
    void*                 mappedPtr;
);

GPU_EXTEND_OBJECT(Vk, Texture,
    const GPU_VkInstance* renderer;
    u16                   width, height;
    GPU_MemType           memType;
    GPU_TextureUsage      usages;
    GPU_TextureFormat     format;
    VkImage               actual;
    VmaAllocation         allocation;
    VkImageView           view;
);

EXTERN_C_END
#endif
