#pragma once
#include <__init.h>
#include <ExtDeps_Platform.h>

#ifdef REN_DX12
    #error "REN_DX12 is already defined"
#endif

#define REN_DX12 (MSR_WINDOWS || MSR_XSERIES)

MSR_SUPPRESS_WARN
#if REN_DX12
    #ifndef __cplusplus
        #define CINTERFACE
        #define COBJMACROS
    #endif

    #include <combaseapi.h>
    #include "ExtDeps/d3d12/d3d12.h"
    #include <dxgi1_6.h>
    #include <d3dcompiler.h>
    #if MSR_DBG
        #include <dxgidebug.h>
    #endif

    #undef COBJMACROS
    #undef CINTERFACE

    #ifdef __cplusplus
        #define D3DX12_NO_STATE_OBJECT_HELPERS
        #define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
        #include "ExtDeps/d3d12/d3dx12/d3dx12.h"
        #undef D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
        #undef D3DX12_NO_STATE_OBJECT_HELPERS
    #endif
#endif
MSR_UNSUPPRESS_WARN
