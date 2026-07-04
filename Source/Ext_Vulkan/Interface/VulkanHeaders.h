#pragma once
#include <__init.h>
#include <ExtDeps_Platform.h>

#ifndef REN_VK
    // TODO: add MoltenVK framework linking to support iOS
    #define REN_VK (MSR_WINDOWS || MSR_LINUX || MSR_ANDROID || MSR_OSX)
#endif

MSR_SUPPRESS_WARN
#if REN_VK
    #if MSR_WINDOWS
        #define VK_USE_PLATFORM_WIN32_KHR
    #elif MSR_ANDROID
        #define VK_USE_PLATFORM_ANDROID_KHR
    #elif MSR_LINUX
        #define VK_USE_PLATFORM_XLIB_KHR
        #define VK_USE_PLATFORM_XCB_KHR
    #elif MSR_APPLE
        #define VK_USE_PLATFORM_METAL_EXT
    #endif

    #if !MSR_IOS
        #define VK_NO_PROTOTYPES 1
    #endif
    #include "ExtDeps/vulkan/vulkan.h"

    // don't wanna enable vulkan beta so here's the workaround
    #ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        #define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
    #endif

    #if MSR_APPLE
        #include <QuartzCore/QuartzCore.h>
    #endif
#endif
MSR_UNSUPPRESS_WARN
