#include "VkPrivate.h"

#if GPU_VK

VkFormat GPU_BreakVkTextureFormat(GPU_TextureFormat fmt)
{
    switch ((enum GPU_TextureFormats) fmt)
    {
        case GPU_TexFmt_Unknown:            return VK_FORMAT_UNDEFINED;
        case GPU_TexFmt_D32_Float:          return VK_FORMAT_D32_SFLOAT;
        case GPU_TexFmt_R8G8B8A8_UNorm:     return VK_FORMAT_R8G8B8A8_UNORM;
        case GPU_TexFmt_B8G8R8A8_UNorm:     return VK_FORMAT_B8G8R8A8_UNORM;
        case GPU_TexFmt_R16G16B16A16_UNorm: return VK_FORMAT_R16G16B16A16_UNORM;
    }

    return VK_FORMAT_UNDEFINED;
}

GPU_TextureFormat GPU_MakeVkTextureFormat(VkFormat fmt)
{
    MSR_SUPPRESS_WARN // the enum has like 250+ cases...
    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wswitch"
    #endif
    #ifdef __clang__
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wswitch"
    #endif
    switch (fmt)
    {
        case VK_FORMAT_D32_SFLOAT:         return GPU_TexFmt_D32_Float;
        case VK_FORMAT_R8G8B8A8_UNORM:     return GPU_TexFmt_R8G8B8A8_UNorm;
        case VK_FORMAT_B8G8R8A8_UNORM:     return GPU_TexFmt_B8G8R8A8_UNorm;
        case VK_FORMAT_R16G16B16A16_UNORM: return GPU_TexFmt_R16G16B16A16_UNorm;
    }
    #ifdef __clang__
        #pragma clang diagnostic pop
    #endif
    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif
    MSR_UNSUPPRESS_WARN

    return GPU_TexFmt_Unknown;
}

VkImageLayout GPU_BreakVkTextureLayout(GPU_TextureLayout layout)
{
    switch ((enum GPU_TextureLayouts) layout)
    {
        case GPU_TexLyt_Unknown:            return VK_IMAGE_LAYOUT_UNDEFINED;
        case GPU_TexLyt_Generic:            return VK_IMAGE_LAYOUT_GENERAL;
        case GPU_TexLyt_ReadOnly:           return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case GPU_TexLyt_ReadWrite:          return VK_IMAGE_LAYOUT_GENERAL;
        case GPU_TexLyt_DrawTarget:         return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case GPU_TexLyt_Present:            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case GPU_TexLyt_DepthStencilRead:   return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case GPU_TexLyt_DepthStencilWrite:  return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case GPU_TexLyt_CopySrc:            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case GPU_TexLyt_CopyDst:            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case GPU_TexLyt_ShadingRateSrc:     return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
        case GPU_TexLyt_MAX:                MSR_ASSERT(false && "Invalid GPU_TextureLayout value"); break;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags2 GPU_BreakVkBarrierStage(GPU_BarrierStage stages)
{
    if (stages == GPU_BarStg_None)
        return VK_PIPELINE_STAGE_2_NONE;

    VkPipelineStageFlags2 stage = 0;
    if (stages & GPU_BarStg_IndexInput)     stage |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
    if (stages & GPU_BarStg_VertexPgmStg)   stage |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
    if (stages & GPU_BarStg_FragmentPgmStg) stage |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    if (stages & GPU_BarStg_DepthStencil)   stage |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    if (stages & GPU_BarStg_DrawTarget)     stage |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (stages & GPU_BarStg_Present)        stage |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (stages & GPU_BarStg_ComputePgmStg)  stage |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (stages & GPU_BarStg_Copy)           stage |= VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT;
    if (stages & GPU_BarStg_Clear)          stage |= VK_PIPELINE_STAGE_2_CLEAR_BIT;
    if (stages & GPU_BarStg_Resolve)        stage |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
    if (stages & GPU_BarStg_IndirectDraw)   stage |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;

    return stage;
}

VkAccessFlags2 GPU_BreakVkBarrierAccess(GPU_BarrierAccess accesses)
{
    if (accesses == GPU_BarAcc_None)
        return VK_ACCESS_2_NONE;

    VkAccessFlags2 access = 0;
    if (accesses & GPU_BarAcs_Common)            access |= VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    if (accesses & GPU_BarAcs_VertexBuffer)      access |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    if (accesses & GPU_BarAcs_IndexBuffer)       access |= VK_ACCESS_2_INDEX_READ_BIT;
    if (accesses & GPU_BarAcs_ReadROBuffer)      access |= VK_ACCESS_2_UNIFORM_READ_BIT;
    if (accesses & GPU_BarAcs_ReadROTexture)     access |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    if (accesses & GPU_BarAcs_ReadRWResource)    access |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    if (accesses & GPU_BarAcs_WriteRWResource)   access |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    if (accesses & GPU_BarAcs_DrawTarget)        access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (accesses & GPU_BarAcs_DepthStencilRead)  access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (accesses & GPU_BarAcs_DepthStencilWrite) access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (accesses & GPU_BarAcs_CopySrc)           access |= VK_ACCESS_2_TRANSFER_READ_BIT;
    if (accesses & GPU_BarAcs_CopyDst)           access |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    if (accesses & GPU_BarAcs_ShadingRateSrc)    access |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    if (accesses & GPU_BarAcs_IndirectDrawArgs)  access |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

    return access;
}

VkAttachmentLoadOp GPU_BreakVkLoadOp(GPU_LoadOp op)
{
    switch ((enum GPU_LoadOps) op)
    {
        case GPU_LoadOp_Clear:     return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case GPU_LoadOp_Load:      return VK_ATTACHMENT_LOAD_OP_LOAD;
        case GPU_LoadOp_DontCare:  return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:                   MSR_ASSERT(false && "Invalid GPU_LoadOp value"); break;
    }

    return VK_ATTACHMENT_LOAD_OP_LOAD;
}

VkAttachmentStoreOp GPU_BreakVkStoreOp(GPU_StoreOp op)
{
    switch ((enum GPU_StoreOps) op)
    {
        case GPU_StoreOp_Store:     return VK_ATTACHMENT_STORE_OP_STORE;
        case GPU_StoreOp_DontCare:  return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:                    MSR_ASSERT(false && "Invalid GPU_StoreOp value"); break;
    }

    return VK_ATTACHMENT_STORE_OP_STORE;
}

#endif
