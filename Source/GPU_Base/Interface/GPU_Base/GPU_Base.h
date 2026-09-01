#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>

EXTERN_C_BEGIN

typedef struct GPU_Instance  GPU_Instance;
typedef struct GPU_CmdBuffer GPU_CmdBuffer;
typedef struct GPU_SwapChain GPU_SwapChain;
typedef struct GPU_Buffer    GPU_Buffer;
typedef struct GPU_Texture   GPU_Texture;
typedef struct GPU_Program   GPU_Program;

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
#define GPU_DECLARE_OBJECT(name, extensionPadding, ...) \
    typedef struct GPU_##name##_Base \
    { \
        GPU_GfxAPIType type; \
        __VA_ARGS__ \
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

    union
    {
        struct
        {
            usize _padding;
        } vk;

        struct
        {
            struct
            {
                u32 cbvSrvUav, rtv, dsv;
            } globalDescriptorHeapSizes;
        } dx12;

        struct
        {
            usize _padding;
        } mtl;
    } apiSpecific;
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
 * A context for a single frame in a swap-chain.
 */
typedef struct
{
    // whether or not this structure is valid
    b8 valid;

    // The index of the frame in flight that this context corresponds to.
    // Guaranteed to be in the range [0, GPU_FRAMES_IN_FLIGHT).
    u8 frameInFlightIdx;

    // The command buffer to use for recording commands for this frame.
    // The command buffer will be returned in a "closed" state.
    GPU_CmdBuffer* cmdBuffer;

    // The texture that corresponds to the swap-chain image for this frame.
    // The user will be responsible for managing the layout and transitions
    // of this texture. The lifetime will be tied to the lifetime of the
    // swap-chain, but reconfiguring the swap-chain will recreate the textures.
    GPU_Texture* swapChainImg;
} GPU_SwapChainFrameContext;

/**
 * A swap-chain corresponding to a window that can be rendered to.
 * A swap-chain manages the images that are presented to the screen, and handles
 * synchronization between rendering and presentation.
 */
GPU_DECLARE_OBJECT(SwapChain, 2048);

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
    GPU_BufUsg_ReadOnly = 1 << 0,

    // read-write buffer in shaders, typically used for a compute shader's state
    // VK -> storage buffer, DX12 -> UAV buffer
    GPU_BufUsg_ReadWrite = 1 << 1,

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

/*
 * Defines the available usages for a texture resource.
 */
typedef u8 GPU_TextureUsage;
enum GPU_TextureUsages
{
    // read-only texture in shaders, typically used for different texture maps
    // VK -> sampled image, DX12 -> SRV texture
    GPU_TexUsg_ReadOnly = 1 << 0,

    // read-write texture in shaders, typically used for a compute shader's state
    // VK -> storage image, DX12 -> UAV texture
    GPU_TexUsg_ReadWrite = 1 << 1,

    // texture that can be rendered to, typically used for a swap-chain or offscreen render target
    // VK -> color attachment, DX12 -> RTV texture
    GPU_TexUsg_DrawTarget = 1 << 2,

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

/*
 * Defines the available layouts that a texture resource can be in.
 * Using a specific layout requires the texture to be created with the appropriate usage flags.
 * It also requires that the texture is appropriately transitioned to the target layout before use.
 */
typedef u8 GPU_TextureLayout;
enum GPU_TextureLayouts
{
    // unknown layout (invalid)
    // VK -> VK_IMAGE_LAYOUT_UNDEFINED
    // DX12 -> D3D12_BARRIER_LAYOUT_UNDEFINED
    GPU_TexLyt_Unknown,

    // general layout (read/write)
    // VK -> VK_IMAGE_LAYOUT_GENERAL
    // DX12 -> D3D12_BARRIER_LAYOUT_COMMON
    GPU_TexLyt_Generic,

    // basic read-only layout (read-only)
    // VK -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // DX12 -> D3D12_BARRIER_LAYOUT_SHADER_RESOURCE
    GPU_TexLyt_ReadOnly,

    // basic read-write layout (read/write)
    // VK -> VK_IMAGE_LAYOUT_GENERAL
    // DX12 -> D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS
    GPU_TexLyt_ReadWrite,

    // draw target layout (write-only)
    // VK -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    // DX12 -> D3D12_BARRIER_LAYOUT_RENDER_TARGET
    GPU_TexLyt_DrawTarget,

    // presentable layout (read by the driver)
    // VK -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    // DX12 -> D3D12_BARRIER_LAYOUT_PRESENT
    GPU_TexLyt_Present,

    // depth-stencil layout (read-only)
    // VK -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    // DX12 -> D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ
    GPU_TexLyt_DepthStencilRead,

    // depth-stencil layout (write-only)
    // VK -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    // DX12 -> D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE
    GPU_TexLyt_DepthStencilWrite,

    // copy source layout (read-only)
    // VK -> VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    // DX12 -> D3D12_BARRIER_LAYOUT_COPY_SOURCE
    GPU_TexLyt_CopySrc,

    // copy destination layout (write-only)
    // VK -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // DX12 -> D3D12_BARRIER_LAYOUT_COPY_DEST
    GPU_TexLyt_CopyDst,

    // shading rate source layout (read-only)
    // VK -> VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR
    // DX12 -> D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE
    GPU_TexLyt_ShadingRateSrc,

    GPU_TexLyt_MAX,
};

static_assert(GPU_TexLyt_MAX < INTEGER_MAX(GPU_TextureLayout), "GPU_TextureLayout must be able to fit all GPU_TextureLayouts");

/**
 * Defines the available texture formats.
 */
typedef u8 GPU_TextureFormat;
enum GPU_TextureFormats
{
    GPU_TexFmt_Unknown,
    GPU_TexFmt_D32_Float,
    GPU_TexFmt_D24_UNorm_S8_UInt,
    GPU_TexFmt_B8G8R8A8_UNorm,
    GPU_TexFmt_R8G8B8A8_UNorm,
    GPU_TexFmt_R16G16B16A16_UNorm,
};

COL_DECLARE_FOR(GPU_TextureFormat);

/**
 * Represents a texture resource that can be used on the GPU.
 */
GPU_DECLARE_OBJECT(Texture, 256);

typedef struct
{
    u16               width, height;
    GPU_MemType       memType;
    GPU_TextureUsage  usages;
    GPU_TextureFormat format;
    GPU_TextureLayout initialLayout;
    utf8str           objectName;
} GPU_TextureCfg;

/*
 * Defines the available pipeline barrier stages for GPU synchronization.
 * These are to be used with a barrier to specify the stage in a program
 * before/after which synchronisation is required.
 */
typedef u16 GPU_BarrierStage;
enum GPU_BarrierStages
{
    // no stages
    // VK -> VK_PIPELINE_STAGE_2_NONE
    // DX12 -> D3D12_BARRIER_SYNC_NONE
    GPU_BarStg_None = 0,

    // index buffer read
    // VK -> VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT
    // DX12 -> D3D12_BARRIER_SYNC_INDEX_INPUT
    GPU_BarStg_IndexInput = 1 << 0,

    // vertex stage execution
    // VK -> VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT
    // DX12 -> D3D12_BARRIER_SYNC_VERTEX_SHADING
    GPU_BarStg_VertexPgmStg = 1 << 2,

    // fragment stage execution
    // VK -> VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
    // DX12 -> D3D12_BARRIER_SYNC_PIXEL_SHADING
    GPU_BarStg_FragmentPgmStg = 1 << 3,

    // depth-stencil read/write
    // VK -> VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT
    // DX12 -> D3D12_BARRIER_SYNC_DEPTH_STENCIL
    GPU_BarStg_DepthStencil = 1 << 4,

    // draw target read/write
    // VK -> VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    // DX12 -> D3D12_BARRIER_SYNC_RENDER_TARGET
    GPU_BarStg_DrawTarget = 1 << 5,

    // presentation stage
    // VK -> VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    // DX12 -> D3D12_BARRIER_SYNC_NONE (presentation is handled by the driver)
    GPU_BarStg_Present = 1 << 6,

    // compute stage execution
    // VK -> VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
    // DX12 -> D3D12_BARRIER_SYNC_COMPUTE_SHADING
    GPU_BarStg_ComputePgmStg = 1 << 7,

    // copy/blit operations
    // VK -> VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT
    // DX12 -> D3D12_BARRIER_SYNC_COPY
    GPU_BarStg_Copy = 1 << 8,

    // clear operations (including output and depth-stencil clears)
    // VK -> VK_PIPELINE_STAGE_2_CLEAR_BIT
    // DX12 -> D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW | D3D12_BARRIER_SYNC_RENDER_TARGET | D3D12_BARRIER_SYNC_DEPTH_STENCIL
    GPU_BarStg_Clear = 1 << 9,

    // resolve operations
    // VK -> VK_PIPELINE_STAGE_2_RESOLVE_BIT
    // DX12 -> D3D12_BARRIER_SYNC_RESOLVE
    GPU_BarStg_Resolve = 1 << 10,

    // indirect draw/dispatch operations
    // VK -> VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT
    // DX12 -> D3D12_BARRIER_SYNC_EXECUTE_INDIRECT
    GPU_BarStg_IndirectDraw = 1 << 11,

    // all stages
    GPU_BarStg_All = GPU_BarStg_IndexInput |
                     GPU_BarStg_VertexPgmStg |
                     GPU_BarStg_FragmentPgmStg |
                     GPU_BarStg_DepthStencil |
                     GPU_BarStg_DrawTarget |
                     GPU_BarStg_ComputePgmStg |
                     GPU_BarStg_Copy |
                     GPU_BarStg_Clear |
                     GPU_BarStg_Resolve |
                     GPU_BarStg_IndirectDraw,

    // all graphics stages (vertex, fragment, depth, output)
    GPU_BarStg_AllGraphics = GPU_BarStg_IndexInput |
                             GPU_BarStg_VertexPgmStg |
                             GPU_BarStg_FragmentPgmStg |
                             GPU_BarStg_DepthStencil |
                             GPU_BarStg_DrawTarget,

    // all shader stages (vertex, fragment, compute)
    GPU_BarStg_AllPgmStgs = GPU_BarStg_VertexPgmStg |
                            GPU_BarStg_FragmentPgmStg |
                            GPU_BarStg_ComputePgmStg,

    GPU_BarStg_MAX,
};

static_assert(GPU_BarStg_MAX < INTEGER_MAX(GPU_BarrierStage), "GPU_BarrierStage must be able to fit all GPU_BarrierStages");

/*
 * Defines the available access types for GPU synchronization.
 * These are to be used with a barrier to specify the type of
 * memory access before/after which synchronisation is required.
 */
typedef u16 GPU_BarrierAccess;
enum GPU_BarrierAccesses
{
    // no access
    // VK -> VK_ACCESS_2_NONE
    // DX12 -> D3D12_BARRIER_ACCESS_NO_ACCESS
    GPU_BarAcc_None = 0,

    // common access
    // VK -> VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_COMMON
    GPU_BarAcs_Common = 1 << 0,

    // reading the vertex buffer
    // VK -> VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_VERTEX_BUFFER
    GPU_BarAcs_VertexBuffer = 1 << 1,

    // reading the index buffer
    // VK -> VK_ACCESS_2_INDEX_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_INDEX_BUFFER
    GPU_BarAcs_IndexBuffer = 1 << 2,

    // reading a read-only buffer
    // VK -> VK_ACCESS_2_UNIFORM_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_CONSTANT_BUFFER
    GPU_BarAcs_ReadROBuffer = 1 << 3,

    // reading a read-only texture
    // VK -> VK_ACCESS_2_SHADER_SAMPLED_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_SHADER_RESOURCE
    GPU_BarAcs_ReadROTexture = 1 << 4,

    // reading a read-write buffer/texture
    // VK -> VK_ACCESS_2_SHADER_STORAGE_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_SHADER_RESOURCE | D3D12_BARRIER_ACCESS_UNORDERED_ACCESS
    GPU_BarAcs_ReadRWResource = 1 << 5,

    // writing a read-write buffer/texture
    // VK -> VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_UNORDERED_ACCESS
    GPU_BarAcs_WriteRWResource = 1 << 6,

    // accessing the draw target
    // VK -> VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_RENDER_TARGET
    GPU_BarAcs_DrawTarget = 1 << 7,

    // reading the depth-stencil buffer
    // VK -> VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ
    GPU_BarAcs_DepthStencilRead = 1 << 8,

    // writing the depth-stencil buffer
    // VK -> VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE
    GPU_BarAcs_DepthStencilWrite = 1 << 9,

    // copy/resolve source access
    // VK -> VK_ACCESS_2_TRANSFER_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_COPY_SOURCE | D3D12_BARRIER_ACCESS_RESOLVE_SOURCE
    GPU_BarAcs_CopySrc = 1 << 10,

    // copy/resolve destination access
    // VK -> VK_ACCESS_2_TRANSFER_WRITE_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_COPY_DEST | D3D12_BARRIER_ACCESS_RESOLVE_DEST
    GPU_BarAcs_CopyDst = 1 << 11,

    // reading the shading rate image (VRS)
    // VK -> VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR
    // DX12 -> D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE
    GPU_BarAcs_ShadingRateSrc = 1 << 12,

    // reading the indirect draw/dispatch arguments
    // VK -> VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT
    // DX12 -> D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT
    GPU_BarAcs_IndirectDrawArgs = 1 << 13,

    GPU_BarAcc_MAX,
};

/*
 * Defines the source and destination synchronization scopes for a barrier.
 * The barrier guarantees to run after all source stages+accesses have ended.
 * The destination stages+accesses will not begin until the barrier has completed.
 */
typedef struct
{
    GPU_BarrierStage  stage;
    GPU_BarrierAccess access;
} GPU_SyncScopeCfg;

/*
 * Defines the configuration for a global barrier.
 * A global barrier synchronizes all GPU work across all resources.
 */
typedef struct
{
    GPU_SyncScopeCfg src, dst;
} GPU_GlobalBarrierCfg;

COL_DECLARE_FOR(GPU_GlobalBarrierCfg);

/*
 * Defines the configuration for a buffer barrier.
 * A buffer barrier synchronizes access to a range of a specific buffer
 * resource across the specified source and destination stages.
 */
typedef struct
{
    GPU_SyncScopeCfg src, dst;
    GPU_Buffer*      buffer;
    u32              offset, size;
} GPU_BufferBarrierCfg;

COL_DECLARE_FOR(GPU_BufferBarrierCfg);

/*
 * Defines the configuration for a texture barrier.
 * A texture barrier synchronizes access to a specific texture resource across
 * the specified source and destination stages, and also allows for layout transitions.
 */
typedef struct
{
    GPU_SyncScopeCfg  src, dst;
    GPU_Texture*      texture;
    GPU_TextureLayout srcLayout, dstLayout;
    #if 0 // uncomment when we need to support subresource barriers
    u8                baseMipLvl, mipLvlCount;
    u16               baseArrayLayer, arrayLayerCount;
    #endif
} GPU_TextureBarrierCfg;

COL_DECLARE_FOR(GPU_TextureBarrierCfg);

/*
 * Defines the configuration for a set of barriers.
 * A barrier configuration can contain multiple global, buffer, and texture barriers.
 */
typedef struct
{
    Slice_(GPU_GlobalBarrierCfg)  globalBarriers;
    Slice_(GPU_BufferBarrierCfg)  bufferBarriers;
    Slice_(GPU_TextureBarrierCfg) textureBarriers;
} GPU_BarrierCfg;

/**
 * Defines the available load operations for a render pass.
 * These are applied at the beginning of a pass.
 */
typedef u8 GPU_LoadOp;
enum GPU_LoadOps
{
    // default; load the target with its existing contents
    GPU_LoadOp_Load,

    // clear the target to a specified value
    GPU_LoadOp_Clear,

    // don't care about the target's contents
    GPU_LoadOp_DontCare,
};

/**
 * Defines the available store operations for a render pass.
 * These are applied at the end of a pass.
 */
typedef u8 GPU_StoreOp;
enum GPU_StoreOps
{
    // default; store the target's contents to memory at the end of the pass
    GPU_StoreOp_Store,

    // don't care about the target's contents at the end of the pass
    GPU_StoreOp_DontCare,
};

/**
 * Defines the targets for a GPU pass.
 */
typedef struct
{
    // the target texture to draw to
    GPU_Texture* target;

    // set to `true` if the target is supposed to be
    // in the ideal layout to draw to
    // set to `false` if the target is supposed to be
    // in a common layout
    b8 idealLayout;

    // how to handle the contents at the start of the pass
    GPU_LoadOp loadOp;

    // how to handle the contents at the end of the pass
    GPU_StoreOp storeOp;

    // if loadOp is `GPU_LoadOp_Clear`, this is the color to clear to
    float clearColor[4];
} GPU_PassDrawTargetCfg;

COL_DECLARE_FOR(GPU_PassDrawTargetCfg);

typedef struct
{
    // the target texture to use for depth-stencil
    // if this is `nil`, then no depth-stencil target will be used for the pass
    GPU_Texture* target;

    // set to `true` if the target is supposed to be
    // in the ideal layout to use for depth-stencil
    // set to `false` if the target is supposed to be
    // in a common layout
    b8 idealLayout;

    // how to handle the contents at the start of the pass
    GPU_LoadOp loadOp;

    // how to handle the contents at the end of the pass
    GPU_StoreOp storeOp;

    // if loadOp is `GPU_LoadOp_Clear`, this is the depth value to clear to
    float clearDepth;

    // if loadOp is `GPU_LoadOp_Clear`, this is the stencil value to clear to
    u8    clearStencil;
} GPU_PassDepthStencilTargetCfg;

/**
 * Defines the configuration for a GPU pass.
 * A pass is a collection of rendering commands that are executed together on a specific
 * set of draw targets and depth-stencil targets.
 *
 * Note that all the targets (draw/depth-stencil) within a pass must be the same resolution.
 */
typedef struct
{
    // the draw targets for the pass
    // all the draw targets must have a valid texture
    Slice_(GPU_PassDrawTargetCfg) drawTargets;

    // the depth-stencil target for the pass
    // zero-init if not wanted
    GPU_PassDepthStencilTargetCfg depthStencilTarget;
} GPU_PassCfg;

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
} GPU_ProgramStageByteCode;

COL_DECLARE_FOR(GPU_ProgramStageByteCode);

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
} GPU_ProgramStageByteCodeCfg;

