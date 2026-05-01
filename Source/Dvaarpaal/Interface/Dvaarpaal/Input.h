/**
 * The input module for Dvaarpaal.
 * PS - will not work without using the Window module. On all platforms.
 *
 * In stark contrast to how the Window module uses opaque handles for OS-specific
 * objects/APIs, the Input module uses an abstraction approach, where input events
 * are declared as a cross-platform structure and the implementation files handle
 * any relevant translation that might be needed.
 */
#ifndef DVRPL_INPUT_H // ===========================================================
#define DVRPL_INPUT_H
#include "__Prelude.h"
#include "Window.h"
EXTERN_C_BEGIN

/**
 * The type of input event that was logged.
 */
ENUM_START(DVRPL_EvtTy, u8)
    #define DVRPL_EvtTy_Unknown    ((DVRPL_EvtTy) 0)
    #define DVRPL_EvtTy_Keyboard   ((DVRPL_EvtTy) 1)
    #define DVRPL_EvtTy_MouseWheel ((DVRPL_EvtTy) 2)
    #define DVRPL_EvtTy_Touch      ((DVRPL_EvtTy) 3)
    #define DVRPL_EvtTy_TextInput  ((DVRPL_EvtTy) 4)
    #define DVRPL_EvtTy_DropFile   ((DVRPL_EvtTy) 5)
    #define DVRPL_EvtTy_Quit       ((DVRPL_EvtTy) 6)
ENUM_END

/**
 * The information regarding a window move event.
 */
typedef struct DVRPL_WindowMoveData
{
    DVRPL_Window id;
    i16          posX;
    i16          posY;
} DVRPL_WindowMoveData;

PNSLR_DECLARE_ARRAY_SLICE(DVRPL_WindowMoveData);

/**
 * The information regarding a window resize event.
 */
typedef struct DVRPL_WindowResizeData
{
    DVRPL_Window id;
    u16          sizeX;
    u16          sizeY;
} DVRPL_WindowResizeData;

PNSLR_DECLARE_ARRAY_SLICE(DVRPL_WindowResizeData);

/**
 * The status of a touch event.
 */
ENUM_START(DVRPL_TouchStatus, u8)
    #define DVRPL_TouchStatus_Moved    ((DVRPL_TouchStatus) 0)
    #define DVRPL_TouchStatus_Pressed  ((DVRPL_TouchStatus) 1)
    #define DVRPL_TouchStatus_Released ((DVRPL_TouchStatus) 2)
ENUM_END

// Technically can just bundle this into `DVRPL_EvtTy` but considering
// how people usually use input events, first they wanna know  which key
// it was, and then whether it was pressed or released, not specifically
// tracking downs/ups and then figuring out which key.
/**
 * The status of a key event.
 */
ENUM_START(DVRPL_KeyStatus, u8)
    #define DVRPL_KeyStatus_Pressed  ((DVRPL_KeyStatus) 0)
    #define DVRPL_KeyStatus_Released ((DVRPL_KeyStatus) 1)
ENUM_END

/**
 * The possible states of a key event.
 */
ENUM_FLAGS_START(DVRPL_KeyState, u8)
    #define DVRPL_KeyState_None     ((DVRPL_KeyState) (        0))
    #define DVRPL_KeyState_Pressed  ((DVRPL_KeyState) (1ULL << 0))
    #define DVRPL_KeyState_Held     ((DVRPL_KeyState) (1ULL << 1))
    #define DVRPL_KeyState_Released ((DVRPL_KeyState) (1ULL << 2))
ENUM_END

/**
 * Any modifiers that are added to a key event.
 */
ENUM_FLAGS_START(DVRPL_KeyModifier, u8)
    #define DVRPL_KeyModifier_None      ((DVRPL_KeyModifier) (        0))
    #define DVRPL_KeyModifier_Alt       ((DVRPL_KeyModifier) (1ULL << 0))
    #define DVRPL_KeyModifier_Control   ((DVRPL_KeyModifier) (1ULL << 1))
    #define DVRPL_KeyModifier_Shift     ((DVRPL_KeyModifier) (1ULL << 2))
    #define DVRPL_KeyModifier_CmdOrMeta ((DVRPL_KeyModifier) (1ULL << 3))
ENUM_END

/**
 * The code of a key on the keyboard.
 * Compatible with ASCII for standard keys.
 */
