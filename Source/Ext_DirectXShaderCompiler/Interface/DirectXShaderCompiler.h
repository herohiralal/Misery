#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN

#if MSR_WINDOWS
    #include "Unknwn.h"
    #define DX_SHADER_COMPILER_PATH_INTERNAL "dxcompiler.dll"
#elif MSR_LINUX
    #define DX_SHADER_COMPILER_PATH_INTERNAL "libdxcompiler.so"
#elif MSR_OSX
    #define DX_SHADER_COMPILER_PATH_INTERNAL "libdxcompiler.dylib"
#endif

#ifndef DX_SHADER_COMPILER_PATH_INTERNAL

    /**
     * Whether or not the DirectX Shader Compiler is available on this platform.
     */
    #define DX_SHADER_COMPILER 0

#else

    /**
     * Whether or not the DirectX Shader Compiler is available on this platform.
     */
    #define DX_SHADER_COMPILER 1

    /**
    * The relative path to the DirectX Shader Compiler library, based on the platform.
    * This path is relative to the output directory of the Brahma package.
    */
    #define DX_SHADER_COMPILER_PATH "DXC/" DX_SHADER_COMPILER_PATH_INTERNAL

#endif

// regardless whether shader compiler is available or not,
// the api is cpp-only
#if DX_SHADER_COMPILER && defined(__cplusplus)
    #include "ExtDeps/dxc/dxcapi.h"
#endif
