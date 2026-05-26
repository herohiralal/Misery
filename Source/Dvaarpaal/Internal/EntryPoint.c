#include <Dvaarpaal/EntryPoint.h>

#ifndef DVRPL_SKIP_ENTRY_PT

    #if PNSLR_WINDOWS // windows only entry points

        i32 WWinMainImpl(rawptr hInstancePtr, u16* pCmdLinePtr, DVRPL_MainDelegate mainFn)
        {
            HINSTANCE hInstance = (HINSTANCE) hInstancePtr;
            LPWSTR    pCmdLine  = (LPWSTR)    pCmdLinePtr;

            int argc;
            WCHAR** argv = CommandLineToArgvW(pCmdLine, &argc);

            PNSLR_ArraySlice(utf8str) args = PNSLR_MakeSlice(utf8str, argc, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
            if (!args.data || !args.count) return -1;

            for (i32 i = 0; i < argc; i++)
            {
                WCHAR* arg = argv[i];
                i64    len = 0;
                while (arg[len] != L'\0') len++;
                len++; // count null term
                PNSLR_ArraySlice(u16) utf16Str = (PNSLR_ArraySlice(u16)) {.data = (u16*) arg, .count = (i64) len};
                args.data[i] = PNSLR_UTF8FromUTF16WindowsOnly(utf16Str, PNSLR_GetAllocator_DefaultHeap());
                if (!args.data[i].data || !args.data[i].count) return -1;
            }

            LocalFree(argv);

            i32 returnCode = mainFn(DVRPL_MAKE_APP_HANDLE(hInstance), args);
            for (i32 i = 0; i < argc; i++) PNSLR_FreeString(args.data[i], PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
            PNSLR_FreeSlice(&args, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

            return returnCode;
        }

    #else

        i32 WWinMainImpl(rawptr hInstancePtr, u16* pCmdLinePtr, DVRPL_MainDelegate mainFn)
        {
            (void) hInstancePtr;
            (void) pCmdLinePtr;
            (void) mainFn;
            return -1;
        }

    #endif

    #if PNSLR_LINUX || PNSLR_WINDOWS

        i32 MainImpl(i32 argc, cstring* argv, DVRPL_MainDelegate mainFn)
        {
            PNSLR_ArraySlice(utf8str) args = PNSLR_MakeSlice(utf8str, argc, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
            if (!args.data || !args.count) return -1;

            for (i32 i = 0; i < argc; ++i)
            {
                args.data[i] = PNSLR_StringFromCString(argv[i]);
                if (!args.data[i].data || !args.data[i].count) return -1;
            }

            #if PNSLR_WINDOWS
                HINSTANCE hInstance = GetModuleHandleW(nil);
                DVRPL_App app = DVRPL_MAKE_APP_HANDLE(hInstance);
            #else
                DVRPL_App app = (DVRPL_App) {0};
            #endif

            i32 returnCode = mainFn(app, args);
            PNSLR_FreeSlice(&args, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

            return returnCode;
        }

    #elif PNSLR_OSX || PNSLR_IOS

        // implementation in objc files

    #else

        i32 MainImpl(i32 argc, cstring* argv, DVRPL_MainDelegate mainFn)
        {
            (void) argc;
            (void) argv;
            (void) mainFn;
            return -1;
        }

    #endif

    #if PNSLR_ANDROID

        cstring* DVRPL_Internal_GetAndroidCmdLineArgs(i32* argc_out)
        {
            FILE* f = fopen("/proc/self/cmdline", "rb");
            if (!f) return NULL;

            cstring buf = NULL;
            size_t size = 0;
            ssize_t len = getdelim(&buf, &size, '\0', f); // reads until EOF
            fclose(f);

            if (len <= 0)
            {
                free(buf);
                return NULL;
            }

            // Count arguments
            i32 argc = 0;
            for (ssize_t i = 0; i < len; i++)
            {
                if (buf[i] == '\0')
                    argc++;
            }

            // Build argv
            cstring* argv = calloc((size_t) argc + 1, sizeof(cstring));
            i32 argi = 0;
            cstring p = buf;
            for (ssize_t i = 0; i < len; i++)
            {
                if (buf[i] == '\0')
                {
                    argv[argi++] = p;
                    p = &buf[i + 1];
                }
            }
            argv[argc] = NULL;

            *argc_out = argc;
            return argv;
        }

        void DVRPL_Internal_DisposeAndroidCmdLineArgs(cstring* argv)
        {
            if (argv)
            {
                free(argv[0]); // the buffer
                free(argv);    // the array
            }
        }

        void AndroidMainImpl(rawptr appPtr, DVRPL_MainDelegate mainFn)
        {
            struct android_app* app = (struct android_app*) appPtr;

            i32 argc; cstring* argv;
            argv = DVRPL_Internal_GetAndroidCmdLineArgs(&argc);

            PNSLR_ArraySlice(utf8str) args = PNSLR_MakeSlice(utf8str, argc, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
            if (!args.data || !args.count) return;

            for (i32 i = 0; i < argc; ++i)
            {
                args.data[i] = PNSLR_StringFromCString(argv[i]);
                if (!args.data[i].data || !args.data[i].count) return;

                #if PNSLR_DBG
                    __android_log_print(ANDROID_LOG_INFO, "Dvaarpaal", "Arg [%d]: %s", (i + 1), argv[i]);
                #endif
            }

            DVRPL_Internal_FlushEventsTillInFocus(app);
            i32 returnCode = mainFn(DVRPL_MAKE_APP_HANDLE(app), args);

            #if PNSLR_DBG
                __android_log_print(ANDROID_LOG_INFO, "Dvaarpaal", "Exiting with code %d", returnCode);
            #endif

            PNSLR_FreeSlice(&args, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

            DVRPL_Internal_DisposeAndroidCmdLineArgs(argv);
            ANativeActivity_finish(app->activity);
        }

    #else

        void AndroidMainImpl(rawptr appPtr, DVRPL_MainDelegate mainFn)
        {
            (void) appPtr;
            (void) mainFn;
        }

    #endif

#endif
