#include "Dvaarpaal/EntryPoint.h"

#if PNSLR_OSX

    i32 MainImpl(i32 argc, cstring* argv, DVRPL_MainDelegate mainFn)
    {
        @autoreleasepool {
            PNSLR_ArraySlice(utf8str) args = PNSLR_MakeSlice(utf8str, argc, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
            if (!args.data || !args.count) return -1;

            for (i32 i = 0; i < argc; ++i)
            {
                args.data[i] = PNSLR_StringFromCString(argv[i]);
                if (!args.data[i].data || !args.data[i].count) return -1;
            }

            NSApplication* nativeApp = [NSApplication sharedApplication];

            DVRPL_App app = DVRPL_MAKE_APP_HANDLE((__bridge_retained rawptr) nativeApp);

            i32 returnCode = mainFn(app, args);

            nativeApp = (__bridge_transfer NSApplication*) DVRPL_BREAK_APP_HANDLE(app);

            [nativeApp terminate:nil];

            PNSLR_FreeSlice(&args, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

            return returnCode;
        }
    }

#elif PNSLR_IOS

#else
    #error "Unimplemented."
#endif
