#include "Dx12Private.h"

#if REN_DX12

DXGI_FORMAT REN_BreakDx12TextureFormat(REN_TextureFormat fmt)
{
    switch ((enum REN_TextureFormats) fmt)
    {
        case REN_TexFmt_D32_Float:          return DXGI_FORMAT_D32_FLOAT;
        case REN_TexFmt_R8G8B8A8_UNorm:     return DXGI_FORMAT_R8G8B8A8_UNORM;
        case REN_TexFmt_B8G8R8A8_UNorm:     return DXGI_FORMAT_B8G8R8A8_UNORM;
        case REN_TexFmt_R16G16B16A16_UNorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case REN_TexFmt_Unknown:
        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

REN_TextureFormat REN_MakeDx12TextureFormat(DXGI_FORMAT fmt)
{
    MSR_SUPPRESS_WARN
    switch (fmt)
    {
        case DXGI_FORMAT_D32_FLOAT:          return REN_TexFmt_D32_Float;
        case DXGI_FORMAT_R8G8B8A8_UNORM:     return REN_TexFmt_R8G8B8A8_UNorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM:     return REN_TexFmt_B8G8R8A8_UNorm;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return REN_TexFmt_R16G16B16A16_UNorm;
        default:
            break;
    }
    MSR_UNSUPPRESS_WARN

    return REN_TexFmt_Unknown;
}

#endif
