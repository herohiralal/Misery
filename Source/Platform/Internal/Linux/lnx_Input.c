#include "lnx_Platform.h"

#if MSR_LINUX

// ─── Key mapping ─────────────────────────────────────────────────────────────

static xcb_key_symbols_t* G_INP_KeySymbols = nil;

static xcb_key_symbols_t* INP_GetKeySymbols(void)
{
    if (!G_INP_KeySymbols)
    {
        WND_XCBContext* ctx = WND_GetXCBContext();
        if (ctx)
            G_INP_KeySymbols = xcb_key_symbols_alloc(ctx->connection);
    }
    return G_INP_KeySymbols;
}

static INP_KeyCode INP_KeySymToKeyCode(xcb_keysym_t ks)
{
    // ASCII printable: pass through
    if (ks >= 0x20 && ks <= 0x7E)
    {
        // normalise lowercase letters to uppercase to match INP_KC convention
        if (ks >= 'a' && ks <= 'z') return (INP_KeyCode)(ks - 32);
        return (INP_KeyCode) ks;
    }

    switch (ks)
    {
        case XK_BackSpace:  return INP_KC_Backspace;
        case XK_Tab:        return INP_KC_Tab;
        case XK_Return:     return INP_KC_Enter;
        case XK_Escape:     return INP_KC_Escape;
        case XK_Delete:     return INP_KC_Delete;
        case XK_Up:         return INP_KC_ArrowUp;
        case XK_Down:       return INP_KC_ArrowDown;
        case XK_Left:       return INP_KC_ArrowLeft;
        case XK_Right:      return INP_KC_ArrowRight;
        case XK_Page_Up:    return INP_KC_PgUp;
        case XK_Page_Down:  return INP_KC_PgDown;
        case XK_Home:       return INP_KC_Home;
        case XK_End:        return INP_KC_End;
        case XK_Insert:     return INP_KC_Insert;
        case XK_Pause:      return INP_KC_Pause;
        case XK_Scroll_Lock:return INP_KC_ScrollLock;
        case XK_Alt_L:
        case XK_Alt_R:      return INP_KC_Alt;
        case XK_Control_L:
        case XK_Control_R:  return INP_KC_Control;
        case XK_Shift_L:
        case XK_Shift_R:    return INP_KC_Shift;
        case XK_Super_L:
        case XK_Super_R:    return INP_KC_Cmd;
        case XK_Print:      return INP_KC_PrtScrn;
        case XK_F1:         return INP_KC_F1;
        case XK_F2:         return INP_KC_F2;
        case XK_F3:         return INP_KC_F3;
        case XK_F4:         return INP_KC_F4;
        case XK_F5:         return INP_KC_F5;
        case XK_F6:         return INP_KC_F6;
        case XK_F7:         return INP_KC_F7;
        case XK_F8:         return INP_KC_F8;
        case XK_F9:         return INP_KC_F9;
        case XK_F10:        return INP_KC_F10;
        case XK_F11:        return INP_KC_F11;
        case XK_F12:        return INP_KC_F12;
        default:            return INP_KC_Unknown;
    }
}

// ─── Modifier tracking ───────────────────────────────────────────────────────

static INP_KeyModifier INP_XCBStateToModifiers(u16 state)
{
    INP_KeyModifier m = INP_KM_None;
    if (state & XCB_MOD_MASK_SHIFT)   m |= INP_KM_Shift;
    if (state & XCB_MOD_MASK_CONTROL) m |= INP_KM_Ctrl;
    if (state & XCB_MOD_MASK_1)       m |= INP_KM_Alt;   // Mod1 = Alt
    if (state & XCB_MOD_MASK_4)       m |= INP_KM_CmdOrMeta; // Mod4 = Super
    return m;
}

// ─── Key helpers ─────────────────────────────────────────────────────────────

