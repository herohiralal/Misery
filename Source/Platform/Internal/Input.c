#include "InputPrivate.h"

void INP_Internal_ClearTempData(INP_Internal_State* state)
{
    MEM_DeallocateAll(state->tempAllocator);

    state->droppedFiles = COL_NewList(utf8str, 8, state->tempAllocator);
    state->resizes = COL_NewList(INP_WindowResizeData, 4, state->tempAllocator);
    state->moves = COL_NewList(INP_WindowMoveData, 4, state->tempAllocator);
    state->evts = COL_NewList(INP_Evt, 16, state->tempAllocator);
}

b8 INP_IterateEvts(isize* iterator, INP_Evt* val OPT_ARG)
{
    if (!iterator)
    {
        if (val) *val = (INP_Evt) {0};
        return false;
    }

    INP_INTERNAL_STATE(state);

    if (*iterator >= state->evts.count)
    {
        *iterator = ISIZE_MAX; // invalidate iterator
        if (val) *val = (INP_Evt) {0};
        return false;
    }

    if (val) *val = state->evts.data[*iterator];
    (*iterator)++;
    return true;
}

b8 INP_IterateResizeEvts(isize* iterator, INP_WindowResizeData* val OPT_ARG)
{
    if (!iterator)
    {
        if (val) *val = (INP_WindowResizeData) {0};
        return false;
    }

    INP_INTERNAL_STATE(state);
    if (*iterator >= state->resizes.count)
    {
        *iterator = ISIZE_MAX; // invalidate iterator
        if (val) *val = (INP_WindowResizeData) {0};
        return false;
    }

    if (val) *val = state->resizes.data[*iterator];
    (*iterator)++;
    return true;
}

b8 INP_IterateMoveEvts(isize* iterator, INP_WindowMoveData* val OPT_ARG)
{
    if (!iterator)
    {
        if (val) *val = (INP_WindowMoveData) {0};
        return false;
    }

    INP_INTERNAL_STATE(state);
    if (*iterator >= state->moves.count)
    {
        *iterator = ISIZE_MAX; // invalidate iterator
        if (val) *val = (INP_WindowMoveData) {0};
        return false;
    }

    if (val) *val = state->moves.data[*iterator];
    (*iterator)++;
    return true;
}

INP_CurrentKeyState INP_GetKeyState(INP_KeyCode key)
{
    INP_INTERNAL_STATE(state);
    MSR_ASSERT(state && state->keyStates.count == INP_KC_NUM && "invalid key states array");

    if (key >= INP_KC_NUM) return INP_CKS_None;
    return state->keyStates.data[key];
}

void INP_GetMouseDelta(i32* deltaX OPT_ARG, i32* deltaY OPT_ARG, i32* deltaWheel OPT_ARG)
{
    INP_INTERNAL_STATE(state);
    if (deltaX)     *deltaX     = state->mouseDelta[0];
    if (deltaY)     *deltaY     = state->mouseDelta[1];
    if (deltaWheel) *deltaWheel = state->mouseDelta[2];
}

b8 INP_AppHasFocus(void)
{
    INP_INTERNAL_STATE(state);
    return state->appHasFocus;
}

utf8str INP_GetDroppedFile(u16 fileId)
{
    INP_INTERNAL_STATE(state);
    if ((state->droppedFiles.count <= (isize) fileId) ||
        !state->droppedFiles.data)
        return (utf8str) {0};

    return state->droppedFiles.data[fileId];
}
