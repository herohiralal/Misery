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

#endif
