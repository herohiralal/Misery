#include "osx_Platform.h"

#if MSR_OSX

static const void* G_INP_OSXWindowDelegateAssocKey = &G_INP_OSXWindowDelegateAssocKey;
static usize G_INP_OSXPrevModifierFlags = 0;

static INP_KeyCode INP_Internal_KeyCodeFromMacKeyCode(u16 keyCode)
{
    switch (keyCode)
    {
        case 36:  return INP_KC_Enter;
        case 48:  return INP_KC_Tab;
        case 49:  return INP_KC_Space;
        case 51:  return INP_KC_Backspace;
        case 53:  return INP_KC_Escape;
        case 71:  return INP_KC_ScrollLock;
        case 113: return INP_KC_Pause;
        case 115: return INP_KC_Home;
        case 116: return INP_KC_PgUp;
        case 117: return INP_KC_Delete;
        case 119: return INP_KC_End;
        case 121: return INP_KC_PgDown;
        case 123: return INP_KC_ArrowLeft;
        case 124: return INP_KC_ArrowRight;
        case 125: return INP_KC_ArrowDown;
        case 126: return INP_KC_ArrowUp;

        case 122: return INP_KC_F1;
        case 120: return INP_KC_F2;
        case 99:  return INP_KC_F3;
        case 118: return INP_KC_F4;
        case 96:  return INP_KC_F5;
        case 97:  return INP_KC_F6;
        case 98:  return INP_KC_F7;
        case 100: return INP_KC_F8;
        case 101: return INP_KC_F9;
        case 109: return INP_KC_F10;
        case 103: return INP_KC_F11;
        case 111: return INP_KC_F12;

        case 56:
        case 60:  return INP_KC_Shift;
        case 59:
        case 62:  return INP_KC_Control;
        case 58:
        case 61:  return INP_KC_Alt;
        case 55:
        case 54:  return INP_KC_Cmd;
    }

    return INP_KC_Unknown;
}

static INP_KeyCode INP_Internal_KeyCodeFromEvent(NSEvent* event)
{
    INP_KeyCode key = INP_Internal_KeyCodeFromMacKeyCode((u16) [event keyCode]);
    if (key != INP_KC_Unknown)
        return key;

    NSString* chars = [event charactersIgnoringModifiers];
    if ([chars length] > 0)
    {
        unichar c = [chars characterAtIndex:0];
        if (c >= 0x20 && c <= 0x7E)
        {
            if (c >= 'a' && c <= 'z')
                c = (unichar) (c - 32);
            return (INP_KeyCode) c;
        }
    }

    return INP_KC_Unknown;
}

static INP_KeyModifier INP_Internal_ModifiersFromFlags(usize flags)
{
    INP_KeyModifier m = INP_KM_None;
    if (flags & NSEventModifierFlagOption)  m |= INP_KM_Alt;
    if (flags & NSEventModifierFlagControl) m |= INP_KM_Ctrl;
    if (flags & NSEventModifierFlagShift)   m |= INP_KM_Shift;
    if (flags & NSEventModifierFlagCommand) m |= INP_KM_CmdOrMeta;
    return m;
}

static void INP_Internal_AddOrUpdateResizeEvt(NSWindow* wnd)
{
    INP_INTERNAL_STATE(state);

    WND_Handle wid = WND_ToHandle(wnd);
    INP_WindowResizeData* resize = nil;
    for (isize i = 0; i < state->resizes.count; ++i)
    {
        if (state->resizes.data[i].id.handle == wid.handle)
        {
            resize = &(state->resizes.data[i]);
            break;
        }
    }

    if (!resize)
    {
        COL_AppendToList(&(state->resizes), (INP_WindowResizeData) {0});
        resize = &(state->resizes.data[state->resizes.count - 1]);
    }

    NSRect frame = [wnd frame];
    NSRect contentRect = [wnd contentRectForFrameRect:frame];
    resize->id = wid;
    resize->sizeX = (u16) contentRect.size.width;
    resize->sizeY = (u16) contentRect.size.height;
}

static void INP_Internal_AddOrUpdateMoveEvt(NSWindow* wnd)
{
    INP_INTERNAL_STATE(state);

    WND_Handle wid = WND_ToHandle(wnd);
    INP_WindowMoveData* move = nil;
    for (isize i = 0; i < state->moves.count; ++i)
    {
        if (state->moves.data[i].id.handle == wid.handle)
        {
            move = &(state->moves.data[i]);
            break;
        }
    }

    if (!move)
    {
        COL_AppendToList(&(state->moves), (INP_WindowMoveData) {0});
        move = &(state->moves.data[state->moves.count - 1]);
    }

    NSRect frame = [wnd frame];
    move->id = wid;
    move->posX = (i16) frame.origin.x;
    move->posY = (i16) frame.origin.y;
}

