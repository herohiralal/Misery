#ifdef MZNT_RENDERER_SWITCHBOARD_ENABLED
#include "Disable.h"
#endif

#define MZNT_RENDERER_SWITCHBOARD_ENABLED

#define MZNT_RHI_ARG_PASS_STAT(ty, arg, api) arg
#define MZNT_RHI_ARG_PASS_DYNA(ty, arg, api) (MZNT_##api##ty*) arg

#define MZNT_RHI_ARG_DECL_STAT(ty) ty
#define MZNT_RHI_ARG_DECL_DYNA(ty) MZNT_##ty*

#define MZNT_RHI_RET_START_STAT /*nothing*/
#define MZNT_RHI_RET_START_DYNA &(

#define MZNT_RHI_RET_END_STAT /*nothing*/
#define MZNT_RHI_RET_END_DYNA ->parent)

#define MZNT_RHI_ARG_CHECK_STAT(tyQ, arg) /*nothing*/
#if PNSLR_DBG
    #define MZNT_RHI_ARG_CHECK_DYNA(tyQ, arg) do { if (arg->type != tyQ) { PNSLR_LogE(PNSLR_StringLiteral("Arg mismatch: " #arg " doesn't match " #tyQ "."), PNSLR_GET_LOC()); FORCE_DBG_TRAP; } } while (0)
#else
    #define MZNT_RHI_ARG_CHECK_DYNA(tyQ, arg) /*nothing*/
#endif

#define MZNT_RHI_FN_NULL_BRANCH(ret)                case MZNT_RendererType_Null:     return (ret) {0};

#if MZNT_VULKAN
    #define MZNT_RHI_FN_VK_BRANCH(tyRet, fn, ...)   case MZNT_RendererType_Vulkan:   return MZNT_RHI_RET_START_##tyRet MZNT_##fn##_Vulkan     ( __VA_ARGS__ ) MZNT_RHI_RET_END_##tyRet;
#else
    #define MZNT_RHI_FN_VK_BRANCH(tyRet, fn, ...)
#endif

#if MZNT_DX12
    #define MZNT_RHI_FN_DX12_BRANCH(tyRet, fn, ...) case MZNT_RendererType_DirectX12: return MZNT_RHI_RET_START_##tyRet MZNT_##fn##_DirectX12 ( __VA_ARGS__ ) MZNT_RHI_RET_END_##tyRet;
#else
    #define MZNT_RHI_FN_DX12_BRANCH(tyRet, fn, ...)
#endif

#if MZNT_METAL
    #define MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, ...) case MZNT_RendererType_Metal:    return MZNT_RHI_RET_START_##tyRet MZNT_##fn##_Metal     ( __VA_ARGS__ ) MZNT_RHI_RET_END_##tyRet;
#else
    #define MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, ...)
#endif

#define MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret)          default: PNSLR_LogE(PNSLR_StringLiteral("Unsupported renderer type."), PNSLR_GET_LOC()); FORCE_DBG_TRAP; return (ret) {0};

#define MZNT_RHI_SWITCH_ONE_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1) \
    switch (tyQ) \
    { \
        MZNT_RHI_FN_NULL_BRANCH(ret) \
        MZNT_RHI_FN_VK_BRANCH   (tyRet, fn, (MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Vulkan   ))) \
        MZNT_RHI_FN_DX12_BRANCH (tyRet, fn, (MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, DirectX12))) \
        MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, (MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Metal    ))) \
        MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret) \
    }

#define MZNT_RHI_SWITCH_TWO_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2) \
    switch (tyQ) \
    { \
        MZNT_RHI_FN_NULL_BRANCH(ret) \
        MZNT_RHI_FN_VK_BRANCH   (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Vulkan   )) \
        MZNT_RHI_FN_DX12_BRANCH (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, DirectX12), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, DirectX12)) \
        MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Metal    ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Metal    )) \
        MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret) \
    }

#define MZNT_RHI_SWITCH_THREE_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3) \
    switch (tyQ) \
    { \
        MZNT_RHI_FN_NULL_BRANCH(ret) \
        MZNT_RHI_FN_VK_BRANCH   (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Vulkan   )) \
        MZNT_RHI_FN_DX12_BRANCH (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, DirectX12), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, DirectX12), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, DirectX12)) \
        MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Metal    ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Metal    ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Metal    )) \
        MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret) \
    }