static void INP_SendKeyEvt(
    INP_Internal_State* state,
    WND_Handle          windowId,
    INP_KeyCode         key,
    b8                  down,
    b8                  repeat,
    INP_KeyModifier     mods)
{
    if (key == INP_KC_Alt)     { if (down) state->cachedModifierStates |= INP_KM_Alt;       else state->cachedModifierStates &= ~INP_KM_Alt;       }
    if (key == INP_KC_Control) { if (down) state->cachedModifierStates |= INP_KM_Ctrl;      else state->cachedModifierStates &= ~INP_KM_Ctrl;      }
    if (key == INP_KC_Shift)   { if (down) state->cachedModifierStates |= INP_KM_Shift;     else state->cachedModifierStates &= ~INP_KM_Shift;     }
    if (key == INP_KC_Cmd)     { if (down) state->cachedModifierStates |= INP_KM_CmdOrMeta; else state->cachedModifierStates &= ~INP_KM_CmdOrMeta; }

    INP_Evt evt =
    {
        .ty           = INP_Evt_Keyboard,
        .keyStatus    = down ? INP_KS_Pressed : INP_KS_Released,
        .keyModifiers = state->cachedModifierStates,
        .repeat       = repeat,
        .keyCode      = key,
        .windowId     = windowId,
    };

    COL_AppendToList(&(state->evts), evt);

    if (key < INP_KC_NUM)
    {
        if (down) state->keyStates.data[key] |= (INP_CKS_Pressed | INP_CKS_Held);
        else      state->keyStates.data[key] |= INP_CKS_Released;
    }
}

// ─── Event processing ────────────────────────────────────────────────────────

