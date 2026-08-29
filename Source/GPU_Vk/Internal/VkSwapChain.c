#include "GPU_Vk/VkFns.h"
#include "VkPrivate.h"

#if GPU_VK

static void GPU_CreateVkSwapChain(GPU_VkSwapChain* swapChain, GPU_SwapChainCfg cfg)
{
    VkSurfaceCapabilitiesKHR surfaceCaps;
    GPU_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &surfaceCaps));

    u32 imageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount)
        imageCount = surfaceCaps.maxImageCount;

    u32 presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &presentModesCount, nil);
    Slice_(VkPresentModeKHR) presentModes = COL_NewSlice(VkPresentModeKHR, presentModesCount, true, MEM_temp);
    vkGetPhysicalDeviceSurfacePresentModesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &presentModesCount, presentModes.data);

    VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR; // always available
    const VkPresentModeKHR preferredMode = cfg.vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (isize i = 0; i < presentModes.count; i++)
    {
        if (presentModes.data[i] == preferredMode) // best quality
        {
            selectedPresentMode = preferredMode;
            break;
        }
    }

    swapChain->surfaceSize = surfaceCaps.currentExtent;
    LOG_Dbg(VULKAN, "Swap-chain extent: %x%", FMT(swapChain->surfaceSize.width), FMT(swapChain->surfaceSize.height));
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

    GPU_VK_CHECKED_CALL(vkCreateSwapchainKHR(swapChain->renderer->device, &swapchainCI, nil, &(swapChain->actual)));

    if (swapchainCI.oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(swapChain->renderer->device, swapchainCI.oldSwapchain, nil);
    }

    GPU_VK_SET_OBJ_DEBUG_NAME(swapChain->renderer, swapChain->actual, "%.swpch_%", FMT(swapChain->renderer->appName), FMT(cfg.objectName));

    swapChain->vSync = cfg.vSync;

    for (isize i = 0; i < GPU_FRAMES_IN_FLIGHT; i++)
    {
        GPU_CmdBuffer* baseCmdBuf = &(swapChain->perFrameInFlight[i].cmdBuffer);
        baseCmdBuf->base.type = GPU_GfxAPIType_Vk;

        GPU_VkCmdBuffer* cmdBuf = GPU_ToVkCmdBuffer(&(swapChain->perFrameInFlight[i].cmdBuffer));
        cmdBuf->renderer = swapChain->renderer;

        MSR_ASSERT(
            ((cmdBuf->cmdPool == VK_NULL_HANDLE) == (cmdBuf->cmdBuffer == VK_NULL_HANDLE)) &&
            "cmdPool and cmdBuffer must either both be null or both be non-null"
        );

        if (cmdBuf->cmdPool == VK_NULL_HANDLE)
        {
            GPU_VK_CHECKED_CALL(vkCreateCommandPool(swapChain->renderer->device, &(VkCommandPoolCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = swapChain->renderer->gfxQueueFamilyIndex,
            }, nil, &(cmdBuf->cmdPool)));

            GPU_VK_SET_OBJ_DEBUG_NAME(swapChain->renderer, cmdBuf->cmdPool, "%.swpch_%.cmdPool_%",
                FMT(swapChain->renderer->appName), FMT(cfg.objectName), FMT(i));
        }

        if (cmdBuf->cmdBuffer == VK_NULL_HANDLE)
        {
            GPU_VK_CHECKED_CALL(vkAllocateCommandBuffers(swapChain->renderer->device, &(VkCommandBufferAllocateInfo)
            {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = cmdBuf->cmdPool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            }, &(cmdBuf->cmdBuffer)));

            GPU_VK_SET_OBJ_DEBUG_NAME(swapChain->renderer, cmdBuf->cmdBuffer, "%.swpch_%.cmdBuf_%",
                FMT(swapChain->renderer->appName), FMT(cfg.objectName), FMT(i));
        }
    }
}

