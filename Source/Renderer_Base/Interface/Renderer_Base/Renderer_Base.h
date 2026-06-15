#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>

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
#define REN_DECLARE_OBJECT(name, basePadding, extensionPadding) \
    typedef struct REN_##name##_Base \
    { \
        REN_GfxAPIType type; \
        u8 padding[basePadding]; \
    } REN_##name##_Base; \
    typedef struct REN_##name \
    { \
        REN_##name##_Base base; \
        u8 padding[extensionPadding]; \
    } REN_##name;

/**
 * The main renderer interface.
 * This is the main object that represents the renderer instance. While a process may have
 * multiple renderers, it's more common to have just one. The renderer is used as the primary
 * entry point for creating other renderer objects.
 */
REN_DECLARE_OBJECT(Instance, 1, 1);

/**
 * A command buffer for recording rendering commands.
 * Modern graphics APIs often require command buffers that are allocated from the renderer,
 * and get submitted to the GPU for execution.
 */
REN_DECLARE_OBJECT(CmdBuffer, 1, 1);

/**
 * Represents a texture resource that can be used for rendering.
 */
REN_DECLARE_OBJECT(Texture, 1, 1);

/**
 * A swap-chain corresponding to a window that can be rendered to.
 * A swap-chain manages the images that are presented to the screen, and handles
 * synchronization between rendering and presentation.
 */
REN_DECLARE_OBJECT(SwapChain, 1, 1);

/**
 * Represents a shader program.
 * It represents a "pipeline" of shaders that can be used together, and the resources they
 * require.
 */
REN_DECLARE_OBJECT(Program, 1, 1);

#undef REN_DECLARE_OBJECT

#ifndef REN_OBJ_SIZE_CHECK
    #define REN_OBJ_SIZE_CHECK(gfxApi, name) // no-op
#endif

// extend a renderer object; will also do some static checks to
// ensure that the extended object can fit within the opaque padding
// of the base object
#define REN_EXTEND_OBJECT(gfxApi, name, ...) \
    typedef struct REN_##gfxAPi##name \
    { \
        REN_##name##_Base base; \
        __VA_ARGS__ \
    } REN_##gfxAPi##name; \
    REN_OBJ_SIZE_CHECK(gfxApi, name)

EXTERN_C_END
