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

#endif
