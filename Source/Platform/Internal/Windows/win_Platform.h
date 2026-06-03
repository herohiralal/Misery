#pragma once
#include <Platform/Platform.h>

#if MSR_WINDOWS

LRESULT CALLBACK INP_Internal_WindowsInputCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct
{
    RECT rect;
    LONG savedStyle;
    LONG savedExStyle;
} WND_NativeSavedData;

static inline WND_NativeSavedData WND_ToSavedData(WND_SavedData d)
{
    return *(WND_NativeSavedData*) &d;
}

static inline WND_SavedData WND_FromSavedData(WND_NativeSavedData d)
{
    WND_SavedData result;
    MEM_Set(&result, 0, sizeof(WND_SavedData));
    MEM_Copy(&result, &d, sizeof(WND_NativeSavedData));
    return result;
}

#endif
