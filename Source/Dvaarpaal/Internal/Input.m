#include "Dvaarpaal/Input.h"

void DVRPL_Internal_AppleResizeEventsIfBufferFull(void);
void DVRPL_Internal_AppleClearExistingInputData(void);

#if PNSLR_OSX
    static NSApplication* G_DVRPL_Internal_OSXApp = nil;
    static b8 G_DVRPL_Internal_InputSystemInitialised = false;
    static DVRPL_KeyModifier G_DVRPL_Internal_CachedModifierStates = DVRPL_KeyModifier_None;
#endif

static void DVRPL_Internal_AppleSetApp(NSApplication* app)
{
    G_DVRPL_Internal_OSXApp = app;
}

static DVRPL_KeyCode DVRPL_Internal_GetKeyCodeFromNSEvent(NSEvent* event)
{
    // Use the keyCode for special keys, characters for regular keys
    unsigned short keyCode = [event keyCode];
    NSString* chars = [event charactersIgnoringModifiers];

    // Handle special keys first
    switch (keyCode)
    {
        case 36:  return DVRPL_KeyCode_Enter;
        case 48:  return DVRPL_KeyCode_Tab;
        case 49:  return DVRPL_KeyCode_Space;
        case 51:  return DVRPL_KeyCode_Backspace;
        case 53:  return DVRPL_KeyCode_Escape;
        case 71:  return DVRPL_KeyCode_ScrollLock;
        case 113: return DVRPL_KeyCode_Pause;
        case 115: return DVRPL_KeyCode_Home;
        case 116: return DVRPL_KeyCode_PgUp;
        case 117: return DVRPL_KeyCode_Delete;
        case 119: return DVRPL_KeyCode_End;
        case 121: return DVRPL_KeyCode_PgDown;
        case 123: return DVRPL_KeyCode_ArrowLeft;
        case 124: return DVRPL_KeyCode_ArrowRight;
        case 125: return DVRPL_KeyCode_ArrowDown;
        case 126: return DVRPL_KeyCode_ArrowUp;

        // Function keys
        case 122: return DVRPL_KeyCode_F1;
        case 120: return DVRPL_KeyCode_F2;
        case 99:  return DVRPL_KeyCode_F3;
        case 118: return DVRPL_KeyCode_F4;
        case 96:  return DVRPL_KeyCode_F5;
        case 97:  return DVRPL_KeyCode_F6;
        case 98:  return DVRPL_KeyCode_F7;
        case 100: return DVRPL_KeyCode_F8;
        case 101: return DVRPL_KeyCode_F9;
        case 109: return DVRPL_KeyCode_F10;
        case 103: return DVRPL_KeyCode_F11;
        case 111: return DVRPL_KeyCode_F12;

        // Modifiers
        case 56:  // Shift
        case 60:  // Right Shift
            return DVRPL_KeyCode_Shift;
        case 59:  // Control
        case 62:  // Right Control
            return DVRPL_KeyCode_Control;
        case 58:  // Alt/Option
        case 61:  // Right Alt/Option
            return DVRPL_KeyCode_Alt;
        case 55:  // Command
        case 54:  // Right Command
            return DVRPL_KeyCode_Cmd;
    }

    // Handle regular characters
    if ([chars length] > 0)
    {
        unichar ch = [chars characterAtIndex:0];

        // ASCII printable range
        if (ch >= 32 && ch <= 126)
        {
            // Convert to uppercase for letter keys
            if (ch >= 'a' && ch <= 'z')
            {
                return (DVRPL_KeyCode)(ch - 32);
            }

            return (DVRPL_KeyCode)ch;
        }
    }

    return DVRPL_KeyCode_Unknown;
}

static DVRPL_KeyModifier DVRPL_Internal_GetModifiersFromNSEvent(NSEvent* event)
{
    NSEventModifierFlags flags = [event modifierFlags];
    DVRPL_KeyModifier modifiers = DVRPL_KeyModifier_None;

    if (flags & NSEventModifierFlagOption)   modifiers |= DVRPL_KeyModifier_Alt;
    if (flags & NSEventModifierFlagControl)  modifiers |= DVRPL_KeyModifier_Control;
    if (flags & NSEventModifierFlagShift)    modifiers |= DVRPL_KeyModifier_Shift;
    if (flags & NSEventModifierFlagCommand)  modifiers |= DVRPL_KeyModifier_CmdOrMeta;

    return modifiers;
}