static void GPU_DestroyVkSwapChain(GPU_VkSwapChain* swapChain)
{
    for (isize i = 0; i < GPU_FRAMES_IN_FLIGHT; i++)
    {
        GPU_VkCmdBuffer* cmdBuf = GPU_ToVkCmdBuffer(&(swapChain->perFrameInFlight[i].cmdBuffer));

        vkFreeCommandBuffers(swapChain->renderer->device, cmdBuf->cmdPool, 1, &(cmdBuf->cmdBuffer));
        vkDestroyCommandPool(swapChain->renderer->device, cmdBuf->cmdPool, nil);

        cmdBuf->cmdBuffer = VK_NULL_HANDLE;
        cmdBuf->cmdPool   = VK_NULL_HANDLE;
    }

    vkDestroySwapchainKHR(swapChain->renderer->device, swapChain->actual, nil);
    vkDestroySurfaceKHR(swapChain->renderer->instance, swapChain->surface, nil);
    #if MSR_OSX
    {
        NSWindow* window = WND_FromHandle(swapChain->window);
        NSView* contentView = [window contentView];
        CAMetalLayer* metalLayer = (CAMetalLayer*) [contentView layer];
        contentView.wantsLayer = NO;
        contentView.layer = nil;
        [metalLayer release];
    }
    #endif
}

