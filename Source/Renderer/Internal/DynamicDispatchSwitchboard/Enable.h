#include <Renderer/Renderer.h>
#include <Renderer_Vk/Renderer_Vk.h>
#include <Renderer_Dx12/Renderer_Dx12.h>
#include <Renderer_Mtl/Renderer_Mtl.h>

#ifdef RHI_SWITCHBOARD_ENABLED
#include "Disable.h"
#endif

#define RHI_SWITCHBOARD_ENABLED

#if REN_VK
    #define RHI_FN_VK_BRANCH_RET(fnName, ...)    case REN_GfxAPIType_Vk:   return REN_Vk##fnName(__VA_ARGS__);
    #define RHI_FN_VK_BRANCH_VOID(fnName, ...)   case REN_GfxAPIType_Vk:          REN_Vk##fnName(__VA_ARGS__);
    #define RHI_FN_VK_BRANCH_FALLBACK()
#else
    #define RHI_FN_VK_BRANCH_RET(fnName, ...)
    #define RHI_FN_VK_BRANCH_VOID(fnName, ...)
    #define RHI_FN_VK_BRANCH_FALLBACK()          case REN_GfxAPIType_Vk:
#endif

#if REN_DX12
    #define RHI_FN_DX12_BRANCH_RET(fnName, ...)  case REN_GfxAPIType_Dx12: return REN_Dx12##fnName(__VA_ARGS__);
    #define RHI_FN_DX12_BRANCH_VOID(fnName, ...) case REN_GfxAPIType_Dx12:        REN_Dx12##fnName(__VA_ARGS__);
    #define RHI_FN_DX12_BRANCH_FALLBACK()
#else
    #define RHI_FN_DX12_BRANCH_RET(fnName, ...)
    #define RHI_FN_DX12_BRANCH_VOID(fnName, ...)
    #define RHI_FN_DX12_BRANCH_FALLBACK()        case REN_GfxAPIType_Dx12:
#endif

#if REN_MTL
    #define RHI_FN_MTL_BRANCH_RET(fnName, ...)   case REN_GfxAPIType_Mtl:  return REN_Mtl##fnName(__VA_ARGS__);
    #define RHI_FN_MTL_BRANCH_VOID(fnName, ...)  case REN_GfxAPIType_Mtl:         REN_Mtl##fnName(__VA_ARGS__);
    #define RHI_FN_MTL_BRANCH_FALLBACK()
#else
    #define RHI_FN_MTL_BRANCH_RET(fnName, ...)
    #define RHI_FN_MTL_BRANCH_VOID(fnName, ...)
    #define RHI_FN_MTL_BRANCH_FALLBACK()         case REN_GfxAPIType_Mtl:
#endif

#define RHI_FN_SWITCH_RET(tyQ, ret, ...) \
    switch ((enum REN_GfxAPITypes) (tyQ)) \
    { \
        RHI_FN_VK_BRANCH_RET(__VA_ARGS__) \
        RHI_FN_DX12_BRANCH_RET(__VA_ARGS__) \
        RHI_FN_MTL_BRANCH_RET(__VA_ARGS__) \
        case REN_GfxAPIType_Null: \
            return (ret); \
        RHI_FN_VK_BRANCH_FALLBACK() \
        RHI_FN_DX12_BRANCH_FALLBACK() \
        RHI_FN_MTL_BRANCH_FALLBACK() \
            MSR_ASSERT(false && "Unsupported graphics API type!"); \
            return (ret); \
        default: \
            MSR_ASSERT(false && "Invalid graphics API type!"); \
            return (ret); \
    }

#define RHI_FN_SWITCH_VOID(tyQ, ...) \
    switch ((enum REN_GfxAPITypes) (tyQ)) \
    { \
        RHI_FN_VK_BRANCH_VOID(__VA_ARGS__) \
        RHI_FN_DX12_BRANCH_VOID(__VA_ARGS__) \
        RHI_FN_MTL_BRANCH_VOID(__VA_ARGS__) \
        case REN_GfxAPIType_Null: \
            break; \
        RHI_FN_VK_BRANCH_FALLBACK() \
        RHI_FN_DX12_BRANCH_FALLBACK() \
        RHI_FN_MTL_BRANCH_FALLBACK() \
            MSR_ASSERT(false && "Unsupported graphics API type!"); \
            break; \
        default: \
            MSR_ASSERT(false && "Invalid graphics API type!"); \
            break; \
    }