static void INP_Internal_SendKeyEvt(WND_Handle wid, INP_KeyCode key, b8 down, b8 repeat, INP_KeyModifier mods)
{
    INP_INTERNAL_STATE(state);

    if (key == INP_KC_Unknown || key >= INP_KC_NUM)
        return;

    if (key == INP_KC_Alt)     { if (down) state->cachedModifierStates |= INP_KM_Alt;       else state->cachedModifierStates &= ~INP_KM_Alt; }
    if (key == INP_KC_Control) { if (down) state->cachedModifierStates |= INP_KM_Ctrl;      else state->cachedModifierStates &= ~INP_KM_Ctrl; }
    if (key == INP_KC_Shift)   { if (down) state->cachedModifierStates |= INP_KM_Shift;     else state->cachedModifierStates &= ~INP_KM_Shift; }
    if (key == INP_KC_Cmd)     { if (down) state->cachedModifierStates |= INP_KM_CmdOrMeta; else state->cachedModifierStates &= ~INP_KM_CmdOrMeta; }

    INP_Evt evt =
    {
        .ty = INP_Evt_Keyboard,
        .keyStatus = down ? INP_KS_Pressed : INP_KS_Released,
        .keyModifiers = (mods == INP_KM_None) ? state->cachedModifierStates : mods,
        .repeat = repeat,
        .keyCode = key,
        .windowId = wid,
    };

    COL_AppendToList(&(state->evts), evt);

    if (down) state->keyStates.data[key] |= (INP_CKS_Pressed | INP_CKS_Held);
    else      state->keyStates.data[key] |= INP_CKS_Released;
}

static void INP_Internal_ReleaseAllHeldKeys(WND_Handle wid)
{
    INP_INTERNAL_STATE(state);

    for (INP_KeyCode k = 0; k < INP_KC_NUM; ++k)
    {
        if (state->keyStates.data[k] & INP_CKS_Held)
            INP_Internal_SendKeyEvt(wid, (INP_KeyCode) k, false, false, INP_KM_None);
    }

    state->cachedModifierStates = INP_KM_None;
}

@interface MIS_OSXWindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation MIS_OSXWindowDelegate

- (BOOL)windowShouldClose:(id)sender
{
    INP_INTERNAL_STATE(state);

    NSWindow* wnd = (NSWindow*) sender;
    INP_Evt evt =
    {
        .ty = INP_Evt_Quit,
        .windowId = WND_ToHandle(wnd),
    };
    COL_AppendToList(&(state->evts), evt);

    return NO;
}

- (void)windowDidResize:(NSNotification*)notification
{
    NSWindow* wnd = [notification object];
    if (wnd) INP_Internal_AddOrUpdateResizeEvt(wnd);
}

- (void)windowDidMove:(NSNotification*)notification
{
    NSWindow* wnd = [notification object];
    if (wnd) INP_Internal_AddOrUpdateMoveEvt(wnd);
}

@end

