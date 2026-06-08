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
    static inline T COL_Internal_SliceFromRaw(const COL_RawSlice& raw)
    {
        static_assert( sizeof(T) ==  sizeof(COL_RawSlice));
        static_assert(alignof(T) == alignof(COL_RawSlice));
        return *(T*) (&raw);
    }

    template <typename T>
    static inline T COL_Internal_ListFromRaw(const COL_RawList& raw)
    {
        static_assert( sizeof(T) ==  sizeof(COL_RawList));
        static_assert(alignof(T) == alignof(COL_RawList));
        return *(T*) (&raw);
    }

    EXTERN_C_BEGIN

    #define COL_DECLARE_FOR(ty) \
        typedef struct \
        { \
            ty*   data; \
            isize count; \
        } Slice_(ty); \
        \
        typedef struct \
        { \
            ty*           data; \
            isize         count; \
            isize         capacity; \
            MEM_Allocator allocator; \
        } List_(ty); \
        \
        EXTERN_C_END \
        \
        static inline Slice_(ty) COL_AsSlice(const Slice_(ty)& s) { return s; } \
        static inline Slice_(ty) COL_AsSlice(const List_(ty)& l) { return Slice_(ty) {l.data, l.count}; } \
        static inline COL_RawSlice* COL_CreateRawSlicePtr(Slice_(ty)* slice) { return (COL_RawSlice*) (slice); } \
        static inline COL_RawList* COL_CreateRawListPtr(List_(ty)* list) { return (COL_RawList*) (list); } \
        static inline Slice_(ty) COL_SliceFromInitList_##ty(std::initializer_list<ty> init) { return Slice_(ty) {const_cast<ty*>(init.begin()), (isize) init.size()}; } \
        \
        EXTERN_C_BEGIN

    #define COL_SLICE_FROM_RAW(ty, raw) (COL_Internal_SliceFromRaw<Slice_(ty)>(raw))

    #define COL_LIST_FROM_RAW(ty, raw) (COL_Internal_ListFromRaw<List_(ty)>(raw))

    #define COL_RAW_PTR_FROM_SLICE_PTR(sl) (COL_CreateRawSlicePtr(sl))

    #define COL_RAW_PTR_FROM_LIST_PTR(ls) (COL_CreateRawListPtr(ls))

#else

    #define COL_DECLARE_FOR(ty) \
        typedef union \
        { \
            struct \
            { \
                ty*   data; \
                isize count; \
            }; \
            COL_RawSlice raw; \
            COL_RawSlice slice; \
        } Slice_(ty); \
        \
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
            Slice_(ty) slice; \
        } List_(ty);

    #define COL_SLICE_FROM_RAW(ty, in) ((Slice_(ty)) {.raw = in})

    #define COL_LIST_FROM_RAW(ty, in) ((List_(ty)) {.raw = in})

    #define COL_RAW_PTR_FROM_SLICE_PTR(sl) (&((sl)->raw))

    #define COL_RAW_PTR_FROM_LIST_PTR(ls) (&((ls)->raw))

#endif

COL_DECLARE_FOR(  b8);
COL_DECLARE_FOR(  u8);
COL_DECLARE_FOR( u16);
COL_DECLARE_FOR( u32);
COL_DECLARE_FOR( u64);
COL_DECLARE_FOR(  i8);
COL_DECLARE_FOR( i16);
COL_DECLARE_FOR( i32);
COL_DECLARE_FOR( i64);
COL_DECLARE_FOR( f32);
COL_DECLARE_FOR( f64);
COL_DECLARE_FOR(char);

COL_DECLARE_FOR(isize);
COL_DECLARE_FOR(usize);

COL_DECLARE_FOR( rawptr);
COL_DECLARE_FOR(cstring);

COL_DECLARE_FOR(MEM_Allocator);

// allocation/deallocation -----------------------------------------------------------------------------------------------------

// internal function; creates a new slice (without any type info)
COL_RawSlice COL_NewRawSlice(usize tySize, usize tyAlign, isize count, b8 skipInit, MEM_Allocator);

// internal function; clones an existing slice (without any type info)
COL_RawSlice COL_CloneRawSlice(usize tySize, usize tyAlign, COL_RawSlice slice, MEM_Allocator);

// internal function; resizes an existing slice (without any type info)
void COL_ResizeRawSlice(usize tySize, usize tyAlign, COL_RawSlice* slice, isize newCount, b8 skipInit, MEM_Allocator);

// internal function; deletes an existing slice (without any type info)
void COL_DeleteRawSlice(COL_RawSlice* slice, MEM_Allocator);

#define COL_NewSlice(ty, count, skipInit, allocator) \
    COL_SLICE_FROM_RAW( \
        ty, \
        COL_NewRawSlice( \
            sizeof(ty), \
            alignof(ty), \
            (isize) (count), \
            (b8) (skipInit), \
            (allocator) \
        ) \
    )

#ifdef __cplusplus

    EXTERN_C_END

    template <typename T>
    struct COL_SliceFromRaw
    {
        static_assert( sizeof(T) ==  sizeof(COL_RawSlice));
        static_assert(alignof(T) == alignof(COL_RawSlice));

        static inline T Convert(const COL_RawSlice& raw) { return *(T*) (&raw); }
    };

    EXTERN_C_BEGIN

    #define COL_CloneSliceInternal(sl, allocator) \
        (COL_SliceFromRaw<decltype(COL_AsSlice(sl))>::Convert( \
            COL_CloneRawSlice( \
                sizeof((sl).data[0]), \
                alignof(MSR_TYPEOF((sl).data[0])), \
                COL_RawSlice {(rawptr) (sl).data, (sl).count}, \
                (allocator) \
            ) \
        ))
