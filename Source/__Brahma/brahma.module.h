#ifndef BRAHMA_EXEC
/**
 * `BRAHMA_EXEC` can be defined to `1` to generate the build tool in executable mode.
 * This mode includes a `main` entry point that can be executed from a CLI.
 */
#define BRAHMA_EXEC 0
#endif

#ifndef BRAHMA_LIB
/**
 * `BRAHMA_LIB` can be defined to `1` to include Brahma utility functions inside your own
 * codebase, and be able to use it to build apps that Brahma does.
 */
#define BRAHMA_LIB 0
#endif

#ifndef BRAHMA_CLANG
    #if defined(__clang__)
        #define BRAHMA_CLANG 1
    #else
        #define BRAHMA_CLANG 0
    #endif
#endif

#ifndef BRAHMA_GCC
    #if defined(__GNUC__)
        #define BRAHMA_GCC 1
    #else
        #define BRAHMA_GCC 0
    #endif
#endif

#ifndef BRAHMA_MSVC
    #if defined(_MSC_VER)
        #define BRAHMA_MSVC 1
    #else
        #define BRAHMA_MSVC 0
    #endif
#endif

#if BRAHMA_EXEC

#if BRAHMA_MSVC
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #pragma warning(disable: 5045) // spectre mitigation
    #pragma warning(disable: 4324) // structure was padded due to alignment specifier
    #pragma warning(disable: 4820) // bytes padding added after data member
    #pragma warning(disable: 4127) // conditional expression is constant
    #pragma warning(disable: 4514) // unreferenced inline function has been removed
    #pragma warning(disable: 4710) // function not inlined
    #pragma warning(disable: 4711) // function selected for automatic inline expansion
    #pragma warning(disable: 4464) // relative include path contains '..'
    #pragma warning(disable: 5038) // data member will be initialized after base class
    #pragma warning(disable: 4577) // 'noexcept' used with no exception handling mode specified

    #define BRAHMA_SUPPRESS_WARN \
        __pragma(warning(push, 0))

    #define BRAHMA_UNSUPPRESS_WARN \
        __pragma(warning(pop))
#endif

#if BRAHMA_GCC
    #pragma GCC diagnostic error   "-Wall"
    #pragma GCC diagnostic error   "-Wextra"
    #pragma GCC diagnostic error   "-Wshadow"
    #pragma GCC diagnostic error   "-Wconversion"
    #pragma GCC diagnostic error   "-Wsign-conversion"
    #pragma GCC diagnostic error   "-Wdouble-promotion"
    #pragma GCC diagnostic error   "-Wfloat-equal"
    #pragma GCC diagnostic error   "-Wundef"
    #pragma GCC diagnostic error   "-Wswitch-enum"
    #pragma GCC diagnostic error   "-Wstrict-prototypes"
    #pragma GCC diagnostic ignored "-Wunused-parameter"

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("GCC diagnostic push")  \
        _Pragma("GCC diagnostic ignored \"-Weverything\"")

    #define BRAHMA_UNSUPPRESS_WARN \
        _Pragma("GCC diagnostic pop")
#endif

#if BRAHMA_CLANG
    #pragma clang diagnostic error   "-Wall"
    #pragma clang diagnostic error   "-Wextra"
    #pragma clang diagnostic error   "-Wshadow"
    #pragma clang diagnostic error   "-Wconversion"
    #pragma clang diagnostic error   "-Wsign-conversion"
    #pragma clang diagnostic error   "-Wdouble-promotion"
    #pragma clang diagnostic error   "-Wfloat-equal"
    #pragma clang diagnostic error   "-Wundef"
    #pragma clang diagnostic error   "-Wswitch-enum"
    #pragma clang diagnostic error   "-Wstrict-prototypes"
    #pragma clang diagnostic ignored "-Wunused-parameter"

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("clang diagnostic push")  \
        _Pragma("clang diagnostic ignored \"-Weverything\"")

    #define BRAHMA_UNSUPPRESS_WARN \
        _Pragma("clang diagnostic pop")
#endif

BRAHMA_SUPPRESS_WARN
#include <stdio.h>
BRAHMA_UNSUPPRESS_WARN

int main(int argc, char* argv[])
{
    printf("Hello, world!");
    return 0;
}

#endif
