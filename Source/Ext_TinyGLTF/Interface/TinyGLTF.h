#pragma once
#include <__init.h>
#include <StbImage.h>

MSR_SUPPRESS_WARN
#define TINYGLTF3_ENABLE_STB_IMAGE
#include "ExtDeps/tiny_gltf_v3.h"
#undef TINYGLTF3_ENABLE_STB_IMAGE
MSR_UNSUPPRESS_WARN

// if we're not building the dependencies, add our own context to override certain functionalities

#if !MSR_BUILD_DEPS

    #include <Core/Core.h>

    EXTERN_C_BEGIN

    /**
    * The context required to use TinyGLTF.
    * Overrides certain functionalities to use our own systems.
    */
    typedef struct
    {
        MEM_Allocator allocator;
    } TGLTF_Ctx;

    /**
    * Get an allocator to use with TinyGLTF.
    * The memory pertaining to `ctx` must stay valid for the lifetime
    * of the allocator.
    */
    tg3_allocator TGLTF_GetAllocator(TGLTF_Ctx* ctx);

    /**
    * Get a file-system callbacks to use with TinyGLTF.
    * The memory pertaining to `ctx` must stay valid for the lifetime
    * of the file-system callbacks.
    */
    tg3_fs_callbacks TGLTF_GetFileSystem(TGLTF_Ctx* ctx);

    EXTERN_C_END

#endif
