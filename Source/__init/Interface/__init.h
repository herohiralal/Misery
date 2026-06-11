#pragma once

// compilers -------------------------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
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

    #define MSR_SUPPRESS_WARN \
        __pragma(warning(push, 0))

    #define MSR_UNSUPPRESS_WARN \
        __pragma(warning(pop))

    #define MSR_MSVC 1
#endif

#ifdef __GNUC__
    #pragma GCC diagnostic error   "-Wall"
    #pragma GCC diagnostic error   "-Wextra"
    #pragma GCC diagnostic error   "-Wshadow"
    #pragma GCC diagnostic error   "-Wconversion"
    #pragma GCC diagnostic error   "-Wdouble-promotion"
    #pragma GCC diagnostic error   "-Wfloat-equal"
    #pragma GCC diagnostic error   "-Wundef"
    #pragma GCC diagnostic error   "-Wswitch-enum"
    #ifndef __cplusplus
        #pragma GCC diagnostic error   "-Wstrict-prototypes"
    #endif
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wunused-result"
    #pragma GCC diagnostic ignored "-Wuninitialized"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
    #pragma GCC diagnostic ignored "-Wmissing-braces"
    #pragma GCC diagnostic ignored "-Wdouble-promotion"

    #define MSR_SUPPRESS_WARN \
        _Pragma("GCC diagnostic push")  \
        _Pragma("GCC diagnostic ignored \"-Wall\"") \
        _Pragma("GCC diagnostic ignored \"-Wextra\"") \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \

    #define MSR_UNSUPPRESS_WARN \
        _Pragma("GCC diagnostic pop")

    #define MSR_GCC 1
#endif

#if defined(__clang__) && !defined(_MSC_VER) && !defined(__GNUC__)
    #pragma clang diagnostic error   "-Wall"
    #pragma clang diagnostic error   "-Wextra"
    #pragma clang diagnostic error   "-Wshadow"
    #pragma clang diagnostic error   "-Wconversion"
    #pragma clang diagnostic error   "-Wdouble-promotion"
    #pragma clang diagnostic error   "-Wfloat-equal"
    #pragma clang diagnostic error   "-Wundef"
    #pragma clang diagnostic error   "-Wswitch-enum"
    #ifndef __cplusplus
        #pragma clang diagnostic error   "-Wstrict-prototypes"
    #endif
    #pragma clang diagnostic ignored "-Wunused-parameter"
    #pragma clang diagnostic ignored "-Wunused-result"
    #pragma clang diagnostic ignored "-Wuninitialized"
    #pragma clang diagnostic ignored "-Wsign-conversion"
    #pragma clang diagnostic ignored "-Wmissing-braces"
    #pragma clang diagnostic ignored "-Wdouble-promotion"

    #define MSR_SUPPRESS_WARN \
        _Pragma("clang diagnostic push")  \
        _Pragma("clang diagnostic ignored \"-Weverything\"")

    #define MSR_UNSUPPRESS_WARN \
        _Pragma("clang diagnostic pop")

    #define MSR_CLANG 1
#endif

#ifndef MSR_MSVC
    #define MSR_MSVC 0
#endif
#ifndef MSR_GCC
    #define MSR_GCC 0
#endif
#ifndef MSR_CLANG
    #define MSR_CLANG 0
#endif

// macros ----------------------------------------------------------------------------------------------------------------------

#if MSR_MSVC

    #define MSR_NOINLINE                    __declspec(noinline)
    #define MSR_FORCEINLINE                 __forceinline
    #define MSR_NORETURN                    __declspec(noreturn)
    // offsetof impl depends on c/c++

#elif (MSR_CLANG || MSR_GCC)

    #define MSR_NOINLINE                    __attribute__((noinline))
    #define MSR_FORCEINLINE                 inline __attribute__((always_inline))
    #define MSR_NORETURN                    __attribute__((noreturn))
    #define MSR_OFFSETOF(type, member)      __builtin_offsetof(type, member)

#else
    #error "Required features not supported by this compiler."
#endif

#ifdef __cplusplus

    // used as-is:
    // - thread_local
    // - inline
    // - alignas
    // - alignof

    #define MSR_DEPRECATED                  [[deprecated]]
    #define MSR_TYPEOF(ty)                  decltype(ty)

    #if MSR_MSVC
        #define MSR_OFFSETOF(type, member)  ((u64)&reinterpret_cast<char const volatile&>((((type*)0)->member)))
    #endif

    // static_assert is used as-is