void INP_Internal_ProcessXCBEvents(void)
{
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return;

    INP_INTERNAL_STATE(state);
    xcb_connection_t* c = ctx->connection;
    xcb_key_symbols_t* syms = INP_GetKeySymbols();

    xcb_generic_event_t* ev;
    while ((ev = xcb_poll_for_event(c)) != nil)
    {
        u8 evType = ev->response_type & ~0x80;

        switch (evType)
        {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE:
            {
                xcb_key_press_event_t* kev = (xcb_key_press_event_t*) ev;
                b8 down   = (evType == XCB_KEY_PRESS);
                b8 repeat = false;

                xcb_keysym_t ks = syms
                    ? xcb_key_symbols_get_keysym(syms, kev->detail, 0)
                    : XCB_NO_SYMBOL;

                INP_KeyCode key = INP_KeySymToKeyCode(ks);
                INP_KeyModifier mods = INP_XCBStateToModifiers(kev->state);

                // Detect repeat: key press while already held
                if (down && key < INP_KC_NUM)
                    repeat = !!(state->keyStates.data[key] & INP_CKS_Held);

                WND_Handle wid = { .handle = (usize) kev->event };
                INP_SendKeyEvt(state, wid, key, down, repeat, mods);

                // Text input for printable keys on press only
                if (down && !repeat && ks >= 0x20 && ks <= 0x7E)
                {
                    INP_Evt tevt =
                    {
                        .ty        = INP_Evt_TextInput,
                        .utf32Char = (u32) ks,
                        .windowId  = wid,
                    };
                    COL_AppendToList(&(state->evts), tevt);
                }
                break;
            }

            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_press_event_t* bev = (xcb_button_press_event_t*) ev;
                b8 down = (evType == XCB_BUTTON_PRESS);
                WND_Handle wid = { .handle = (usize) bev->event };
                INP_KeyModifier mods = INP_XCBStateToModifiers(bev->state);

                INP_KeyCode key = INP_KC_Unknown;
                switch (bev->detail)
                {
                    case 1: key = INP_KC_MouseBtnLeft;   break;
                    case 2: key = INP_KC_MouseBtnMiddle; break;
                    case 3: key = INP_KC_MouseBtnRight;  break;

                    // Scroll wheel: buttons 4/5 are discrete scroll events
                    case 4:
                    case 5:
                        if (down) // only fire on press, not release
                        {
                            i32 delta = (bev->detail == 4) ? 120 : -120;
                            INP_Evt wevt =
                            {
                                .ty           = INP_Evt_MouseWheel,
                                .rawWheelData = 120,
                                .wheelData    = delta,
                                .windowId     = wid,
                            };
                            COL_AppendToList(&(state->evts), wevt);
                            state->mouseDelta[2] += delta;
                        }
                        key = (bev->detail == 4) ? INP_KC_MouseWhlUp : INP_KC_MouseWhlDown;
                        break;

                    default: break;
                }

                if (key != INP_KC_Unknown && key != INP_KC_MouseWhlUp && key != INP_KC_MouseWhlDown)
                    INP_SendKeyEvt(state, wid, key, down, false, mods);

                break;
            }

            case XCB_FOCUS_IN:
                state->appHasFocus = true;
                break;

            case XCB_FOCUS_OUT:
                state->appHasFocus = false;

                // Release all held keys on focus loss (mirrors WM_ACTIVATEAPP behaviour)
                for (INP_KeyCode k = 0; k < INP_KC_NUM; k++)
                {
                    if (state->keyStates.data[k] & INP_CKS_Held)
                    {
                        xcb_focus_out_event_t* fev = (xcb_focus_out_event_t*) ev;
                        WND_Handle wid = { .handle = (usize) fev->event };
                        INP_SendKeyEvt(state, wid, (INP_KeyCode) k, false, false, INP_KM_None);
                    }
                }
                state->cachedModifierStates = INP_KM_None;
                break;

            case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t* cev = (xcb_configure_notify_event_t*) ev;
                WND_Handle wid = { .handle = (usize) cev->window };

                // Resize
                {
                    INP_WindowResizeData* resize = nil;
                    for (isize i = 0; i < state->resizes.count; i++)
                        if (state->resizes.data[i].id.handle == wid.handle)
                            { resize = &state->resizes.data[i]; break; }

                    if (!resize)
                    {
                        COL_AppendToList(&(state->resizes), (INP_WindowResizeData) {0});
                        resize = &state->resizes.data[state->resizes.count - 1];
                    }

                    resize->id    = wid;
                    resize->sizeX = (u16) cev->width;
                    resize->sizeY = (u16) cev->height;
                }

                // Move
                {
                    INP_WindowMoveData* move = nil;
                    for (isize i = 0; i < state->moves.count; i++)
                        if (state->moves.data[i].id.handle == wid.handle)
                            { move = &state->moves.data[i]; break; }

                    if (!move)
                    {
                        COL_AppendToList(&(state->moves), (INP_WindowMoveData) {0});
                        move = &state->moves.data[state->moves.count - 1];
                    }

                    move->id   = wid;
                    move->posX = (i16) cev->x;
                    move->posY = (i16) cev->y;
                }
                break;
            }

            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* cmev = (xcb_client_message_event_t*) ev;
                if (cmev->type == ctx->WM_PROTOCOLS &&
                    cmev->data.data32[0] == ctx->WM_DELETE_WINDOW)
                {
                    WND_Handle wid = { .handle = (usize) cmev->window };
                    INP_Evt qevt = { .ty = INP_Evt_Quit, .windowId = wid };
                    COL_AppendToList(&(state->evts), qevt);
                }
                break;
            }

            case XCB_GE_GENERIC:
            {
                // XInput2 raw events (raw mouse motion)
                if (!ctx->xi2Available) { free(ev); continue; }

                xcb_ge_generic_event_t* gev = (xcb_ge_generic_event_t*) ev;
                if (gev->extension != ctx->xi2Opcode) break;

                if (gev->event_type == XCB_INPUT_RAW_MOTION)
                {
                    xcb_input_raw_button_press_event_t* rev = (xcb_input_raw_button_press_event_t*) ev;

                    u32* vmask = xcb_input_raw_button_press_valuator_mask(rev);
                    int maskLen = xcb_input_raw_button_press_valuator_mask_length(rev);
                    xcb_input_fp3232_t* vals = xcb_input_raw_button_press_axisvalues_raw(rev);

                    // Axis 0 = X, axis 1 = Y
                    i32 vidx = 0;
                    for (int mi = 0; mi < maskLen; mi++)
                    {
                        for (int bit = 0; bit < 32; bit++)
                        {
                            if (vmask[mi] & (1u << bit))
                            {
                                i32 intPart = vals[vidx].integral;
                                u32 fracPart = vals[vidx].frac;
                                (void) fracPart; // sub-pixel precision not needed

                                if      (mi == 0 && bit == 0) state->mouseDelta[0] += intPart;
                                else if (mi == 0 && bit == 1) state->mouseDelta[1] += intPart;

                                vidx++;
                            }
                        }
                    }
                }
                break;
            }

            default:
                break;
        }

        free(ev);
    }
}

// ─── INP_GatherEvts ──────────────────────────────────────────────────────────

void INP_GatherEvts(void)
{
    INP_INTERNAL_STATE(state);

    // Clear held flags from last frame; set Released for anything that was pressed
    for (INP_KeyCode k = 0; k < INP_KC_NUM; k++)
    {
        INP_CurrentKeyState s = state->keyStates.data[k];

        // Clear Pressed (it's only true the first frame a key goes down)
        s &= ~INP_CKS_Pressed;

        // If it was released last frame, clear Held and Released
        if (s & INP_CKS_Released)
            s &= ~(INP_CKS_Held | INP_CKS_Released);

        state->keyStates.data[k] = s;
    }

    INP_Internal_ClearTempData(state);

    INP_Internal_ProcessXCBEvents();
}

#endif
