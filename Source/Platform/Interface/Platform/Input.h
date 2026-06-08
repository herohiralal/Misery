#pragma once
#include <__init.h>
#include <Core/Core.h>
#include "Window.h"

EXTERN_C_BEGIN

/**
 * The type of input event that was logged.
 */
typedef u8 INP_EvtTy;
enum INP_EvtTys
{
    INP_Evt_Unknown = 0,
    INP_Evt_Keyboard,
    INP_Evt_MouseWheel,
    INP_Evt_Touch,
    INP_Evt_TextInput,
    INP_Evt_DropFile,
    INP_Evt_Quit,
};

/**
 * The information regarding a window move event.
 */
typedef struct
{
    WND_Handle id;
    i16 posX, posY;
} INP_WindowMoveData;

COL_DECLARE_FOR(INP_WindowMoveData);

/**
 * The information regarding a window resize event.
 */
typedef struct
{
    WND_Handle id;
    u16 sizeX, sizeY;
} INP_WindowResizeData;

COL_DECLARE_FOR(INP_WindowResizeData);

/**
 * The status of a touch event.
 */
typedef u8 INP_TouchStatus;
enum INP_TouchStatuses
{
    INP_TS_Moved,
    INP_TS_Pressed,
    INP_TS_Released,
};

// Technically can just bundle this into `DVRPL_EvtTy` but considering
// how people usually use input events, first they wanna know  which key
// it was, and then whether it was pressed or released, not specifically
// tracking downs/ups and then figuring out which key.

/**
 * The status of a key event.
 */
typedef u8 INP_KeyStatus;
enum INP_KeyStatuses
{
    INP_KS_Pressed,
    INP_KS_Released,
};

/**
 * The possible states of a key event.
 */
typedef u8 INP_CurrentKeyState;
enum INP_CurrentKeyStates
{
    INP_CKS_None     = 0,
    INP_CKS_Pressed  = 1 << 0,
    INP_CKS_Held     = 1 << 1,
    INP_CKS_Released = 1 << 2,
};

COL_DECLARE_FOR(INP_CurrentKeyState);

/**
 * Any modifiers that are added to a key event.
 */
typedef u8 INP_KeyModifier;
enum INP_KeyModifiers
{
    INP_KM_None      =      0,
    INP_KM_Alt       = 1 << 0,
    INP_KM_Ctrl      = 1 << 1,
    INP_KM_Shift     = 1 << 2,
    INP_KM_CmdOrMeta = 1 << 3,
};

/**
 * The code of a key on the keyboard.
 * Compatible with ASCII for standard keys.
 */
typedef u16 INP_KeyCode;
enum INP_KeyCodes
{
    INP_KC_Unknown            =   0,
    INP_KC_Backspace          =   8,
    INP_KC_Tab                =   9,
    INP_KC_Enter              =  13,
    INP_KC_Escape             =  27,
    INP_KC_Space              =  32,
    INP_KC_Delete             = 127,
    INP_KC_ArrowUp            = 128,
    INP_KC_ArrowDown          = 129,
    INP_KC_ArrowLeft          = 130,
    INP_KC_ArrowRight         = 131,
    INP_KC_PgUp               = 132,
    INP_KC_PgDown             = 133,
    INP_KC_Home               = 134,
    INP_KC_End                = 135,
    INP_KC_Insert             = 136,
    INP_KC_Pause              = 137,
    INP_KC_ScrollLock         = 138,
    INP_KC_Alt                = 139,
    INP_KC_Control            = 140,
    INP_KC_Shift              = 141,
    INP_KC_Cmd                = 142,
    INP_KC_Meta               = 142, // intentionally same as prev
    INP_KC_F1                 = 143,
    INP_KC_F2                 = 144,
    INP_KC_F3                 = 145,
    INP_KC_F4                 = 146,
    INP_KC_F5                 = 147,
    INP_KC_F6                 = 148,
    INP_KC_F7                 = 149,
    INP_KC_F8                 = 150,
    INP_KC_F9                 = 151,
    INP_KC_F10                = 152,
    INP_KC_F11                = 153,
    INP_KC_F12                = 154,
    INP_KC_PrtScrn            = 167,
    INP_KC_MouseBtnLeft       = 168,
    INP_KC_MouseBtnMiddle     = 169,
    INP_KC_MouseBtnRight      = 170,
    INP_KC_MouseWhlUp         = 171,
    INP_KC_MouseWhlDown       = 172,
    INP_KC_GamePad0Bgn        = 173,
    INP_KC_GamePad0End        = 205, // bgn + 32 buttons
    INP_KC_GamePad1Bgn        = 206,
    INP_KC_GamePad1End        = 238, // bgn + 32 buttons
    INP_KC_GamePad2Bgn        = 239,
    INP_KC_GamePad2End        = 271, // bgn + 32 buttons
    INP_KC_GamePad3Bgn        = 272,
    INP_KC_GamePad3End        = 304, // bgn + 32 buttons
    INP_KC_Touch              = 305,
    INP_KC_NUM                = 306,
};

/**
 * An input event that was logged.
 */
typedef struct
{
    INP_EvtTy       ty;
    INP_KeyStatus   keyStatus;
    INP_KeyModifier keyModifiers;
    b8              repeat;
    INP_KeyCode     keyCode;
    u16             textCount;
    u32             utf32Char;
    i32             rawWheelData;
    i32             wheelData;
    INP_TouchStatus touchStatus;
    u8              touchId;
    u16             droppedFileId;
    WND_Handle      windowId;
} INP_Evt;

COL_DECLARE_FOR(INP_Evt);

/**
 * Gather all input events for this frame.
 * Must be called once per frame before accessing events.
 * Clears previous frame's events and processes new Windows messages.
 * Not thread-safe.
 */
void INP_GatherEvts(void);

/**
 * Iterate across all events that were gathered this frame.
 */
b8 INP_IterateEvts(isize* iterator, INP_Evt* val OPT_ARG);

/**
 * Iterate across window resize events.
 * Automatically cleans up the internal resources when iteration is over.
 * Use in a `while` loop, ideally.
 * Not thread-safe.
 */
b8 INP_IterateResizeEvts(isize* iterator, INP_WindowResizeData* val OPT_ARG);

/**
 * Iterate across window move events.
 * Automatically cleans up the internal resources when iteration is over.
 * Use in a `while` loop, ideally.
 * Not thread-safe.
 */
b8 INP_IterateMoveEvts(isize* iterator, INP_WindowMoveData* val OPT_ARG);

/**
 * Get the current state of a key.
 * Returns a bitmask indicating if the key is pressed, held, or released this frame.
 * Not thread-safe.
 */
INP_CurrentKeyState INP_GetKeyState(INP_KeyCode key);

/**
 * Get the mouse movement delta for this frame.
 * Sets the provided pointers to the delta values (can pass NULL to ignore).
 * deltaX, deltaY are relative mouse movement, deltaWheel is scroll wheel delta.
 * Not thread-safe.
 */
void INP_GetMouseDelta(i32* deltaX OPT_ARG, i32* deltaY OPT_ARG, i32* deltaWheel OPT_ARG);

/**
 * Check if the application currently has focus.
 * Returns true if the application has focus, false otherwise.
 * Not thread-safe.
 */
b8 INP_AppHasFocus(void);

/**
 * Get a dropped file path by its ID.
 * The fileId comes from a DropFile event's droppedFileId field.
 * Returns an empty string if the ID is invalid.
 * The returned string is valid until the next call to DVRPL_GatherEvents.
 * Not thread-safe.
 */
utf8str INP_GetDroppedFile(u16 fileId);

EXTERN_C_END
