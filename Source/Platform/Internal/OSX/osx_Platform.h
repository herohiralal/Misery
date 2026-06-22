#pragma once
#include "../InputPrivate.h"

#if MSR_OSX

EXTERN_C_BEGIN

typedef struct
{
    f64 x, y;
    f64 w, h;
    u8 wasFullscreen;
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

void INP_Internal_OSXAttachWindowDelegate(NSWindow* window);

EXTERN_C_END

#endif
