#pragma once
#include <DirectX12.h>

MSR_SUPPRESS_WARN
#if REN_DX12
    #ifdef __cplusplus
        #define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
        #include "ExtDeps/D3D12MA/D3D12MemAlloc.h"
        #undef D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
    #endif
#endif
MSR_UNSUPPRESS_WARN
