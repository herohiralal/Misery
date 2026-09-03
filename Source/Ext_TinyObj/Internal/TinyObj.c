#include <__init.h>
#include <TinyObj.h>

#if MSR_BUILD_DEPS
MSR_SUPPRESS_WARN
#define TOBJ_NO_LIBC
#define TOBJ_ENABLE_MULTITHREADING
#define TOBJ_ENABLE_SIMD
#include <ExtDeps/tiny_obj_c.c>
#include <ExtDeps/tobj_tess.c>
#undef TOBJ_ENABLE_SIMD
#undef TOBJ_ENABLE_MULTITHREADING
#undef TOBJ_NO_LIBC
MSR_UNSUPPRESS_WARN
#else

#endif
