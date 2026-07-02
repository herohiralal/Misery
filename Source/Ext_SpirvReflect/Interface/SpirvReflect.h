#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN
#define SPV_ENABLE_UTILITY_CODE
#define SPIRV_REFLECT_DISABLE_CPP_BINDINGS
#include "ExtDeps/spirv_reflect.h"
MSR_UNSUPPRESS_WARN