void INP_Internal_OSXAttachWindowDelegate(NSWindow* window)
{
    if (!window)
        return;

    id existing = objc_getAssociatedObject(window, G_INP_OSXWindowDelegateAssocKey);
    if (existing)
    {
        [window setDelegate:existing];
        return;
    }

    MIS_OSXWindowDelegate* delegate = [[MIS_OSXWindowDelegate alloc] init];
    objc_setAssociatedObject(window, G_INP_OSXWindowDelegateAssocKey, delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [window setDelegate:delegate];
    [delegate release];
}

void INP_GatherEvts(void)
{
    INP_INTERNAL_STATE(state);

    for (INP_KeyCode k = 0; k < INP_KC_NUM; ++k)
    {
        INP_CurrentKeyState s = state->keyStates.data[k];
        s &= ~INP_CKS_Pressed;
        if (s & INP_CKS_Released)
            s &= ~(INP_CKS_Held | INP_CKS_Released);
        state->keyStates.data[k] = s;
    }

    INP_Internal_ClearTempData(state);

    NSApplication* app = [NSApplication sharedApplication];
    b8 hadFocus = state->appHasFocus;
    state->appHasFocus = [app isActive];

    if (hadFocus && !state->appHasFocus)
        INP_Internal_ReleaseAllHeldKeys((WND_Handle) {0});

    while (true)
    {
        NSEvent* event = [app nextEventMatchingMask:NSEventMaskAny
                                          untilDate:[NSDate distantPast]
                                             inMode:NSDefaultRunLoopMode
                                            dequeue:YES];
        if (!event)
            break;

        NSWindow* wndObj = [event window];
        WND_Handle wid = WND_ToHandle(wndObj);

        NSEventType eventType = [event type];

        if (eventType == NSEventTypeKeyDown)
        {
            INP_KeyCode key = INP_Internal_KeyCodeFromEvent(event);
            if (key != INP_KC_Unknown)
            {
                INP_KeyModifier mods = INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]);
                INP_Internal_SendKeyEvt(wid, key, true, [event isARepeat], mods);
            }

            NSString* chars = [event characters];
            if ([chars length] > 0)
            {
                unichar c = [chars characterAtIndex:0];
                if (c > 31 && c != 127)
                {
                    INP_Evt tevt =
                    {
                        .ty = INP_Evt_TextInput,
                        .utf32Char = (u32) c,
                        .windowId = wid,
                    };
                    COL_AppendToList(&(state->evts), tevt);
                }
            }
        }
        else if (eventType == NSEventTypeKeyUp)
        {
            INP_KeyCode key = INP_Internal_KeyCodeFromEvent(event);
            if (key != INP_KC_Unknown && (state->keyStates.data[key] & INP_CKS_Held))
            {
                INP_KeyModifier mods = INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]);
                INP_Internal_SendKeyEvt(wid, key, false, false, mods);
            }
        }
        else if (eventType == NSEventTypeFlagsChanged)
        {
            usize flags = (usize) [event modifierFlags];
            struct FlagToKey { usize mask; INP_KeyCode key; } map[] =
            {
                { NSEventModifierFlagShift,   INP_KC_Shift   },
                { NSEventModifierFlagControl, INP_KC_Control },
                { NSEventModifierFlagOption,  INP_KC_Alt     },
                { NSEventModifierFlagCommand, INP_KC_Cmd     },
            };

            INP_KeyModifier mods = INP_Internal_ModifiersFromFlags(flags);
            for (usize i = 0; i < (sizeof(map) / sizeof(map[0])); ++i)
            {
                b8 nowDown = !!(flags & map[i].mask);
                b8 wasDown = !!(G_INP_OSXPrevModifierFlags & map[i].mask);
                if (nowDown != wasDown)
                    INP_Internal_SendKeyEvt(wid, map[i].key, nowDown, false, mods);
            }

            G_INP_OSXPrevModifierFlags = flags;
        }
        else if (eventType == NSEventTypeLeftMouseDown)
        {
            INP_Internal_SendKeyEvt(wid, INP_KC_MouseBtnLeft, true, false, INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]));
        }
        else if (eventType == NSEventTypeLeftMouseUp)
        {
            if (state->keyStates.data[INP_KC_MouseBtnLeft] & INP_CKS_Held)
                INP_Internal_SendKeyEvt(wid, INP_KC_MouseBtnLeft, false, false, INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]));
        }
        else if (eventType == NSEventTypeRightMouseDown)
        {
            INP_Internal_SendKeyEvt(wid, INP_KC_MouseBtnRight, true, false, INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]));
        }
        else if (eventType == NSEventTypeRightMouseUp)
        {
            if (state->keyStates.data[INP_KC_MouseBtnRight] & INP_CKS_Held)
                INP_Internal_SendKeyEvt(wid, INP_KC_MouseBtnRight, false, false, INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]));
        }
        else if (eventType == NSEventTypeOtherMouseDown)
        {
            INP_Internal_SendKeyEvt(wid, INP_KC_MouseBtnMiddle, true, false, INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]));
        }
        else if (eventType == NSEventTypeOtherMouseUp)
        {
            if (state->keyStates.data[INP_KC_MouseBtnMiddle] & INP_CKS_Held)
                INP_Internal_SendKeyEvt(wid, INP_KC_MouseBtnMiddle, false, false, INP_Internal_ModifiersFromFlags((usize) [event modifierFlags]));
        }
        else if (eventType == NSEventTypeMouseMoved ||
                 eventType == NSEventTypeLeftMouseDragged ||
                 eventType == NSEventTypeRightMouseDragged ||
                 eventType == NSEventTypeOtherMouseDragged)
        {
            state->mouseDelta[0] += (i32) [event deltaX];
            state->mouseDelta[1] += (i32) [event deltaY];
        }
        else if (eventType == NSEventTypeScrollWheel)
        {
            i32 wheel = (i32) ([event scrollingDeltaY] * 120.0);
            INP_Evt wevt =
            {
                .ty = INP_Evt_MouseWheel,
                .rawWheelData = 120,
                .wheelData = wheel,
                .windowId = wid,
            };
            COL_AppendToList(&(state->evts), wevt);
            state->mouseDelta[2] += wheel;
        }

        [app sendEvent:event];
    }

    [app updateWindows];
}

#endif
