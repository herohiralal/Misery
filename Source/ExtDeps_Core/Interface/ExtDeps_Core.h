#pragma once
#include <__init.h>

MSR_SUPPRESS_WARN

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

#if MSR_LINUX
    #include <sys/inotify.h>
#endif

#if MSR_APPLE
    extern char** environ;

    #include <mach/mach.h>
    #include <mach/mach_time.h>
    #include <TargetConditionals.h>
    #include <signal.h>
    #include <dispatch/dispatch.h>
    #include <os/log.h>
    #include <sys/event.h>
    #include <sys/time.h>
    #ifndef O_EVTONLY
        #define O_EVTONLY O_RDONLY
    #endif
#endif

#include <AppKitHeaders.h>

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

#include <RadDbgMarkup.h>

MSR_UNSUPPRESS_WARN
