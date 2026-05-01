#include "Dvaarpaal/Window.h"
#include "Dvaarpaal/Input.h"
#if !PNSLR_APPLE // apple stuff will be on objc file

#if PNSLR_WINDOWS

static b8 G_DVRPL_Internal_WindowClassInitialised = false;
static const WCHAR* const DVRPL_INTERNAL_WND_CLS_NM = L"DVAARPAAL DEFAULT WINDOW CLASS";
static void DVRPL_Internal_InitialiseWindowClass(DVRPL_App app, u8 bgColR, u8 bgColG, u8 bgColB, u8 bgColA)
{
    if (G_DVRPL_Internal_WindowClassInitialised)
        return;

    HICON icon = LoadIconW(DVRPL_BREAK_APP_HANDLE(app), MAKEINTRESOURCEW(2));
    if (icon == NULL)
    {
        WCHAR exePath[260];
        GetModuleFileNameW(DVRPL_BREAK_APP_HANDLE(app), exePath, 260);
        ExtractIconExW(exePath, 0, NULL, &icon, 1);
    }

    HBRUSH brush = CreateSolidBrush(RGB(bgColR, bgColG, bgColB));
    WNDCLASSEXW wc =
    {
        .cbSize        = sizeof(WNDCLASSEXW),
        .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc   = DVRPL_Internal_WindowsInputCallback,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = DVRPL_BREAK_APP_HANDLE(app),
        .hIcon         = icon,
        .hCursor       = LoadCursorW(NULL, (LPCWSTR) IDC_ARROW),
        .hbrBackground = brush,
        .lpszMenuName  = NULL,
        .lpszClassName = DVRPL_INTERNAL_WND_CLS_NM,
        .hIconSm       = icon
    };

    G_DVRPL_Internal_WindowClassInitialised = RegisterClassExW(&wc) != 0;
}

#endif

DVRPL_WindowData DVRPL_CreateWindow(DVRPL_WindowCreationOptions options)
{
    #if PNSLR_WINDOWS
    {
        DVRPL_Internal_InitialiseWindowClass(options.app, options.bgColR, options.bgColG, options.bgColB, options.bgColA);

        if (options.posX <= 0 && options.posY <= 0)
        {
            RECT workArea;
            if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
            {
                options.posX = (i16) workArea.left;
                options.posY = (i16) workArea.top;
            }
            else
            {
                options.posX = 15;
                options.posY = 15;
            }
        }

        RECT rect =
        {
            .left   = (LONG) options.posX,
            .top    = (LONG) options.posY,
            .right  = (LONG) options.posX + (LONG) options.sizeX,
            .bottom = (LONG) options.posY + (LONG) options.sizeY
        };
        DWORD style = options.parent.handle == 0 ? WS_OVERLAPPEDWINDOW : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);

        AdjustWindowRect(&rect, style, FALSE);
        LONG cW = rect.right - rect.left, cH = rect.bottom - rect.top;

        PNSLR_ArraySlice(u16) title = PNSLR_UTF16FromUTF8WindowsOnly(options.title, PNSLR_GetAllocator_DefaultHeap());

        HWND output = CreateWindowExW(
            0,
            DVRPL_INTERNAL_WND_CLS_NM,
            (LPCWSTR) title.data,
            style,
            (LONG) options.posX,
            (LONG) options.posY,
            cW,
            cH,
            DVRPL_BREAK_WINDOW_HANDLE(options.parent),
            NULL, NULL, NULL
        );

        PNSLR_FreeSlice(&title, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

        if (output == DVRPL_Internal_InvalidWindowHandle) // TODO: handle failure
            return (DVRPL_WindowData){.window = DVRPL_MAKE_WINDOW_HANDLE(DVRPL_Internal_InvalidWindowHandle)};

        UpdateWindow(output);
        ShowWindow(output, SW_SHOW);

        if (options.acceptDropFiles)
            DragAcceptFiles(output, TRUE);

        DVRPL_Internal_NativeSavedWindowData savedData =
        {
            .rect = rect,
            .savedStyle = GetWindowLongW(output, GWL_STYLE),
            .savedExStyle = GetWindowLongW(output, GWL_EXSTYLE),
        };

        return (DVRPL_WindowData)
        {
            .window = DVRPL_MAKE_WINDOW_HANDLE(output),
            .savedData = DVRPL_MAKE_SAVED_WINDOW_DATA(savedData),
        };
    }
    #elif PNSLR_ANDROID
    {
        if (!options.app.handle)
            return (DVRPL_WindowData) {.window = DVRPL_MAKE_WINDOW_HANDLE(DVRPL_Internal_InvalidWindowHandle)};

        struct android_app* h = DVRPL_BREAK_APP_HANDLE(options.app);
        if (h)
        {
            DVRPL_Internal_AndroidSetApp(h);
            return (DVRPL_WindowData)
            {
                .window    = DVRPL_MAKE_WINDOW_HANDLE((h->window)),
                .savedData = DVRPL_MAKE_SAVED_WINDOW_DATA((DVRPL_Internal_NativeSavedWindowData){.app = h}),
            };
        }
        else
        {
            return (DVRPL_WindowData) {0};
        }
    }
    #else
        #error "Unimplemented."
    #endif
}

