#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN
#if MSR_WINDOWS || MSR_LINUX
    #ifdef __cplusplus
        #if MSR_WINDOWS
            #include "Unknwn.h"
        #endif
        #include "ExtDeps/dxc/dxcapi.h"
        #define DX_SHADER_COMPILER 1
    #endif
#endif
MSR_UNSUPPRESS_WARN

#ifndef DX_SHADER_COMPILER
    #define DX_SHADER_COMPILER 0
#endif