#else
    #define COL_CloneSliceInternal(sl, allocator) \
        (MSR_TYPEOF(_Generic(((sl).raw), COL_RawSlice: (sl), COL_RawList: (sl).slice))) \
        { \
            .raw = COL_CloneRawSlice( \
                sizeof((sl).data[0]), \
                alignof(MSR_TYPEOF((sl).data[0])), \
                sl.raw, \
                (allocator) \
            ) \
        }
#endif

#define COL_CloneSlice(slice, allocator) COL_CloneSliceInternal(slice, allocator)

#define COL_ResizeSlice(slicePtr, newCount, skipInit, allocator) \
    COL_ResizeRawSlice( \
        sizeof((slicePtr)->data[0]), \
        alignof(MSR_TYPEOF((slicePtr)->data[0])), \
        COL_RAW_PTR_FROM_SLICE_PTR(slicePtr), \
        (isize) newCount, \
        (b8) skipInit, \
        (allocator) \
    )

#define COL_DeleteSlice(slicePtr, allocator) \
    COL_DeleteRawSlice( \
        COL_RAW_PTR_FROM_SLICE_PTR(slicePtr), \
        (allocator) \
    )

// internal function; creates a new list (without any type info)
COL_RawList COL_NewRawList(usize tySize, usize tyAlign, isize initialCap, MEM_Allocator);

// internal function; resizes an existing list (without any type info)
void COL_ResizeRawList(usize tySize, usize tyAlign, COL_RawList* list, isize newCap);

// internal function; ensures that the list has enough capacity to hold a given number of elements (without any type info)
void COL_EnsureRawListCapacity(usize tySize, usize tyAlign, COL_RawList* list, isize additionalCap);

// internal function; clears an existing list by setting the count to zero (without any type info)
void COL_ClearRawList(COL_RawList* list);

// internal function; deletes an existing list (without any type info)
void COL_DeleteRawList(COL_RawList* list);

#define COL_NewList(ty, initialCap, allocator) \
    COL_LIST_FROM_RAW( \
        ty, \
        COL_NewRawList( \
            sizeof(ty), \
            alignof(ty), \
            (initialCap), \
            (allocator) \
        ) \
    )

#define COL_AppendToList(listPtr, item) \
    do \
    { \
        COL_EnsureRawListCapacity( \
            sizeof((listPtr)->data[0]), \
            alignof(MSR_TYPEOF((listPtr)->data[0])), \
            COL_RAW_PTR_FROM_LIST_PTR(listPtr), \
            (listPtr)->count + (isize) (1) \
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
            COL_EnsureRawListCapacity( \
                sizeof((listPtr)->data[0]), \
                alignof(MSR_TYPEOF((listPtr)->data[0])), \
                COL_RAW_PTR_FROM_LIST_PTR(listPtr), \
                (listPtr)->count + (items).count \
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

#define COL_RemoveIdxFromList(listPtr, idx) \
    do \
    { \
        if ((idx) >= 0 && (idx) < (listPtr)->count) \
        { \
            MEM_Move( \
                &((listPtr)->data[(idx)]), \
                &((listPtr)->data[(idx) + 1]), \
                ((listPtr)->count - (idx) - 1) * (isize) sizeof((listPtr)->data[0]) \
            ); \
            (listPtr)->count--; \
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

#define COL_DeleteList(listPtr) \
    COL_DeleteRawList(COL_RAW_PTR_FROM_LIST_PTR(listPtr))

// slice/list utils ------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus

    #define COL_SLICE_INTERNAL(ty, ...) \
        (COL_SliceFromInitList_##ty({__VA_ARGS__}))

#else

    #define COL_SLICE_INTERNAL(ty, ...) \
        ((Slice_(ty)) {.data = (ty[]) {__VA_ARGS__}, .count = sizeof((ty[]) {__VA_ARGS__}) / sizeof(ty)})

#endif

/**
 * Declare an inline slice literal. Note that this will not be allocated using any specific
 * allocator, and is embedded in the application binary.
 */
#define SLICE(ty, ...) COL_SLICE_INTERNAL(ty, __VA_ARGS__)

#ifdef __cplusplus

    #define COL_SubSliceInternal(sl, st, cn) \
        (decltype(COL_AsSlice(sl)) \
        { \
            (0 > (isize) (st) || 0 > (isize) (cn) || ((isize) (st) + (isize) (cn)) > (sl).count) ? nil : &((sl).data[(st)]), \
            (0 > (isize) (st) || 0 > (isize) (cn) || ((isize) (st) + (isize) (cn)) > (sl).count) ? 0   : (isize) (cn), \
        })

#else

    #define COL_SubSliceInternal(sl, st, cn) \
        (MSR_TYPEOF(_Generic(((sl).raw), COL_RawSlice: (sl), COL_RawList: (sl).slice))) \
        { \
            .data  = (0 > (isize) (st) || 0 > (isize) (cn) || ((isize) (st) + (isize) (cn)) > (sl).count) ? nil : &((sl).data[(st)]), \
            .count = (0 > (isize) (st) || 0 > (isize) (cn) || ((isize) (st) + (isize) (cn)) > (sl).count) ? 0   : (isize) (cn), \
        }

#endif

/**
 * Sub-slice an slice with a start and count value.
 * Use as:
```
Slice_(i32) existing = some_func();
Slice_(i32) sub = COL_SubSlice(existing, 1, 3);
```
 * Returns an empty slice if the bounds checking fails.
 */
#define COL_SubSlice(sl, st, cn) (COL_SubSliceInternal(sl, st, cn))

EXTERN_C_END
