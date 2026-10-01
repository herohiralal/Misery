#include "CorePrivate.h"

void MSR_YieldProcessor(void)
{
    #if MSR_MSVC && (MSR_X64 || MSR_X86)
        _mm_pause();
    #elif MSR_MSVC && (MSR_ARM64 || MSR_ARM)
        __yield();
    #elif (MSR_CLANG || MSR_GCC) && (MSR_X64 || MSR_X86)
        __builtin_ia32_pause();
    #elif (MSR_CLANG || MSR_GCC) && (MSR_ARM64 || MSR_ARM)
        __builtin_arm_yield();
    #else
        #error "Unsupported compiler or architecture for MSR_YieldProcessor."
    #endif
}
