#include "Dvaarpaal/Input.h"

// private platform-unspecific globals =============================================

static PNSLR_Allocator                          G_DVRPL_Internal_CurrentTempAllocator = {0};
static PNSLR_ArraySlice(utf8str)                G_DVRPL_Internal_TempDroppedFiles     = {0};
static i64                                      G_DVRPL_Internal_NumTempDroppedFiles  = {0};
static PNSLR_ArraySlice(DVRPL_WindowResizeData) G_DVRPL_Internal_TempResizes          = {0};
static i64                                      G_DVRPL_Internal_NumTempResizes       = {0};
static PNSLR_ArraySlice(DVRPL_WindowMoveData)   G_DVRPL_Internal_TempMoves            = {0};
static i64                                      G_DVRPL_Internal_NumTempMoves         = {0};

// private platform-specific globals ===============================================

#if PNSLR_WINDOWS

static PNSLR_ArraySlice(u8)  G_DVRPL_Internal_RawInputBuffer         = {0};
static i64                   G_DVRPL_Internal_NumRawInputBuffer      = 0;
static b8                    G_DVRPL_Internal_WindowMinimised        = false;
static DVRPL_KeyModifier     G_DVRPL_Internal_CachedModifierStates   = DVRPL_KeyModifier_None;
static PNSLR_ArraySlice(i32) G_DVRPL_Internal_KeysDown               = {0}; // acts as a set
static i64                   G_DVRPL_Internal_NumKeysDown            = 0;
static b8                    G_DVRPL_Internal_InputSystemInitialised = false;

#elif PNSLR_ANDROID

typedef struct DVRPL_Internal_AndroidPtrInfo
{
    DVRPL_KeyState state;
    f32            posX;
    f32            posY;
} DVRPL_Internal_AndroidPtrInfo;

static struct android_app*           G_DVRPL_Internal_AndroidApp             = nil;
static DVRPL_Internal_AndroidPtrInfo G_DVRPL_Internal_AndroidPointers[10]    = {0};
static b8                            G_DVRPL_Internal_InputSystemInitialised = false;
static b8                            G_DVRPL_Internal_ResizeEventCalledOnce  = false;
static b8                            G_DVRPL_Internal_InFocus                = false;

#endif

// public platform-unspecific globals ==============================================

static PNSLR_ArraySlice(DVRPL_Event) G_DVRPL_Internal_Events                              = {0};
static i64                           G_DVRPL_Internal_NumEvents                           = 0;
static i32                           G_DVRPL_Internal_MouseDelta[3]                       = {0, 0, 0}; // x, y, scroll
static DVRPL_KeyState                G_DVRPL_Internal_KeyStates[(i32)(DVRPL_KeyCode_NUM)] = {0};
static b8                            G_DVRPL_Internal_AppHasFocus                         = false;

// private plataform-unspecific functions ==========================================

