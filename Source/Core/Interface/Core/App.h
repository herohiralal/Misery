#pragma once
#include <__init.h>
#include "Strings.h"

EXTERN_C_BEGIN

/**
 * A cross-plataform opaque handle to the application instance.
 * On Windows, this is an HINSTANCE.
 * On OSX, this is an NSApplication*.
 * On Android, this is a struct android_app*.
 */
typedef struct
{
    usize handle;
} APP_Handle;

/**
 * The type of delegate for the main entry point of the application.
 * (If you choose to use it as so).
 */
typedef i32 (*APP_EntryPointProc)(APP_Handle app, Slice_(utf8str) args);

// internal fn; `main` will redirect here
i32 APP_Main(i32 argc, cstring* argv, APP_EntryPointProc mainFn, b8 isGui);

#if MSR_WINDOWS

    typedef int BOOL;
    struct HINSTANCE__;
    typedef struct HINSTANCE__ *HINSTANCE;
    typedef char CHAR;
    typedef CHAR *PSTR;
    typedef unsigned long DWORD;
    typedef void *LPVOID;

    static_assert( sizeof(APP_Handle) ==  sizeof(HINSTANCE), "app struct size mismatch");
    static_assert(alignof(APP_Handle) == alignof(HINSTANCE), "app struct alignment mismatch");

    static inline APP_Handle APP_ToHandle(HINSTANCE hInstance) { return *(APP_Handle*) &hInstance; }
    static inline HINSTANCE APP_FromHandle(APP_Handle app) { return *(HINSTANCE*) &app; }

    // internal fn; `WinMain` will redirect here
    i32 APP_WinMain(HINSTANCE hInstance, PSTR pCmdLinePtr, APP_EntryPointProc mainFn);

    #define APP_AS_DYNA_LIB_INTERNAL() \
        BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) \
        { \
            (void) hinstDLL; \
            (void) fdwReason; \
            (void) lpvReserved; \
            return 1; /* successful attach */ \
        }

    #define APP_AS_GUI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 APP_FnFwd(APP_Handle app, Slice_(utf8str) args) { return x(app, args); } \
        EXTERN_C_END \
        int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, PSTR pCmdLine, int nCmdShow) \
        { \
            (void) hPrev; \
            (void) nCmdShow; \
            return APP_WinMain( \
                hInst, \
                pCmdLine, \
                APP_FnFwd \
            ); \
        }

#elif MSR_OSX || MSR_LINUX

    #if MSR_OSX
        // using `rawptr` instead of `NSApplication*`
        static_assert( sizeof(rawptr) ==  sizeof(APP_Handle), "app struct size mismatch");
        static_assert(alignof(rawptr) == alignof(APP_Handle), "app struct alignment mismatch");

        static inline APP_Handle APP_ToHandle(rawptr app) { return *(APP_Handle*) &app; }
        static inline rawptr APP_FromHandle(APP_Handle app) { return *(rawptr*) &app; }
    #elif MSR_LINUX
        // using usize stub, to aid compilation
        static_assert( sizeof(usize) ==  sizeof(APP_Handle), "app struct size mismatch");
        static_assert(alignof(usize) == alignof(APP_Handle), "app struct alignment mismatch");

        static inline APP_Handle APP_ToHandle(usize app) { return *(APP_Handle*) &app; }
        static inline usize APP_FromHandle(APP_Handle app) { return *(usize*) &app; }
    #endif

    #define APP_AS_DYNA_LIB_INTERNAL() \
        EXTERN_C_BEGIN \
        int main(int argc, char** argv) { return 0; } \
        EXTERN_C_END

    #define APP_AS_GUI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 APP_FnFwd(APP_Handle app, Slice_(utf8str) args) { return x(app, args); } \
        int main(int argc, char** argv) \
        { \
            APP_Main( \
                (i32) argc, \
                (cstring*) argv, \
                APP_FnFwd, \
                true \
            ); \
            return 0; \
        } \
        EXTERN_C_END

#elif MSR_ANDROID

    struct android_app;

    static_assert( sizeof(APP_Handle) ==  sizeof(struct android_app*), "app struct size mismatch");
    static_assert(alignof(APP_Handle) == alignof(struct android_app*), "app struct alignment mismatch");

    static inline APP_Handle APP_ToHandle(struct android_app* app) { return *(APP_Handle*) &app; }
    static inline struct android_app* APP_FromHandle(APP_Handle app) { return *(struct android_app**) &app; }

    // internal fn; `android_main` will redirect here
    void APP_AndroidMain(struct android_app* app, APP_EntryPointProc mainFn, b8 isGui);

    #define APP_AS_DYNA_LIB_INTERNAL() \
        EXTERN_C_BEGIN \
        void android_main(struct android_app* app) { return; } \
        EXTERN_C_END

    #define APP_AS_CLI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 APP_FnFwd(APP_Handle app, Slice_(utf8str) args) { return x(app, args); } \
        void android_main(struct android_app* app) \
        { \
            APP_AndroidMain(app, APP_FnFwd, false); \
        } \
        EXTERN_C_END

    #define APP_AS_GUI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 APP_FnFwd(APP_Handle app, Slice_(utf8str) args) { return x(app, args); } \
        void android_main(struct android_app* app) \
        { \
            APP_AndroidMain(app, APP_FnFwd, true); \
        } \
        EXTERN_C_END

#else

    #error "entry point unimplemented"

#endif

#if MSR_WINDOWS || MSR_LINUX || MSR_OSX

    #define APP_AS_CLI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 APP_FnFwd(APP_Handle app, Slice_(utf8str) args) { return x(app, args); } \
        int main(int argc, char** argv) \
        { \
            APP_Main( \
                (i32) argc, \
                (cstring*) argv, \
                APP_FnFwd, \
                false \
            ); \
            return 0; \
        } \
        EXTERN_C_END

#endif

#ifndef APP_AS_DYNA_LIB_INTERNAL
    #error "dyna lib entry point not defined for this platform"
#endif

#ifndef APP_AS_GUI_EXEC_INTERNAL
    #error "gui executable entry point not defined for this platform"
#endif

#ifndef APP_AS_CLI_EXEC_INTERNAL
    #error "cli executable entry point not defined for this platform"
#endif

/**
 * Declare an entry point as a dynamic library.
 * This will cause the application to declare an entry point function, so that the linkers
 * do not complain, but the function will not forward to any other functions.
 * The macro does not take any arguments.
 * Keep this macro outside an `extern "C"` block, if using C++.
 */
#define APP_AS_DYNA_LIB() \
    APP_AS_DYNA_LIB_INTERNAL()

/**
 * Declare an entry point as a CLI executable.
 * The provided function will be called with the application instance and the command-line arguments.
 * The macro takes one argument, which is the function to call for the entry point.
 * This function must match the signature of `APP_EntryPointProc`.
 * Keep this macro outside an `extern "C"` block, if using C++.
 */
#define APP_AS_CLI_EXEC(x) \
    APP_AS_CLI_EXEC_INTERNAL(x)

/**
 * Declare an entry point as a GUI executable.
 * The provided function will be called with the application instance and the command-line arguments.
 * The macro takes one argument, which is the function to call for the entry point.
 * This function must match the signature of `APP_EntryPointProc`.
 * Keep this macro outside an `extern "C"` block, if using C++.
 */
#define APP_AS_GUI_EXEC(x) \
    APP_AS_GUI_EXEC_INTERNAL(x)

EXTERN_C_END
