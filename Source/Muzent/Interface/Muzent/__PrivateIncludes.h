#ifndef MZNT_PRIVATE_INLCUDES_H
#define MZNT_PRIVATE_INLCUDES_H

#include "Panshilar/__PrivateIncludes.h"

/*
This is where we define what all rendering backends Muzent supports.
These can be turned off by defining the relevant macros to 0 before including
Muzent.
*/

#ifndef MZNT_VULKAN
    // TODO: add MoltenVK to support OSX/iOS
    #define MZNT_VULKAN (PNSLR_WINDOWS || PNSLR_LINUX || PNSLR_ANDROID)
#endif

#ifndef MZNT_DX12
    // TODO: implement
    #define MZNT_DX12 (PNSLR_WINDOWS || PNSLR_XSERIES)
#endif

#ifndef MZNT_METAL
    // TODO: implement
    #define MZNT_METAL 0 /*(PNSLR_APPLE)*/
#endif

PNSLR_SUPPRESS_WARN

// VULKAN
#if MZNT_VULKAN
    #if PNSLR_WINDOWS
        #define VK_USE_PLATFORM_WIN32_KHR
    #elif PNSLR_ANDROID
        #define VK_USE_PLATFORM_ANDROID_KHR
        #include <android_native_app_glue.h>
    #elif PNSLR_LINUX
        #define VK_USE_PLATFORM_XLIB_KHR
    #endif

    #if !defined(__cplusplus) && defined(MZNT_IMPLEMENTATION)
        #define VOLK_IMPLEMENTATION
    #endif
    #define VOLK_VULKAN_H_PATH "../vulkan/vulkan.h"
    #include "Dependencies/volk/volk.h"
    #undef VOLK_VULKAN_H_PATH

    #if defined(__cplusplus) && defined(MZNT_IMPLEMENTATION) // contains some includes that only work with c++
        #define VMA_IMPLEMENTATION
    #endif
    #include "Dependencies/VMA/vk_mem_alloc.h"
    #undef VMA_IMPLEMENTATION
#endif

#if MZNT_DX12
    #ifndef __cplusplus
        #define CINTERFACE
        #define COBJMACROS
    #endif

    #include <combaseapi.h>
    #include "Dependencies/d3d12/d3d12.h"
    #include <dxgi1_6.h>
    #include <d3dcompiler.h>
    #if PNSLR_DBG
        #include <dxgidebug.h>
    #endif

    #undef COBJMACROS
    #undef CINTERFACE

    #ifdef __cplusplus
        #include "Dependencies/D3D12MA/D3D12MemAlloc.h"
        #include "Dependencies/D3D12MA/D3D12MemAlloc.cpp"

        #define D3DX12_NO_STATE_OBJECT_HELPERS
        #define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
        #define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
        #include "Dependencies/d3d12/d3dx12/d3dx12.h"
        #undef D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
        #undef D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
        #undef D3DX12_NO_STATE_OBJECT_HELPERS

        #pragma comment(lib, "dxguid.lib")
        #pragma comment(lib, "d3d12.lib")
        #pragma comment(lib, "dxgi.lib")
        #pragma comment(lib, "d3dcompiler.lib")
    #endif
#endif

#if PNSLR_WINDOWS || PNSLR_LINUX
    #ifdef __cplusplus
        #include "Dependencies/dxc/dxcapi.h"
    #endif
#endif

PNSLR_UNSUPPRESS_WARN

#endif//MZNT_PRIVATE_INCLUDES_H
