#pragma once
#include <__init.h>

EXTERN_C_BEGIN

#if MSR_MSVC && MSR_DBG
    #define MSR_ASSERT(expr) do { if (!(expr)) {   __debugbreak(); } } while (0)
#elif (MSR_CLANG || MSR_GCC) && MSR_DBG
    #define MSR_ASSERT(expr) do { if (!(expr)) { __builtin_trap(); } } while (0)
#else
    #define MSR_ASSERT(expr) ((void) 0)
#endif

/**
 * Yield the processor to allow other threads to run.
 * This is a hint to the scheduler that the current thread is willing to yield its remaining time slice.
 */
void MSR_YieldProcessor(void);

EXTERN_C_END