static void GPU_CreateVkSwapChainImagesAndViews(GPU_VkSwapChain* swapChain, GPU_SwapChainCfg cfg)
{
    // get swapchain images
    {
        u32 imgCount = 0;
        GPU_VK_CHECKED_CALL(vkGetSwapchainImagesKHR(swapChain->renderer->device, swapChain->actual, &imgCount, nil));
        swapChain->imgs = (List_(VkImage))
        {
            .data      = &(swapChain->buffers.imgs[0]),
            .count     = (isize) imgCount,
            .capacity  = sizeof(swapChain->buffers.imgs) / sizeof(swapChain->buffers.imgs[0]),
            .allocator = (MEM_Allocator) {0},
        };
        MSR_ASSERT(swapChain->imgs.count <= swapChain->imgs.capacity &&
            "Swap-chain image count cannot be greater than the pre-allocated fixed-size buffer");

        GPU_VK_CHECKED_CALL(vkGetSwapchainImagesKHR(swapChain->renderer->device, swapChain->actual, &imgCount, swapChain->imgs.data));

        for (isize i = 0; i < swapChain->imgs.count; i++)
        {
            GPU_VK_SET_OBJ_DEBUG_NAME(swapChain->renderer, swapChain->imgs.data[i], "%.swpch_%.img_%",
                FMT(swapChain->renderer->appName), FMT(cfg.objectName), FMT(i));
        }
    }

    {
        swapChain->imgViews = (List_(VkImageView))
        {
            .data      = &(swapChain->buffers.imgViews[0]),
            .count     = swapChain->imgs.count,
            .capacity  = sizeof(swapChain->buffers.imgViews) / sizeof(swapChain->buffers.imgViews[0]),
            .allocator = (MEM_Allocator) {0},
        };
        MSR_ASSERT(swapChain->imgViews.count <= swapChain->imgViews.capacity &&
            "Swap-chain image view count cannot be greater than the pre-allocated fixed-size buffer");

        for (isize i = 0; i < swapChain->imgs.count; i++)
        {
            GPU_VK_CHECKED_CALL(vkCreateImageView(swapChain->renderer->device, &(VkImageViewCreateInfo)
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

            GPU_VK_SET_OBJ_DEBUG_NAME(swapChain->renderer, swapChain->imgViews.data[i], "%.swpch_%.imgview_%",
                FMT(swapChain->renderer->appName), FMT(cfg.objectName), FMT(i));
        }
    }

    {
        swapChain->renderCompleteSems = (List_(VkSemaphore))
        {
            .data      = &(swapChain->buffers.renderCompleteSems[0]),
            .count     = swapChain->imgs.count,
            .capacity  = sizeof(swapChain->buffers.renderCompleteSems) / sizeof(swapChain->buffers.renderCompleteSems[0]),
            .allocator = (MEM_Allocator) {0},
        };
        MSR_ASSERT(swapChain->renderCompleteSems.count <= swapChain->renderCompleteSems.capacity &&
            "Swap-chain render complete semaphore count cannot be greater than the pre-allocated fixed-size buffer");

        for (isize i = 0; i < swapChain->imgs.count; i++)
        {
            GPU_VK_CHECKED_CALL(vkCreateSemaphore(swapChain->renderer->device, &(VkSemaphoreCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            }, nil, &(swapChain->renderCompleteSems.data[i])));

            GPU_VK_SET_OBJ_DEBUG_NAME(swapChain->renderer, swapChain->renderCompleteSems.data[i], "%.swpch_%.rendercompletesem_%",
                FMT(swapChain->renderer->appName), FMT(cfg.objectName), FMT(i));
        }
    }
}

static void GPU_DestroyVkSwapChainImagesAndViews(GPU_VkSwapChain* swapChain)
{
    for (isize i = 0; i < swapChain->renderCompleteSems.count; i++)
    {
        vkDestroySemaphore(swapChain->renderer->device, swapChain->renderCompleteSems.data[i], nil);
        swapChain->renderCompleteSems.data[i] = VK_NULL_HANDLE;
    }
    COL_ClearList(&(swapChain->renderCompleteSems));

    for (isize i = 0; i < swapChain->imgViews.count; i++)
    {
        vkDestroyImageView(swapChain->renderer->device, swapChain->imgViews.data[i], nil);
        swapChain->imgViews.data[i] = VK_NULL_HANDLE;
    }
    COL_ClearList(&(swapChain->imgViews));

    for (isize i = 0; i < swapChain->imgs.count; i++)
    {
        swapChain->imgs.data[i] = VK_NULL_HANDLE;
    }
    COL_ClearList(&(swapChain->imgs));
}

void GPU_VkCreateSwapChainFromWindow(GPU_SwapChain* outBaseSwapChain, GPU_Instance* baseRenderer, WND_Handle windowHandle, GPU_SwapChainCfg cfg)
{
    GPU_VkInstance* renderer = GPU_ToVkInstance(baseRenderer);
    if (!renderer)
        return;

    MSR_ASSERT(!!outBaseSwapChain && "outBaseSwapChain can't be null");
    outBaseSwapChain->base.type = GPU_GfxAPIType_Vk;
    GPU_VkSwapChain* output = GPU_ToVkSwapChain(outBaseSwapChain);
    MSR_ASSERT(output && "output must not be null");

    output->renderer = renderer;
    output->window   = windowHandle;

    #if MSR_WINDOWS
    {
        GPU_VK_CHECKED_CALL(vkCreateWin32SurfaceKHR(renderer->instance, &(VkWin32SurfaceCreateInfoKHR)
        {
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = APP_FromHandle(renderer->appHandle),
            .hwnd      = WND_FromHandle(windowHandle),
        }, nil, &output->surface));
    }
    #elif MSR_ANDROID
    {
        GPU_VK_CHECKED_CALL(vkCreateAndroidSurfaceKHR(renderer->instance, &(VkAndroidSurfaceCreateInfoKHR)
        {
            .sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .window = WND_FromHandle(windowHandle),
        }, nil, &output->surface));
    }
    #elif MSR_OSX
    {
        NSWindow* window = WND_FromHandle(windowHandle);
        NSView* contentView = [window contentView];
        contentView.wantsLayer = YES;

        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        contentView.layer = metalLayer;

        GPU_VK_CHECKED_CALL(vkCreateMetalSurfaceEXT(renderer->instance, &(VkMetalSurfaceCreateInfoEXT)
        {
            .sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
            .pLayer = metalLayer,
        }, nil, &output->surface));
    }
    #elif MSR_LINUX
    {
        GPU_VK_CHECKED_CALL(vkCreateXcbSurfaceKHR(renderer->instance, &(VkXcbSurfaceCreateInfoKHR)
        {
            .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
            .connection = WND_GetXCBConnection(),
            .window     = WND_FromHandle(windowHandle),
        }, nil, &output->surface));
    }
    #else
    {
        #error "unimplemented"
    }
    #endif

    GPU_VK_SET_OBJ_DEBUG_NAME(renderer, output->surface, "%.swpch_%.surface",
        FMT(renderer->appName), FMT(cfg.objectName));

    // select format type
    {
        u32 formatCount = 0;
        GPU_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(output->renderer->physicalDevice, output->surface, &formatCount, nil));
        Slice_(VkSurfaceFormatKHR) surfaceFormats = COL_NewSlice(VkSurfaceFormatKHR, formatCount, true, MEM_temp);
        GPU_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(output->renderer->physicalDevice, output->surface, &formatCount, surfaceFormats.data));
        MSR_ASSERT(formatCount != 0 && "No surface formats available for swapchain");

        output->surfaceFmt = surfaceFormats.data[0];
        for (isize i = 0; i < surfaceFormats.count; i++)
        {
            b8 preferredFormat = false;
            #if MSR_DESKTOP
            {
                preferredFormat = (surfaceFormats.data[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
                    (surfaceFormats.data[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
            }
            #elif MSR_ANDROID
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

    GPU_CreateVkSwapChain(output, cfg);
    GPU_CreateVkSwapChainImagesAndViews(output, cfg);

    output->frameIdx = 0;
    output->nextSignalValue = GPU_FRAMES_IN_FLIGHT + 1;

    {
        GPU_VK_CHECKED_CALL(vkCreateSemaphore(renderer->device, &(VkSemaphoreCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &(VkSemaphoreTypeCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                .initialValue  = GPU_FRAMES_IN_FLIGHT,
            },
        }, nil, &(output->timelineSem)));

        GPU_VK_SET_OBJ_DEBUG_NAME(renderer, output->timelineSem, "%.swpch_%.timelinesem",
            FMT(renderer->appName), FMT(cfg.objectName));
    }

    for (isize i = 0; i < GPU_FRAMES_IN_FLIGHT; i++)
    {
        VkSemaphore* imgAcquiredSem = &(output->perFrameInFlight[i].imgAcquiredSem);
        GPU_VK_CHECKED_CALL(vkCreateSemaphore(renderer->device, &(VkSemaphoreCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        }, nil, imgAcquiredSem));

        GPU_VK_SET_OBJ_DEBUG_NAME(renderer, *imgAcquiredSem, "%.swpch_%.imgacquiredsem_%",
            FMT(renderer->appName), FMT(cfg.objectName), FMT(i));
    }
}

void GPU_VkReconfigureSwapChain(GPU_SwapChain* baseSwapChain, GPU_SwapChainCfg cfg)
{
    GPU_VkSwapChain* swapChain = GPU_ToVkSwapChain(baseSwapChain);
    MSR_ASSERT(swapChain && "baseSwapChain must not be null");

    GPU_VkWaitTillIdle(GPU_FromVkInstance(swapChain->renderer));

    GPU_DestroyVkSwapChainImagesAndViews(swapChain);

    GPU_CreateVkSwapChain(swapChain, cfg);
    GPU_CreateVkSwapChainImagesAndViews(swapChain, cfg);
}

void GPU_VkDestroySwapChain(GPU_SwapChain* baseSwapChain)
{
    GPU_VkSwapChain* swapChain = GPU_ToVkSwapChain(baseSwapChain);
    MSR_ASSERT(swapChain->renderer && "swapChain->renderer must not be null");

    GPU_VkWaitTillIdle(GPU_FromVkInstance(swapChain->renderer));

    for (isize i = 0; i < GPU_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(swapChain->renderer->device, swapChain->perFrameInFlight[i].imgAcquiredSem, nil);
        swapChain->perFrameInFlight[i].imgAcquiredSem = VK_NULL_HANDLE;
    }

    vkDestroySemaphore(swapChain->renderer->device, swapChain->timelineSem, nil);
    swapChain->timelineSem = VK_NULL_HANDLE;

    GPU_DestroyVkSwapChainImagesAndViews(swapChain);
    GPU_DestroyVkSwapChain(swapChain);
}

GPU_TextureFormat GPU_VkGetSwapChainTextureFormat(GPU_SwapChain* baseSwapChain)
{
    GPU_VkSwapChain* swapChain = GPU_ToVkSwapChain(baseSwapChain);
    if (!swapChain) return GPU_TexFmt_Unknown;

    GPU_TextureFormat output = GPU_MakeVkTextureFormat(swapChain->surfaceFmt.format);
    MSR_ASSERT(output != GPU_TexFmt_Unknown && "Failed to convert VkFormat to GPU_TextureFormat");
    return output;
}

void GPU_VkIterateSwapChain(GPU_SwapChain* baseSwapChain)
{
    GPU_VkSwapChain* swapChain = GPU_ToVkSwapChain(baseSwapChain);
    MSR_ASSERT(swapChain->renderer && "swapChain->renderer must not be null");

    swapChain->allowCmdBuff = false;

    // DOING THESE SHENNANIGANS TO FIX MINIMISE ISSUES
    {
        VkSurfaceCapabilitiesKHR surfaceCaps = {0};
        GPU_VK_CHECKED_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapChain->renderer->physicalDevice, swapChain->surface, &surfaceCaps));
        if (!surfaceCaps.currentExtent.width && !surfaceCaps.currentExtent.height) // minimised window
            return;
    }

    // update swapchain indexing
    swapChain->frameIdx++;
    swapChain->nextSignalValue++;

    // wait on the timeline semaphore
    u64 waitValue = swapChain->nextSignalValue - GPU_FRAMES_IN_FLIGHT;
    GPU_VK_CHECKED_CALL(vkWaitSemaphores(swapChain->renderer->device, &(VkSemaphoreWaitInfo)
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &(swapChain->timelineSem),
        .pValues = (uint64_t*) &(waitValue),
    }, U64_MAX));

    // get the next image, and wait if the image of that is still processing
    GPU_VK_CHECKED_CALL(vkAcquireNextImageKHR(
        swapChain->renderer->device,
        swapChain->actual,
        U64_MAX, // timeout
        swapChain->perFrameInFlight[swapChain->frameIdx % GPU_FRAMES_IN_FLIGHT].imgAcquiredSem,
        VK_NULL_HANDLE, // fence
        &(swapChain->acquiredSwpchImgIdx)
    ));

    swapChain->allowCmdBuff = true;
}

GPU_CmdBuffer* GPU_VkGetSwapChainCommandBuffer(GPU_SwapChain* baseSwapChain, u8* outImgIdx)
{
    u8 outImgIdxThrowaway = 0;
    outImgIdx = outImgIdx ? outImgIdx : &outImgIdxThrowaway;
    *outImgIdx = U8_MAX;

    GPU_VkSwapChain* swapChain = GPU_ToVkSwapChain(baseSwapChain);
    if (!swapChain->allowCmdBuff) return nil;

    u64 frameInFlightIdx = swapChain->frameIdx % GPU_FRAMES_IN_FLIGHT;
    GPU_CmdBuffer* baseCmdBuf = &(swapChain->perFrameInFlight[frameInFlightIdx].cmdBuffer);
    GPU_VkCmdBuffer* cmdBuf = GPU_ToVkCmdBuffer(baseCmdBuf);
    GPU_VK_CHECKED_CALL(vkResetCommandPool(swapChain->renderer->device, cmdBuf->cmdPool, 0));

    *outImgIdx = (u8) frameInFlightIdx;
    return baseCmdBuf;
}

void GPU_VkPresentSwapChain(GPU_SwapChain* baseSwapChain)
{
    GPU_VkSwapChain* swapChain = GPU_ToVkSwapChain(baseSwapChain);
    MSR_ASSERT(swapChain->renderer && "swapChain->renderer must not be null");

    if (!swapChain->allowCmdBuff) return;

    u64 frameInFlightIdx = swapChain->frameIdx % GPU_FRAMES_IN_FLIGHT;
    GPU_VkCmdBuffer* cmdBuf = GPU_ToVkCmdBuffer(&(swapChain->perFrameInFlight[frameInFlightIdx].cmdBuffer));

    // TODO: REMOVEEEE - command buffer begin
    GPU_VK_CHECKED_CALL(vkBeginCommandBuffer(cmdBuf->cmdBuffer, &(VkCommandBufferBeginInfo)
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
                .image               = swapChain->imgs.data[swapChain->acquiredSwpchImgIdx],
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
                .imageView   = swapChain->imgViews.data[swapChain->acquiredSwpchImgIdx],
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
                .image               = swapChain->imgs.data[swapChain->acquiredSwpchImgIdx],
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
    GPU_VK_CHECKED_CALL(vkEndCommandBuffer(cmdBuf->cmdBuffer));

    // submit command buffer
    GPU_VK_CHECKED_CALL(vkQueueSubmit2(swapChain->renderer->gfxQueue, 1, &(VkSubmitInfo2)
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nil,
        .flags = 0,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos    = (VkSemaphoreSubmitInfo[])
        {
            { // wait to acquire the image
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = swapChain->perFrameInFlight[frameInFlightIdx].imgAcquiredSem,
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
        .signalSemaphoreInfoCount = 2,
        .pSignalSemaphoreInfos    = (VkSemaphoreSubmitInfo[])
        {
            { // render work completion signal
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = swapChain->renderCompleteSems.data[swapChain->acquiredSwpchImgIdx],
                .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            },
            { // entire frame is completed
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = swapChain->timelineSem,
                .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                .value     = swapChain->nextSignalValue,
            },
        },
    }, VK_NULL_HANDLE));

    // present
    GPU_VK_CHECKED_CALL(vkQueuePresentKHR(swapChain->renderer->gfxQueue, &(VkPresentInfoKHR)
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nil,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(swapChain->renderCompleteSems.data[swapChain->acquiredSwpchImgIdx]),
        .swapchainCount = 1,
        .pSwapchains = &(swapChain->actual),
        .pImageIndices = &(swapChain->acquiredSwpchImgIdx),
        .pResults = nil,
    }));
}

#endif
