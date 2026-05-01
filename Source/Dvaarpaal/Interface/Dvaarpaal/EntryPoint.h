/**
 * Provides platform-agnostic entry-point implementations that forward
 * execution to a provided main delegate (unless, it's a dynamic library).
 *
 *  Q: How to use?
 *  A: Define one of the following symbols with a function name,
 *     before including this file:
 *         - DVRPL_EXECUTABLE_IMPL_WIN_MAIN
 *         - DVRPL_EXECUTABLE_IMPL_DLL_MAIN
 *             - won't actually call the function, but will define DllMain.
 *         - DVRPL_EXECUTABLE_IMPL_MAIN
 *         - DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN
 *         - DVRPL_EXECUTABLE_IMPL_ANY
 *             - defines one based on the platform
 *     Note that the function must match the DVRPL_MainDelegate signature.
 *
 *  Q: Are all of them available on all platforms?
 *  A: There are some platform restrictions:
 *         - WIN_MAIN and DLL_MAIN are Windows-only.
 *             - These both imply windows subsystem (if nothing specified).
 *         - ANDROID_MAIN is Android-only.
 *         - MAIN is available on Windows, Linux, OSX and iOS.
 *             - On Windows, this implies console subsystem (if nothing specified).
 *
 *  Q: Can I define multiple symbols?
 *  A: Technically you can, but it doesn't make much sense. If you'd like to be
 *     cross-platform, define only ANY.
 */
#ifndef DVRPL_ENTRY_PT_H // ========================================================
#define DVRPL_ENTRY_PT_H
#include "__Prelude.h"
#include "Window.h"
#include "Input.h"
EXTERN_C_BEGIN

/**
 * The type of delegate for the main entry point of the application.
 * (If you choose to use it as so).
 */
typedef i32 (*DVRPL_MainDelegate)(DVRPL_App app, PNSLR_ArraySlice(utf8str) args);

/**
 * Implementation for a Windows wWinMain entry point.
 * The execution will be forwarded to the provided mainFn delegate.
 *
 * Available on:
 * - Windows
 */
i32 WWinMainImpl(rawptr hInstancePtr, u16* pCmdLinePtr, DVRPL_MainDelegate mainFn);

/**
 * Implementation for a standard main entry point.
 * The execution will be forwarded to the provided mainFn delegate.
 *
 * Available on:
 * - Windows
 * - Linux
 * - OSX
 * - iOS
 */
i32 MainImpl(i32 argc, cstring* argv, DVRPL_MainDelegate mainFn);

/**
 * Implementation for an Android android_main entry point, as expected by
 * the android_native_app_glue library.
 * The execution will be forwarded to the provided mainFn delegate.
 *
 * Available on:
 * - Android
 */
void AndroidMainImpl(rawptr appPtr, DVRPL_MainDelegate mainFn);

EXTERN_C_END
//+skipreflect
#ifdef DVRPL_EXECUTABLE_IMPL_ANY
    #if PNSLR_WINDOWS
        #define DVRPL_EXECUTABLE_IMPL_WIN_MAIN     DVRPL_EXECUTABLE_IMPL_ANY
    #elif PNSLR_ANDROID
        #define DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN DVRPL_EXECUTABLE_IMPL_ANY
    #elif PNSLR_LINUX || PNSLR_OSX || PNSLR_IOS
        #define DVRPL_EXECUTABLE_IMPL_MAIN         DVRPL_EXECUTABLE_IMPL_ANY
    #endif
#endif

#ifdef DVRPL_EXECUTABLE_IMPL_WIN_MAIN

    i32 DVRPL_EXECUTABLE_IMPL_WIN_MAIN(DVRPL_App app, PNSLR_ArraySlice(utf8str) args);

    struct HINSTANCE__;
    typedef struct HINSTANCE__ *HINSTANCE;
    typedef unsigned short WCHAR;
    typedef WCHAR *LPWSTR;

    int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
    {
        (void) hPrevInstance;
        (void) nCmdShow;
        return WWinMainImpl((rawptr) hInstance, (u16*) pCmdLine, DVRPL_EXECUTABLE_IMPL_WIN_MAIN);
    }

    #undef DVRPL_EXECUTABLE_IMPL_ANY
    #undef DVRPL_EXECUTABLE_IMPL_WIN_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_DLL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN

#endif

#ifdef DVRPL_EXECUTABLE_IMPL_DLL_MAIN

    i32 DVRPL_EXECUTABLE_IMPL_DLL_MAIN(DVRPL_App app, PNSLR_ArraySlice(utf8str) args);

    typedef int BOOL;
    struct HINSTANCE__;
    typedef struct HINSTANCE__ *HINSTANCE;
    typedef unsigned long DWORD;
    typedef void *LPVOID;

    BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
    {
        (void) hinstDLL;
        (void) fdwReason;
        (void) lpvReserved;
        return 1; // successful attach
    }

    #undef DVRPL_EXECUTABLE_IMPL_ANY
    #undef DVRPL_EXECUTABLE_IMPL_WIN_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_DLL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN

#endif

#ifdef DVRPL_EXECUTABLE_IMPL_MAIN

    i32 DVRPL_EXECUTABLE_IMPL_MAIN(DVRPL_App app, PNSLR_ArraySlice(utf8str) args);

    int main(int argc, char** argv)
    {
        return MainImpl((i32) argc, (cstring*) argv, DVRPL_EXECUTABLE_IMPL_MAIN);
    }

    #undef DVRPL_EXECUTABLE_IMPL_ANY
    #undef DVRPL_EXECUTABLE_IMPL_WIN_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_DLL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN

#endif

#ifdef DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN

    i32 DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN(DVRPL_App app, PNSLR_ArraySlice(utf8str) args);

    struct android_app;

    #ifdef __cplusplus
    extern "C"
    #endif
    void android_main(struct android_app* app)
    {
        AndroidMainImpl((rawptr) app, DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN);
    }

    #undef DVRPL_EXECUTABLE_IMPL_ANY
    #undef DVRPL_EXECUTABLE_IMPL_WIN_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_DLL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_MAIN
    #undef DVRPL_EXECUTABLE_IMPL_ANDROID_MAIN

#endif
//-skipreflect
#endif // DVRPL_ENTRY_PT_H =========================================================