#else

    #if MSR_MSVC

        #define thread_local                __declspec(thread)
        #define inline                      __inline
        #define alignas(x)                  __declspec(align(x))
        #define alignof(type)               __alignof(type)
        #define MSR_DEPRECATED              __declspec(deprecated)
        #define MSR_OFFSETOF(type, member)  ((unsigned __int64)&(((type*)0)->member))

    #elif (MSR_CLANG || MSR_GCC)

        #define thread_local                __thread
        #define inline                      __inline__
        #define alignas(x)                  __attribute__((aligned(x)))
        #define alignof(type)               __alignof__(type)
        #define MSR_DEPRECATED              __attribute__((deprecated))
        // offsetof declared previously

    #else
        #error "Required features not supported by this compiler."
    #endif

    #define static_assert                   _Static_assert
    #define MSR_TYPEOF(ty)                  __typeof__(ty)

#endif

#ifdef __cplusplus
    #define EXTERN_C_BEGIN extern "C" {
    #define EXTERN_C_END   }
#else
    #define EXTERN_C_BEGIN
    #define EXTERN_C_END
#endif

#ifdef __cplusplus
    #define OPT_ARG = { }
#else
    #define OPT_ARG
#endif

#define COUNTVARARGS_INTERNAL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, COUNT, ...) COUNT
#define COUNT_VARARGS(...) COUNTVARARGS_INTERNAL(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#ifdef __cplusplus
    #define MSR_TY_INITIALISER(ty) ty
#else
    #define MSR_TY_INITIALISER(ty) (ty)
#endif

static_assert(MSR_MSVC + MSR_GCC + MSR_CLANG == 1, "Exactly one compiler must be defined.");

// configurations --------------------------------------------------------------------------------------------------------------

#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
    #define MSR_DBG 1
#else
    #define MSR_REL 1
#endif

#ifndef MSR_DBG
    #define MSR_DBG 0
#endif
#ifndef MSR_REL
    #define MSR_REL 0
#endif

#if MSR_MSVC && MSR_DBG
    #define MSR_ASSERT(expr) do { if (!(expr)) {   __debugbreak(); } } while (0)
#elif (MSR_CLANG || MSR_GCC) && MSR_DBG
    #define MSR_ASSERT(expr) do { if (!(expr)) { __builtin_trap(); } } while (0)
#else
    #define MSR_ASSERT(expr) ((void) 0)
#endif

static_assert(MSR_DBG + MSR_REL == 1, "Exactly one configuration must be defined.");

enum MSR_Configurations
{
    MSR_CFG_Unknown,
    MSR_CFG_Debug,
    MSR_CFG_Release,
};

typedef unsigned char MSR_Configuration;

static const MSR_Configuration MSR_CONFIGURATION =
    MSR_DBG ? MSR_CFG_Debug   :
    MSR_REL ? MSR_CFG_Release :
              MSR_CFG_Unknown;

// platforms -------------------------------------------------------------------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64)
    #define MSR_WINDOWS 1
#elif defined(__APPLE__)
    MSR_SUPPRESS_WARN
        #include <TargetConditionals.h>
    MSR_UNSUPPRESS_WARN

    #if TARGET_OS_IPHONE
        #define MSR_IOS 1
    #else
        #define MSR_OSX 1
    #endif
#elif defined(__linux__)
    #if defined(__ANDROID__)
        #define MSR_ANDROID 1
    #else
        #define MSR_LINUX 1
    #endif
#endif

#ifndef MSR_WINDOWS
    #define MSR_WINDOWS 0
#endif
#ifndef MSR_LINUX
    #define MSR_LINUX 0
#endif
#ifndef MSR_OSX
    #define MSR_OSX 0
#endif
#ifndef MSR_ANDROID
    #define MSR_ANDROID 0
#endif
#ifndef MSR_IOS
    #define MSR_IOS 0
#endif
#ifndef MSR_PS5
    #define MSR_PS5 0
#endif
#ifndef MSR_XSERIES
    #define MSR_XSERIES 0
#endif
#ifndef MSR_SWITCH
    #define MSR_SWITCH 0
#endif

#define MSR_DESKTOP (MSR_WINDOWS || MSR_LINUX || MSR_OSX)
#define MSR_MOBILE (MSR_ANDROID || MSR_IOS)
#define MSR_UNIX (MSR_LINUX || MSR_OSX || MSR_ANDROID || MSR_IOS)
#define MSR_APPLE (MSR_OSX || MSR_IOS)

static_assert(MSR_WINDOWS + MSR_LINUX + MSR_OSX + MSR_ANDROID + MSR_IOS + MSR_PS5 + MSR_XSERIES + MSR_SWITCH == 1, "Exactly one platform must be defined.");