/**
 * Represents a program stage loaded onto the GPU.
 */
GPU_DECLARE_OBJECT(ProgramStage, 128);

typedef u8 GPU_ProgramType;
enum GPU_ProgramTypes
{
    GPU_ProgramType_VertexFragment,   // vertex + fragment stages
    GPU_ProgramType_MeshFragment,     // mesh + fragment stages
    GPU_ProgramTYpe_TaskMeshFragment, // task + mesh + fragment stages
    GPU_ProgramType_Compute,          // compute stage only
};

/**
 * Configuration structure to refer to a program stage for program creation.
 */
typedef struct
{
    GPU_ProgramStage* stage;
} GPU_ProgramStageCfg;

COL_DECLARE_FOR(GPU_ProgramStageCfg);

/**
 * Defines the available culling modes for a GPU program.
 */
typedef u8 GPU_CullMode;
enum GPU_CullModes
{
    // cull counter-clockwise faces
    GPU_CullMode_CounterClockwise,

    // cull clockwise faces
    GPU_CullMode_Clockwise,

    // don't cull any faces
    GPU_CullMode_None,
};

/**
 * Configuration structure for GPU program creation.
 */
typedef struct
{
    // different stages that make up this program
    Slice_(GPU_ProgramStageCfg) stages;

    struct
    {
        // the set of draw target formats
        Slice_(GPU_TextureFormat) draw;

        // the depth-stencil format
        GPU_TextureFormat         depthStencil;
    } targetFormats;

    utf8str objectName;
} GPU_ProgramCfg;

/**
 * Represents a GPU program.
 * It represents a "pipeline" of stages that can be used together, and the resources they
 * require.
 */
GPU_DECLARE_OBJECT(Program, 128);

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