ENUM_START(DVRPL_KeyCode, u16)
    #define DVRPL_KeyCode_Unknown            ((DVRPL_KeyCode)   0)
    #define DVRPL_KeyCode_Backspace          ((DVRPL_KeyCode)   8)
    #define DVRPL_KeyCode_Tab                ((DVRPL_KeyCode)   9)
    #define DVRPL_KeyCode_Enter              ((DVRPL_KeyCode)  13)
    #define DVRPL_KeyCode_Escape             ((DVRPL_KeyCode)  27)
    #define DVRPL_KeyCode_Space              ((DVRPL_KeyCode)  32)
    #define DVRPL_KeyCode_Delete             ((DVRPL_KeyCode) 127)
    #define DVRPL_KeyCode_ArrowUp            ((DVRPL_KeyCode) 128)
    #define DVRPL_KeyCode_ArrowDown          ((DVRPL_KeyCode) 129)
    #define DVRPL_KeyCode_ArrowLeft          ((DVRPL_KeyCode) 130)
    #define DVRPL_KeyCode_ArrowRight         ((DVRPL_KeyCode) 131)
    #define DVRPL_KeyCode_PgUp               ((DVRPL_KeyCode) 132)
    #define DVRPL_KeyCode_PgDown             ((DVRPL_KeyCode) 133)
    #define DVRPL_KeyCode_Home               ((DVRPL_KeyCode) 134)
    #define DVRPL_KeyCode_End                ((DVRPL_KeyCode) 135)
    #define DVRPL_KeyCode_Insert             ((DVRPL_KeyCode) 136)
    #define DVRPL_KeyCode_Pause              ((DVRPL_KeyCode) 137)
    #define DVRPL_KeyCode_ScrollLock         ((DVRPL_KeyCode) 138)
    #define DVRPL_KeyCode_Alt                ((DVRPL_KeyCode) 139)
    #define DVRPL_KeyCode_Control            ((DVRPL_KeyCode) 140)
    #define DVRPL_KeyCode_Shift              ((DVRPL_KeyCode) 141)
    #define DVRPL_KeyCode_Cmd                ((DVRPL_KeyCode) 142)
    #define DVRPL_KeyCode_Meta               ((DVRPL_KeyCode) 142) // intentionally same as prev
    #define DVRPL_KeyCode_F1                 ((DVRPL_KeyCode) 143)
    #define DVRPL_KeyCode_F2                 ((DVRPL_KeyCode) 144)
    #define DVRPL_KeyCode_F3                 ((DVRPL_KeyCode) 145)
    #define DVRPL_KeyCode_F4                 ((DVRPL_KeyCode) 146)
    #define DVRPL_KeyCode_F5                 ((DVRPL_KeyCode) 147)
    #define DVRPL_KeyCode_F6                 ((DVRPL_KeyCode) 148)
    #define DVRPL_KeyCode_F7                 ((DVRPL_KeyCode) 149)
    #define DVRPL_KeyCode_F8                 ((DVRPL_KeyCode) 150)
    #define DVRPL_KeyCode_F9                 ((DVRPL_KeyCode) 151)
    #define DVRPL_KeyCode_F10                ((DVRPL_KeyCode) 152)
    #define DVRPL_KeyCode_F11                ((DVRPL_KeyCode) 153)
    #define DVRPL_KeyCode_F12                ((DVRPL_KeyCode) 154)
    #define DVRPL_KeyCode_PrtScrn            ((DVRPL_KeyCode) 167)
    #define DVRPL_KeyCode_MouseBtnLeft       ((DVRPL_KeyCode) 168)
    #define DVRPL_KeyCode_MouseBtnMiddle     ((DVRPL_KeyCode) 169)
    #define DVRPL_KeyCode_MouseBtnRight      ((DVRPL_KeyCode) 170)
    #define DVRPL_KeyCode_MouseWhlUp         ((DVRPL_KeyCode) 171)
    #define DVRPL_KeyCode_MouseWhlDown       ((DVRPL_KeyCode) 172)
    #define DVRPL_KeyCode_GamePad0Bgn        ((DVRPL_KeyCode) 173)
    #define DVRPL_KeyCode_GamePad0End        ((DVRPL_KeyCode) 205) // bgn + 32 buttons
    #define DVRPL_KeyCode_GamePad1Bgn        ((DVRPL_KeyCode) 206)
    #define DVRPL_KeyCode_GamePad1End        ((DVRPL_KeyCode) 238) // bgn + 32 buttons
    #define DVRPL_KeyCode_GamePad2Bgn        ((DVRPL_KeyCode) 239)
    #define DVRPL_KeyCode_GamePad2End        ((DVRPL_KeyCode) 271) // bgn + 32 buttons
    #define DVRPL_KeyCode_GamePad3Bgn        ((DVRPL_KeyCode) 272)
    #define DVRPL_KeyCode_GamePad3End        ((DVRPL_KeyCode) 304) // bgn + 32 buttons
    #define DVRPL_KeyCode_Touch              ((DVRPL_KeyCode) 305)
    #define DVRPL_KeyCode_NUM                ((DVRPL_KeyCode) 306)
