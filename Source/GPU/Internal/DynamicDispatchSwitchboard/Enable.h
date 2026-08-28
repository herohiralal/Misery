#include <GPU/GPU.h>
#include <GPU_Vk/GPU_Vk.h>
#include <GPU_Dx12/GPU_Dx12.h>
#include <GPU_Mtl/GPU_Mtl.h>

#ifdef RHI_SWITCHBOARD_ENABLED
#include "Disable.h"
#endif

#define RHI_SWITCHBOARD_ENABLED

#if GPU_VK
    #define RHI_FN_VK_BRANCH_RET(fnName, ...)    case GPU_GfxAPIType_Vk:   return GPU_Vk##fnName(__VA_ARGS__);
    #define RHI_FN_VK_BRANCH_VOID(fnName, ...)   case GPU_GfxAPIType_Vk:          GPU_Vk##fnName(__VA_ARGS__); break;
    #define RHI_FN_VK_BRANCH_FALLBACK()
#else
    #define RHI_FN_VK_BRANCH_RET(fnName, ...)
    #define RHI_FN_VK_BRANCH_VOID(fnName, ...)
    #define RHI_FN_VK_BRANCH_FALLBACK()          case GPU_GfxAPIType_Vk:
#endif

#if GPU_DX12
    #define RHI_FN_DX12_BRANCH_RET(fnName, ...)  case GPU_GfxAPIType_Dx12: return GPU_Dx12##fnName(__VA_ARGS__);
    #define RHI_FN_DX12_BRANCH_VOID(fnName, ...) case GPU_GfxAPIType_Dx12:        GPU_Dx12##fnName(__VA_ARGS__); break;
    #define RHI_FN_DX12_BRANCH_FALLBACK()
#else
    #define RHI_FN_DX12_BRANCH_RET(fnName, ...)
    #define RHI_FN_DX12_BRANCH_VOID(fnName, ...)
    #define RHI_FN_DX12_BRANCH_FALLBACK()        case GPU_GfxAPIType_Dx12:
#endif

#if GPU_MTL
    #define RHI_FN_MTL_BRANCH_RET(fnName, ...)   case GPU_GfxAPIType_Mtl:  return GPU_Mtl##fnName(__VA_ARGS__);
    #define RHI_FN_MTL_BRANCH_VOID(fnName, ...)  case GPU_GfxAPIType_Mtl:         GPU_Mtl##fnName(__VA_ARGS__); break;
    #define RHI_FN_MTL_BRANCH_FALLBACK()
#else
    #define RHI_FN_MTL_BRANCH_RET(fnName, ...)
    #define RHI_FN_MTL_BRANCH_VOID(fnName, ...)
    #define RHI_FN_MTL_BRANCH_FALLBACK()         case GPU_GfxAPIType_Mtl:
#endif

#define RHI_FN_SWITCH_RET(tyQ, ret, ...) \
    switch ((enum GPU_GfxAPITypes) (tyQ)) \
    { \
        RHI_FN_VK_BRANCH_RET(__VA_ARGS__) \
        RHI_FN_DX12_BRANCH_RET(__VA_ARGS__) \
        RHI_FN_MTL_BRANCH_RET(__VA_ARGS__) \
        case GPU_GfxAPIType_Null: \
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
    switch ((enum GPU_GfxAPITypes) (tyQ)) \
    { \
        RHI_FN_VK_BRANCH_VOID(__VA_ARGS__) \
        RHI_FN_DX12_BRANCH_VOID(__VA_ARGS__) \
        RHI_FN_MTL_BRANCH_VOID(__VA_ARGS__) \
        case GPU_GfxAPIType_Null: \
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