static void DVRPL_Internal_AddKeyEvent(
    NSEvent* event,
    b8 isDown,
    PNSLR_ArraySlice(DVRPL_Event)* events,
    i64* numEvents,
    DVRPL_KeyState* keyStates
)
{
    DVRPL_KeyCode keyCode = DVRPL_Internal_GetKeyCodeFromNSEvent(event);
    if (keyCode == DVRPL_KeyCode_Unknown) return;

    DVRPL_KeyModifier modifiers = DVRPL_Internal_GetModifiersFromNSEvent(event);
    b8 repeat = [event isARepeat];

    // Update cached modifier states
    if (keyCode == DVRPL_KeyCode_Alt)
    {
        if (isDown) G_DVRPL_Internal_CachedModifierStates |= DVRPL_KeyModifier_Alt;
        else G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_Alt;
    }
    if (keyCode == DVRPL_KeyCode_Control)
    {
        if (isDown) G_DVRPL_Internal_CachedModifierStates |= DVRPL_KeyModifier_Control;
        else G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_Control;
    }
    if (keyCode == DVRPL_KeyCode_Shift)
    {
        if (isDown) G_DVRPL_Internal_CachedModifierStates |= DVRPL_KeyModifier_Shift;
        else G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_Shift;
    }
    if (keyCode == DVRPL_KeyCode_Cmd)
    {
        if (isDown) G_DVRPL_Internal_CachedModifierStates |= DVRPL_KeyModifier_CmdOrMeta;
        else G_DVRPL_Internal_CachedModifierStates &= ~DVRPL_KeyModifier_CmdOrMeta;
    }

    // Add event
    DVRPL_Internal_AppleResizeEventsIfBufferFull();

    if (*numEvents < events->count)
    {
        NSWindow* keyWindow = [event window];

        DVRPL_Event evt =
        {
            .ty = DVRPL_EvtTy_Keyboard,
            .keyStatus = (isDown ? DVRPL_KeyStatus_Pressed : DVRPL_KeyStatus_Released),
            .keyModifiers = modifiers,
            .repeat = repeat,
            .keyCode = keyCode,
            .windowId = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)keyWindow),
        };

        events->data[*numEvents] = evt;
        (*numEvents)++;
    }

    // Update key states
    if (isDown)
    {
        keyStates[keyCode] |= (DVRPL_KeyState_Pressed | DVRPL_KeyState_Held);
    }
    else
    {
        keyStates[keyCode] |= DVRPL_KeyState_Released;
        keyStates[keyCode] &= ~DVRPL_KeyState_Held;
    }
}

