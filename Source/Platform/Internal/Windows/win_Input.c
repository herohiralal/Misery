#include "Platform/Input.h"
#include "Platform/Window.h"
#include "win_Platform.h"

#if MSR_WINDOWS

static INP_KeyCode INP_Internal_ToKeyCode(i32 vKey)
{
    if (vKey >= '0' && vKey <= 'Z')
        return (INP_KeyCode) vKey;

    if (vKey == VK_LBUTTON)  return INP_KC_MouseBtnLeft;
    if (vKey == VK_MBUTTON)  return INP_KC_MouseBtnMiddle;
    if (vKey == VK_RBUTTON)  return INP_KC_MouseBtnRight;
    if (vKey == VK_SPACE)    return INP_KC_Space;
    if (vKey == VK_HOME)     return INP_KC_Home;
    if (vKey == VK_END)      return INP_KC_End;
    if (vKey == VK_PAUSE)    return INP_KC_Pause;
    if (vKey == VK_SCROLL)   return INP_KC_ScrollLock;
    if (vKey == VK_PRIOR)    return INP_KC_PgUp;
    if (vKey == VK_NEXT)     return INP_KC_PgDown;
    if (vKey == VK_UP)       return INP_KC_ArrowUp;
    if (vKey == VK_DOWN)     return INP_KC_ArrowDown;
    if (vKey == VK_LEFT)     return INP_KC_ArrowLeft;
    if (vKey == VK_RIGHT)    return INP_KC_ArrowRight;
    if (vKey == VK_MENU)     return INP_KC_Alt;
    if (vKey == VK_CONTROL)  return INP_KC_Control;
    if (vKey == VK_SHIFT)    return INP_KC_Shift;
    if (vKey == VK_BACK)     return INP_KC_Backspace;
    if (vKey == VK_DELETE)   return INP_KC_Delete;
    if (vKey == VK_INSERT)   return INP_KC_Insert;
    if (vKey == VK_ESCAPE)   return INP_KC_Escape;
    if (vKey == VK_TAB)      return INP_KC_Tab;
    if (vKey == VK_RETURN)   return INP_KC_Enter;

    if (vKey == VK_OEM_1)    return (INP_KeyCode) ';';
    if (vKey == VK_OEM_2)    return (INP_KeyCode) '/';
    if (vKey == VK_OEM_3)    return (INP_KeyCode) '`';
    if (vKey == VK_OEM_4)    return (INP_KeyCode) '[';
    if (vKey == VK_OEM_5)    return (INP_KeyCode) '\\';
    if (vKey == VK_OEM_6)    return (INP_KeyCode) ']';
    if (vKey == VK_OEM_7)    return (INP_KeyCode) '\'';

    if (vKey >= VK_F1 && vKey <= VK_F12)
        return INP_KC_F1 + (INP_KeyCode)(vKey - VK_F1);

    if (vKey == VK_SNAPSHOT) return INP_KC_PrtScrn;

    return INP_KC_Unknown;
}

