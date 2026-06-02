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
} MAIN_App;

/**
 * The type of delegate for the main entry point of the application.
 * (If you choose to use it as so).
 */
typedef i32 (*MAIN_EntryPointProc)(MAIN_App app, Slice_(utf8str) args);

// internal fn; `main` will redirect here
i32 MAIN_Main(i32 argc, cstring* argv, MAIN_EntryPointProc mainFn, b8 isGui);

#if MSR_WINDOWS

    typedef int BOOL;
    struct HINSTANCE__;
    typedef struct HINSTANCE__ *HINSTANCE;
    typedef char CHAR;
    typedef CHAR *PSTR;
    typedef unsigned long DWORD;
    typedef void *LPVOID;

    static_assert( sizeof(MAIN_App) ==  sizeof(HINSTANCE), "app struct size mismatch");
    static_assert(alignof(MAIN_App) == alignof(HINSTANCE), "app struct alignment mismatch");

    static inline MAIN_App MAIN_ToApp(HINSTANCE hInstance) { return *(MAIN_App*) &hInstance; }
    static inline HINSTANCE MAIN_FromApp(MAIN_App app) { return *(HINSTANCE*) &app; }

    // internal fn; `WinMain` will redirect here
    i32 MAIN_WinMain(HINSTANCE hInstance, PSTR pCmdLinePtr, MAIN_EntryPointProc mainFn);

    #define MAIN_AS_DYNA_LIB_INTERNAL() \
        BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) \
        { \
            (void) hinstDLL; \
            (void) fdwReason; \
            (void) lpvReserved; \
            return 1; /* successful attach */ \
        }

    #define MAIN_AS_GUI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 MAIN_FnFwd(MAIN_App app, Slice_(utf8str) args) { return x(app, args); } \
        EXTERN_C_END \
        int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, PSTR pCmdLine, int nCmdShow) \
        { \
            (void) hPrev; \
            (void) nCmdShow; \
            return MAIN_WinMain( \
                hInst, \
                pCmdLine, \
                MAIN_FnFwd \
            ); \
        }

#elif MSR_OSX || MSR_LINUX

    #if MSR_OSX
        // using `rawptr` instead of `NSApplication*`
        static_assert( sizeof(rawptr) ==  sizeof(MAIN_App), "app struct size mismatch");
        static_assert(alignof(rawptr) == alignof(MAIN_App), "app struct alignment mismatch");

        static inline MAIN_App MAIN_ToApp(rawptr app) { return *(MAIN_App*) &app; }
        static inline rawptr MAIN_FromApp(MAIN_App app) { return *(rawptr*) &app; }
    #elif MSR_LINUX
        // using usize stub, to aid compilation
        static_assert( sizeof(usize) ==  sizeof(MAIN_App), "app struct size mismatch");
        static_assert(alignof(usize) == alignof(MAIN_App), "app struct alignment mismatch");

        static inline MAIN_App MAIN_ToApp(usize app) { return *(MAIN_App*) &app; }
        static inline usize MAIN_FromApp(MAIN_App app) { return *(usize*) &app; }
    #endif

    #define MAIN_AS_DYNA_LIB_INTERNAL() \
        EXTERN_C_BEGIN \
        int main(int argc, char** argv) { return 0; } \
        EXTERN_C_END

    #define MAIN_AS_GUI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 MAIN_FnFwd(MAIN_App app, Slice_(utf8str) args) { return x(app, args); } \
        int main(int argc, char** argv) \
        { \
            MAIN_Main( \
                (i32) argc, \
                (cstring*) argv, \
                MAIN_FnFwd, \
                true \
            ); \
            return 0; \
        } \
        EXTERN_C_END

#elif MSR_ANDROID

    struct android_app;

    static_assert( sizeof(MAIN_App) ==  sizeof(struct android_app*), "app struct size mismatch");
    static_assert(alignof(MAIN_App) == alignof(struct android_app*), "app struct alignment mismatch");

    static inline MAIN_App MAIN_ToApp(struct android_app* app) { return *(MAIN_App*) &app; }
    static inline struct android_app* MAIN_FromApp(MAIN_App app) { return *(struct android_app**) &app; }

    // internal fn; `android_main` will redirect here
    void MAIN_AndroidMain(struct android_app* app, MAIN_EntryPointProc mainFn, b8 isGui);

    #define MAIN_AS_DYNA_LIB_INTERNAL() \
        EXTERN_C_BEGIN \
        void android_main(struct android_app* app) { return; } \
        EXTERN_C_END

    #define MAIN_AS_CLI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 MAIN_FnFwd(MAIN_App app, Slice_(utf8str) args) { return x(app, args); } \
        void android_main(struct android_app* app) \
        { \
            MAIN_AndroidMain(app, MAIN_FnFwd, false); \
        } \
        EXTERN_C_END

    #define MAIN_AS_GUI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 MAIN_FnFwd(MAIN_App app, Slice_(utf8str) args) { return x(app, args); } \
        void android_main(struct android_app* app) \
        { \
            MAIN_AndroidMain(app, MAIN_FnFwd, true); \
        } \
        EXTERN_C_END

#else

    #error "entry point unimplemented"

#endif

#if MSR_WINDOWS || MSR_LINUX || MSR_OSX

    #define MAIN_AS_CLI_EXEC_INTERNAL(x) \
        EXTERN_C_BEGIN \
        static i32 MAIN_FnFwd(MAIN_App app, Slice_(utf8str) args) { return x(app, args); } \
        int main(int argc, char** argv) \
        { \
            MAIN_Main( \
                (i32) argc, \
                (cstring*) argv, \
                MAIN_FnFwd, \
                false \
            ); \
            return 0; \
        } \
        EXTERN_C_END

#endif

#ifndef MAIN_AS_DYNA_LIB_INTERNAL
    #error "dyna lib entry point not defined for this platform"
#endif

#ifndef MAIN_AS_GUI_EXEC_INTERNAL
    #error "gui executable entry point not defined for this platform"
#endif

#ifndef MAIN_AS_CLI_EXEC_INTERNAL
    #error "cli executable entry point not defined for this platform"
#endif

/**
 * Declare an entry point as a dynamic library.
 * This will cause the application to declare an entry point function, so that the linkers
 * do not complain, but the function will not forward to any other functions.
 * The macro does not take any arguments.
 */
#define MAIN_AS_DYNA_LIB() \
    MAIN_AS_DYNA_LIB_INTERNAL()

/**
 * Declare an entry point as a CLI executable.
 * The provided function will be called with the application instance and the command-line arguments.
 * The macro takes one argument, which is the function to call for the entry point.
 * This function must match the signature of `MAIN_EntryPointProc`.
 */
#define MAIN_AS_CLI_EXEC(x) \
    MAIN_AS_CLI_EXEC_INTERNAL(x)

/**
 * Declare an entry point as a GUI executable.
 * The provided function will be called with the application instance and the command-line arguments.
 * The macro takes one argument, which is the function to call for the entry point.
 * This function must match the signature of `MAIN_EntryPointProc`.
 */
#define MAIN_AS_GUI_EXEC(x) \
    MAIN_AS_GUI_EXEC_INTERNAL(x)

EXTERN_C_END
