#include "Dx12Fns.h"
#if MZNT_DX12

DXGI_FORMAT MZNT_Internal_BreakDx12TextureFormat(MZNT_TextureFormat fmt)
{
    switch (fmt)
    {
        case MZNT_TextureFormat_D32_Float:          return DXGI_FORMAT_D32_FLOAT;
        case MZNT_TextureFormat_R8G8B8A8_UNorm:     return DXGI_FORMAT_R8G8B8A8_UNORM;
        case MZNT_TextureFormat_B8G8R8A8_UNorm:     return DXGI_FORMAT_B8G8R8A8_UNORM;
        case MZNT_TextureFormat_R16G16B16A16_UNorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
    }

    return DXGI_FORMAT_UNKNOWN;
}

MZNT_TextureFormat MZNT_Internal_MakeDx12TextureFormat(DXGI_FORMAT fmt)
{
    PNSLR_SUPPRESS_WARN
    switch (fmt)
    {
        case DXGI_FORMAT_D32_FLOAT:          return MZNT_TextureFormat_D32_Float;
        case DXGI_FORMAT_R8G8B8A8_UNORM:     return MZNT_TextureFormat_R8G8B8A8_UNorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM:     return MZNT_TextureFormat_B8G8R8A8_UNorm;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return MZNT_TextureFormat_R16G16B16A16_UNorm;
    }
    PNSLR_UNSUPPRESS_WARN

    return MZNT_TextureFormat_Unknown;
}

#endif