void DVRPL_DestroyWindow(DVRPL_WindowData* window)
{
    if (window == nil || DVRPL_BREAK_WINDOW_HANDLE(window->window) == DVRPL_Internal_InvalidWindowHandle)
        return;

    #if PNSLR_WINDOWS
    {
        DestroyWindow(DVRPL_BREAK_WINDOW_HANDLE(window->window));
        window->window = DVRPL_MAKE_WINDOW_HANDLE(DVRPL_Internal_InvalidWindowHandle);
    }
    #elif PNSLR_ANDROID
    {
        // no-op
    }
    #else
        #error "Unimplemented."
    #endif
}

b8 DVRPL_SetFullScreen(DVRPL_WindowData* window, b8 status, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (window == nil || DVRPL_BREAK_WINDOW_HANDLE(window->window) == DVRPL_Internal_InvalidWindowHandle)
        return false;

    DVRPL_Internal_NativeWindowHandle    windowHandle = DVRPL_BREAK_WINDOW_HANDLE(window->window);
    DVRPL_Internal_NativeSavedWindowData savedData    = DVRPL_BREAK_SAVED_WINDOW_DATA(window->savedData);

    ((void) windowHandle); // silence unused var

    i16 x, y;
    u16 w, h;
    #if PNSLR_WINDOWS
    {
        if (status)
        {
            LONG savedStyle = GetWindowLongW(windowHandle, GWL_STYLE);
            LONG savedExStyle = GetWindowLongW(windowHandle, GWL_EXSTYLE);

            SetWindowLongW(windowHandle, GWL_STYLE,   savedStyle   & ~(WS_CAPTION | WS_THICKFRAME));
            SetWindowLongW(windowHandle, GWL_EXSTYLE, savedExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

            HMONITOR    monitor = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
            MONITORINFO info    = {.cbSize = sizeof(MONITORINFO)};

            if (!GetMonitorInfoW(monitor, &info))
                return false;

            GetWindowRect(windowHandle, &savedData.rect);
            savedData.savedStyle   = savedStyle;
            savedData.savedExStyle = savedExStyle;

            window->savedData = DVRPL_MAKE_SAVED_WINDOW_DATA(savedData);

            x = (i16) info.rcMonitor.left;
            y = (i16) info.rcMonitor.top;
            w = (u16)(info.rcMonitor.right  - info.rcMonitor.left);
            h = (u16)(info.rcMonitor.bottom - info.rcMonitor.top);

            SetWindowPos(windowHandle, HWND_TOPMOST, (LONG) x, (LONG) y, (LONG) w, (LONG) h, SWP_FRAMECHANGED);
        }
        else
        {
            SetWindowLongW(windowHandle, GWL_STYLE, savedData.savedStyle);
            SetWindowLongW(windowHandle, GWL_EXSTYLE, savedData.savedExStyle);

            x = (i16) savedData.rect.left;
            y = (i16) savedData.rect.top;
            w = (u16)(savedData.rect.right  - savedData.rect.left);
            h = (u16)(savedData.rect.bottom - savedData.rect.top);
            SetWindowPos(windowHandle, HWND_NOTOPMOST, (LONG) x, (LONG) y, (LONG) w, (LONG) h, SWP_FRAMECHANGED);
        }
    }
    #elif PNSLR_ANDROID
    {
        if (status) ANativeActivity_setWindowFlags(savedData.app->activity, AWINDOW_FLAG_FULLSCREEN, 0);
        else        ANativeActivity_setWindowFlags(savedData.app->activity, 0, AWINDOW_FLAG_FULLSCREEN);

        DVRPL_GetWindowDimensions(window, &x, &y, &w, &h);
    }
    #else
        #error "Unimplemented."
    #endif

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;
    return true;
}

b8 DVRPL_GetWindowDimensions(DVRPL_WindowData* window, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (window == nil || DVRPL_BREAK_WINDOW_HANDLE(window->window) == DVRPL_Internal_InvalidWindowHandle)
        return false;

    i16 x, y;
    u16 w, h;
    #if PNSLR_WINDOWS
    {
        RECT screenRect;
        if (!GetWindowRect(DVRPL_BREAK_WINDOW_HANDLE(window->window), &screenRect))
            return false;

        RECT localRect;
        if (!GetClientRect(DVRPL_BREAK_WINDOW_HANDLE(window->window), &localRect))
            return false;

        x = (i16) screenRect.left;
        y = (i16) screenRect.top;
        w = (u16) (localRect.right  - localRect.left);
        h = (u16) (localRect.bottom - localRect.top);
    }
    #elif PNSLR_ANDROID
    {
        x = y = 0;
        w = (u16) ANativeWindow_getWidth(DVRPL_BREAK_WINDOW_HANDLE(window->window));
        h = (u16) ANativeWindow_getHeight(DVRPL_BREAK_WINDOW_HANDLE(window->window));
    }
    #else
        #error "Unimplemented."
    #endif

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;
    return true;
}

b8 DVRPL_GetPtrPosFromWindow(DVRPL_Window window, i16* posX, i16* posY)
{
    if (DVRPL_BREAK_WINDOW_HANDLE(window) == DVRPL_Internal_InvalidWindowHandle)
        return false;

    i16 x, y;
    #if PNSLR_WINDOWS
    {
        POINT p;
        if (!GetCursorPos(&p))
            return false;
        if (!ScreenToClient(DVRPL_BREAK_WINDOW_HANDLE(window), &p))
            return false;

        x = (i16) p.x;
        y = (i16) p.y;
    }
    #elif PNSLR_ANDROID
    {
        return false; // TODO: implement
    }
    #else
        #error "Unimplemented."
    #endif

    if (posX) *posX = x;
    if (posY) *posY = y;
    return true;
}

b8 DVRPL_GetPtrPos(i16* posX, i16* posY)
{
    #if PNSLR_WINDOWS
    {
        HWND w = GetActiveWindow();
        return DVRPL_GetPtrPosFromWindow(DVRPL_MAKE_WINDOW_HANDLE(w), posX, posY);
    }
    #elif PNSLR_ANDROID
    {
        return false; // TODO: implement
    }
    #else
        #error "Unimplemented."
    #endif
}

b8 DVRPL_RenameWindow(DVRPL_Window window, utf8str newName)
{
    if (DVRPL_BREAK_WINDOW_HANDLE(window) == DVRPL_Internal_InvalidWindowHandle)
        return false;

    #if PNSLR_WINDOWS
    {
        PNSLR_ArraySlice(u16) newNameW = PNSLR_UTF16FromUTF8WindowsOnly(newName, PNSLR_GetAllocator_DefaultHeap());
        b8 output = !!SetWindowTextW(DVRPL_BREAK_WINDOW_HANDLE(window), (LPCWSTR) newNameW.data);
        PNSLR_FreeSlice(&newNameW, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
        return output;
    }
    #elif PNSLR_ANDROID
    {
        return false;
    }
    #else
        #error "Unimplemented."
    #endif
}

#endif
