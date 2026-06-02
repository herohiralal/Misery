#include "Core/Memory.h"
#include "__init.h"
#include <Core/Core.h>
#include <processenv.h>
#include <shellapi.h>
#include <stringapiset.h>
#include <winnls.h>

static inline Slice_(utf8str) MAIN_Internal_GetCmdArgs(i32 argc, cstring* argv)
{
    Slice_(utf8str) args = COL_NewSlice(utf8str, (isize) argc, false, MEM_main);
    if (!args.data || !args.count) return (Slice_(utf8str)) {0};

    for (i32 i = 0; i < argc; ++i)
    {
        args.data[i] = STR_AliasCStr(argv[i]);
    }

    return args;
}

i32 MAIN_Main(i32 argc, cstring* argv, MAIN_EntryPointProc mainFn, b8 isGui)
{
    #if MSR_WINDOWS
    {
        (void) isGui; // on windows, gui entry point will go to winmain

        MAIN_App app = MAIN_ToApp(GetModuleHandleA(nil));
        Slice_(utf8str) args = MAIN_Internal_GetCmdArgs(argc, argv);

        // actual execution
        i32 exitCode = mainFn(app, args);

        COL_DeleteSlice(&args, MEM_main);

        return exitCode;
    }
    #elif MSR_OSX
    {
        @autoreleasepool {
            NSApplication* nativeApp = [NSApplication sharedApplication];
            MAIN_App app = MAIN_ToApp((__bridge_retained rawptr) nativeApp);
            Slice_(utf8str) args = MAIN_Internal_GetCmdArgs(argc, argv);

            // actual execution
            i32 exitCode = mainFn(app, args);

            COL_DeleteSlice(&args, MEM_main);

            nativeApp = (__bridge_transfer NSApplication*) MAIN_FromApp(app);
            [nativeApp terminate:nil];

            return exitCode;
        }
    }
    #elif MSR_LINUX
    {
        MAIN_App app = (MAIN_App) {0}; // no app handle on linux
        Slice_(utf8str) args = MAIN_Internal_GetCmdArgs(argc, argv);

        // actual execution
        i32 exitCode = mainFn(app, args);

        COL_DeleteSlice(&args, MEM_main);

        return exitCode;
    }
    #endif
}

#if MSR_WINDOWS

    i32 MAIN_WinMain(HINSTANCE hInstance, PSTR pCmdLinePtr, MAIN_EntryPointProc mainFn)
    {
        MAIN_App app = MAIN_ToApp(hInstance);

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
