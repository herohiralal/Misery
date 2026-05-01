#include "VkFns.h"
#if MZNT_VULKAN

static void MZNT_Internal_CreateVkSwapChain(MZNT_VulkanSwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    VkSurfaceCapabilitiesKHR surfaceCaps;
    MZNT_INTERNAL_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &surfaceCaps));

    u32 imageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount)
        imageCount = surfaceCaps.maxImageCount;

    u32 presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &presentModesCount, nil);
    PNSLR_ArraySlice(VkPresentModeKHR) presentModes = PNSLR_MakeSlice(VkPresentModeKHR, presentModesCount, false, tempAllocator, PNSLR_GET_LOC(), nil);
    vkGetPhysicalDeviceSurfacePresentModesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &presentModesCount, presentModes.data);

    VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR; // always available
    const VkPresentModeKHR preferredMode = cfg.vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (i64 i = 0; i < presentModes.count; i++)
    {
        if (presentModes.data[i] == preferredMode) // best quality
        {
            selectedPresentMode = preferredMode;
            break;
        }
    }

    swapChain->surfaceSize = surfaceCaps.currentExtent;
    PNSLR_LogDf(PNSLR_StringLiteral("Swapchain extent: $x$"), PNSLR_FmtArgs(PNSLR_FmtU32(swapChain->surfaceSize.width, 0), PNSLR_FmtU32(swapChain->surfaceSize.height, 0)), PNSLR_GET_LOC());
    VkSwapchainCreateInfoKHR swapchainCI = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = swapChain->surface,
        .minImageCount    = imageCount,
        .imageFormat      = swapChain->surfaceFmt.format,
        .imageColorSpace  = swapChain->surfaceFmt.colorSpace,
        .imageExtent      = swapChain->surfaceSize,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform     = surfaceCaps.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = selectedPresentMode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = swapChain->actual,
    };

    if (swapChain->renderer->gfxQueueFamilyIndex != swapChain->renderer->presQueueFamilyIndex)
    {
        swapchainCI.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = 2;
        swapchainCI.pQueueFamilyIndices   = (u32[]) {swapChain->renderer->gfxQueueFamilyIndex, swapChain->renderer->presQueueFamilyIndex};
    }
    else
    {
        swapchainCI.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCI.queueFamilyIndexCount = 0;
        swapchainCI.pQueueFamilyIndices   = nil;
    }

    MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateSwapchainKHR(swapChain->renderer->device, &swapchainCI, nil, &(swapChain->actual)));

    if (swapchainCI.oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(swapChain->renderer->device, swapchainCI.oldSwapchain, nil);
    }

    MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->actual, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->actual),
        PNSLR_StringLiteral("$.swpch_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName)),
        tempAllocator);

    swapChain->vSync = cfg.vSync;

    if (swapChain->framesInFlight != cfg.framesInFlight)
    {
        // destroy unneeded command buffers
        for (i32 i = cfg.framesInFlight; i < swapChain->framesInFlight; i++)
        {
            MZNT_VulkanRendererCommandBuffer* cmdBuf = &(swapChain->cmdBuffers.data[i]);

            vkFreeCommandBuffers(swapChain->renderer->device, cmdBuf->cmdPool, 1, &(cmdBuf->cmdBuffer));
            vkDestroyCommandPool(swapChain->renderer->device, cmdBuf->cmdPool, nil);
        }

        // resize
        PNSLR_ResizeSlice(
            MZNT_VulkanRendererCommandBuffer,
            &(swapChain->cmdBuffers),
            cfg.framesInFlight,
            false,
            swapChain->renderer->parent.allocator,
            PNSLR_GET_LOC(),
            nil
        );

        // initialise new command buffers
        for (i32 i = swapChain->framesInFlight; i < cfg.framesInFlight; i++)
        {
            MZNT_VulkanRendererCommandBuffer* cmdBuf = &(swapChain->cmdBuffers.data[i]);

            cmdBuf->parent.type = MZNT_RendererType_Vulkan;
            cmdBuf->renderer    = swapChain->renderer;

            MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateCommandPool(swapChain->renderer->device, &(VkCommandPoolCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = swapChain->renderer->gfxQueueFamilyIndex,
            }, nil, &(cmdBuf->cmdPool)));

            MZNT_INTERNAL_VK_CHECKED_CALL(vkAllocateCommandBuffers(swapChain->renderer->device, &(VkCommandBufferAllocateInfo)
            {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = cmdBuf->cmdPool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            }, &(cmdBuf->cmdBuffer)));
        }

        // update count
        swapChain->framesInFlight = cfg.framesInFlight;
    }

    for (i32 i = 0; i < cfg.framesInFlight; i++)
    {
        MZNT_VulkanRendererCommandBuffer* cmdBuf = &(swapChain->cmdBuffers.data[i]);

        MZNT_Internal_SetVkObjDebugName(swapChain->renderer, cmdBuf->cmdPool, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(cmdBuf->cmdPool),
            PNSLR_StringLiteral("$.swpch_$.cmdPool_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);

        MZNT_Internal_SetVkObjDebugName(swapChain->renderer, cmdBuf->cmdBuffer, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(cmdBuf->cmdBuffer),
            PNSLR_StringLiteral("$.swpch_$.cmdBuf_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);
    }
}

static void MZNT_Internal_DestroyVkSwapChain(MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    for (i32 i = 0; i < swapChain->framesInFlight; i++)
    {
        MZNT_VulkanRendererCommandBuffer* cmdBuf = &(swapChain->cmdBuffers.data[i]);

        vkFreeCommandBuffers(swapChain->renderer->device, cmdBuf->cmdPool, 1, &(cmdBuf->cmdBuffer));
        vkDestroyCommandPool(swapChain->renderer->device, cmdBuf->cmdPool, nil);
    }
    PNSLR_FreeSlice(&(swapChain->cmdBuffers), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    vkDestroySwapchainKHR(swapChain->renderer->device, swapChain->actual, nil);
    vkDestroySurfaceKHR(swapChain->renderer->instance, swapChain->surface, nil);
}

static void MZNT_Internal_CreateVkSwapChainImagesAndViews(MZNT_VulkanSwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    // get swapchain images
    {
        u32 imgCount = 0;
        MZNT_INTERNAL_VK_CHECKED_CALL(vkGetSwapchainImagesKHR(swapChain->renderer->device, swapChain->actual, &imgCount, nil));
        swapChain->imgs = PNSLR_MakeSlice(VkImage, imgCount, false, swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
        MZNT_INTERNAL_VK_CHECKED_CALL(vkGetSwapchainImagesKHR(swapChain->renderer->device, swapChain->actual, &imgCount, swapChain->imgs.data));
        swapChain->imgs.count = (i64) imgCount;

        for (i32 i = 0; i < (i32) swapChain->imgs.count; i++)
        {
            MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->imgs.data[i], MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->imgs.data[i]),
                PNSLR_StringLiteral("$.swpch_$.img_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
                tempAllocator);
        }
    }

    {
        swapChain->imgViews = PNSLR_MakeSlice(VkImageView, swapChain->imgs.count, false, swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
        for (i64 i = 0; i < swapChain->imgs.count; i++)
        {
            MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateImageView(swapChain->renderer->device, &(VkImageViewCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapChain->imgs.data[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapChain->surfaceFmt.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                },
            }, nil, &(swapChain->imgViews.data[i])));

            MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->imgViews.data[i], MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->imgViews.data[i]),
                PNSLR_StringLiteral("$.swpch_$.imgview_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI64(i, 0)),
                tempAllocator);
        }
    }
}

static void MZNT_Internal_DestroyVkSwapChainImagesAndViews(MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    for (i64 i = 0; i < swapChain->imgViews.count; i++)
    {
        vkDestroyImageView(swapChain->renderer->device, swapChain->imgViews.data[i], nil);
    }

    PNSLR_FreeSlice(&(swapChain->imgViews), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    PNSLR_FreeSlice(&(swapChain->imgs), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
}

static void MZNT_Internal_UpdateVkSwapChainObjsDebugNames(MZNT_VulkanSwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->surface, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->surface),
        PNSLR_StringLiteral("$.swpch_$.surface"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName)),
        tempAllocator);

    i32 imgCount = (i32) swapChain->imgs.count;
    for (i32 i = 0; i < imgCount; i++)
    {
        MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->presentCompleteSemaphores.data[i], MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->presentCompleteSemaphores.data[i]),
            PNSLR_StringLiteral("$.swpch_$.prescompsem_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);

        MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->renderFinishedSemaphores.data[i], MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->renderFinishedSemaphores.data[i]),
            PNSLR_StringLiteral("$.swpch_$.renderfinsem_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);

        MZNT_Internal_SetVkObjDebugName(swapChain->renderer, swapChain->inFlightFences.data[i], MZNT_INTERNAL_GET_VK_OBJECT_TYPE(swapChain->inFlightFences.data[i]),
            PNSLR_StringLiteral("$.swpch_$.inflightfence_$"), PNSLR_FmtArgs(PNSLR_FmtString(swapChain->renderer->appName), PNSLR_FmtString(cfg.objectName), PNSLR_FmtI32(i, 0)),
            tempAllocator);
    }
}

MZNT_VulkanSwapChain* MZNT_CreateSwapChainFromWindow_Vulkan(MZNT_VulkanRenderer* renderer, MZNT_WindowHandle windowHandle, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    if (!renderer) return nil;

    MZNT_VulkanSwapChain* output = PNSLR_New(MZNT_VulkanSwapChain, renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    if (!output) FORCE_DBG_TRAP;

    output->parent.type = MZNT_RendererType_Vulkan;
    output->renderer    = renderer;

    #if PNSLR_WINDOWS
    {
        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateWin32SurfaceKHR(renderer->instance, &(VkWin32SurfaceCreateInfoKHR)
        {
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = (HINSTANCE)(uintptr_t) (renderer->parent.appHandle.handle),
            .hwnd      = (HWND)(uintptr_t) (windowHandle.handle),
        }, nil, &output->surface));
    }
    #elif PNSLR_ANDROID
    {
        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateAndroidSurfaceKHR(renderer->instance, &(VkAndroidSurfaceCreateInfoKHR)
        {
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .window = (ANativeWindow*) (windowHandle.handle),
        }, nil, &output->surface));
    }
    #else
    {
        #error "unimplemented"
    }
    #endif

    // select format type
    {
        u32 formatCount = 0;
        MZNT_INTERNAL_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(output->renderer->physicalDevice, output->surface, &formatCount, nil));
        PNSLR_ArraySlice(VkSurfaceFormatKHR) surfaceFormats = PNSLR_MakeSlice(VkSurfaceFormatKHR, formatCount, false, tempAllocator, PNSLR_GET_LOC(), nil);
        MZNT_INTERNAL_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(output->renderer->physicalDevice, output->surface, &formatCount, surfaceFormats.data));

        if (formatCount == 0)
        {
            PNSLR_LogE(PNSLR_StringLiteral("Failed to get any surface formats for swapchain"), PNSLR_GET_LOC());
            FORCE_DBG_TRAP;
        }

        output->surfaceFmt = surfaceFormats.data[0];
        for (i64 i = 0; i < surfaceFormats.count; i++)
        {
            b8 preferredFormat = false;
            #if PNSLR_DESKTOP
            {
                preferredFormat = (surfaceFormats.data[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
                    (surfaceFormats.data[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
            }
            #elif PNSLR_ANDROID
            {
                preferredFormat = (surfaceFormats.data[i].format == VK_FORMAT_R8G8B8A8_UNORM);
            }
            #else
            {
                #error "unimplemented"
            }
            #endif

            if (preferredFormat)
            {
                output->surfaceFmt = surfaceFormats.data[i];
                break;
            }
        }
    }

    MZNT_Internal_CreateVkSwapChain(output, cfg, tempAllocator);
    MZNT_Internal_CreateVkSwapChainImagesAndViews(output, cfg, tempAllocator);

    output->semIdx = U32_MAX;
    output->curFrame = U32_MAX;

    i64 imgCount = output->imgs.count;
    output->presentCompleteSemaphores = PNSLR_MakeSlice(VkSemaphore, imgCount, false, renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    output->renderFinishedSemaphores = PNSLR_MakeSlice(VkSemaphore, imgCount, false, renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    output->inFlightFences = PNSLR_MakeSlice(VkFence, imgCount, false, renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    for (i64 i = 0; i < imgCount; i++)
    {
        VkSemaphoreCreateInfo semaphoreCI = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateSemaphore(renderer->device, &semaphoreCI, nil, &(output->presentCompleteSemaphores.data[i])));
        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateSemaphore(renderer->device, &semaphoreCI, nil, &(output->renderFinishedSemaphores.data[i])));

        VkFenceCreateInfo fenceCI = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateFence(renderer->device, &fenceCI, nil, &(output->inFlightFences.data[i])));
    }

    MZNT_Internal_UpdateVkSwapChainObjsDebugNames(output, cfg, tempAllocator);

    return output;
}

b8 MZNT_ReconfigureSwapChain_Vulkan(MZNT_VulkanSwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    MZNT_WaitTillRendererIdle_Vulkan(swapChain->renderer);

    MZNT_Internal_DestroyVkSwapChainImagesAndViews(swapChain, tempAllocator);

    MZNT_Internal_CreateVkSwapChain(swapChain, cfg, tempAllocator);
    MZNT_Internal_CreateVkSwapChainImagesAndViews(swapChain, cfg, tempAllocator);

    MZNT_Internal_UpdateVkSwapChainObjsDebugNames(swapChain, cfg, tempAllocator);

    return true;
}

b8 MZNT_DestroySwapChain_Vulkan(MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    MZNT_WaitTillRendererIdle_Vulkan(swapChain->renderer);

    i64 imgCount = swapChain->imgs.count;
    for (i32 i = 0; i < imgCount; i++)
    {
        vkDestroyFence(swapChain->renderer->device, swapChain->inFlightFences.data[i], nil);
        vkDestroySemaphore(swapChain->renderer->device, swapChain->renderFinishedSemaphores.data[i], nil);
        vkDestroySemaphore(swapChain->renderer->device, swapChain->presentCompleteSemaphores.data[i], nil);
    }

    PNSLR_FreeSlice(&(swapChain->renderFinishedSemaphores), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    PNSLR_FreeSlice(&(swapChain->presentCompleteSemaphores), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    PNSLR_FreeSlice(&(swapChain->inFlightFences), swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    MZNT_Internal_DestroyVkSwapChainImagesAndViews(swapChain, tempAllocator);
    MZNT_Internal_DestroyVkSwapChain(swapChain, tempAllocator);

    PNSLR_Delete(swapChain, swapChain->renderer->parent.allocator, PNSLR_GET_LOC(), nil);
    return true;
}

MZNT_TextureFormat MZNT_GetSwapChainTextureFormat_Vulkan(const MZNT_VulkanSwapChain* swapChain)
{
    if (!swapChain) return MZNT_TextureFormat_Unknown;

    MZNT_TextureFormat output = MZNT_Internal_MakeVkTextureFormat(swapChain->surfaceFmt.format);
    if (output == MZNT_TextureFormat_Unknown) { FORCE_DBG_TRAP; }

    return output;
}

b8 MZNT_IterateSwapChain_Vulkan(MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    swapChain->allowCmdBuff = false;

    // DOING THESE SHENNANIGANS TO FIX MINIMISE ISSUES
    {
        VkSurfaceCapabilitiesKHR surfaceCaps = {0};
        MZNT_INTERNAL_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &surfaceCaps));
        if (!surfaceCaps.currentExtent.width && !surfaceCaps.currentExtent.height) // minimised window
            return false;
    }

    // update swapchain indexing
    swapChain->semIdx = (swapChain->semIdx + 1) % (u32) swapChain->presentCompleteSemaphores.count;
    swapChain->curFrame = (swapChain->curFrame + 1) % swapChain->framesInFlight;

    // wait on submission of the new frame's command buffer
    MZNT_INTERNAL_VK_CHECKED_CALL(vkWaitForFences(
        swapChain->renderer->device,
        1, &(swapChain->inFlightFences.data[swapChain->curFrame]), // fences to wait on
        VK_TRUE, // wait all
        U64_MAX // timeout
    ));

    // get the next image, and wait if the image of that is still processing
    MZNT_INTERNAL_VK_CHECKED_CALL(vkAcquireNextImageKHR(
        swapChain->renderer->device,
        swapChain->actual,
        U64_MAX, // timeout
        swapChain->presentCompleteSemaphores.data[swapChain->semIdx],
        VK_NULL_HANDLE, // fence
        &(swapChain->curImgIdx)
    ));

    // reset the fences, so they can be used again, once submission is done
    MZNT_INTERNAL_VK_CHECKED_CALL(vkResetFences(
        swapChain->renderer->device,
        1, &(swapChain->inFlightFences.data[swapChain->curFrame]) // reset fences
    ));

    swapChain->allowCmdBuff = true;
    return true;
}

MZNT_VulkanRendererCommandBuffer* MZNT_GetSwapChainCommandBuffer_Vulkan(const MZNT_VulkanSwapChain* swapChain, u8* outImgIdx, PNSLR_Allocator tempAllocator)
{
    u8 outImgIdxThrowaway = 0;
    outImgIdx = outImgIdx ? outImgIdx : &outImgIdxThrowaway;
    *outImgIdx = U8_MAX;

    if (!swapChain->allowCmdBuff) return nil;

    MZNT_VulkanRendererCommandBuffer* cmdBuf = &(swapChain->cmdBuffers.data[swapChain->curFrame]);
    MZNT_INTERNAL_VK_CHECKED_CALL(vkResetCommandPool(swapChain->renderer->device, cmdBuf->cmdPool, 0));

    *outImgIdx = (u8) swapChain->curFrame;
    return cmdBuf;
}

b8 MZNT_PresentSwapChain_Vulkan(const MZNT_VulkanSwapChain* swapChain, PNSLR_Allocator tempAllocator)
{
    if (!swapChain) return false;
    if (!swapChain->renderer) FORCE_DBG_TRAP;

    if (!swapChain->allowCmdBuff) return false;

    MZNT_VulkanRendererCommandBuffer* cmdBuf = &(swapChain->cmdBuffers.data[swapChain->curFrame]);

    // TODO: REMOVEEEE - command buffer begin
    MZNT_INTERNAL_VK_CHECKED_CALL(vkBeginCommandBuffer(cmdBuf->cmdBuffer, &(VkCommandBufferBeginInfo)
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    }));

    // TODO: REMOVEEEE - swapchain: undefined -> rt
    vkCmdPipelineBarrier2(cmdBuf->cmdBuffer, &(VkDependencyInfo)
    {
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = (VkImageMemoryBarrier2[])
        {
            {
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask       = VK_ACCESS_2_NONE,
                .dstStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask       = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .image               = swapChain->imgs.data[swapChain->curImgIdx],
                .subresourceRange    =
                {
                    .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel    = 0,
                    .levelCount      = 1,
                    .baseArrayLayer  = 0,
                    .layerCount      = 1,
                },
            },
        }
    });

    // TODO: REMOVEEEE - bind swapchain to output
    {
        vkCmdBeginRendering(cmdBuf->cmdBuffer, &(VkRenderingInfo)
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {.offset = {0, 0}, .extent = swapChain->surfaceSize},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &(VkRenderingAttachmentInfo)
            {
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView   = swapChain->imgViews.data[swapChain->curImgIdx],
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue  = {.color = {.float32 = {1.0, 0.0, 1.0, 1.0}}},
            },
            // TODO: bind
            .pDepthAttachment = nil,
            .pStencilAttachment = nil,
        });

        vkCmdEndRendering(cmdBuf->cmdBuffer);
    }

    // TODO: REMOVEEEE - swapchain: rt -> present
    vkCmdPipelineBarrier2(cmdBuf->cmdBuffer, &(VkDependencyInfo)
    {
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = (VkImageMemoryBarrier2[])
        {
            {
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask       = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask        = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask       = VK_ACCESS_2_NONE,
                .oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .image               = swapChain->imgs.data[swapChain->curImgIdx],
                .subresourceRange    =
                {
                    .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel    = 0,
                    .levelCount      = 1,
                    .baseArrayLayer  = 0,
                    .layerCount      = 1,
                },
            },
        }
    });

    // TODO: REMOVEEEE - command buffer over
    MZNT_INTERNAL_VK_CHECKED_CALL(vkEndCommandBuffer(cmdBuf->cmdBuffer));

    // submit command buffer
    MZNT_INTERNAL_VK_CHECKED_CALL(vkQueueSubmit2(swapChain->renderer->gfxQueue, 1, &(VkSubmitInfo2)
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nil,
        .flags = 0,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos    = (VkSemaphoreSubmitInfo[])
        {
            {
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = swapChain->presentCompleteSemaphores.data[swapChain->semIdx],
                .value     = 1,
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            },
        },
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos    = (VkCommandBufferSubmitInfo[])
        {
            {
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = cmdBuf->cmdBuffer,
            },
        },
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos    = (VkSemaphoreSubmitInfo[])
        {
            {
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = swapChain->renderFinishedSemaphores.data[swapChain->curImgIdx],
                .value     = 1,
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            },
        },
    }, swapChain->inFlightFences.data[swapChain->curFrame]));

    // present
    MZNT_INTERNAL_VK_CHECKED_CALL(vkQueuePresentKHR(swapChain->renderer->gfxQueue, &(VkPresentInfoKHR)
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nil,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(swapChain->renderFinishedSemaphores.data[swapChain->curImgIdx]),
        .swapchainCount = 1,
        .pSwapchains = &(swapChain->actual),
        .pImageIndices = &(swapChain->curImgIdx),
        .pResults = nil,
    }));

    return true;
}

#endif
