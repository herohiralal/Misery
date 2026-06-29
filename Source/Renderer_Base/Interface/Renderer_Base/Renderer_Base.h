#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>
#include "Shaders_Base.h"

EXTERN_C_BEGIN

/**
 * Defines the available gfx API types.
 */
typedef u64 REN_GfxAPIType; // we don't need 64-bits, but doing this to ensure better size/alignments
enum REN_GfxAPITypes
{
    REN_GfxAPIType_Null,
    REN_GfxAPIType_Vk,
    REN_GfxAPIType_Dx12,
    REN_GfxAPIType_Mtl,
};

// declare a renderer object (with opaque padding)
#define REN_DECLARE_OBJECT(name, extensionPadding) \
    typedef struct REN_##name##_Base \
    { \
        REN_GfxAPIType type; \
    } REN_##name##_Base; \
    typedef struct REN_##name \
    { \
        REN_##name##_Base base; \
        u8 padding[extensionPadding]; \
    } REN_##name; \
    COL_DECLARE_FOR(REN_##name)

/**
 * Configuration structure for renderer creation.
 */
typedef struct
{
    REN_GfxAPIType type;
    APP_Handle appHandle;
    utf8str appName;
} REN_InstanceCfg;

/**
 * The main renderer interface.
 * This is the main object that represents the renderer instance. While a process may have
 * multiple renderers, it's more common to have just one. The renderer is used as the primary
 * entry point for creating other renderer objects.
 */
REN_DECLARE_OBJECT(Instance, 112);

/**
 * A command buffer for recording rendering commands.
 * Modern graphics APIs often require command buffers that are allocated from the renderer,
 * and get submitted to the GPU for execution.
 */
REN_DECLARE_OBJECT(CmdBuffer, 40);

/**
 * Defines the available texture formats.
 */
typedef u8 REN_TextureFormat;
enum REN_TextureFormats
{
    REN_TexFmt_Unknown,
    REN_TexFmt_D32_Float,
    REN_TexFmt_B8G8R8A8_UNorm,
    REN_TexFmt_R8G8B8A8_UNorm,
    REN_TexFmt_R16G16B16A16_UNorm,
};

/**
 * Represents a texture resource that can be used for rendering.
 */
REN_DECLARE_OBJECT(Texture, 1);

/**
 * Configuration structure for swap-chain creation.
 */
typedef struct
{
    u16     width;
    u16     height;
    b8      vSync;
    u8      framesInFlight;
    utf8str objectName;
} REN_SwapChainCfg;

/**
 * A swap-chain corresponding to a window that can be rendered to.
 * A swap-chain manages the images that are presented to the screen, and handles
 * synchronization between rendering and presentation.
 */
REN_DECLARE_OBJECT(SwapChain, 168);

/**
 * Configuration structure for shader program creation.
 */
typedef struct
{
    Slice_(SHD_ByteCode) shaders;
    utf8str objectName;
} REN_ProgramCfg;

/**
 * Represents a shader program.
 * It represents a "pipeline" of shaders that can be used together, and the resources they
 * require.
 */
REN_DECLARE_OBJECT(Program, 1);

#undef REN_DECLARE_OBJECT

#ifndef REN_OBJ_SIZE_CHECK
    #define REN_OBJ_SIZE_CHECK(gfxApi, name) // no-op
#endif

// extend a renderer object; will also do some static checks to
// ensure that the extended object can fit within the opaque padding
// of the base object
#define REN_EXTEND_OBJECT(gfxApi, name, ...) \
    typedef struct REN_##gfxApi##name \
    { \
        REN_##name##_Base base; \
        __VA_ARGS__ \
    } REN_##gfxApi##name; \
    REN_OBJ_SIZE_CHECK(gfxApi, name) \
    static inline REN_##gfxApi##name* REN_To##gfxApi##name(REN_##name* base) \
    { \
        MSR_ASSERT(!!base && base->base.type == REN_GfxAPIType_##gfxApi && "Type mismatch!"); \
        return (REN_##gfxApi##name*) base; \
    } \
    static inline REN_##name* REN_From##gfxApi##name(REN_##gfxApi##name* extended) \
    { \
        return (REN_##name*) extended; \
    }

EXTERN_C_END
