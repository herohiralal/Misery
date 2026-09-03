#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN
#define TOBJ_NO_LIBC
#define TOBJ_ENABLE_MULTITHREADING
#define TOBJ_ENABLE_SIMD
#include "ExtDeps/tiny_obj_c.h"
#include "ExtDeps/tobj_tess.h"
#undef TOBJ_ENABLE_SIMD
#undef TOBJ_ENABLE_MULTITHREADING
#undef TOBJ_NO_LIBC
MSR_UNSUPPRESS_WARN

// if we're not building the dependencies, add our own context to override certain functionalities

#if !MSR_BUILD_DEPS

    #include <Core/Core.h>

    EXTERN_C_BEGIN

    EXTERN_C_END

#endif
