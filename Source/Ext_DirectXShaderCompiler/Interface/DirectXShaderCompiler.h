#pragma once
#include <__init.h>

#if MSR_WINDOWS || MSR_LINUX
    #ifdef __cplusplus
        #include "Unknwn.h"
        #include "ExtDeps/dxc/dxcapi.h"
        #define DX_SHADER_COMPILER 1
    #endif
#endif

#ifndef DX_SHADER_COMPILER
    #define DX_SHADER_COMPILER 0
#endif
