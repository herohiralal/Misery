#include "VkFns.h"
#if MZNT_VULKAN

inline VkFormat MZNT_Internal_BreakVkTextureFormat(MZNT_TextureFormat fmt)
{
    switch (fmt)
    {
        case MZNT_TextureFormat_D32_Float:          return VK_FORMAT_D32_SFLOAT;
        case MZNT_TextureFormat_R8G8B8A8_UNorm:     return VK_FORMAT_R8G8B8A8_UNORM;
        case MZNT_TextureFormat_B8G8R8A8_UNorm:     return VK_FORMAT_B8G8R8A8_UNORM;
        case MZNT_TextureFormat_R16G16B16A16_UNorm: return VK_FORMAT_R16G16B16A16_UNORM;
    }

    return VK_FORMAT_UNDEFINED;
}

inline MZNT_TextureFormat MZNT_Internal_MakeVkTextureFormat(VkFormat fmt)
{
    PNSLR_SUPPRESS_WARN // the enum has like a 100 cases...
    switch (fmt)
    {
        case VK_FORMAT_D32_SFLOAT:         return MZNT_TextureFormat_D32_Float;
        case VK_FORMAT_R8G8B8A8_UNORM:     return MZNT_TextureFormat_R8G8B8A8_UNorm;
        case VK_FORMAT_B8G8R8A8_UNORM:     return MZNT_TextureFormat_B8G8R8A8_UNorm;
        case VK_FORMAT_R16G16B16A16_UNORM: return MZNT_TextureFormat_R16G16B16A16_UNorm;
    }
    PNSLR_UNSUPPRESS_WARN

    return MZNT_TextureFormat_Unknown;
}

#endif
