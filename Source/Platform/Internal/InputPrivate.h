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

    INP_KeyModifier cachedModifierStates;

#endif
} INP_Internal_State;

// get the internal state
INP_Internal_State* INP_Internal_GetState(void);

#define INP_INTERNAL_STATE(varName) \
    INP_Internal_State* varName = INP_Internal_GetState()

void INP_Internal_ClearTempData(INP_Internal_State* state);

EXTERN_C_END
