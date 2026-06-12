#pragma once
#include "../InputPrivate.h"

#if MSR_LINUX

EXTERN_C_BEGIN

// XCB connection + atoms used across window and input
typedef struct
{
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
} WND_XCBContext;

// Stored in WND_SavedData when in windowed mode
typedef struct
{
    i16 posX,  posY;
    u16 sizeX, sizeY;
    u32 savedStyle; // WM_NORMAL_HINTS flags placeholder; unused but kept for size parity
} WND_NativeSavedData;

static_assert(sizeof(WND_NativeSavedData) <= sizeof(WND_SavedData), "WND_NativeSavedData too large");

static inline WND_NativeSavedData WND_ToSavedData(WND_SavedData d)
{
    WND_NativeSavedData r;
    MEM_Copy(&r, &d, sizeof(WND_NativeSavedData));
    return r;
}

static inline WND_SavedData WND_FromSavedData(WND_NativeSavedData d)
{
    WND_SavedData result;
    MEM_Set(&result, 0, sizeof(WND_SavedData));
    MEM_Copy(&result, &d, sizeof(WND_NativeSavedData));
    return result;
}

// Get (or lazily init) the global XCB context
WND_XCBContext* WND_GetXCBContext(void);

// Forward: input event pump called from INP_GatherEvts
void INP_Internal_ProcessXCBEvents(void);

EXTERN_C_END

#endif