ENUM_END

/**
 * An input event that was logged.
 */
typedef struct alignas(32) DVRPL_Event
{
    DVRPL_EvtTy       ty;            // 8-bits
    DVRPL_KeyStatus   keyStatus;     // 8-bits
    DVRPL_KeyModifier keyModifiers;  // 8-bits
    b8                repeat;        // 8-bits; for keyboard evts
    DVRPL_KeyCode     keyCode;       // 16-bits
    u16               textCount;     // 16-bits; for TextInput evts - how many text-input evts were generated after a keyboard evt
    u32               utf32Char;     // 32-bits; for TextInput evts
    i32               rawWheelData;  // 32-bits; for MouseWheel evts - unprocessed info
    i32               wheelData;     // 32-bits; for MouseWheel evts
    DVRPL_TouchStatus touchStatus;   // 8-bits; for Touch evts
    u8                touchId;       // 8-bits; for Touch evts - which finger is it
    u16               droppedFileId; // 16-bits; for DropFile evts - id of the file
    DVRPL_Window      windowId;      // 64-bits; for Window evts - the id of the window
} DVRPL_Event;

PNSLR_DECLARE_ARRAY_SLICE(DVRPL_Event);

//+skipreflect
static_assert( sizeof(DVRPL_Event) == 32, "DVRPL_Event must be exactly 32 bytes in size.");
static_assert(alignof(DVRPL_Event) == 32, "DVRPL_Event must be 32-byte aligned.");
//-skipreflect

/**
 * Gather all input events for this frame.
 * Must be called once per frame before accessing events.
 * Clears previous frame's events and processes new Windows messages.
 * Requires a temp allocator to store a bunch of temporary stuff.
 * Not thread-safe.
 */
void DVRPL_GatherEvents(PNSLR_Allocator tempAllocator);

/**
 * Iterate across all events that were gathered this frame.
 */
b8 DVRPL_IterateEvents(i64* iterator, DVRPL_Event* val OPT_ARG);

/**
 * Iterate across window resize events.
 * Automatically cleans up the internal resources when iteration is over.
 * Use in a `while` loop, ideally.
 * Not thread-safe.
 */
b8 DVRPL_IterateResizeEvent(i32* iterator, DVRPL_WindowResizeData* val OPT_ARG);

/**
 * Iterate across window move events.
 * Automatically cleans up the internal resources when iteration is over.
 * Use in a `while` loop, ideally.
 * Not thread-safe.
 */
b8 DVRPL_IterateMoveEvent(i32* iterator, DVRPL_WindowMoveData* val OPT_ARG);

/**
 * Get the current state of a key.
 * Returns a bitmask indicating if the key is pressed, held, or released this frame.
 * Not thread-safe.
 */
DVRPL_KeyState DVRPL_GetKeyState(DVRPL_KeyCode key);

/**
 * Get the mouse movement delta for this frame.
 * Sets the provided pointers to the delta values (can pass NULL to ignore).
 * deltaX, deltaY are relative mouse movement, deltaScroll is scroll wheel delta.
 * Not thread-safe.
 */
void DVRPL_GetMouseDelta(
    i32* deltaX      OPT_ARG,
    i32* deltaY      OPT_ARG,
    i32* deltaScroll OPT_ARG
);

/**
 * Check if the application currently has focus.
 * Returns true if the application has focus, false otherwise.
 * Not thread-safe.
 */
b8 DVRPL_DoesApplicationHaveFocus(void);

/**
 * Get a dropped file path by its ID.
 * The fileId comes from a DropFile event's droppedFileId field.
 * Returns an empty string if the ID is invalid.
 * The returned string is valid until the next call to DVRPL_GatherEvents.
 * Not thread-safe.
 */
utf8str DVRPL_GetDroppedFile(u16 fileId);

//+skipreflect
#if defined(DVRPL_IMPLEMENTATION) && !defined(__cplusplus) // c-only

    #if PNSLR_WINDOWS
        static LRESULT CALLBACK DVRPL_Internal_WindowsInputCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
    #endif

    #if PNSLR_ANDROID
        static void DVRPL_Internal_AndroidSetApp(struct android_app* app);
        static void DVRPL_Internal_FlushEventsTillInFocus(struct android_app* app);
    #endif

    #if PNSLR_OSX && defined(__OBJC__)
        static void DVRPL_Internal_AppleSetApp(NSApplication* app);
    #endif

#endif
//-skipreflect

EXTERN_C_END
#endif // DVRPL_INPUT_H ============================================================
