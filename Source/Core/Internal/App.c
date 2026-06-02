#include "Core/Memory.h"
#include "__init.h"
#include <Core/Core.h>
#include <processenv.h>
#include <shellapi.h>
#include <stringapiset.h>
#include <winnls.h>

static inline Slice_(utf8str) APP_Internal_GetCmdArgs(i32 argc, cstring* argv)
{
    Slice_(utf8str) args = COL_NewSlice(utf8str, (isize) argc, false, MEM_main);
    if (!args.data || !args.count) return (Slice_(utf8str)) {0};

    for (i32 i = 0; i < argc; ++i)
    {
        args.data[i] = STR_AliasCStr(argv[i]);
    }

    return args;
}

i32 APP_Main(i32 argc, cstring* argv, APP_EntryPointProc mainFn, b8 isGui)
{
    #if MSR_WINDOWS
    {
        (void) isGui; // on windows, gui entry point will go to winmain

        APP_Handle app = APP_ToHandle(GetModuleHandleA(nil));
        Slice_(utf8str) args = APP_Internal_GetCmdArgs(argc, argv);

        // actual execution
        i32 exitCode = mainFn(app, args);

        COL_DeleteSlice(&args, MEM_main);

        return exitCode;
    }
    #elif MSR_OSX
    {
        @autoreleasepool {
            NSApplication* nativeApp = [NSApplication sharedApplication];
            APP_Handle app = APP_ToHandle((__bridge_retained rawptr) nativeApp);
            Slice_(utf8str) args = APP_Internal_GetCmdArgs(argc, argv);

            // actual execution
            i32 exitCode = mainFn(app, args);

            COL_DeleteSlice(&args, MEM_main);

            nativeApp = (__bridge_transfer NSApplication*) APP_FromHandle(app);
            [nativeApp terminate:nil];

            return exitCode;
        }
    }
    #elif MSR_LINUX
    {
        APP_Handle app = (APP_Handle) {0}; // no app handle on linux
        Slice_(utf8str) args = APP_Internal_GetCmdArgs(argc, argv);

        // actual execution
        i32 exitCode = mainFn(app, args);

        COL_DeleteSlice(&args, MEM_main);

        return exitCode;
    }
    #endif
}

#if MSR_WINDOWS

    i32 APP_WinMain(HINSTANCE hInstance, PSTR pCmdLinePtr, APP_EntryPointProc mainFn)
    {
        APP_Handle app = APP_ToHandle(hInstance);

        int argc;
        WCHAR** argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argvW) return -1;

        MEM_ArenaAllocator argsArena = MEM_CreateArenaAllocator(16 * 1024, MEM_main);
        MEM_Allocator argsAllocator = MEM_AllocatorFromArena(&argsArena);

        b8 argsFail = false;

        Slice_(utf8str) args = COL_NewSlice(utf8str, (isize) argc, false, argsAllocator);
        if (!args.data || !args.count)
            argsFail = true;

        for (i32 i = 0; i < argc; i++)
        {
            if (argsFail)
                break;

            i32 len = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nil, 0, nil, nil);
            if (len <= 0)
            {
                argsFail = true;
                break;
            }

            args.data[i] = COL_NewSlice(u8, (isize) len, false, argsAllocator);
            args.data[i].count--; // exclude null terminator
            if (!args.data[i].data || !args.data[i].count)
            {
                argsFail = true;
                break;
            }

            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, (CHAR*) args.data[i].data, len, nil, nil);
        }

        LocalFree(argvW);

        if (argsFail)
        {
            MEM_DestroyArenaAllocator(&argsArena);
            return -1; // couldn't read commandline
        }

        // actual execution
        i32 exitCode = mainFn(app, args);

        MEM_DestroyArenaAllocator(&argsArena);

        return exitCode;
    }

#endif

#if MSR_ANDROID
    #error "todo" // see dvaarpaal's android entry point implementation
#endif
