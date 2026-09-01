#include "Dx12Private.h"

#if GPU_DX12

DXGI_FORMAT GPU_BreakDx12TextureFormat(GPU_TextureFormat fmt)
{
    switch ((enum GPU_TextureFormats) fmt)
    {
        case GPU_TexFmt_D32_Float:          return DXGI_FORMAT_D32_FLOAT;
        case GPU_TexFmt_R8G8B8A8_UNorm:     return DXGI_FORMAT_R8G8B8A8_UNORM;
        case GPU_TexFmt_B8G8R8A8_UNorm:     return DXGI_FORMAT_B8G8R8A8_UNORM;
        case GPU_TexFmt_R16G16B16A16_UNorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case GPU_TexFmt_Unknown:
        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

GPU_TextureFormat GPU_MakeDx12TextureFormat(DXGI_FORMAT fmt)
{
    MSR_SUPPRESS_WARN
    switch (fmt)
    {
        case DXGI_FORMAT_D32_FLOAT:          return GPU_TexFmt_D32_Float;
        case DXGI_FORMAT_R8G8B8A8_UNORM:     return GPU_TexFmt_R8G8B8A8_UNorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM:     return GPU_TexFmt_B8G8R8A8_UNorm;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return GPU_TexFmt_R16G16B16A16_UNorm;
        default:
            break;
    }
    MSR_UNSUPPRESS_WARN

    return GPU_TexFmt_Unknown;
}

D3D12_BARRIER_LAYOUT GPU_BreakDx12TextureLayout(GPU_TextureLayout layout)
{
    switch ((enum GPU_TextureLayouts) layout)
    {
        case GPU_TexLyt_Unknown:            return D3D12_BARRIER_LAYOUT_UNDEFINED;
        case GPU_TexLyt_Generic:            return D3D12_BARRIER_LAYOUT_COMMON;
        case GPU_TexLyt_ReadOnly:           return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
        case GPU_TexLyt_ReadWrite:          return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
        case GPU_TexLyt_DrawTarget:         return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        case GPU_TexLyt_Present:            return D3D12_BARRIER_LAYOUT_PRESENT;
        case GPU_TexLyt_DepthStencilRead:   return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
        case GPU_TexLyt_DepthStencilWrite:  return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
        case GPU_TexLyt_CopySrc:            return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
        case GPU_TexLyt_CopyDst:            return D3D12_BARRIER_LAYOUT_COPY_DEST;
        case GPU_TexLyt_ShadingRateSrc:     return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
        case GPU_TexLyt_MAX:                MSR_ASSERT(false && "Invalid GPU_TextureLayout value"); break;
    }

    return D3D12_BARRIER_LAYOUT_UNDEFINED;
}

D3D12_BARRIER_SYNC GPU_BreakDx12BarrierStage(GPU_BarrierStage stages)
{
    if (stages == GPU_BarStg_None || stages == GPU_BarStg_Present)
        return D3D12_BARRIER_SYNC_NONE;

    D3D12_BARRIER_SYNC sync = 0;
    if (stages & GPU_BarStg_IndexInput)     sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
    if (stages & GPU_BarStg_VertexPgmStg)   sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
    if (stages & GPU_BarStg_FragmentPgmStg) sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
    if (stages & GPU_BarStg_DepthStencil)   sync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    if (stages & GPU_BarStg_DrawTarget)     sync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
    if (stages & GPU_BarStg_ComputePgmStg)  sync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
    if (stages & GPU_BarStg_Copy)           sync |= D3D12_BARRIER_SYNC_COPY;
    if (stages & GPU_BarStg_Clear)          sync |= D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW | D3D12_BARRIER_SYNC_RENDER_TARGET | D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    if (stages & GPU_BarStg_Resolve)        sync |= D3D12_BARRIER_SYNC_RESOLVE;
    if (stages & GPU_BarStg_IndirectDraw)   sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;

    return sync;
}

D3D12_BARRIER_ACCESS GPU_BreakDx12BarrierAccess(GPU_BarrierAccess accesses)
{
    if (accesses == GPU_BarAcc_None)
        return D3D12_BARRIER_ACCESS_NO_ACCESS;

    D3D12_BARRIER_ACCESS access = 0;
    if (accesses & GPU_BarAcs_Common)            access |= D3D12_BARRIER_ACCESS_COMMON;
    if (accesses & GPU_BarAcs_VertexBuffer)      access |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
    if (accesses & GPU_BarAcs_IndexBuffer)       access |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
    if (accesses & GPU_BarAcs_ReadROBuffer)      access |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
    if (accesses & GPU_BarAcs_ReadROTexture)     access |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    if (accesses & GPU_BarAcs_ReadRWResource)    access |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE | D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    if (accesses & GPU_BarAcs_WriteRWResource)   access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    if (accesses & GPU_BarAcs_DrawTarget)        access |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
    if (accesses & GPU_BarAcs_DepthStencilRead)  access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    if (accesses & GPU_BarAcs_DepthStencilWrite) access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
    if (accesses & GPU_BarAcs_CopySrc)           access |= D3D12_BARRIER_ACCESS_COPY_SOURCE | D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
    if (accesses & GPU_BarAcs_CopyDst)           access |= D3D12_BARRIER_ACCESS_COPY_DEST | D3D12_BARRIER_ACCESS_RESOLVE_DEST;
    if (accesses & GPU_BarAcs_ShadingRateSrc)    access |= D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;
    if (accesses & GPU_BarAcs_IndirectDrawArgs)  access |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;

    return access;
}

#endif