static void DVRPL_Internal_ResizeEventsIfBufferFull(void)
{
    if (G_DVRPL_Internal_NumEvents >= G_DVRPL_Internal_Events.count)
    {
        i64 newCount = (G_DVRPL_Internal_Events.count ? (G_DVRPL_Internal_Events.count * 2) : 16);
        PNSLR_ResizeSlice(DVRPL_Event, &G_DVRPL_Internal_Events, newCount, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
    }
}

static void DVRPL_Internal_ClearExistingInputData(void)
{
    if (G_DVRPL_Internal_TempDroppedFiles.data != nil)
    {
        for (i64 i = 0; i < G_DVRPL_Internal_NumTempDroppedFiles; i++)
            PNSLR_FreeString(
                G_DVRPL_Internal_TempDroppedFiles.data[i],
                G_DVRPL_Internal_CurrentTempAllocator,
                PNSLR_GET_LOC(),
                nil
            );

        PNSLR_FreeSlice(
            &G_DVRPL_Internal_TempDroppedFiles,
            G_DVRPL_Internal_CurrentTempAllocator,
            PNSLR_GET_LOC(),
            nil
        );

        G_DVRPL_Internal_TempDroppedFiles = (PNSLR_ArraySlice(utf8str)) {0};
        G_DVRPL_Internal_NumTempDroppedFiles = 0;
    }

    #if PNSLR_WINDOWS
        G_DVRPL_Internal_NumRawInputBuffer = 0;
    #endif

    G_DVRPL_Internal_NumEvents = 0;

    {
        i32 resizeIt = 0, moveIt = 0;
        while (DVRPL_IterateResizeEvent(&resizeIt, nil)) { }
        while (DVRPL_IterateMoveEvent  (&moveIt,   nil)) { }
    }

    // Clear key states
    for (i32 i = 0; i < (i32)(DVRPL_KeyCode_NUM); i++)
    {
        DVRPL_KeyState* curr = &(G_DVRPL_Internal_KeyStates[i]);

        if ((*curr) & DVRPL_KeyState_Released)
            (*curr) &= ~(DVRPL_KeyState_Released | DVRPL_KeyState_Pressed | DVRPL_KeyState_Held);
        else
            (*curr) &= ~(DVRPL_KeyState_Pressed);
    }

    // Clear mouse delta
    G_DVRPL_Internal_MouseDelta[0] = G_DVRPL_Internal_MouseDelta[1] = G_DVRPL_Internal_MouseDelta[2] = 0;
}

#if PNSLR_WINDOWS

static DVRPL_KeyCode DVRPL_Internal_GetKeyCode(i32 vKey)
{
    if (vKey >= '0' && vKey <= 'Z')
        return (DVRPL_KeyCode) vKey;

    if (vKey == VK_LBUTTON)  return DVRPL_KeyCode_MouseBtnLeft;
    if (vKey == VK_MBUTTON)  return DVRPL_KeyCode_MouseBtnMiddle;
    if (vKey == VK_RBUTTON)  return DVRPL_KeyCode_MouseBtnRight;
    if (vKey == VK_SPACE)    return DVRPL_KeyCode_Space;
    if (vKey == VK_HOME)     return DVRPL_KeyCode_Home;
    if (vKey == VK_END)      return DVRPL_KeyCode_End;
    if (vKey == VK_PAUSE)    return DVRPL_KeyCode_Pause;
    if (vKey == VK_SCROLL)   return DVRPL_KeyCode_ScrollLock;
    if (vKey == VK_PRIOR)    return DVRPL_KeyCode_PgUp;
    if (vKey == VK_NEXT)     return DVRPL_KeyCode_PgDown;
    if (vKey == VK_UP)       return DVRPL_KeyCode_ArrowUp;
    if (vKey == VK_DOWN)     return DVRPL_KeyCode_ArrowDown;
    if (vKey == VK_LEFT)     return DVRPL_KeyCode_ArrowLeft;
    if (vKey == VK_RIGHT)    return DVRPL_KeyCode_ArrowRight;
    if (vKey == VK_MENU)     return DVRPL_KeyCode_Alt;
    if (vKey == VK_CONTROL)  return DVRPL_KeyCode_Control;
    if (vKey == VK_SHIFT)    return DVRPL_KeyCode_Shift;
    if (vKey == VK_BACK)     return DVRPL_KeyCode_Backspace;
    if (vKey == VK_DELETE)   return DVRPL_KeyCode_Delete;
    if (vKey == VK_INSERT)   return DVRPL_KeyCode_Insert;
    if (vKey == VK_ESCAPE)   return DVRPL_KeyCode_Escape;
    if (vKey == VK_TAB)      return DVRPL_KeyCode_Tab;
    if (vKey == VK_RETURN)   return DVRPL_KeyCode_Enter;

    if (vKey == VK_OEM_1)    return (DVRPL_KeyCode) ';';
    if (vKey == VK_OEM_2)    return (DVRPL_KeyCode) '/';
    if (vKey == VK_OEM_3)    return (DVRPL_KeyCode) '`';
    if (vKey == VK_OEM_4)    return (DVRPL_KeyCode) '[';
    if (vKey == VK_OEM_5)    return (DVRPL_KeyCode) '\\';
    if (vKey == VK_OEM_6)    return (DVRPL_KeyCode) ']';
    if (vKey == VK_OEM_7)    return (DVRPL_KeyCode) '\'';

    if (vKey >= VK_F1 && vKey <= VK_F12)
        return DVRPL_KeyCode_F1 + (DVRPL_KeyCode)(vKey - VK_F1);

    if (vKey == VK_SNAPSHOT) return DVRPL_KeyCode_PrtScrn;

    return DVRPL_KeyCode_Unknown;
}

static i32 DVRPL_Internal_GetVirtualKey(DVRPL_KeyCode keyCode)
{
    if (keyCode >= (DVRPL_KeyCode) '0' && keyCode <= (DVRPL_KeyCode) 'Z')
        return (i32) keyCode;

    if (keyCode == DVRPL_KeyCode_MouseBtnLeft)   return VK_LBUTTON;
    if (keyCode == DVRPL_KeyCode_MouseBtnMiddle) return VK_MBUTTON;
    if (keyCode == DVRPL_KeyCode_MouseBtnRight)  return VK_RBUTTON;

    if (keyCode == (DVRPL_KeyCode) ' ')         return VK_SPACE;
    if (keyCode == DVRPL_KeyCode_Home)          return VK_HOME;
    if (keyCode == DVRPL_KeyCode_End)           return VK_END;
    if (keyCode == DVRPL_KeyCode_Pause)         return VK_PAUSE;
    if (keyCode == DVRPL_KeyCode_ScrollLock)    return VK_SCROLL;
    if (keyCode == DVRPL_KeyCode_PgUp)          return VK_PRIOR;
    if (keyCode == DVRPL_KeyCode_PgDown)        return VK_NEXT;
    if (keyCode == DVRPL_KeyCode_ArrowUp)       return VK_UP;
    if (keyCode == DVRPL_KeyCode_ArrowDown)     return VK_DOWN;
    if (keyCode == DVRPL_KeyCode_ArrowLeft)     return VK_LEFT;
    if (keyCode == DVRPL_KeyCode_ArrowRight)    return VK_RIGHT;
    if (keyCode == DVRPL_KeyCode_Alt)           return VK_MENU;
    if (keyCode == DVRPL_KeyCode_Control)       return VK_CONTROL;
    if (keyCode == DVRPL_KeyCode_Shift)         return VK_SHIFT;
    if (keyCode == DVRPL_KeyCode_Backspace)     return VK_BACK;
    if (keyCode == DVRPL_KeyCode_Delete)        return VK_DELETE;
    if (keyCode == DVRPL_KeyCode_Insert)        return VK_INSERT;
    if (keyCode == DVRPL_KeyCode_Escape)        return VK_ESCAPE;
    if (keyCode == DVRPL_KeyCode_Tab)           return VK_TAB;
    if (keyCode == DVRPL_KeyCode_Enter)         return VK_RETURN;

    if (keyCode == (DVRPL_KeyCode) ';')         return VK_OEM_1;
    if (keyCode == (DVRPL_KeyCode) '/')         return VK_OEM_2;
    if (keyCode == (DVRPL_KeyCode) '`')         return VK_OEM_3;
    if (keyCode == (DVRPL_KeyCode) '[')         return VK_OEM_4;
    if (keyCode == (DVRPL_KeyCode) '\\')        return VK_OEM_5;
    if (keyCode == (DVRPL_KeyCode) ']')         return VK_OEM_6;
    if (keyCode == (DVRPL_KeyCode) '\'')        return VK_OEM_7;

    if (keyCode >= DVRPL_KeyCode_F1 && keyCode <= DVRPL_KeyCode_F12)
        return VK_F1 + (i32)(keyCode - DVRPL_KeyCode_F1);

    if (keyCode == DVRPL_KeyCode_PrtScrn)       return VK_SNAPSHOT;

    return 0;
}

static b8 DVRPL_Internal_InitialiseInputSystem(void)
{
    if (G_DVRPL_Internal_InputSystemInitialised)
        return true;

    G_DVRPL_Internal_Events           = PNSLR_MakeSlice(DVRPL_Event, 32, true, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
    G_DVRPL_Internal_RawInputBuffer   = PNSLR_MakeSlice(u8,          64, true, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
    G_DVRPL_Internal_KeysDown         = PNSLR_MakeSlice(i32,         64, true, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

    G_DVRPL_Internal_NumEvents         = 0;
    G_DVRPL_Internal_NumRawInputBuffer = 0;
    G_DVRPL_Internal_NumKeysDown       = 0;

    // Register raw input devices
    RAWINPUTDEVICE rid[2] = {0};

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage     = HID_USAGE_GENERIC_MOUSE;
    rid[0].dwFlags     = 0;
    rid[0].hwndTarget  = nil;

    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
    rid[1].dwFlags     = 0;
    rid[1].hwndTarget  = nil;

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
        return false;

    // Disable sticky keys, toggle keys, and filter keys

    STICKYKEYS sk;
    SystemParametersInfoW(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
    if ((sk.dwFlags & SKF_STICKYKEYSON) == 0)
    {
        sk.dwFlags &= ~SKF_HOTKEYACTIVE;
        sk.dwFlags &= ~SKF_CONFIRMHOTKEY;
        sk.dwFlags &= ~SKF_HOTKEYSOUND;

        SystemParametersInfoW(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
    }

    TOGGLEKEYS tk;
    SystemParametersInfoW(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
    if ((tk.dwFlags & TKF_TOGGLEKEYSON) == 0)
    {
        tk.dwFlags &= ~TKF_HOTKEYACTIVE;
        tk.dwFlags &= ~TKF_CONFIRMHOTKEY;
        tk.dwFlags &= ~TKF_HOTKEYSOUND;

        SystemParametersInfoW(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
    }

    FILTERKEYS fk;
    SystemParametersInfoW(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
    if ((fk.dwFlags & FKF_FILTERKEYSON) == 0)
    {
        fk.dwFlags &= ~FKF_HOTKEYACTIVE;
        fk.dwFlags &= ~FKF_CONFIRMHOTKEY;
        fk.dwFlags &= ~FKF_HOTKEYSOUND;

        SystemParametersInfoW(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
    }

    G_DVRPL_Internal_InputSystemInitialised = true;
    return true;
}

static b8 DVRPL_Internal_SetKeyDownState(i32 vKey, b8 down)
{
    b8 wasDown = false;
    i64 foundIndex = -1;
    for (i64 i = 0; i < G_DVRPL_Internal_NumKeysDown; i++)
    {
        if (G_DVRPL_Internal_KeysDown.data[i] == vKey)
        {
            wasDown = true;
            foundIndex = i;
            break;
        }
    }

    if (down && !wasDown)
    {
        // Add to table

        if (G_DVRPL_Internal_NumKeysDown >= G_DVRPL_Internal_KeysDown.count)
        {
            i64 newCount = (G_DVRPL_Internal_KeysDown.count ? (G_DVRPL_Internal_KeysDown.count * 2) : 16);
            PNSLR_ResizeSlice(i32, &G_DVRPL_Internal_KeysDown, newCount, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
        }

        if (G_DVRPL_Internal_NumKeysDown < G_DVRPL_Internal_KeysDown.count)
        {
            G_DVRPL_Internal_KeysDown.data[G_DVRPL_Internal_NumKeysDown] = vKey;
            G_DVRPL_Internal_NumKeysDown++;
        }
    }
    else if (!down && wasDown)
    {
        // Remove from table
        if (foundIndex != -1)
        {
            for (i64 i = foundIndex; i < (G_DVRPL_Internal_NumKeysDown - 1); i++)
                G_DVRPL_Internal_KeysDown.data[i] = G_DVRPL_Internal_KeysDown.data[i + 1];

            G_DVRPL_Internal_NumKeysDown--;
        }
    }

    return wasDown;
}

static void DVRPL_Internal_SendKeyEvent(HWND wnd, DVRPL_KeyCode key, b8 down, b8 repeat)
{
    if (key == DVRPL_KeyCode_Alt)
    {
        if (down) G_DVRPL_Internal_CachedModifierStates |=  DVRPL_KeyModifier_Alt;
        else      G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_Alt;
    }

    if (key == DVRPL_KeyCode_Control)
    {
        if (down) G_DVRPL_Internal_CachedModifierStates |=  DVRPL_KeyModifier_Control;
        else      G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_Control;
    }

    if (key == DVRPL_KeyCode_Shift)
    {
        if (down) G_DVRPL_Internal_CachedModifierStates |=  DVRPL_KeyModifier_Shift;
        else      G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_Shift;
    }

    // Add event
    DVRPL_Internal_ResizeEventsIfBufferFull();

    if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
    {
        DVRPL_Event evt =
        {
            .ty           = DVRPL_EvtTy_Keyboard,
            .keyStatus    = (down ? DVRPL_KeyStatus_Pressed : DVRPL_KeyStatus_Released),
            .keyModifiers = G_DVRPL_Internal_CachedModifierStates,
            .repeat       = repeat,
            .keyCode      = key,
            .windowId     = DVRPL_MAKE_WINDOW_HANDLE(wnd),
        };

        G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
        G_DVRPL_Internal_NumEvents++;
    }

    // Update key states
    if (down) G_DVRPL_Internal_KeyStates[key] |= (DVRPL_KeyState_Pressed | DVRPL_KeyState_Held);
    else      G_DVRPL_Internal_KeyStates[key] |= DVRPL_KeyState_Released;
}

static void DVRPL_Internal_SendKeyEventIfRequired(HWND wnd, i32 vKey, b8 down, b8 repeat)
{
    b8 wasDown = DVRPL_Internal_SetKeyDownState(vKey, down);
    if (!down && !wasDown) // redundant release
        return;

    b8 repeatReal = repeat;
    if (down && repeat && !wasDown)
    {
        // key was pressed while we didn't have focus
        repeatReal = false;
    }

    DVRPL_Internal_SendKeyEvent(wnd, DVRPL_Internal_GetKeyCode(vKey), down, repeatReal);
}

static void DVRPL_Internal_AddResizeEvent(HWND wnd)
{
    DVRPL_WindowResizeData* resize = nil;

    // Find existing resize for this window
    for (i64 i = 0; i < G_DVRPL_Internal_NumTempResizes; i++)
    {
        if (DVRPL_BREAK_WINDOW_HANDLE(G_DVRPL_Internal_TempResizes.data[i].id) == wnd)
        {
            resize = &(G_DVRPL_Internal_TempResizes.data[i]);
            break;
        }
    }

    // add new data if none was found
    if (resize == nil)
    {
        if (G_DVRPL_Internal_NumTempResizes >= G_DVRPL_Internal_TempResizes.count)
        {
            i64 newCount = (G_DVRPL_Internal_TempResizes.count ? (G_DVRPL_Internal_TempResizes.count * 2) : 16);
            PNSLR_ResizeSlice(
                DVRPL_WindowResizeData,
                &G_DVRPL_Internal_TempResizes,
                newCount,
                false,
                G_DVRPL_Internal_CurrentTempAllocator,
                PNSLR_GET_LOC(),
                nil
            );
        }

        if (G_DVRPL_Internal_NumTempResizes < G_DVRPL_Internal_TempResizes.count)
        {
            resize = &(G_DVRPL_Internal_TempResizes.data[G_DVRPL_Internal_NumTempResizes]);
            G_DVRPL_Internal_NumTempResizes++;
        }
    }

    // set info
    if (resize != nil)
    {
        RECT clientRect;
        if (GetClientRect(wnd, &clientRect))
        {
            resize->id    = DVRPL_MAKE_WINDOW_HANDLE(wnd);
            resize->sizeX = (u16) (clientRect.right);
            resize->sizeY = (u16) (clientRect.bottom);
        }
    }
}

static void DVRPL_Internal_AddMoveEvent(HWND wnd)
{
    DVRPL_WindowMoveData* move = nil;

    // Find existing move for this window
    for (i64 i = 0; i < G_DVRPL_Internal_NumTempMoves; i++)
    {
        if (DVRPL_BREAK_WINDOW_HANDLE(G_DVRPL_Internal_TempMoves.data[i].id) == wnd)
        {
            move = &(G_DVRPL_Internal_TempMoves.data[i]);
            break;
        }
    }

    // add new data if none was found
    if (move == nil)
    {
        if (G_DVRPL_Internal_NumTempMoves >= G_DVRPL_Internal_TempMoves.count)
        {
            i64 newCount = (G_DVRPL_Internal_TempMoves.count ? (G_DVRPL_Internal_TempMoves.count * 2) : 16);
            PNSLR_ResizeSlice(
                DVRPL_WindowMoveData,
                &G_DVRPL_Internal_TempMoves,
                newCount,
                false,
                G_DVRPL_Internal_CurrentTempAllocator,
                PNSLR_GET_LOC(),
                nil
            );
        }

        if (G_DVRPL_Internal_NumTempMoves < G_DVRPL_Internal_TempMoves.count)
        {
            move = &(G_DVRPL_Internal_TempMoves.data[G_DVRPL_Internal_NumTempMoves]);
            G_DVRPL_Internal_NumTempMoves++;
        }
    }

    // set info
    if (move != nil)
    {
        RECT clientRect;
        if (GetClientRect(wnd, &clientRect))
        {
            move->id   = DVRPL_MAKE_WINDOW_HANDLE(wnd);
            move->posX = (i16) (clientRect.left);
            move->posY = (i16) (clientRect.top);
        }
    }
}

static void DVRPL_ProcessRawInput(HWND wnd, HRAWINPUT handle)
{
    UINT dwSize;
    GetRawInputData(handle, RID_INPUT, nil, &dwSize, sizeof(RAWINPUTHEADER));

    if (G_DVRPL_Internal_NumRawInputBuffer < (i64) dwSize)
        PNSLR_ResizeSlice(
            u8,
            &G_DVRPL_Internal_RawInputBuffer,
            dwSize,
            false,
            PNSLR_GetAllocator_DefaultHeap(),
            PNSLR_GET_LOC(),
            nil
        );

    UINT writtenBytes = GetRawInputData(
        handle,
        RID_INPUT,
        G_DVRPL_Internal_RawInputBuffer.data,
        &dwSize,
        sizeof(RAWINPUTHEADER)
    );

    if (writtenBytes > dwSize) {
        return; // buffer overflow
    }

    RAWINPUT* raw = (RAWINPUT*) G_DVRPL_Internal_RawInputBuffer.data;

    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        RAWMOUSE* mouse = &raw->data.mouse;

        if (!(mouse->usFlags & MOUSE_MOVE_ABSOLUTE))
        {
            G_DVRPL_Internal_MouseDelta[0] += mouse->lLastX;
            G_DVRPL_Internal_MouseDelta[1] += mouse->lLastY;
        }
    }
    else if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        b8 isDown = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
        i32 vKey = raw->data.keyboard.VKey;

        if (vKey == VK_SNAPSHOT)
        {
            DVRPL_KeyCode keyCode = DVRPL_Internal_GetKeyCode(vKey);
            b8 repeat = isDown && !!(G_DVRPL_Internal_KeyStates[keyCode] & DVRPL_KeyState_Held);

            DVRPL_Internal_SendKeyEventIfRequired(wnd, vKey, isDown, repeat);
        }
    }
}

static LRESULT CALLBACK DVRPL_Internal_WindowsInputCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_SYSCOMMAND:
            return DefWindowProcW(wnd, msg, wParam, lParam);

        case WM_ACTIVATEAPP:
            if (wParam != 0)
            {
                for (i64 i = (G_DVRPL_Internal_NumKeysDown - 1); i >= 0; i--)
                {
                    i32 vKey = G_DVRPL_Internal_KeysDown.data[i];

                    SHORT state = GetAsyncKeyState(vKey);
                    if (!(state >> 15)) // is released now
                    {
                        DVRPL_Internal_SendKeyEvent(wnd, DVRPL_Internal_GetKeyCode(vKey), false, false);

                        // remove the key
                        for (i64 j = i; j < (G_DVRPL_Internal_NumKeysDown - 1); j++)
                            G_DVRPL_Internal_KeysDown.data[j] = G_DVRPL_Internal_KeysDown.data[j + 1];

                        G_DVRPL_Internal_NumKeysDown--;
                    }
                }
            }

            return DefWindowProcW(wnd, msg, wParam, lParam);

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            DVRPL_Internal_SendKeyEventIfRequired(wnd, (i32) wParam, true, !!(((i32) lParam) & 0x40000000));
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
            DVRPL_Internal_SendKeyEventIfRequired(wnd, (i32) wParam, false, false);
            break;

        case WM_SYSCHAR: break; // Prevent beeps for Alt key combos

        case WM_CHAR:
            if (wParam > 31)
            {
                DVRPL_Internal_ResizeEventsIfBufferFull();

                if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
                {
                    DVRPL_Event evt =
                    {
                        .ty        = DVRPL_EvtTy_TextInput,
                        .utf32Char = (u32) wParam,
                        .windowId  = DVRPL_MAKE_WINDOW_HANDLE(wnd),
                    };

                    G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                    G_DVRPL_Internal_NumEvents++;
                }
            }
            break;

        case WM_SETFOCUS:  G_DVRPL_Internal_AppHasFocus = true;  break;
        case WM_KILLFOCUS: G_DVRPL_Internal_AppHasFocus = false; break;

        case WM_PAINT:
            ValidateRect(wnd, nil);
            return DefWindowProcW(wnd, msg, wParam, lParam);

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            DVRPL_Internal_SendKeyEventIfRequired(wnd, VK_LBUTTON, msg == WM_LBUTTONDOWN, false);
            if (msg == WM_LBUTTONDOWN) SetCapture(wnd); else ReleaseCapture();
            break;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            DVRPL_Internal_SendKeyEventIfRequired(wnd, VK_RBUTTON, msg == WM_RBUTTONDOWN, false);
            if (msg == WM_RBUTTONDOWN) SetCapture(wnd); else ReleaseCapture();
            break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            DVRPL_Internal_SendKeyEventIfRequired(wnd, VK_MBUTTON, msg == WM_MBUTTONDOWN, false);
            if (msg == WM_MBUTTONDOWN) SetCapture(wnd); else ReleaseCapture();
            break;

        case WM_MOUSEMOVE: break; // Using raw input for this

        case WM_MOUSEWHEEL:
            DVRPL_Internal_ResizeEventsIfBufferFull();
            if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
            {
                DVRPL_Event evt =
                {
                    .ty = DVRPL_EvtTy_MouseWheel,
                    .rawWheelData = 120,
                    .wheelData = (i32) (wParam >> 16),
                    .windowId = DVRPL_MAKE_WINDOW_HANDLE(wnd),
                };

                G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                G_DVRPL_Internal_NumEvents++;
            }

            G_DVRPL_Internal_MouseDelta[2] += (i32) (wParam >> 16);
            break;

        case WM_CLOSE:
        case WM_DESTROY:
            DVRPL_Internal_ResizeEventsIfBufferFull();
            if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
            {
                DVRPL_Event evt = {.ty = DVRPL_EvtTy_Quit, .windowId = DVRPL_MAKE_WINDOW_HANDLE(wnd)};

                G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                G_DVRPL_Internal_NumEvents++;
            }

            return DefWindowProcW(wnd, msg, wParam, lParam);

        case WM_INPUT:
            DVRPL_ProcessRawInput(wnd, (HRAWINPUT) lParam);
            return DefWindowProcW(wnd, msg, wParam, lParam);

        case WM_SIZE:
            switch (wParam)
            {
                case SIZE_MAXIMIZED:
                    G_DVRPL_Internal_WindowMinimised = false;
                    DVRPL_Internal_AddResizeEvent(wnd);
                    break;

                case SIZE_RESTORED:
                    if (G_DVRPL_Internal_WindowMinimised) G_DVRPL_Internal_WindowMinimised = false;
                    else                                  DVRPL_Internal_AddResizeEvent(wnd);
                    break;

                case SIZE_MINIMIZED:
                    G_DVRPL_Internal_WindowMinimised = true;
                    break;
            }

            break;

        case WM_MOVE:         DVRPL_Internal_AddMoveEvent(wnd);   break;
        case WM_EXITSIZEMOVE: DVRPL_Internal_AddResizeEvent(wnd); break;

        case WM_DPICHANGED:
        {
            RECT* rect = (RECT*) lParam;
            LONG w = rect->right  - rect->left;
            LONG h = rect->bottom - rect->top;
            SetWindowPos(wnd, HWND_TOPMOST, rect->left, rect->top, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
            break;
        }

        case WM_DROPFILES:
        {
            HDROP drop = (HDROP)wParam;
            UINT filePathsCount = DragQueryFileW(drop, 0xFFFFFFFF, nil, 0);

            if (filePathsCount > 0)
            {
                // Ensure we have enough space for dropped files
                if (G_DVRPL_Internal_TempDroppedFiles.count < (i64) filePathsCount)
                    PNSLR_ResizeSlice(
                        utf8str,
                        &G_DVRPL_Internal_TempDroppedFiles,
                        filePathsCount,
                        false,
                        G_DVRPL_Internal_CurrentTempAllocator,
                        PNSLR_GET_LOC(),
                        nil
                    );

                // failed to reallocate
                if (G_DVRPL_Internal_TempDroppedFiles.count < (i64) filePathsCount)
                    break;

                for (UINT i = 0; i < filePathsCount; i++)
                {
                    // +2 for null terminator and any trailing '\0'
                    UINT fileNameWideCount = DragQueryFileW(drop, i, nil, 0) + 2;
                    if (fileNameWideCount == 0) continue;

                    PNSLR_ArraySlice(u16) fileNameWide = PNSLR_MakeSlice(
                        u16,
                        fileNameWideCount, // +1 for null terminator
                        false,
                        G_DVRPL_Internal_CurrentTempAllocator,
                        PNSLR_GET_LOC(),
                        nil
                    );

                    UINT ok = DragQueryFileW(drop, i, (LPWSTR) fileNameWide.data, fileNameWideCount);
                    if (ok > 0)
                    {
                        utf8str fileName = PNSLR_UTF8FromUTF16WindowsOnly(fileNameWide, G_DVRPL_Internal_CurrentTempAllocator);

                        if (G_DVRPL_Internal_NumTempDroppedFiles < G_DVRPL_Internal_TempDroppedFiles.count)
                        {
                            G_DVRPL_Internal_TempDroppedFiles.data[G_DVRPL_Internal_NumTempDroppedFiles] = fileName;

                            // Add drop file event
                            DVRPL_Internal_ResizeEventsIfBufferFull();
                            if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
                            {
                                DVRPL_Event evt =
                                {
                                    .ty            = DVRPL_EvtTy_DropFile,
                                    .droppedFileId = (u16) G_DVRPL_Internal_NumTempDroppedFiles,
                                    .windowId      = DVRPL_MAKE_WINDOW_HANDLE(wnd),
                                };

                                G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                                G_DVRPL_Internal_NumEvents++;
                            }

                            G_DVRPL_Internal_NumTempDroppedFiles++;
                        }
                    }

                    PNSLR_FreeSlice(
                        &fileNameWide,
                        G_DVRPL_Internal_CurrentTempAllocator,
                        PNSLR_GET_LOC(),
                        nil
                    );
                }
            }

            DragFinish(drop);
            break;
        }

        default:
            return DefWindowProcW(wnd, msg, wParam, lParam);
    }

    return 0;
}

#elif PNSLR_ANDROID

static void DVRPL_Internal_AddResizeEvent(ANativeWindow* window)
{
    if (!window)
        return;

    DVRPL_WindowResizeData* resize = nil;

    // Find existing resize for this window
    for (i64 i = 0; i < G_DVRPL_Internal_NumTempResizes; i++)
    {
        if (DVRPL_BREAK_WINDOW_HANDLE(G_DVRPL_Internal_TempResizes.data[i].id) ==
            G_DVRPL_Internal_AndroidApp->window)
        {
            resize = &(G_DVRPL_Internal_TempResizes.data[i]);
            break;
        }
    }

    // add new data if none was found
    if (resize == nil)
    {
        if (G_DVRPL_Internal_NumTempResizes >= G_DVRPL_Internal_TempResizes.count)
        {
            i64 newCount = (G_DVRPL_Internal_TempResizes.count ? (G_DVRPL_Internal_TempResizes.count * 2) : 16);
            PNSLR_ResizeSlice(
                DVRPL_WindowResizeData,
                &G_DVRPL_Internal_TempResizes,
                newCount,
                false,
                G_DVRPL_Internal_CurrentTempAllocator,
                PNSLR_GET_LOC(),
                nil
            );
        }

        if (G_DVRPL_Internal_NumTempResizes < G_DVRPL_Internal_TempResizes.count)
        {
            resize = &(G_DVRPL_Internal_TempResizes.data[G_DVRPL_Internal_NumTempResizes]);
            G_DVRPL_Internal_NumTempResizes++;
        }
    }

    // set info
    if (resize != nil)
    {
        resize->id    = DVRPL_MAKE_WINDOW_HANDLE(window);
        resize->sizeX = (u16) ANativeWindow_getWidth(window);
        resize->sizeY = (u16) ANativeWindow_getHeight(window);
    }
}

static b8 DVRPL_Internal_InitialiseInputSystem(void)
{
    if (G_DVRPL_Internal_InputSystemInitialised)
        return true;

    G_DVRPL_Internal_Events = PNSLR_MakeSlice(DVRPL_Event, 32, true, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
    G_DVRPL_Internal_NumEvents = 0;

    G_DVRPL_Internal_ResizeEventCalledOnce  = false;

    G_DVRPL_Internal_InputSystemInitialised = true;
    return true;
}

static void DVRPL_Internal_AndroidCommandCallback(struct android_app* app, int32_t cmd)
{
    utf8str log = {0};
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            G_DVRPL_Internal_InFocus = true;
            break;
        case APP_CMD_TERM_WINDOW:
            G_DVRPL_Internal_InFocus = false;
            break;
        case APP_CMD_GAINED_FOCUS:
            G_DVRPL_Internal_InFocus = true;
            break;
        case APP_CMD_LOST_FOCUS:
            G_DVRPL_Internal_InFocus = false;
            break;
        case APP_CMD_CONTENT_RECT_CHANGED: log = PNSLR_StringLiteral("CONTENT_RECT_CHANGED"); break;
        case APP_CMD_INPUT_CHANGED:        log = PNSLR_StringLiteral("INPUT_CHANGED");        break;
        case APP_CMD_WINDOW_RESIZED:       log = PNSLR_StringLiteral("WINDOW_RESIZED");       break;
        case APP_CMD_WINDOW_REDRAW_NEEDED: log = PNSLR_StringLiteral("WINDOW_REDRAW_NEEDED"); break;
        case APP_CMD_CONFIG_CHANGED:       log = PNSLR_StringLiteral("CONFIG_CHANGED");       break;
        case APP_CMD_LOW_MEMORY:           log = PNSLR_StringLiteral("LOW_MEMORY");           break;
        case APP_CMD_START:                log = PNSLR_StringLiteral("START");                break;
        case APP_CMD_RESUME:               log = PNSLR_StringLiteral("RESUME");               break;
        case APP_CMD_SAVE_STATE:           log = PNSLR_StringLiteral("SAVE_STATE");           break;
        case APP_CMD_PAUSE:                log = PNSLR_StringLiteral("PAUSE");                break;
        case APP_CMD_STOP:                 log = PNSLR_StringLiteral("STOP");                 break;
        case APP_CMD_DESTROY:              log = PNSLR_StringLiteral("DESTROY");              break;
        default:                           log = PNSLR_StringLiteral("UNKNOWN");              break;
    }

    if (!!log.data && !!log.count)
    {
        __android_log_print(ANDROID_LOG_WARN, "DVRPL_Input", "Unhandled Android command: %.*s", (i32) log.count, log.data);
    }
}

static int32_t DVRPL_Internal_AndroidInputCallback(struct android_app* app, AInputEvent* event)
{
    b8 handled = false;

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        int32_t actionAndPtrIdx = AMotionEvent_getAction(event);
        int32_t action = actionAndPtrIdx & AMOTION_EVENT_ACTION_MASK;
        int32_t ptrIdx = (actionAndPtrIdx & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                            >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        int32_t ptrId = AMotionEvent_getPointerId(event, (size_t) ptrIdx);
        int32_t pointerCount = (int32_t) AMotionEvent_getPointerCount(event);

        float primaryX = AMotionEvent_getX(event, (size_t) ptrIdx);
        float primaryY = AMotionEvent_getY(event, (size_t) ptrIdx);

        // first touch is a down/up; subsequent will be pointer down/up
        switch (action)
        {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
            {
                G_DVRPL_Internal_AndroidPointers[ptrId].state = DVRPL_KeyState_Pressed | DVRPL_KeyState_Held;
                G_DVRPL_Internal_AndroidPointers[ptrId].posX = primaryX;
                G_DVRPL_Internal_AndroidPointers[ptrId].posY = primaryY;

                handled = true;

                DVRPL_Internal_ResizeEventsIfBufferFull();
                if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
                {
                    DVRPL_Event evt =
                    {
                        .ty          = DVRPL_EvtTy_Touch,
                        .windowId    = DVRPL_MAKE_WINDOW_HANDLE(app->window),
                        .touchId     = (u8) ptrId,
                        .touchStatus = DVRPL_TouchStatus_Pressed,
                    };

                    G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                    G_DVRPL_Internal_NumEvents++;
                }

                break;
            }
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
            {
                G_DVRPL_Internal_AndroidPointers[ptrId].state = DVRPL_KeyState_Released;
                G_DVRPL_Internal_AndroidPointers[ptrId].posX = primaryX;
                G_DVRPL_Internal_AndroidPointers[ptrId].posY = primaryY;

                handled = true;

                DVRPL_Internal_ResizeEventsIfBufferFull();
                if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
                {
                    DVRPL_Event evt =
                    {
                        .ty          = DVRPL_EvtTy_Touch,
                        .windowId    = DVRPL_MAKE_WINDOW_HANDLE(app->window),
                        .touchId     = (u8) ptrId,
                        .touchStatus = DVRPL_TouchStatus_Released,
                    };

                    G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                    G_DVRPL_Internal_NumEvents++;
                }

                break;
            }

            case AMOTION_EVENT_ACTION_MOVE:
            {
                for (i32 i = 0; i < pointerCount; i++)
                {
                    ptrId = AMotionEvent_getPointerId(event, (size_t) i);

                    DVRPL_Internal_AndroidPtrInfo* ptr = &(G_DVRPL_Internal_AndroidPointers[ptrId]);
                    if (ptr->state & DVRPL_KeyState_Held)
                    {
                        ptr->posX = AMotionEvent_getX(event, (size_t) i);
                        ptr->posY = AMotionEvent_getY(event, (size_t) i);
                    }
                }

                handled = true;

                DVRPL_Internal_ResizeEventsIfBufferFull();
                if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
                {
                    DVRPL_Event evt =
                    {
                        .ty          = DVRPL_EvtTy_Touch,
                        .windowId    = DVRPL_MAKE_WINDOW_HANDLE(app->window),
                        .touchId     = (u8) ptrId,
                        .touchStatus = DVRPL_TouchStatus_Moved,
                    };

                    G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                    G_DVRPL_Internal_NumEvents++;
                }

                break;
            }

            default:
                __android_log_print(ANDROID_LOG_WARN, "DVRPL_Input", "Unhandled Android motion event action: %d", action);
                break;
        }
    }

    return (int32_t) handled;
}

static void DVRPL_Internal_AndroidSetApp(struct android_app* app)
{
    if (G_DVRPL_Internal_AndroidApp)
    {
        G_DVRPL_Internal_AndroidApp->onAppCmd = nil;
        G_DVRPL_Internal_AndroidApp->onInputEvent = nil;
    }

    G_DVRPL_Internal_AndroidApp = app;
    if (app)
    {
        app->onAppCmd = DVRPL_Internal_AndroidCommandCallback;
        app->onInputEvent = DVRPL_Internal_AndroidInputCallback;
    }
}

static void DVRPL_Internal_ProcessEvents(void)
{
    while (true)
    {
        i32 ident = 0, evts = 0;
        struct android_poll_source* source = nil;
        i32 timeout = G_DVRPL_Internal_InFocus ? 0 : -1;

        // Process all available events
        do {
            ident = ALooper_pollOnce(timeout, nil, &evts, (void**) &source);
            if (ident < 0) break;

            if (source)
            {
                source->process(G_DVRPL_Internal_AndroidApp, source);

                if (!!G_DVRPL_Internal_AndroidApp->destroyRequested)
                {
                    DVRPL_Event evt =
                    {
                        .ty       = DVRPL_EvtTy_Quit,
                        .windowId = DVRPL_MAKE_WINDOW_HANDLE(G_DVRPL_Internal_AndroidApp->window),
                    };

                    DVRPL_Internal_ResizeEventsIfBufferFull();
                    if (G_DVRPL_Internal_NumEvents < G_DVRPL_Internal_Events.count)
                    {
                        G_DVRPL_Internal_Events.data[G_DVRPL_Internal_NumEvents] = evt;
                        G_DVRPL_Internal_NumEvents++;
                    }
                }
            }

            // After first poll, use 0 timeout for remaining events
            timeout = 0;

        } while (ident >= 0);

        if (ident < 0) break;
    }
}

static void DVRPL_Internal_FlushEventsTillInFocus(struct android_app* app)
{
    if (!app) return;
    DVRPL_Internal_AndroidSetApp(app);
    while (!G_DVRPL_Internal_InFocus)
    {
        DVRPL_Internal_ProcessEvents();
    }
    DVRPL_Internal_AndroidSetApp(nil);
}

#elif PNSLR_APPLE

    void DVRPL_Internal_AppleResizeEventsIfBufferFull(void) { DVRPL_Internal_ResizeEventsIfBufferFull(); }
    void DVRPL_Internal_AppleClearExistingInputData(void) { DVRPL_Internal_ClearExistingInputData(); }
    void DVRPL_Internal_AppleGatherEvents(
        PNSLR_Allocator* currTempAllocator,
        PNSLR_ArraySlice(utf8str)* tempDroppedFiles,
        i64* numTempDroppedFiles,
        PNSLR_ArraySlice(DVRPL_WindowResizeData)* tempResizes,
        i64* numTempResizes,
        PNSLR_ArraySlice(DVRPL_WindowMoveData)* tempMoves,
        i64* numTempMoves,
        PNSLR_ArraySlice(DVRPL_Event)* events,
        i64* numEvents,
        i32* mouseDeltas,
        DVRPL_KeyState* keyStates,
        b8* appHasFocus,
        PNSLR_Allocator tempAllocator
    );

#else
    #error "Unimplemented platform for input system."
#endif

// public platform-unspecific functions ============================================

b8 DVRPL_IterateEvents(i64* iterator, DVRPL_Event* val)
{
    if (!iterator)
    {
        if (val) *val = (DVRPL_Event) {0};
        return false;
    }

    if (*iterator >= G_DVRPL_Internal_NumEvents)
    {
        *iterator = I64_MAX; // invalidate iterator
        if (val) *val = (DVRPL_Event) {0};
        return false;
    }

    if (val) *val = G_DVRPL_Internal_Events.data[*iterator];
    (*iterator)++;
    return true;
}

DVRPL_KeyState DVRPL_GetKeyState(DVRPL_KeyCode key)
{
    if (key >= (i32) DVRPL_KeyCode_NUM) return DVRPL_KeyState_None;

    return G_DVRPL_Internal_KeyStates[key];
}

void DVRPL_GetMouseDelta(i32* deltaX, i32* deltaY, i32* deltaScroll)
{
    if (deltaX)      *deltaX      = G_DVRPL_Internal_MouseDelta[0];
    if (deltaY)      *deltaY      = G_DVRPL_Internal_MouseDelta[1];
    if (deltaScroll) *deltaScroll = G_DVRPL_Internal_MouseDelta[2];
}

b8 DVRPL_DoesApplicationHaveFocus(void)
{
    return G_DVRPL_Internal_AppHasFocus;
}

utf8str DVRPL_GetDroppedFile(u16 fileId)
{
    if (fileId >= (u16) G_DVRPL_Internal_NumTempDroppedFiles || !G_DVRPL_Internal_TempDroppedFiles.data)
        return (utf8str) {0};

    return G_DVRPL_Internal_TempDroppedFiles.data[fileId];
}

b8 DVRPL_IterateResizeEvent(i32* iterator, DVRPL_WindowResizeData* val)
{
    if (!iterator)
    {
        if (val) *val = (DVRPL_WindowResizeData) {0};
        return false;
    }

    if (*iterator >= G_DVRPL_Internal_NumTempResizes)
    {
        PNSLR_FreeSlice(&G_DVRPL_Internal_TempResizes, G_DVRPL_Internal_CurrentTempAllocator, PNSLR_GET_LOC(), nil);
        G_DVRPL_Internal_NumTempResizes = 0;

        if (val) *val = (DVRPL_WindowResizeData) {0};
        return false;
    }

    if (val) *val = G_DVRPL_Internal_TempResizes.data[*iterator];
    (*iterator)++;
    return true;
}

b8 DVRPL_IterateMoveEvent(i32* iterator, DVRPL_WindowMoveData* val)
{
    if (!iterator)
    {
        if (val) *val = (DVRPL_WindowMoveData) {0};
        return false;
    }

    if (*iterator >= G_DVRPL_Internal_NumTempMoves)
    {
        PNSLR_FreeSlice(&G_DVRPL_Internal_TempMoves, G_DVRPL_Internal_CurrentTempAllocator, PNSLR_GET_LOC(), nil);
        G_DVRPL_Internal_NumTempMoves = 0;

        if (val) *val = (DVRPL_WindowMoveData) {0};
        return false;
    }

    if (val) *val = G_DVRPL_Internal_TempMoves.data[*iterator];
    (*iterator)++;
    return true;
}

void DVRPL_GatherEvents(PNSLR_Allocator tempAllocator)
{
    #if PNSLR_WINDOWS
    {
        if (!DVRPL_Internal_InitialiseInputSystem())
            return;

        DVRPL_Internal_ClearExistingInputData();
        G_DVRPL_Internal_CurrentTempAllocator = tempAllocator;

        DVRPL_KeyModifier modifiers[3] = {DVRPL_KeyModifier_Alt, DVRPL_KeyModifier_Control, DVRPL_KeyModifier_Shift};
        DVRPL_KeyCode      keycodes[3] = {DVRPL_KeyCode_Alt,     DVRPL_KeyCode_Control,     DVRPL_KeyCode_Shift    };
        i32                 vkCodes[3] = {VK_MENU,               VK_CONTROL,                VK_SHIFT               };
        for (i32 i = 0; i < 3; i++)
        {
            if ((G_DVRPL_Internal_CachedModifierStates & modifiers[i]) ||
                (G_DVRPL_Internal_KeyStates[keycodes[i]] & DVRPL_KeyState_Held))
            {
                SHORT state = GetAsyncKeyState(vkCodes[i]);
                if (!(state >> 15)) // released now
                {
                    G_DVRPL_Internal_CachedModifierStates &= ~(modifiers[i]);
                    G_DVRPL_Internal_KeyStates[keycodes[i]] |= DVRPL_KeyState_Released;
                }
            }
        }

        // Process Windows messages
        MSG msg = {0};
        while (PeekMessageW(&msg, nil, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    #elif PNSLR_ANDROID
    {
        if (!G_DVRPL_Internal_AndroidApp)
            return;

        if (!DVRPL_Internal_InitialiseInputSystem())
            return;

        DVRPL_Internal_ClearExistingInputData();
        G_DVRPL_Internal_CurrentTempAllocator = tempAllocator;

        if (!G_DVRPL_Internal_ResizeEventCalledOnce)
        {
            DVRPL_Internal_AddResizeEvent(G_DVRPL_Internal_AndroidApp->window);
            G_DVRPL_Internal_ResizeEventCalledOnce = true;
        }

        DVRPL_Internal_ProcessEvents();
    }
    #elif PNSLR_APPLE
    {
        DVRPL_Internal_AppleGatherEvents(
            &G_DVRPL_Internal_CurrentTempAllocator,
            &G_DVRPL_Internal_TempDroppedFiles,
            &G_DVRPL_Internal_NumTempDroppedFiles,
            &G_DVRPL_Internal_TempResizes,
            &G_DVRPL_Internal_NumTempResizes,
            &G_DVRPL_Internal_TempMoves,
            &G_DVRPL_Internal_NumTempMoves,
            &G_DVRPL_Internal_Events,
            &G_DVRPL_Internal_NumEvents,
            G_DVRPL_Internal_MouseDelta,
            G_DVRPL_Internal_KeyStates,
            &G_DVRPL_Internal_AppHasFocus,
            tempAllocator
        );
    }
    #endif
}