enum MSR_Platforms
{
    MSR_PLT_Unknown,
    MSR_PLT_Windows,
    MSR_PLT_Linux,
    MSR_PLT_OSX,
    MSR_PLT_Android,
    MSR_PLT_iOS,
    MSR_PLT_PS5,
    MSR_PLT_XboxSeries,
    MSR_PLT_Switch,
};

typedef unsigned char MSR_Platform;

static const MSR_Platform MSR_PLATFORM =
    MSR_WINDOWS ? MSR_PLT_Windows    :
    MSR_LINUX   ? MSR_PLT_Linux      :
    MSR_OSX     ? MSR_PLT_OSX        :
    MSR_ANDROID ? MSR_PLT_Android    :
    MSR_IOS     ? MSR_PLT_iOS        :
    MSR_PS5     ? MSR_PLT_PS5        :
    MSR_XSERIES ? MSR_PLT_XboxSeries :
    MSR_SWITCH  ? MSR_PLT_Switch     :
                  MSR_PLT_Unknown;

// architectures ---------------------------------------------------------------------------------------------------------------

#if defined(__x86_64__) || defined(_M_X64)
    #define MSR_X64 1
#elif defined(__i386__) || defined(_M_IX86)
    #define MSR_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define MSR_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
    #define MSR_ARM 1
#endif

#ifndef MSR_X64
    #define MSR_X64 0
#endif
#ifndef MSR_X86
    #define MSR_X86 0
#endif
#ifndef MSR_ARM64
    #define MSR_ARM64 0
#endif
#ifndef MSR_ARM
    #define MSR_ARM 0
#endif

static_assert(MSR_X64 + MSR_X86 + MSR_ARM64 + MSR_ARM == 1, "Exactly one architecture must be defined.");

enum MSR_Architectures
{
    MSR_ACH_Unknown,
    MSR_ACH_x64,
    MSR_ACH_x86,
    MSR_ACH_ARM64,
    MSR_ACH_ARM,
};

typedef unsigned char MSR_Architecture;

static const MSR_Architecture MSR_ARCHITECTURE =
    MSR_X64   ? MSR_ACH_x64   :
    MSR_X86   ? MSR_ACH_x86   :
    MSR_ARM64 ? MSR_ACH_ARM64 :
    MSR_ARM   ? MSR_ACH_ARM   :
                MSR_ACH_Unknown;

// primitives  -----------------------------------------------------------------------------------------------------------------

#ifndef __cplusplus
    typedef _Bool           bool;
#endif

typedef bool                b8;
typedef unsigned char       u8;
typedef unsigned short int  u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         i8;
typedef signed short int    i16;
typedef signed int          i32;
typedef signed long long    i64;
typedef float               f32;
typedef double              f64;
typedef void*               rawptr;
typedef const char*         cstring;

#ifdef __cplusplus
    #undef  nil

    #define nil   (   nullptr)
#else

    #undef  nil
    #undef  false
    #undef  true

    #define nil   ((rawptr) 0)
    #define false ((b8)     0)
    #define true  ((b8)     1)

#endif

#define U8_MIN  ((u8)  (0))
#define U8_MAX  ((u8)  (255))
#define U16_MIN ((u16) (0))
#define U16_MAX ((u16) (65535))
#define U32_MIN ((u32) (0))
#define U32_MAX ((u32) (4294967295U))
#define U64_MIN ((u64) (0))
#define U64_MAX ((u64) (18446744073709551615ULL))
#define I8_MIN  ((i8)  (-128))
#define I8_MAX  ((i8)  (127))
#define I16_MIN ((i16) ((-32768)))
#define I16_MAX ((i16) (32767))
#define I32_MIN ((i32) ((-2147483647 - 1)))
#define I32_MAX ((i32) (2147483647))
#define I64_MIN ((i64) ((-9223372036854775807LL - 1)))
#define I64_MAX ((i64) (9223372036854775807LL))
#define F32_MIN ((f32) ((-3.402823466e+38F)))
#define F32_MAX ((f32) (3.402823466e+38F))
#define F64_MIN ((f64) ((-1.7976931348623157e+308)))
#define F64_MAX ((f64) (1.7976931348623157e+308))

#if MSR_X64 || MSR_ARM64

    #define MSR_PTR_SIZE 8

    #define ISIZE_MAX I64_MAX
    #define ISIZE_MIN I64_MIN
    #define USIZE_MAX U64_MAX
    #define USIZE_MIN U64_MIN

    typedef u64 usize;
    typedef i64 isize;

#elif MSR_X86 || MSR_ARM

    #define MSR_PTR_SIZE 4

    #define ISIZE_MAX I32_MAX
    #define ISIZE_MIN I32_MIN
    #define USIZE_MAX U32_MAX
    #define USIZE_MIN U32_MIN

    typedef u32 usize;
    typedef i32 isize;

