#pragma once
#include <__init.h>

#ifndef REN_MTL
    #define REN_MTL (MSR_APPLE)
#endif

MSR_SUPPRESS_WARN
#if REN_MTL
    #include <Metal/Metal.h>
    #include <MetalKit/MetalKit.h>

    #ifdef __cplusplus
        #include <Metal/Metal.hpp>
        #include <MetalKit/MetalKit.hpp>
    #endif
#endif
MSR_UNSUPPRESS_WARN
