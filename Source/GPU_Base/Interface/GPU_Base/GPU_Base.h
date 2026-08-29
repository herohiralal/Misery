#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>

EXTERN_C_BEGIN

/**
 * Defines the available gfx API types.
 */
typedef u64 GPU_GfxAPIType; // we don't need 64-bits, but doing this to ensure better size/alignments
enum GPU_GfxAPITypes
{
    GPU_GfxAPIType_Null,
    GPU_GfxAPIType_Vk,
    GPU_GfxAPIType_Dx12,
    GPU_GfxAPIType_Mtl,
};

// declare an rhi-unspecific gpu object (with opaque padding)
#define GPU_DECLARE_OBJECT(name, extensionPadding) \
    typedef struct GPU_##name##_Base \
    { \
        GPU_GfxAPIType type; \
    } GPU_##name##_Base; \
    typedef struct GPU_##name \
    { \
        GPU_##name##_Base base; \
        u8 padding[extensionPadding]; \
    } GPU_##name; \
    COL_DECLARE_FOR(GPU_##name)

/**
 * Configuration structure for GPU API creation.
 */
typedef struct
{
    GPU_GfxAPIType type;
    APP_Handle appHandle;
    utf8str appName;
} GPU_InstanceCfg;

/**
 * The main GPU interface.
 * This is the main object that represents the GPU API instance. While a process may have
 * multiple GPU APIs active, it's more common to have just one. This object is used as the primary
 * entry point for creating other GPU objects.
 */
GPU_DECLARE_OBJECT(Instance, 176);

/**
 * A command buffer for recording GPU commands.
 */
GPU_DECLARE_OBJECT(CmdBuffer, 40);

/**
 * Configuration structure for swap-chain creation.
 */
typedef struct
{
    u16     width;
    u16     height;
    b8      vSync;
    utf8str objectName;
} GPU_SwapChainCfg;

#ifndef GPU_FRAMES_IN_FLIGHT
    /**
    * Defines the number of frames that can be in-flight at once.
    * This is used to determine how many command buffers and synchronization objects to create.
    * To change this value, declare it as a package define from Brahma.
    */
    #define GPU_FRAMES_IN_FLIGHT (2u)
#endif

/**
 * A swap-chain corresponding to a window that can be rendered to.
 * A swap-chain manages the images that are presented to the screen, and handles
 * synchronization between rendering and presentation.
 */
GPU_DECLARE_OBJECT(SwapChain, 656);

/**
 * Defines the available memory types for GPU resources.
 */
typedef u8 GPU_MemType;
enum GPU_MemTypes
{
    GPU_MemType_Default,  // device-local when available (UMA/ReBAR), host-writable (write-combined)
    GPU_MemType_GPU,      // device-local                           , not host-visible at all
    GPU_MemType_Readback, // device-local                           , host-readable (cached)
};

/**
 * Defines the available usages for a buffer resource.
 */
typedef u8 GPU_BufferUsage;
enum GPU_BufferUsages
{
    // read-only buffer in shaders, typically used for material properties, camera data, etc.
    // VK -> uniform buffer, DX12 -> constant buffer
    GPU_BufUsg_BasicRead = 1 << 0,

    // read-write buffer in shaders, typically used for a compute shader's state
    // VK -> storage buffer, DX12 -> UAV buffer
    GPU_BufUsg_BasicReadWrite = 1 << 1,

    // special buffer used for indirect draw/dispatch calls, typically used for GPU-driven rendering
    // VK -> indirect buffer, DX12 -> indirect buffer
    GPU_BufUsg_IndirectDrawArgs = 1 << 2,

    // read-only buffer in shaders for a mesh's vertex data
    // DX12/VK -> vertex buffer
    GPU_BufUsg_Vertices = 1 << 3,

    // read-only buffer in shaders for a mesh's index data
    // DX12/VK -> index buffer
    GPU_BufUsg_Indices = 1 << 4,

    // buffer that can be used as a source for a transfer operation, typically used for CPU-side uploads to GPU resources
    // VK -> transfer src, DX12 -> copy src
    GPU_BufUsg_CopySrc = 1 << 5,

    // buffer that can be used as a destination for a transfer operation
    // VK -> transfer dst, DX12 -> copy dst
    GPU_BufUsg_CopyDst = 1 << 6,

    GPU_BufUsg_MAX, // invalid sentinel value for bounds checking
};

static_assert(GPU_BufUsg_MAX < INTEGER_MAX(GPU_BufferUsage), "GPU_BufferUsage must be able to fit all GPU_BufferUsages");

/**
 * Configuration structure for buffer creation.
 */
typedef struct
{
    GPU_MemType     memType;
    GPU_BufferUsage usages;
    usize           size, align;
    utf8str         objectName;
} GPU_BufferCfg;

/**
 * Represents a buffer resource that can be used on the GPU.
 */
GPU_DECLARE_OBJECT(Buffer, 64);

typedef u8 GPU_TextureUsage;
enum GPU_TextureUsages
{
    // read-only texture in shaders, typically used for different texture maps
    // VK -> sampled image, DX12 -> SRV texture
    GPU_TexUsg_BasicRead = 1 << 0,

    // read-write texture in shaders, typically used for a compute shader's state
    // VK -> storage image, DX12 -> UAV texture
    GPU_TexUsg_BasicReadWrite = 1 << 1,

    // texture that can be rendered to, typically used for a swap-chain or offscreen render target
    // VK -> color attachment, DX12 -> RTV texture
    GPU_TexUsg_DrawOutput = 1 << 2,

    // texture that can be presented to the screen, typically used for a swap-chain
    // VK -> presentable image, DX12 -> swap-chain texture
    GPU_TexUsg_Present = 1 << 3,

    // texture that can be used as a depth-stencil attachment, typically used for a depth buffer
    // VK -> depth-stencil, DX12 -> DSV texture
    GPU_TexUsg_DepthStencil = 1 << 4,

    // texture that can be used as a source for a transfer operation, typically used for CPU-side uploads to GPU resources
    // VK -> transfer src, DX12 -> copy src
    GPU_TexUsg_CopySrc = 1 << 5,

    // texture that can be used as a destination for a transfer operation
    // VK -> transfer dst, DX12 -> copy dst
    GPU_TexUsg_CopyDst = 1 << 6,

    GPU_TexUsg_MAX, // invalid sentinel value for bounds checking
};

static_assert(GPU_TexUsg_MAX < INTEGER_MAX(GPU_TextureUsage), "GPU_TextureUsage must be able to fit all GPU_TextureUsages");

/**
 * Defines the available texture formats.
 */
typedef u8 GPU_TextureFormat;
enum GPU_TextureFormats
{
    GPU_TexFmt_Unknown,
    GPU_TexFmt_D32_Float,
    GPU_TexFmt_B8G8R8A8_UNorm,
    GPU_TexFmt_R8G8B8A8_UNorm,
    GPU_TexFmt_R16G16B16A16_UNorm,
};

/**
 * Represents a texture resource that can be used on the GPU.
 */
GPU_DECLARE_OBJECT(Texture, 64);

typedef struct
{
    u16               width, height;
    GPU_MemType       memType;
    GPU_TextureUsage  usages;
    GPU_TextureFormat format;
    utf8str           objectName;
} GPU_TextureCfg;

/**
 * Defines the types of program stages that are supported by this library.
 */
typedef u8 GPU_ProgramStageType;
enum GPU_ProgramStageTypes
{
    GPU_ProgramStageType_Unknown,
    GPU_ProgramStageType_Compute,
    GPU_ProgramStageType_Task,
    GPU_ProgramStageType_Mesh,
    GPU_ProgramStageType_Vertex,
    GPU_ProgramStageType_Fragment,
};

/**
 * Represents a compiled program stage blob.
 * - On Vulkan, this is SPIR-V bytecode.
 * - On D3D12, this is DXIL bytecode.
 * - On Metal, this is MSL source code.
 */
typedef struct
{
    GPU_ProgramStageType stage;
    MEM_Allocator allocator;
    Slice_(u8) code;
    utf8str entryPoint;
} GPU_ProgramStage;

COL_DECLARE_FOR(GPU_ProgramStage);

/**
 * Configuration structure for program stage creation
 */
typedef struct
{
    GPU_GfxAPIType       gfxAPI;
    GPU_ProgramStageType stage;
    FIL_Path             file;
    utf8str              entryPoint;
    MEM_Allocator        allocator;
} GPU_ProgramStageCfg;

/**
 * Configuration structure for GPU program creation.
 */
typedef struct
{
    Slice_(GPU_ProgramStage) stages;
    utf8str objectName;
} GPU_ProgramCfg;

/**
 * Represents a GPU program.
 * It represents a "pipeline" of stages that can be used together, and the resources they
 * require.
 */
GPU_DECLARE_OBJECT(Program, 1);

#undef GPU_DECLARE_OBJECT

#ifndef GPU_OBJ_SIZE_CHECK
    #define GPU_OBJ_SIZE_CHECK(gfxApi, name) // no-op
#endif

// extend a gpu object; will also do some static checks to
// ensure that the extended object can fit within the opaque padding
// of the base object
#define GPU_EXTEND_OBJECT(gfxApi, name, ...) \
    typedef struct GPU_##gfxApi##name \
    { \
        GPU_##name##_Base base; \
        __VA_ARGS__ \
    } GPU_##gfxApi##name; \
    GPU_OBJ_SIZE_CHECK(gfxApi, name) \
    static inline GPU_##gfxApi##name* GPU_To##gfxApi##name(GPU_##name* base) \
    { \
        MSR_ASSERT(!!base && base->base.type == GPU_GfxAPIType_##gfxApi && "Type mismatch!"); \
        return (GPU_##gfxApi##name*) base; \
    } \
    static inline GPU_##name* GPU_From##gfxApi##name(GPU_##gfxApi##name* extended) \
    { \
        return (GPU_##name*) extended; \
    }

EXTERN_C_END
