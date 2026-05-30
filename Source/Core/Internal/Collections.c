#include "Core/Memory.h"
#include <Core/Collections.h>

COL_RawSlice COL_NewRawSlice(usize tySize, usize tyAlign, MEM_Allocator allocator, isize count, b8 skipInit)
{
    rawptr mem = MEM_Allocate(allocator, !skipInit, tySize * (usize) count, tyAlign);
    if (!mem) return (COL_RawSlice) {0};
    return (COL_RawSlice) {.data = mem, .count = count};
}

void COL_ResizeRawSlice(usize tySize, usize tyAlign, MEM_Allocator allocator, COL_RawSlice* slice, isize newCount, b8 skipInit)
{
    if (!slice) return;

    size_t k = sizeof(__typeof__(allocator.procedure));
    rawptr newMem = MEM_Reallocate(
        allocator,
        !skipInit,
        slice->data,
        tySize * (usize) slice->count,
        tySize * (usize) newCount,
        tyAlign
    );
    if (!newMem) return;

    *slice = (COL_RawSlice) {.data = newMem, .count = newCount};
}

void COL_DeleteRawSlice(MEM_Allocator allocator, COL_RawSlice* slice)
{
    if (!slice) return;

    MEM_Deallocate(allocator, slice->data);

    *slice = (COL_RawSlice) {0};
}
