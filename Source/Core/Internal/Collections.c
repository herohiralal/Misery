#include <Core/Core.h>

COL_RawSlice COL_NewRawSlice(usize tySize, usize tyAlign, MEM_Allocator allocator, isize count, b8 skipInit)
{
    rawptr mem = MEM_Allocate(allocator, !skipInit, tySize * (usize) count, tyAlign);
    if (!mem) return (COL_RawSlice) {0};
    return (COL_RawSlice) {.data = mem, .count = count};
}

COL_RawSlice COL_CloneRawSlice(usize tySize, usize tyAlign, MEM_Allocator allocator, COL_RawSlice slice)
{
    COL_RawSlice output = COL_NewRawSlice(tySize, tyAlign, allocator, slice.count, true);
    if (!output.data || !output.count) return (COL_RawSlice) {0};
    MEM_Copy(output.data, slice.data, tySize * (usize) slice.count);
    return output;
}

void COL_ResizeRawSlice(usize tySize, usize tyAlign, MEM_Allocator allocator, COL_RawSlice* slice, isize newCount, b8 skipInit)
{
    if (!slice) return;

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

COL_RawList COL_NewRawList(usize tySize, usize tyAlign, MEM_Allocator allocator, isize initialCap)
{
    COL_RawList output = {0};
    output.allocator = allocator;

    if (initialCap)
    {
        COL_RawSlice sl = COL_NewRawSlice(tySize, tyAlign, allocator, initialCap, true);
        output.data = sl.data;
        output.capacity = sl.count;
    }

    return output;
}

void COL_ResizeRawList(usize tySize, usize tyAlign, COL_RawList* list, isize newCap)
{
    if (!list) return;

    COL_RawSlice sl = {.data = list->data, .count = list->capacity};
    COL_ResizeRawSlice(tySize, tyAlign, list->allocator, &sl, newCap, true);
    list->data = sl.data;
    list->capacity = sl.count;
    if (list->count > sl.count) list->count = sl.count;
}

void COL_EnsureRawListCapacity(usize tySize, usize tyAlign, COL_RawList* list, isize capacity)
{
    if (!list) return;

    if (capacity > list->capacity)
    {
        isize newCap = list->capacity ? list->capacity : 16;
        while (newCap < capacity) newCap *= 2;
        COL_ResizeRawList(tySize, tyAlign, list, newCap);
    }
}

void COL_ClearRawList(COL_RawList* list)
{
    if (!list) return;

    list->count = 0;
}

void COL_DeleteRawList(COL_RawList* list)
{
    if (!list) return;

    MEM_Allocator a = list->allocator;
    MEM_Deallocate(a, list->data);

    *list = (COL_RawList) {0};
    list->allocator = a;
}
