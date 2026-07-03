#include <__init.h>

#if MSR_BUILD_DEPS
MSR_SUPPRESS_WARN
#define STB_IMAGE_IMPLEMENTATION
#include <ExtDeps/stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
MSR_UNSUPPRESS_WARN
#endif
