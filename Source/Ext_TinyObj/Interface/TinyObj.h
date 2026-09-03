#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN
#define TOBJ_NO_LIBC
#define TOBJ_ENABLE_MULTITHREADING
#include "ExtDeps/tiny_obj_c.h"
#include "ExtDeps/tobj_tess.h"
#undef TOBJ_ENABLE_MULTITHREADING
#undef TOBJ_NO_LIBC
MSR_UNSUPPRESS_WARN

// if we're not building the dependencies, add our own context to override certain functionalities

#if !MSR_BUILD_DEPS

    #include <Core/Core.h>

    EXTERN_C_BEGIN

    /**
     * The context required to use TinyObj.
     * Overrides certain functionalities to use our own systems.
     */
    typedef struct
    {
        MEM_Allocator allocator;
    } TOBJ_Ctx;

    /**
     * Get an allocator to use with TinyObj.
     * The memory pertaining to `ctx` must stay valid for the lifetime
     * of the allocator.
     */
    tobj_allocator TOBJ_GetAllocator(TOBJ_Ctx* ctx);

    /**
     * The diagnostics sink required to use TinyObj.
     * Overrides certain functionalities to use our own systems.
     */
    typedef struct
    {
        utf8str objectName;
    } TOBJ_Diagnostics;

    /**
     * Get a diagnostics sink to use with TinyObj.
     * The memory pertaining to `ctx` must stay valid for the lifetime
     * of the diagnostics sink.
     */
    tobj_diag TOBJ_GetDiagnostics(TOBJ_Diagnostics* ctx);

    /**
     * The stream required to use TinyObj.
     * Overrides certain functionalities to use our own systems.
     */
    typedef struct
    {
        TOBJ_Ctx* ctx;
        IO_Stream stream;
    } TOBJ_Stream;

    /**
     * Get a stream to use with TinyObj.
     * The memory pertaining to `stream`, `stream->ctx` and must stay
     * valid for the lifetime of the stream callbacks.
     */
    tobj_io_callbacks TOBJ_GetStream(TOBJ_Stream* stream);

    /**
     * Get a default load configuration to use with TinyObj.
     * The memory pertaining to `ctx` must stay valid for the lifetime
     * of the load configuration.
     */
    tobj_load_config TOBJ_GetDefaultLoadConfig(TOBJ_Ctx* ctx);

    /**
     * Load an OBJ file from memory.
     * The memory pertaining to `output` and `allocator` must stay valid
     * for the lifetime of the output scene.
     */
    tobj_result TOBJ_LoadObjFromMemory(tobj_scene* output, MEM_Allocator allocator,
        utf8str objName, Slice_(u8) data);

    /**
     * Load an OBJ file from a file.
     * The memory pertaining to `output` and `allocator` must stay valid
     * for the lifetime of the output scene.
     */
    tobj_result TOBJ_LoadObjFromFile(tobj_scene* output, MEM_Allocator allocator,
        FIL_Path path);

    /**
     * Load an OBJ file from a stream.
     * The memory pertaining to `output` and `allocator` must stay valid
     * for the lifetime of the output scene.
     */
    tobj_result TOBJ_LoadObjFromStream(tobj_scene* output, MEM_Allocator allocator,
        utf8str objName, IO_Stream stream);

    EXTERN_C_END

#endif