#define MZNT_RHI_SWITCH_FOUR_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4) \
    switch (tyQ) \
    { \
        MZNT_RHI_FN_NULL_BRANCH(ret) \
        MZNT_RHI_FN_VK_BRANCH   (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, Vulkan   )) \
        MZNT_RHI_FN_DX12_BRANCH (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, DirectX12), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, DirectX12), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, DirectX12), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, DirectX12)) \
        MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Metal    ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Metal    ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Metal    ), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, Metal    )) \
        MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret) \
    }

#define MZNT_RHI_SWITCH_FIVE_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4, ty5, arg5, tyArg5) \
    switch (tyQ) \
    { \
        MZNT_RHI_FN_NULL_BRANCH(ret) \
        MZNT_RHI_FN_VK_BRANCH   (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg5 (ty5, arg5, Vulkan   )) \
        MZNT_RHI_FN_DX12_BRANCH (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, DirectX12), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, DirectX12), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, DirectX12), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, DirectX12), MZNT_RHI_ARG_PASS_##tyArg5 (ty5, arg5, DirectX12)) \
        MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Metal    ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Metal    ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Metal    ), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, Metal    ), MZNT_RHI_ARG_PASS_##tyArg5 (ty5, arg5, Metal    )) \
        MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret) \
    }

#define MZNT_RHI_SWITCH_SIX_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4, ty5, arg5, tyArg5, ty6, arg6, tyArg6) \
    switch (tyQ) \
    { \
        MZNT_RHI_FN_NULL_BRANCH(ret) \
        MZNT_RHI_FN_VK_BRANCH   (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg5 (ty5, arg5, Vulkan   ), MZNT_RHI_ARG_PASS_##tyArg6 (ty6, arg6, Vulkan   )) \
        MZNT_RHI_FN_DX12_BRANCH (tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, DirectX12), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, DirectX12), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, DirectX12), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, DirectX12), MZNT_RHI_ARG_PASS_##tyArg5 (ty5, arg5, DirectX12), MZNT_RHI_ARG_PASS_##tyArg6 (ty6, arg6, DirectX12)) \
        MZNT_RHI_FN_METAL_BRANCH(tyRet, fn, MZNT_RHI_ARG_PASS_##tyArg1 (ty1, arg1, Metal    ), MZNT_RHI_ARG_PASS_##tyArg2 (ty2, arg2, Metal    ), MZNT_RHI_ARG_PASS_##tyArg3 (ty3, arg3, Metal    ), MZNT_RHI_ARG_PASS_##tyArg4 (ty4, arg4, Metal    ), MZNT_RHI_ARG_PASS_##tyArg5 (ty5, arg5, Metal    ), MZNT_RHI_ARG_PASS_##tyArg6 (ty6, arg6, Metal    )) \
        MZNT_RHI_FN_UNSUPPORTED_BRANCH(ret) \
    }

#define MZNT_RHI_FN_ONE_ARG(fn, tyQ, ret, tyRet, ty1, arg1, tyArg1) \
    ret MZNT_##fn( MZNT_RHI_ARG_DECL_##tyArg1 (ty1) arg1 ) \
    { \
        MZNT_RHI_ARG_CHECK_##tyArg1(tyQ, arg1); \
        MZNT_RHI_SWITCH_ONE_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1) \
    }

#define MZNT_RHI_FN_TWO_ARG(fn, tyQ, ret, tyRet, ty1, arg1, tyArg1, ty2, arg2, tyArg2) \
    ret MZNT_##fn( MZNT_RHI_ARG_DECL_##tyArg1 (ty1) arg1, MZNT_RHI_ARG_DECL_##tyArg2 (ty2) arg2 ) \
    { \
        MZNT_RHI_ARG_CHECK_##tyArg1(tyQ, arg1); \
        MZNT_RHI_ARG_CHECK_##tyArg2(tyQ, arg2); \
        MZNT_RHI_SWITCH_TWO_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2) \
    }

