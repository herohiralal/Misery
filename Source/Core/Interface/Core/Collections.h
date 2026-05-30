#pragma once
#include <__init.h>
#include "Memory.h"
EXTERN_C_BEGIN

// "generics" ------------------------------------------------------------------------------------------------------------------

/**
 * A raw type-unspecific slice.
 */
typedef struct COL_RawSlice { rawptr data; isize count; } COL_RawSlice;

/**
 * A raw type-unspecific list.
 */
typedef struct COL_RawList { rawptr data; isize count; isize capacity; MEM_Allocator allocator; } COL_RawList;

#define Slice_(ty) COL_Slice_##ty
#define List_(ty)  COL_List_##ty

#ifdef __cplusplus

    EXTERN_C_END

    template <typename T>
    struct Slice { T* data; isize count; };

    template <typename T>
    struct List { T* data; isize count; isize capacity; MEM_Allocator allocator; };

    template <typename T>
    static inline COL_RawSlice* CreateRawSlicePtr(Slice<T>* slice) { return (COL_RawSlice*) (slice); }

    template <typename T>
    static inline COL_RawList* CreateRawListPtr(List<T>* list) { return (COL_RawList*) (list); }

    #define COL_DECLARE_SLICE(ty) \
        typedef struct \
        { \
            ty*   data; \
            isize count; \
        } Slice_(ty); \
        \
        EXTERN_C_END \
        template <> \
        struct Slice<ty> \
        { \
            ty*   data; \
            isize count; \
            \
            Slice<ty>() = default; \
            Slice<ty>(const Slice_(ty)& other) : data(other.data), count(other.count) { } \
            operator Slice_(ty)() const { return Slice_(ty) {data, count}; } \
        }; \
        \
        static inline COL_RawSlice* CreateRawSlicePtr(Slice_(ty)* slice) { return (COL_RawSlice*) (slice); } \
        static inline Slice_(ty) Slice##ty##FromRaw(const COL_RawSlice& raw) { return *(Slice_(ty)*) &raw; } \
        EXTERN_C_BEGIN

    #define COL_DECLARE_LIST(ty) \
        typedef struct \
        { \
            ty*           data; \
            isize         count; \
            isize         capacity; \
            MEM_Allocator allocator; \
        } List_(ty); \
        \
        EXTERN_C_END \
        template <> \
        struct List<ty> \
        { \
            ty*           data; \
            isize         count; \
            isize         capacity; \
            MEM_Allocator allocator; \
            \
            List<ty>() = default; \
            List<ty>(const List_(ty)& other) : data(other.data), count(other.count), capacity(other.capacity), allocator(other.allocator) { } \
            operator List_(ty)() const { return List_(ty) {data, count, capacity, allocator}; } \
        }; \
        \
        static inline COL_RawList* CreateRawListPtr(List_(ty)* list) { return (COL_RawList*) (list); } \
        static inline List_(ty) List##ty##FromRaw(const COL_RawList& raw) { return *(List_(ty)*) &raw; } \
        EXTERN_C_BEGIN

    #define COL_SLICE_FROM_RAW(ty, raw) \
        ((Slice_(ty)) Slice##ty##FromRaw(raw))

    #define COL_LIST_FROM_RAW(ty, raw) \
        ((List_(ty)) List##ty##FromRaw(raw))

    #define COL_RAW_PTR_FROM_SLICE_PTR(sl) (CreateRawSlicePtr(sl))

    #define COL_RAW_PTR_FROM_LIST_PTR(ls) (CreateRawListPtr(ls))

    EXTERN_C_BEGIN

#else

    #define COL_DECLARE_SLICE(ty) \
        typedef union \
        { \
            struct \
            { \
                ty*   data; \
                isize count; \
            }; \
            COL_RawSlice raw; \
        } Slice_(ty);

    #define COL_DECLARE_LIST(ty) \
        typedef union \
        { \
            struct \
            { \
                ty*           data; \
                isize         count; \
                isize         capacity; \
                MEM_Allocator allocator; \
            }; \
            COL_RawList raw; \
        } List_(ty);

    #define COL_SLICE_FROM_RAW(ty, in) \
        ((Slice_(ty)) {.raw = in})

    #define COL_LIST_FROM_RAW(ty, in) \
        ((List_(ty)) {.raw = in})

    #define COL_RAW_PTR_FROM_SLICE_PTR(sl) (&((sl)->raw))

    #define COL_RAW_PTR_FROM_LIST_PTR(ls) (&((ls)->raw))

#endif

COL_DECLARE_SLICE(     b8);
COL_DECLARE_SLICE(     u8);
COL_DECLARE_SLICE(    u16);
COL_DECLARE_SLICE(    u32);
COL_DECLARE_SLICE(    u64);
COL_DECLARE_SLICE(     i8);
COL_DECLARE_SLICE(    i16);
COL_DECLARE_SLICE(    i32);
COL_DECLARE_SLICE(    i64);
COL_DECLARE_SLICE(    f32);
COL_DECLARE_SLICE(    f64);
COL_DECLARE_SLICE(   char);
COL_DECLARE_SLICE( rawptr);
COL_DECLARE_SLICE(cstring);

COL_DECLARE_LIST(     b8);
COL_DECLARE_LIST(     u8);
COL_DECLARE_LIST(    u16);
COL_DECLARE_LIST(    u32);
COL_DECLARE_LIST(    u64);
COL_DECLARE_LIST(     i8);
COL_DECLARE_LIST(    i16);
COL_DECLARE_LIST(    i32);
COL_DECLARE_LIST(    i64);
COL_DECLARE_LIST(    f32);
COL_DECLARE_LIST(    f64);
COL_DECLARE_LIST(   char);
COL_DECLARE_LIST( rawptr);
COL_DECLARE_LIST(cstring);

// allocation/deallocation -----------------------------------------------------------------------------------------------------

// internal function; creates a new slice (without any type info)
COL_RawSlice COL_NewRawSlice(usize tySize, usize tyAlign, MEM_Allocator, isize count, b8 skipInit OPT_ARG);

// internal function; resizes an existing slice (without any type info)
void COL_ResizeRawSlice(usize tySize, usize tyAlign, MEM_Allocator, COL_RawSlice* slice, isize newCount, b8 skipInit OPT_ARG);

// internal function; deletes an existing slice (without any type info)
void COL_DeleteRawSlice(MEM_Allocator, COL_RawSlice* slice);

#define COL_NewSlice(ty, allocator, count, skipInit) \
    COL_SLICE_FROM_RAW( \
        ty, \
        COL_NewRawSlice( \
            sizeof(ty), \
            alignof(ty), \
            (allocator), \
            (isize) (count), \
            (b8) (skipInit) \
        ) \
    )

#define COL_ResizeSlice(allocator, slicePtr, newCount, skipInit) \
    COL_ResizeRawSlice( \
        sizeof((slicePtr)->data[0]), \
        alignof(MSR_TYPEOF((slicePtr)->data[0])), \
        (allocator), \
        COL_RAW_PTR_FROM_SLICE_PTR(slicePtr), \
        (isize) newCount, \
        (b8) skipInit \
    )

#define COL_DeleteSlice(allocator, slicePtr) \
    COL_DeleteRawSlice( \
        (allocator), \
        COL_RAW_PTR_FROM_SLICE_PTR(slicePtr) \
    )

// internal function; creates a new list (without any type info)
COL_RawList COL_NewRawList(usize tySize, usize tyAlign, MEM_Allocator, isize initialCap OPT_ARG);

// internal function; resizes an existing list (without any type info)
void COL_ResizeRawList(usize tySize, usize tyAlign, COL_RawList* list, isize newCap);

// internal function; ensures that the list has enough capacity to append more elements (without any type info)
void COL_EnsureRawListAdditionalCapacity(usize tySize, usize tyAlign, COL_RawList* list, isize additionalCap);

// internal function; clears an existing list by setting the count to zero (without any type info)
void COL_ClearRawList(COL_RawList* list);

// internal function; deletes an existing list (without any type info)
void COL_DeleteRawList(COL_RawList* list);

#define COL_NewList(ty, allocator, initialCap) \
    COL_LIST_FROM_RAW( \
        ty, \
        COL_NewRawList( \
            sizeof(ty), \
            alignof(ty), \
            (allocator), \
            (initialCap) \
        ) \
    )

#define COL_AppendToList(listPtr, item) \
    do \
    { \
        COL_EnsureRawListAdditionalCapacity( \
            sizeof((listPtr)->data[0]), \
            alignof(MSR_TYPEOF((listPtr)->data[0])), \
            COL_RAW_PTR_FROM_LIST_PTR(listPtr), \
            (isize) (1) \
        ); \
        \
        (listPtr)->data[(listPtr)->count] = (item); \
        (listPtr)->count++; \
    } while(0)

#define COL_AppendAllToList(listPtr, items) \
    do \
    { \
        if ((items).count && (items).data) \
        { \
            COL_EnsureRawListAdditionalCapacity( \
                sizeof((listPtr)->data[0]), \
                alignof(MSR_TYPEOF((listPtr)->data[0])), \
                COL_RAW_PTR_FROM_LIST_PTR(listPtr), \
                (items).count \
            ); \
            \
            MEM_Move( \
                ((u8*) (listPtr)->data) + ((listPtr)->count * (isize) sizeof((listPtr)->data[0])), \
                (items).data, \
                (items).count * (isize) sizeof((listPtr)->data[0]) \
            ); \
            \
            (listPtr)->count += (items).count; \
        } \
    } while(0)

#define COL_ResizeList(listPtr, newCap) \
    COL_ResizeRawList( \
        sizeof((listPtr)->data[0]), \
        alignof(MSR_TYPEOF((listPtr)->data[0])), \
        COL_RAW_PTR_FROM_LIST_PTR(listPtr), \
        (isize) (newCap) \
    )

#define COL_ClearList(listPtr) \
    COL_ClearRawList(COL_RAW_PTR_FROM_LIST_PTR(listPtr))

#define COL_FreeList(listPtr) \
    COL_DeleteRawList(COL_RAW_PTR_FROM_LIST_PTR(listPtr))

// slice/list utils ------------------------------------------------------------------------------------------------------------

/**
 * Get a SUB_SLICE of the provided slice/list, starting at 'start' and containing 'count' elements.
 * Note: if the bounds check fail, the returned slice will have a null data pointer and a count of 0.
 * Use as:
```
Slice_(i32) s = COL_SUB_SLICE(sliceOrList, start, count);
```
 * Note that in c++, the following won't work:
```
auto s = COL_SUB_SLICE(sliceOrList, start, count); // WILL NOT COMPILE!!
```
 * Can also be used when passing parameters to functions:
```cpp
int some_func(Slice_(i32) s);
int main()
{
    // ...
    #ifdef __cplusplus
        // c++ supports aggregate initialisation
        some_func(COL_SUB_SLICE(sliceOrList, start, count));
        // OR
        some_func(Slice_(i32) COL_SUB_SLICE(sliceOrList, start, count));
        // OR
        some_func(Slice<i32> COL_SUB_SLICE(sliceOrList, start, count));
    #else
        // c requires this weird cast-like syntax
        some_func((Slice_(i32)) COL_SUB_SLICE(sliceOrList, start, count));
    #endif
    // ...
}
```
 */
#define COL_SUB_SLICE(sl, st, cnt) \
    { \
        ((st) < 0 || (cnt) < 0 || ((st) + (cnt)) > (sl).count) ? nil : &((sl).data[(st)]), \
        ((st) < 0 || (cnt) < 0 || ((st) + (cnt)) > (sl).count) ? 0   : (cnt), \
    }

EXTERN_C_END
