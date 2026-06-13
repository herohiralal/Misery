#pragma once
#include <__init.h>
#include <ExtDeps_Platform.h>

#ifdef REN_VK
    #error "REN_VK is already defined"
#endif

// TODO: add MoltenVK to support OSX/iOS
#define REN_VK (MSR_WINDOWS || MSR_LINUX || MSR_ANDROID)

MSR_SUPPRESS_WARN
#if REN_VK
    #if MSR_WINDOWS
        #define VK_USE_PLATFORM_WIN32_KHR
    #elif MSR_ANDROID
        #define VK_USE_PLATFORM_ANDROID_KHR
    #elif MSR_LINUX
        #define VK_USE_PLATFORM_XLIB_KHR
        #define VK_USE_PLATFORM_XCB_KHR
    #endif

    #if !MSR_APPLE
        #define VK_NO_PROTOTYPES 1
    #endif
    #include "ExtDeps/vulkan/vulkan.h"
#endif
MSR_UNSUPPRESS_WARN