#define MZNT_RHI_FN_THREE_ARG(fn, tyQ, ret, tyRet, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3) \
    ret MZNT_##fn( MZNT_RHI_ARG_DECL_##tyArg1 (ty1) arg1, MZNT_RHI_ARG_DECL_##tyArg2 (ty2) arg2, MZNT_RHI_ARG_DECL_##tyArg3 (ty3) arg3 ) \
    { \
        MZNT_RHI_ARG_CHECK_##tyArg1(tyQ, arg1); \
        MZNT_RHI_ARG_CHECK_##tyArg2(tyQ, arg2); \
        MZNT_RHI_ARG_CHECK_##tyArg3(tyQ, arg3); \
        MZNT_RHI_SWITCH_THREE_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3) \
    }

#define MZNT_RHI_FN_FOUR_ARG(fn, tyQ, ret, tyRet, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4) \
    ret MZNT_##fn( MZNT_RHI_ARG_DECL_##tyArg1 (ty1) arg1, MZNT_RHI_ARG_DECL_##tyArg2 (ty2) arg2, MZNT_RHI_ARG_DECL_##tyArg3 (ty3) arg3, MZNT_RHI_ARG_DECL_##tyArg4 (ty4) arg4 ) \
    { \
        MZNT_RHI_ARG_CHECK_##tyArg1(tyQ, arg1); \
        MZNT_RHI_ARG_CHECK_##tyArg2(tyQ, arg2); \
        MZNT_RHI_ARG_CHECK_##tyArg3(tyQ, arg3); \
        MZNT_RHI_ARG_CHECK_##tyArg4(tyQ, arg4); \
        MZNT_RHI_SWITCH_FOUR_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4) \
    }

#define MZNT_RHI_FN_FIVE_ARG(fn, tyQ, ret, tyRet, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4, ty5, arg5, tyArg5) \
    ret MZNT_##fn( MZNT_RHI_ARG_DECL_##tyArg1 (ty1) arg1, MZNT_RHI_ARG_DECL_##tyArg2 (ty2) arg2, MZNT_RHI_ARG_DECL_##tyArg3 (ty3) arg3, MZNT_RHI_ARG_DECL_##tyArg4 (ty4) arg4, MZNT_RHI_ARG_DECL_##tyArg5 (ty5) arg5 ) \
    { \
        MZNT_RHI_ARG_CHECK_##tyArg1(tyQ, arg1); \
        MZNT_RHI_ARG_CHECK_##tyArg2(tyQ, arg2); \
        MZNT_RHI_ARG_CHECK_##tyArg3(tyQ, arg3); \
        MZNT_RHI_ARG_CHECK_##tyArg4(tyQ, arg4); \
        MZNT_RHI_ARG_CHECK_##tyArg5(tyQ, arg5); \
        MZNT_RHI_SWITCH_FIVE_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4, ty5, arg5, tyArg5) \
    }

#define MZNT_RHI_FN_SIX_ARG(fn, tyQ, ret, tyRet, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4, ty5, arg5, tyArg5, ty6, arg6, tyArg6) \
    ret MZNT_##fn( MZNT_RHI_ARG_DECL_##tyArg1 (ty1) arg1, MZNT_RHI_ARG_DECL_##tyArg2 (ty2) arg2, MZNT_RHI_ARG_DECL_##tyArg3 (ty3) arg3, MZNT_RHI_ARG_DECL_##tyArg4 (ty4) arg4, MZNT_RHI_ARG_DECL_##tyArg5 (ty5) arg5, MZNT_RHI_ARG_DECL_##tyArg6 (ty6) arg6 ) \
    { \
        MZNT_RHI_ARG_CHECK_##tyArg1(tyQ, arg1); \
        MZNT_RHI_ARG_CHECK_##tyArg2(tyQ, arg2); \
        MZNT_RHI_ARG_CHECK_##tyArg3(tyQ, arg3); \
        MZNT_RHI_ARG_CHECK_##tyArg4(tyQ, arg4); \
        MZNT_RHI_ARG_CHECK_##tyArg5(tyQ, arg5); \
        MZNT_RHI_ARG_CHECK_##tyArg6(tyQ, arg6); \
        MZNT_RHI_SWITCH_SIX_ARG(ret, tyRet, fn, tyQ, ty1, arg1, tyArg1, ty2, arg2, tyArg2, ty3, arg3, tyArg3, ty4, arg4, tyArg4, ty5, arg5, tyArg5, ty6, arg6, tyArg6) \
    }
