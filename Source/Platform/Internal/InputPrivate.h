#pragma once

#include <Platform/Platform.h>

EXTERN_C_BEGIN

typedef struct
{
    MEM_Allocator tempAllocator;

    // all of these are allocated on the
    // internal input allocator and only valid
    // for 1 frame

    List_(utf8str) droppedFiles;
    List_(INP_WindowResizeData) resizes;
    List_(INP_WindowMoveData) moves;
    List_(INP_Evt) evts;

    i32 mouseDelta[3]; // x, y, scroll
    b8 appHasFocus;

    // allocated on the main allocator and valid
    // across frames
    Slice_(INP_CurrentKeyState) keyStates;

#if MSR_WINDOWS

    List_(u8) rawInputBuffer;
    b8 windowMinimised;
    INP_KeyModifier cachedModifierStates;
    List_(i32) keysDown; // acts as a set
    b8 inputSystemInitialised;

#elif MSR_LINUX

    xcb_connection_t* connection;
    xcb_screen_t*     screen;

    // atoms
    xcb_atom_t WM_PROTOCOLS;
    xcb_atom_t WM_DELETE_WINDOW;
    xcb_atom_t _NET_WM_STATE;
    xcb_atom_t _NET_WM_STATE_FULLSCREEN;
    xcb_atom_t _NET_WM_NAME;
    xcb_atom_t UTF8_STRING;

    // XInput2 opcode (for raw mouse input)
    int xi2Opcode;
    b8  xi2Available;

    INP_KeyModifier cachedModifierStates;

#endif
} INP_Internal_State;

// get the internal state
INP_Internal_State* INP_Internal_GetState(void);

#define INP_INTERNAL_STATE(varName) \
    INP_Internal_State* varName = INP_Internal_GetState()

void INP_Internal_ClearTempData(INP_Internal_State* state);

EXTERN_C_END