#else
    #error "Unknown architecture. Cannot define usize and isize."
#endif

static_assert(sizeof(b8)  == 1, " b8 must be 1 byte ");
static_assert(sizeof(u8)  == 1, " u8 must be 1 byte ");
static_assert(sizeof(i8)  == 1, " i8 must be 1 byte ");
static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
static_assert(sizeof(i16) == 2, "i16 must be 2 bytes");
static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
static_assert(sizeof(i32) == 4, "i32 must be 4 bytes");
static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");
static_assert(sizeof(i64) == 8, "i64 must be 8 bytes");
static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");

// src loc ---------------------------------------------------------------------------------------------------------------------

/**
 * Defines the source code location for debugging purposes.
 * Primarily used for logging/reporting the location where a call might have been made from.
 * General-purpose.
 */
typedef struct SrcLoc
{
    const char* file;
    isize       fileLen;
    i32         line;
    i32         column;
    const char* function;
    isize       functionLen;
} SrcLoc;

/**
 * Helper macro to get the current source code location. Used with functions that take a SrcLoc
 * parameter, so that the caller doesn't have to manually specify the file and line number every time.
 */
#define SRC_LOC() \
    (MSR_TY_INITIALISER(SrcLoc) \
    { \
        __FILE__, \
        (isize) (sizeof(__FILE__) - 1), \
        __LINE__, \
        0, \
        __FUNCTION__, \
        (isize) (sizeof(__FUNCTION__) - 1), \
    })

// includes --------------------------------------------------------------------------------------------------------------------

MSR_SUPPRESS_WARN

#if MSR_WINDOWS
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "Ws2_32.lib")
    #pragma comment(lib, "User32.lib")
    #pragma comment(lib, "Shell32.lib")
    #pragma comment(lib, "Gdi32.lib")
#endif

#if MSR_WINDOWS
    #define _CRT_SECURE_NO_WARNINGS
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <WinSock2.h>
    #include <ws2ipdef.h>
    #include <iphlpapi.h>
    #include <intrin.h>
    #include <malloc.h>
    #include <stdio.h>
    #include <shellapi.h>
    #include <hidusage.h>
#endif

#if MSR_UNIX

    // since we're on C11
    #if MSR_APPLE
        #ifndef _DARWIN_C_SOURCE
            #define _DARWIN_C_SOURCE
        #endif
    #else
        #ifndef _GNU_SOURCE
            #define _GNU_SOURCE
        #endif
        #ifndef _POSIX_C_SOURCE
            #define _POSIX_C_SOURCE 200809L
        #endif
        #ifndef _XOPEN_SOURCE
            #define _XOPEN_SOURCE 700
        #endif
    #endif

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <ifaddrs.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/stat.h>
    #include <sys/wait.h>
    #include <sys/mman.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <errno.h>
    #include <dirent.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <dlfcn.h>
#endif

#if MSR_APPLE
    extern char** environ;

    #include <mach/mach.h>
    #include <mach/mach_time.h>
    #include <TargetConditionals.h>
    #include <signal.h>
    #include <dispatch/dispatch.h>
    #include <os/log.h>
    #include <Foundation/Foundation.h>

    #if MSR_OSX
        #include <Cocoa/Cocoa.h>
    #elif MSR_IOS
        #include <UIKit/UIKit.h>
    #endif

    #include <Metal/Metal.h>
#endif

#if MSR_ANDROID
    #include <jni.h>
    #include <android/log.h>
    #include <android/asset_manager.h>
    #include <android/native_activity.h>
    #include <android_native_app_glue.h>
    #include <android/window.h>
#endif

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef __cplusplus
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <memory>
#endif

MSR_UNSUPPRESS_WARN

// defer -----------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
namespace DeferInternals
{
    struct Helper
    {
        template <typename TCallable>
        struct Defer
        {
            TCallable func;
            ~Defer() { func(); }
        };

        template <typename TCallable>
        Defer<TCallable> operator+(TCallable&& func)
        {
            return Defer<TCallable>{.func = std::forward<TCallable>(func)};
        }
    };
}

#define DEFER_CONCAT_IMPL(x, y) x##y
#define DEFER_CONCAT(x, y) DEFER_CONCAT_IMPL(x, y)

/**
 * Helper macro similar to `defer` statements in modern languages.
 * Supposed to be used as:
 * ```
 * int main()
 * {
 *     DEFER {
 *         printf("will be printed at the end");
 *     };
 * }
 * ```
 */
#define DEFER auto DEFER_CONCAT(defer_, __COUNTER__) = DeferInternals::Helper() + [&]()
#endif
