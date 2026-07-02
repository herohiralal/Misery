#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN
#define SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#define SPV_ENABLE_UTILITY_CODE

#ifdef __cplusplus
    #include "ExtDeps/GLSL.std.450.h"
    #include "ExtDeps/spirv_common.hpp"
    #include "ExtDeps/spirv_cross_error_handling.hpp"
    #include "ExtDeps/spirv.hpp"
    #include "ExtDeps/spirv_cross.hpp"
    #include "ExtDeps/spirv_parser.hpp"
    #include "ExtDeps/spirv_cross_parsed_ir.hpp"
    #include "ExtDeps/spirv_cfg.hpp"
    #include "ExtDeps/spirv_glsl.hpp"
    #include "ExtDeps/spirv_msl.hpp"
    #include "ExtDeps/spirv_hlsl.hpp"
    #include "ExtDeps/spirv_reflect.hpp"
    #include "ExtDeps/spirv_cross_util.hpp"
#endif

    #include "ExtDeps/spirv.h"
    #include "ExtDeps/spirv_cross_c.h"
MSR_UNSUPPRESS_WARN
