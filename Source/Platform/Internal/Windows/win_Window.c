#include "win_Platform.h"

#if MSR_WINDOWS

static b8 G_WND_Internal_ClassInitialised = false;
static const cstring WND_INTERNAL_WND_CLS_NM = "MISERY_DEFAULT_WINDOW_CLASS";

static void WND_Internal_InitialiseClass(APP_Handle app, u8 bgCol[4])
{
    if (G_WND_Internal_ClassInitialised)
        return;

    HICON icon = LoadIconA(APP_FromHandle(app), MAKEINTRESOURCEA(2));
    if (icon == nil)
    {
        CHAR exePath[260];
        GetModuleFileNameA(APP_FromHandle(app), exePath, 260);
        ExtractIconExA(exePath, 0, nil, &icon, 1);
    }

    G_WND_Internal_ClassInitialised = (0 != RegisterClassExA(&(WNDCLASSEXA)
    {
        .cbSize        = sizeof(WNDCLASSEXA),
        .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc   = INP_Internal_WindowsInputCallback,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = APP_FromHandle(app),
        .hIcon         = icon,
        .hCursor       = LoadCursorA(nil, IDC_ARROW),
        .hbrBackground = CreateSolidBrush(RGB(bgCol[0], bgCol[1], bgCol[2])),
        .lpszMenuName  = nil,
        .lpszClassName = WND_INTERNAL_WND_CLS_NM,
        .hIconSm       = icon
    }));
}

WND_Data WND_Create(WND_Cfg cfg)
{
    WND_Internal_InitialiseClass(cfg.app, cfg.bgCol);

    if (cfg.posX <= 0 && cfg.posY <= 0)
    {
        RECT workArea;
        if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &workArea, 0))
        {
            cfg.posX = (i16) workArea.left;
            cfg.posY = (i16) workArea.top;
        }
        else
        {
            cfg.posX = 15;
            cfg.posY = 15;
        }
    }

    RECT rect =
    {
        .left   = (LONG) cfg.posX,
        .top    = (LONG) cfg.posY,
        .right  = (LONG) cfg.posX + (LONG) cfg.sizeX,
        .bottom = (LONG) cfg.posY + (LONG) cfg.sizeY
    };
    DWORD style = cfg.parent.handle == 0 ? WS_OVERLAPPEDWINDOW : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);

    AdjustWindowRect(&rect, style, FALSE);
    LONG cW = rect.right - rect.left, cH = rect.bottom - rect.top;

    cstring title = STR_CloneToCStr(cfg.title, MEM_temp);

    HWND output = CreateWindowExA(
        0,
        WND_INTERNAL_WND_CLS_NM,
        title,
        style,
        (LONG) cfg.posX, (LONG) cfg.posY,
        cW, cH,
        WND_FromHandle(cfg.parent),
        nil, nil, nil
    );

    if (output == nil) // TODO: handle failure
        return (WND_Data) {0};

    UpdateWindow(output);
    ShowWindow(output, SW_SHOW);

    if (cfg.acceptDropFiles)
        DragAcceptFiles(output, TRUE);

    return (WND_Data)
    {
        .handle = WND_ToHandle(output),
        .savedData = WND_FromSavedData((WND_NativeSavedData)
        {
            .rect = rect,
            .savedStyle = GetWindowLongA(output, GWL_STYLE),
            .savedExStyle = GetWindowLongA(output, GWL_EXSTYLE),
        }),
    };
}

void WND_Destroy(WND_Data* window)
{
    if (window == nil)
        return;

    HWND hWnd = WND_FromHandle(window->handle);
    if (hWnd == nil)
        return;

    DestroyWindow(hWnd);
    window->handle = (WND_Handle) {0};
}

b8 WND_SetFullScreen(WND_Data* window, b8 status, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (window == nil)
        return false;

    HWND hWnd = WND_FromHandle(window->handle);
    if (hWnd == nil)
        return false;

    WND_NativeSavedData savedData = WND_ToSavedData(window->savedData);

    i16 x, y;
    u16 w, h;
    if (status)
    {
        LONG savedStyle = GetWindowLongA(hWnd, GWL_STYLE);
        LONG savedExStyle = GetWindowLongA(hWnd, GWL_EXSTYLE);

        SetWindowLongA(hWnd, GWL_STYLE,   savedStyle   & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowLongA(hWnd, GWL_EXSTYLE, savedExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

        HMONITOR    monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info    = {.cbSize = sizeof(MONITORINFO)};

        if (!GetMonitorInfoA(monitor, &info))
            return false;

        GetWindowRect(hWnd, &savedData.rect);
        savedData.savedStyle   = savedStyle;
        savedData.savedExStyle = savedExStyle;

        window->savedData = WND_FromSavedData(savedData);

        x = (i16) info.rcMonitor.left;
        y = (i16) info.rcMonitor.top;
        w = (u16)(info.rcMonitor.right  - info.rcMonitor.left);
        h = (u16)(info.rcMonitor.bottom - info.rcMonitor.top);

        SetWindowPos(hWnd, HWND_TOPMOST, (LONG) x, (LONG) y, (LONG) w, (LONG) h, SWP_FRAMECHANGED);
    }
    else
    {
        SetWindowLongA(hWnd, GWL_STYLE, savedData.savedStyle);
        SetWindowLongA(hWnd, GWL_EXSTYLE, savedData.savedExStyle);

        x = (i16) savedData.rect.left;
        y = (i16) savedData.rect.top;
        w = (u16)(savedData.rect.right  - savedData.rect.left);
        h = (u16)(savedData.rect.bottom - savedData.rect.top);
        SetWindowPos(hWnd, HWND_NOTOPMOST, (LONG) x, (LONG) y, (LONG) w, (LONG) h, SWP_FRAMECHANGED);
    }

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;
    return true;
}

b8 WND_GetDimensions(WND_Data* window, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (window == nil)
        return false;

    HWND hWnd = WND_FromHandle(window->handle);
    if (hWnd == nil)
        return false;

    i16 x, y;
    u16 w, h;

    RECT screenRect;
    if (!GetWindowRect(hWnd, &screenRect))
        return false;

    RECT localRect;
    if (!GetClientRect(hWnd, &localRect))
        return false;

    x = (i16) screenRect.left;
    y = (i16) screenRect.top;
    w = (u16) (localRect.right  - localRect.left);
    h = (u16) (localRect.bottom - localRect.top);

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;
    return true;
}

b8 WND_GetPtrPos(WND_Data* window, i16* posX, i16* posY)
{
    if (window == nil)
        return false;

    HWND hWnd = WND_FromHandle(window->handle);
    if (hWnd == nil)
        return false;

    i16 x, y;

    POINT p;
    if (!GetCursorPos(&p))
        return false;

    if (!ScreenToClient(hWnd, &p))
        return false;

    x = (i16) p.x;
    y = (i16) p.y;

    if (posX) *posX = x;
    if (posY) *posY = y;
    return true;
}

b8 WND_Rename(WND_Data* window, utf8str newName)
{
    if (window == nil)
        return false;

    HWND hWnd = WND_FromHandle(window->handle);
    if (hWnd == nil)
        return false;

    cstring newNameCStr = STR_CloneToCStr(newName, MEM_temp);
    b8 output = !!SetWindowTextA(hWnd, newNameCStr);
    return output;
}

#endif
