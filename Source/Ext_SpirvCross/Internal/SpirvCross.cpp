#include <__init.h>
#include <SpirvCross.h>

#if MSR_BUILD_DEPS
MSR_SUPPRESS_WARN
#define SPIRV_CROSS_C_API_GLSL 1
#define SPIRV_CROSS_C_API_HLSL 1
#define SPIRV_CROSS_C_API_MSL 1
#define SPIRV_CROSS_C_API_CPP 0
#define SPIRV_CROSS_C_API_REFLECT 1

#include "ExtDeps/spirv_cross.cpp"
#include "ExtDeps/spirv_parser.cpp"
#include "ExtDeps/spirv_cross_parsed_ir.cpp"
#include "ExtDeps/spirv_cfg.cpp"
#include "ExtDeps/spirv_glsl.cpp"
#include "ExtDeps/spirv_msl.cpp"
#include "ExtDeps/spirv_hlsl.cpp"
#include "ExtDeps/spirv_reflect.cpp"
#include "ExtDeps/spirv_cross_util.cpp"
#include "ExtDeps/spirv_cross_c.cpp"
MSR_UNSUPPRESS_WARN
#endif