static i32 INP_Internal_FromKeyCode(INP_KeyCode keyCode)
{
    if (keyCode >= (INP_KeyCode) '0' && keyCode <= (INP_KeyCode) 'Z')
        return (i32) keyCode;

    if (keyCode == INP_KC_MouseBtnLeft)   return VK_LBUTTON;
    if (keyCode == INP_KC_MouseBtnMiddle) return VK_MBUTTON;
    if (keyCode == INP_KC_MouseBtnRight)  return VK_RBUTTON;

    if (keyCode == (INP_KeyCode) ' ') return VK_SPACE;
    if (keyCode == INP_KC_Home)       return VK_HOME;
    if (keyCode == INP_KC_End)        return VK_END;
    if (keyCode == INP_KC_Pause)      return VK_PAUSE;
    if (keyCode == INP_KC_ScrollLock) return VK_SCROLL;
    if (keyCode == INP_KC_PgUp)       return VK_PRIOR;
    if (keyCode == INP_KC_PgDown)     return VK_NEXT;
    if (keyCode == INP_KC_ArrowUp)    return VK_UP;
    if (keyCode == INP_KC_ArrowDown)  return VK_DOWN;
    if (keyCode == INP_KC_ArrowLeft)  return VK_LEFT;
    if (keyCode == INP_KC_ArrowRight) return VK_RIGHT;
    if (keyCode == INP_KC_Alt)        return VK_MENU;
    if (keyCode == INP_KC_Control)    return VK_CONTROL;
    if (keyCode == INP_KC_Shift)      return VK_SHIFT;
    if (keyCode == INP_KC_Backspace)  return VK_BACK;
    if (keyCode == INP_KC_Delete)     return VK_DELETE;
    if (keyCode == INP_KC_Insert)     return VK_INSERT;
    if (keyCode == INP_KC_Escape)     return VK_ESCAPE;
    if (keyCode == INP_KC_Tab)        return VK_TAB;
    if (keyCode == INP_KC_Enter)      return VK_RETURN;

    if (keyCode == (INP_KeyCode) ';')  return VK_OEM_1;
    if (keyCode == (INP_KeyCode) '/')  return VK_OEM_2;
    if (keyCode == (INP_KeyCode) '`')  return VK_OEM_3;
    if (keyCode == (INP_KeyCode) '[')  return VK_OEM_4;
    if (keyCode == (INP_KeyCode) '\\') return VK_OEM_5;
    if (keyCode == (INP_KeyCode) ']')  return VK_OEM_6;
    if (keyCode == (INP_KeyCode) '\'') return VK_OEM_7;

    if (keyCode >= INP_KC_F1 && keyCode <= INP_KC_F12)
        return VK_F1 + (i32)(keyCode - INP_KC_F1);

    if (keyCode == INP_KC_PrtScrn)       return VK_SNAPSHOT;

    return 0;
}

