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
    #pragma GCC diagnostic error   "-Wsign-conversion"
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

    #define MSR_SUPPRESS_WARN \
        _Pragma("GCC diagnostic push")  \
        _Pragma("GCC diagnostic ignored \"-Wall\"") \
        _Pragma("GCC diagnostic ignored \"-Wextra\"") \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \

    #define MSR_UNSUPPRESS_WARN \
        _Pragma("GCC diagnostic pop")

    #define MSR_GCC 1
#endif

#ifdef __clang__
    #pragma clang diagnostic error   "-Wall"
    #pragma clang diagnostic error   "-Wextra"
    #pragma clang diagnostic error   "-Wshadow"
    #pragma clang diagnostic error   "-Wconversion"
    #pragma clang diagnostic error   "-Wsign-conversion"
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
    #define MSR_ASSERT(expr) do { if (!(expr)) { __debugbreak(); } } while (0)
#elif (MSR_CLANG || MSR_GCC) && MSR_DBG
    #define MSR_ASSERT(expr) do { if (!(expr)) { __builtin_trap(); } } while (0)
#else
    #define MSR_ASSERT(expr) ((void)0)
#endif

static_assert(MSR_DBG + MSR_REL == 1, "Exactly one configuration must be defined.");

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

#define MSR_UNIX (MSR_LINUX || MSR_OSX || MSR_ANDROID || MSR_IOS)

static_assert(MSR_WINDOWS + MSR_LINUX + MSR_OSX + MSR_ANDROID + MSR_IOS + MSR_PS5 + MSR_XSERIES + MSR_SWITCH == 1, "Exactly one platform must be defined.");

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

#if MSR_LINUX || MSR_ANDROID
    #define _GNU_SOURCE
#endif

#if MSR_UNIX

    // since we're on C11
    #if MSR_APPLE
        #define _DARWIN_C_SOURCE
    #else
        #define _POSIX_C_SOURCE 200809L
        #define _XOPEN_SOURCE 700
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
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <assert.h>
#include <memory>

MSR_UNSUPPRESS_WARN