static void DVRPL_Internal_AddMouseButtonEvent(
    NSEvent* event,
    DVRPL_KeyCode buttonCode,
    b8 isDown,
    PNSLR_ArraySlice(DVRPL_Event)* events,
    i64* numEvents,
    DVRPL_KeyState* keyStates
)
{
    DVRPL_KeyModifier modifiers = DVRPL_Internal_GetModifiersFromNSEvent(event);

    DVRPL_Internal_AppleResizeEventsIfBufferFull();

    if (*numEvents < events->count)
    {
        NSWindow* window = [event window];

        DVRPL_Event evt =
        {
            .ty = DVRPL_EvtTy_Keyboard,
            .keyStatus = (isDown ? DVRPL_KeyStatus_Pressed : DVRPL_KeyStatus_Released),
            .keyModifiers = modifiers,
            .repeat = false,
            .keyCode = buttonCode,
            .windowId = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)window),
        };

        events->data[*numEvents] = evt;
        (*numEvents)++;
    }

    // Update key states for mouse buttons
    if (isDown)
    {
        keyStates[buttonCode] |= (DVRPL_KeyState_Pressed | DVRPL_KeyState_Held);
    }
    else
    {
        keyStates[buttonCode] |= DVRPL_KeyState_Released;
        keyStates[buttonCode] &= ~DVRPL_KeyState_Held;
    }
}

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
)
{
    @autoreleasepool {
        if (!G_DVRPL_Internal_InputSystemInitialised)
        {
            if (!events->data)
            {
                *events = PNSLR_MakeSlice(DVRPL_Event, 32, true, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
            }
            G_DVRPL_Internal_InputSystemInitialised = true;
        }

        DVRPL_Internal_AppleClearExistingInputData();
        *currTempAllocator = tempAllocator;

        // Check if app has focus
        *appHasFocus = [G_DVRPL_Internal_OSXApp isActive];

        // Process all pending events
        while (true)
        {
            NSEvent* event = [G_DVRPL_Internal_OSXApp nextEventMatchingMask:NSEventMaskAny
                                                                  untilDate:[NSDate distantPast]
                                                                     inMode:NSDefaultRunLoopMode
                                                                    dequeue:YES];

            if (!event) break;

            NSEventType eventType = [event type];

            switch (eventType)
            {
                case NSEventTypeKeyDown:
                {
                    DVRPL_Internal_AddKeyEvent(event, true, events, numEvents, keyStates);

                    // Handle text input
                    NSString* chars = [event characters];
                    if ([chars length] > 0)
                    {
                        unichar ch = [chars characterAtIndex:0];
                        if (ch > 31 && ch != 127)
                        { // Printable character
                            DVRPL_Internal_AppleResizeEventsIfBufferFull();
                            if (*numEvents < events->count)
                            {
                                DVRPL_Event evt =
                                {
                                    .ty = DVRPL_EvtTy_TextInput,
                                    .utf32Char = (u32)ch,
                                    .windowId = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)[event window]),
                                };
                                events->data[*numEvents] = evt;
                                (*numEvents)++;
                            }
                        }
                    }
                    break;
                }

                case NSEventTypeKeyUp:
                    DVRPL_Internal_AddKeyEvent(event, false, events, numEvents, keyStates);
                    break;

                case NSEventTypeLeftMouseDown:
                    DVRPL_Internal_AddMouseButtonEvent(event, DVRPL_KeyCode_MouseBtnLeft, true, events, numEvents, keyStates);
                    break;

                case NSEventTypeLeftMouseUp:
                    DVRPL_Internal_AddMouseButtonEvent(event, DVRPL_KeyCode_MouseBtnLeft, false, events, numEvents, keyStates);
                    break;

                case NSEventTypeRightMouseDown:
                    DVRPL_Internal_AddMouseButtonEvent(event, DVRPL_KeyCode_MouseBtnRight, true, events, numEvents, keyStates);
                    break;

                case NSEventTypeRightMouseUp:
                    DVRPL_Internal_AddMouseButtonEvent(event, DVRPL_KeyCode_MouseBtnRight, false, events, numEvents, keyStates);
                    break;

                case NSEventTypeOtherMouseDown:
                    DVRPL_Internal_AddMouseButtonEvent(event, DVRPL_KeyCode_MouseBtnMiddle, true, events, numEvents, keyStates);
                    break;

                case NSEventTypeOtherMouseUp:
                    DVRPL_Internal_AddMouseButtonEvent(event, DVRPL_KeyCode_MouseBtnMiddle, false, events, numEvents, keyStates);
                    break;

                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged:
                {
                    CGFloat deltaX = [event deltaX];
                    CGFloat deltaY = [event deltaY];
                    mouseDeltas[0] += (i32)deltaX;
                    mouseDeltas[1] += (i32)deltaY;
                    break;
                }

                case NSEventTypeScrollWheel:
                {
                    CGFloat deltaY = [event scrollingDeltaY];
                    i32 wheelDelta = (i32)(deltaY * 120); // Match Windows wheel delta scale

                    DVRPL_Internal_AppleResizeEventsIfBufferFull();
                    if (*numEvents < events->count)
                    {
                        DVRPL_Event evt =
                        {
                            .ty = DVRPL_EvtTy_MouseWheel,
                            .rawWheelData = 120,
                            .wheelData = wheelDelta,
                            .windowId = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)[event window]),
                        };
                        events->data[*numEvents] = evt;
                        (*numEvents)++;
                    }

                    mouseDeltas[2] += wheelDelta;
                    break;
                }

                case NSEventTypeFlagsChanged:
                {
                    // Handle modifier key press/release
                    NSEventModifierFlags flags = [event modifierFlags];
                    static NSEventModifierFlags previousFlags = 0;

                    // Check each modifier
                    if ((flags & NSEventModifierFlagShift) != (previousFlags & NSEventModifierFlagShift))
                    {
                        b8 isDown = !!(flags & NSEventModifierFlagShift);
                        DVRPL_Internal_AddKeyEvent(event, isDown, events, numEvents, keyStates);
                    }
                    if ((flags & NSEventModifierFlagControl) != (previousFlags & NSEventModifierFlagControl))
                    {
                        b8 isDown = !!(flags & NSEventModifierFlagControl);
                        DVRPL_Internal_AddKeyEvent(event, isDown, events, numEvents, keyStates);
                    }
                    if ((flags & NSEventModifierFlagOption) != (previousFlags & NSEventModifierFlagOption))
                    {
                        b8 isDown = !!(flags & NSEventModifierFlagOption);
                        DVRPL_Internal_AddKeyEvent(event, isDown, events, numEvents, keyStates);
                    }
                    if ((flags & NSEventModifierFlagCommand) != (previousFlags & NSEventModifierFlagCommand))
                    {
                        b8 isDown = !!(flags & NSEventModifierFlagCommand);
                        DVRPL_Internal_AddKeyEvent(event, isDown, events, numEvents, keyStates);
                    }

                    previousFlags = flags;
                    break;
                }

                default:
                    break;
            }

            // Send event to the application for default handling
            [G_DVRPL_Internal_OSXApp sendEvent:event];
        }

        // Check for window events (resize/move)
        NSArray* windows = [G_DVRPL_Internal_OSXApp windows];
        for (NSWindow* window in windows)
        {
            static NSMutableDictionary* previousFrames = nil;
            if (!previousFrames)
            {
                previousFrames = [[NSMutableDictionary alloc] init];
            }

            NSValue* windowKey = [NSValue valueWithPointer:(__bridge void*)window];
            NSRect currentFrame = [window frame];
            NSValue* currentFrameValue = [NSValue valueWithRect:currentFrame];
            NSValue* previousFrameValue = [previousFrames objectForKey:windowKey];

            if (!previousFrameValue || ![previousFrameValue isEqualToValue:currentFrameValue])
            {
                NSRect previousFrame = previousFrameValue ? [previousFrameValue rectValue] : currentFrame;

                // Check for resize
                if (currentFrame.size.width != previousFrame.size.width ||
                    currentFrame.size.height != previousFrame.size.height)
                {

                    // Find or add resize event
                    DVRPL_WindowResizeData* resize = nil;
                    for (i64 i = 0; i < *numTempResizes; i++)
                    {
                        if (DVRPL_BREAK_WINDOW_HANDLE(tempResizes->data[i].id) == (__bridge rawptr)window)
                        {
                            resize = &(tempResizes->data[i]);
                            break;
                        }
                    }

                    if (!resize)
                    {
                        if (*numTempResizes >= tempResizes->count)
                        {
                            i64 newCount = (tempResizes->count ? (tempResizes->count * 2) : 16);
                            PNSLR_ResizeSlice(
                                DVRPL_WindowResizeData,
                                tempResizes,
                                newCount,
                                false,
                                tempAllocator,
                                PNSLR_GET_LOC(),
                                nil
                            );
                        }

                        if (*numTempResizes < tempResizes->count)
                        {
                            resize = &(tempResizes->data[*numTempResizes]);
                            (*numTempResizes)++;
                        }
                    }

                    if (resize)
                    {
                        NSRect contentRect = [window contentRectForFrameRect:currentFrame];
                        resize->id = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)window);
                        resize->sizeX = (u16)contentRect.size.width;
                        resize->sizeY = (u16)contentRect.size.height;
                    }
                }

                // Check for move
                if (currentFrame.origin.x != previousFrame.origin.x ||
                    currentFrame.origin.y != previousFrame.origin.y)
                    {

                    // Find or add move event
                    DVRPL_WindowMoveData* move = nil;
                    for (i64 i = 0; i < *numTempMoves; i++)
                    {
                        if (DVRPL_BREAK_WINDOW_HANDLE(tempMoves->data[i].id) == (__bridge rawptr)window)
                        {
                            move = &(tempMoves->data[i]);
                            break;
                        }
                    }

                    if (!move)
                    {
                        if (*numTempMoves >= tempMoves->count)
                        {
                            i64 newCount = (tempMoves->count ? (tempMoves->count * 2) : 16);
                            PNSLR_ResizeSlice(
                                DVRPL_WindowMoveData,
                                tempMoves,
                                newCount,
                                false,
                                tempAllocator,
                                PNSLR_GET_LOC(),
                                nil
                            );
                        }

                        if (*numTempMoves < tempMoves->count)
                        {
                            move = &(tempMoves->data[*numTempMoves]);
                            (*numTempMoves)++;
                        }
                    }

                    if (move)
                    {
                        move->id = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)window);
                        move->posX = (i16)currentFrame.origin.x;
                        move->posY = (i16)currentFrame.origin.y;
                    }
                }

                // Update stored frame
                [previousFrames setObject:currentFrameValue forKey:windowKey];
            }

            // Check if window should close
            if ([window shouldClose])
            {
                DVRPL_Internal_AppleResizeEventsIfBufferFull();
                if (*numEvents < events->count)
                {
                    DVRPL_Event evt =
                    {
                        .ty = DVRPL_EvtTy_Quit,
                        .windowId = DVRPL_MAKE_WINDOW_HANDLE((__bridge rawptr)window),
                    };
                    events->data[*numEvents] = evt;
                    (*numEvents)++;
                }
            }
        }

        [G_DVRPL_Internal_OSXApp updateWindows];
    }
}