static b8 INP_Internal_InitialiseInputSystem(void)
{
    INP_INTERNAL_STATE(state);

    if (state->inputSystemInitialised)
        return true;

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
    SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
    if ((sk.dwFlags & SKF_STICKYKEYSON) == 0)
    {
        sk.dwFlags &= ~SKF_HOTKEYACTIVE;
        sk.dwFlags &= ~SKF_CONFIRMHOTKEY;
        sk.dwFlags &= ~SKF_HOTKEYSOUND;

        SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
    }

    TOGGLEKEYS tk;
    SystemParametersInfoA(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
    if ((tk.dwFlags & TKF_TOGGLEKEYSON) == 0)
    {
        tk.dwFlags &= ~TKF_HOTKEYACTIVE;
        tk.dwFlags &= ~TKF_CONFIRMHOTKEY;
        tk.dwFlags &= ~TKF_HOTKEYSOUND;

        SystemParametersInfoA(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
    }

    FILTERKEYS fk;
    SystemParametersInfoA(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
    if ((fk.dwFlags & FKF_FILTERKEYSON) == 0)
    {
        fk.dwFlags &= ~FKF_HOTKEYACTIVE;
        fk.dwFlags &= ~FKF_CONFIRMHOTKEY;
        fk.dwFlags &= ~FKF_HOTKEYSOUND;

        SystemParametersInfoA(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
    }

    state->inputSystemInitialised = true;
    return true;
}

static b8 INP_Internal_SetKeyDownState(i32 vKey, b8 down)
{
    INP_INTERNAL_STATE(state);

    b8 wasDown = false;
    isize foundIndex = -1;
    for (isize i = 0; i < state->keysDown.count; i++)
    {
        if (state->keysDown.data[i] == vKey)
        {
            wasDown = true;
            foundIndex = i;
            break;
        }
    }

    if (down && !wasDown)
    {
        // Add to table
        COL_AppendToList(&(state->keysDown), vKey);
    }
    else if (!down && wasDown && foundIndex != -1)
    {
        // Remove from table
        COL_RemoveIdxFromList(&(state->keysDown), foundIndex);
    }

    return wasDown;
}

static void INP_Internal_SendKeyEvt(HWND wnd, INP_KeyCode key, b8 down, b8 repeat)
{
    INP_INTERNAL_STATE(state);

    if (key == INP_KC_Alt)
    {
        if (down) state->cachedModifierStates |=  INP_KM_Alt;
        else      state->cachedModifierStates &= ~INP_KM_Alt;
    }

    if (key == INP_KC_Control)
    {
        if (down) state->cachedModifierStates |=  INP_KM_Ctrl;
        else      state->cachedModifierStates &= ~INP_KM_Ctrl;
    }

    if (key == INP_KC_Shift)
    {
        if (down) state->cachedModifierStates |=  INP_KM_Shift;
        else      state->cachedModifierStates &= ~INP_KM_Shift;
    }

    INP_Evt evt =
    {
        .ty           = INP_Evt_Keyboard,
        .keyStatus    = (down ? INP_KS_Pressed : INP_KS_Released),
        .keyModifiers = state->cachedModifierStates,
        .repeat       = repeat,
        .keyCode      = key,
        .windowId     = WND_ToHandle(wnd),
    };

    COL_AppendToList(&(state->evts), evt);

    // Update key states
    if (down) state->keyStates.data[key] |= (INP_CKS_Pressed | INP_CKS_Held);
    else      state->keyStates.data[key] |= INP_CKS_Released;
}

static void INP_Internal_SendKeyEvtIfRequired(HWND wnd, i32 vKey, b8 down, b8 repeat)
{
    b8 wasDown = INP_Internal_SetKeyDownState(vKey, down);
    if (!down && !wasDown) // redundant release
        return;

    b8 repeatReal = repeat;
    if (down && repeat && !wasDown)
    {
        // key was pressed while we didn't have focus
        repeatReal = false;
    }

    INP_Internal_SendKeyEvt(wnd, INP_Internal_ToKeyCode(vKey), down, repeatReal);
}

static void INP_Internal_AddResizeEvt(HWND wnd)
{
    INP_INTERNAL_STATE(state);

    INP_WindowResizeData* resize = nil;

    // Find existing resize for this window
    WND_Handle compare = WND_ToHandle(wnd);
    for (isize i = 0; i < state->resizes.count; i++)
    {
        if (state->resizes.data[i].id.handle == compare.handle)
        {
            resize = &(state->resizes.data[i]);
            break;
        }
    }

    // add new data if none was found
    if (resize == nil)
    {
        COL_AppendToList(&(state->resizes), (INP_WindowResizeData) {0});
        resize = &(state->resizes.data[state->resizes.count - 1]);
    }

    // set info
    if (resize != nil)
    {
        RECT clientRect;
        if (GetClientRect(wnd, &clientRect))
        {
            resize->id    = compare;
            resize->sizeX = (u16) (clientRect.right);
            resize->sizeY = (u16) (clientRect.bottom);
        }
    }
}

static void INP_Internal_AddMoveEvt(HWND wnd)
{
    INP_INTERNAL_STATE(state);

    INP_WindowMoveData* move = nil;

    // Find existing move for this window
    WND_Handle compare = WND_ToHandle(wnd);
    for (isize i = 0; i < state->moves.count; i++)
    {
        if (state->moves.data[i].id.handle == compare.handle)
        {
            move = &(state->moves.data[i]);
            break;
        }
    }

    // add new data if none was found
    if (move == nil)
    {
        COL_AppendToList(&(state->moves), (INP_WindowMoveData) {0});
        move = &(state->moves.data[state->moves.count - 1]);
    }

    // set info
    if (move != nil)
    {
        RECT clientRect;
        if (GetClientRect(wnd, &clientRect))
        {
            move->id   = compare;
            move->posX = (i16) (clientRect.left);
            move->posY = (i16) (clientRect.top);
        }
    }
}

static void INP_Internal_ProcessRawInput(HWND wnd, HRAWINPUT handle)
{
    INP_INTERNAL_STATE(state);

    UINT dwSize;
    GetRawInputData(handle, RID_INPUT, nil, &dwSize, sizeof(RAWINPUTHEADER));

    if (state->rawInputBuffer.count < (isize) dwSize)
    {
        COL_ResizeList(&(state->rawInputBuffer), (isize) dwSize);
        state->rawInputBuffer.count = (isize) dwSize;
    }

    UINT writtenBytes = GetRawInputData(handle, RID_INPUT, state->rawInputBuffer.data, &dwSize, sizeof(RAWINPUTHEADER));

    if (writtenBytes > dwSize)
    {
        return; // buffer overflow
    }

    RAWINPUT* raw = (RAWINPUT*) state->rawInputBuffer.data;

    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        RAWMOUSE* mouse = &raw->data.mouse;

        if (!(mouse->usFlags & MOUSE_MOVE_ABSOLUTE))
        {
            state->mouseDelta[0] += mouse->lLastX;
            state->mouseDelta[1] += mouse->lLastY;
        }
    }
    else if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        b8 isDown = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
        i32 vKey = raw->data.keyboard.VKey;

        if (vKey == VK_SNAPSHOT)
        {
            INP_KeyCode keyCode = INP_Internal_ToKeyCode(vKey);
            b8 repeat = isDown && !!(state->keyStates.data[keyCode] & INP_CKS_Held);

            INP_Internal_SendKeyEvtIfRequired(wnd, vKey, isDown, repeat);
        }
    }
}

LRESULT CALLBACK INP_Internal_WindowsInputCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    INP_INTERNAL_STATE(state);

    switch (msg)
    {
        case WM_SYSCOMMAND:
            return DefWindowProcA(wnd, msg, wParam, lParam);

        case WM_ACTIVATEAPP:
            if (wParam != 0)
            {
                for (isize i = (state->keysDown.count - 1); i >= 0; i--)
                {
                    i32 vKey = state->keysDown.data[i];

                    SHORT asyncKeyState = GetAsyncKeyState(vKey);
                    if (!(asyncKeyState >> 15)) // is released now
                    {
                        INP_Internal_SendKeyEvt(wnd, INP_Internal_ToKeyCode(vKey), false, false);

                        COL_RemoveIdxFromList(&(state->keysDown), i);
                    }
                }
            }

            return DefWindowProcA(wnd, msg, wParam, lParam);

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            INP_Internal_SendKeyEvtIfRequired(wnd, (i32) wParam, true, !!(((i32) lParam) & 0x40000000));
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
            INP_Internal_SendKeyEvtIfRequired(wnd, (i32) wParam, false, false);
            break;

        case WM_SYSCHAR: break; // Prevent beeps for Alt key combos

        case WM_CHAR:
            if (wParam > 31)
            {
                INP_Evt evt =
                {
                    .ty        = INP_Evt_TextInput,
                    .utf32Char = (u32) wParam,
                    .windowId  = WND_ToHandle(wnd),
                };

                COL_AppendToList(&(state->evts), evt);
            }
            break;

        case WM_SETFOCUS:  state->appHasFocus = true;  break;
        case WM_KILLFOCUS: state->appHasFocus = false; break;

        case WM_PAINT:
            ValidateRect(wnd, nil);
            return DefWindowProcA(wnd, msg, wParam, lParam);

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            INP_Internal_SendKeyEvtIfRequired(wnd, VK_LBUTTON, msg == WM_LBUTTONDOWN, false);
            if (msg == WM_LBUTTONDOWN) SetCapture(wnd); else ReleaseCapture();
            break;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            INP_Internal_SendKeyEvtIfRequired(wnd, VK_RBUTTON, msg == WM_RBUTTONDOWN, false);
            if (msg == WM_RBUTTONDOWN) SetCapture(wnd); else ReleaseCapture();
            break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            INP_Internal_SendKeyEvtIfRequired(wnd, VK_MBUTTON, msg == WM_MBUTTONDOWN, false);
            if (msg == WM_MBUTTONDOWN) SetCapture(wnd); else ReleaseCapture();
            break;

        case WM_MOUSEMOVE: break; // Using raw input for this

        case WM_MOUSEWHEEL:
            if (true)
            {
                INP_Evt evt =
                {
                    .ty           = INP_Evt_MouseWheel,
                    .rawWheelData = 120,
                    .wheelData    = (i32) (wParam >> 16),
                    .windowId     = WND_ToHandle(wnd),
                };

                COL_AppendToList(&(state->evts), evt);

                state->mouseDelta[2] += (i32) (wParam >> 16);
            }
            break;

        case WM_CLOSE:
        case WM_DESTROY:
            if (true)
            {
                INP_Evt evt = {.ty = INP_Evt_Quit, .windowId = WND_ToHandle(wnd)};
                COL_AppendToList(&(state->evts), evt);
            }

            return DefWindowProcA(wnd, msg, wParam, lParam);

        case WM_INPUT:
            INP_Internal_ProcessRawInput(wnd, (HRAWINPUT) lParam);
            return DefWindowProcA(wnd, msg, wParam, lParam);

        case WM_SIZE:
            switch (wParam)
            {
                case SIZE_MAXIMIZED:
                    state->windowMinimised = false;
                    INP_Internal_AddResizeEvt(wnd);
                    break;

                case SIZE_RESTORED:
                    if (state->windowMinimised) state->windowMinimised = false;
                    else                        INP_Internal_AddResizeEvt(wnd);
                    break;

                case SIZE_MINIMIZED:
                    state->windowMinimised = true;
                    break;
            }
            break;

        case WM_MOVE:         INP_Internal_AddMoveEvt(wnd);   break;
        case WM_EXITSIZEMOVE: INP_Internal_AddResizeEvt(wnd); break;

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
            UINT filePathsCount = DragQueryFileA(drop, 0xFFFFFFFF, nil, 0);

            if (filePathsCount > 0)
            {
                if (state->droppedFiles.capacity < (isize) filePathsCount)
                    COL_ResizeList(&(state->droppedFiles), (isize) filePathsCount);

                MSR_ASSERT(state->droppedFiles.capacity >= (isize) filePathsCount && "Failed to resize dropped files buffer");

                for (UINT i = 0; i < filePathsCount; i++)
                {
                    // +2 for null terminator and any trailing '\0'
                    UINT fileNameLen = DragQueryFileA(drop, i, nil, 0) + 2;
                    if (fileNameLen == 0) continue;

                    utf8str fileName = COL_NewSlice(u8, fileNameLen, true, state->tempAllocator);
                    UINT ok = DragQueryFileA(drop, i, (PSTR) fileName.data, fileNameLen);
                    if (ok > 0)
                    {
                        COL_AppendToList(&(state->droppedFiles), fileName);

                        INP_Evt evt =
                        {
                            .ty            = INP_Evt_DropFile,
                            .droppedFileId = (u16) (state->droppedFiles.count - 1),
                            .windowId      = WND_ToHandle(wnd),
                        };

                        COL_AppendToList(&(state->evts), evt);
                    }
                }
            }

            DragFinish(drop);
            break;
        }

        default:
            return DefWindowProcA(wnd, msg, wParam, lParam);
    }

    return 0;
}

// public platform-unspecific functions ============================================

void INP_GatherEvts(void)
{
    if (!INP_Internal_InitialiseInputSystem())
        return;

    INP_Internal_ClearTempData();

    INP_INTERNAL_STATE(state);

    INP_KeyModifier modifiers[3] = {INP_KM_Alt, INP_KM_Ctrl,    INP_KM_Shift};
    INP_KeyCode      keycodes[3] = {INP_KC_Alt, INP_KC_Control, INP_KC_Shift};
    i32               vkCodes[3] = {VK_MENU,    VK_CONTROL,     VK_SHIFT    };
    for (i32 i = 0; i < 3; i++)
    {
        if ((state->cachedModifierStates & modifiers[i]) ||
            (state->keyStates.data[keycodes[i]] & INP_CKS_Held))
        {
            SHORT asyncKeyState = GetAsyncKeyState(vkCodes[i]);
            if (!(asyncKeyState >> 15)) // released now
            {
                state->cachedModifierStates &= ~(modifiers[i]);
                state->keyStates.data[keycodes[i]] |= INP_CKS_Released;
            }
        }
    }

    // Process Windows messages
    MSG msg = {0};
    while (PeekMessageA(&msg, nil, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}
#endif
