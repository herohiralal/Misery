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
    static inline COL_RawSlice* COL_CreateRawSlicePtr(Slice<T>* slice) { return (COL_RawSlice*) (slice); }

    template <typename T>
    static inline COL_RawList* COL_CreateRawListPtr(List<T>* list) { return (COL_RawList*) (list); }

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
        template <> \
        struct Slice<ty> \
        { \
            ty*   data; \
            isize count; \
            \
            Slice<ty>() = default; \
            Slice<ty>(const ty* inData, isize inCount) : data(const_cast<ty*>(inData)), count(inCount) { } \
            Slice<ty>(const Slice_(ty)& other) : data(other.data), count(other.count) { } \
            Slice<ty>(const List_(ty)& other) : data(other.data), count(other.count) { } \
            Slice<ty>(std::initializer_list<ty> init) : data(const_cast<ty*>(init.begin())), count((isize) init.size()) { } \
            explicit Slice<ty>(const COL_RawSlice& other) { *this = *reinterpret_cast<const Slice<ty>*>(&other); } \
            operator Slice_(ty)() const { return Slice_(ty) {data, count}; } \
            Slice_(ty) AsCSlice() const { return Slice_(ty) {data, count}; } \
        }; \
        \
        template <> \
        struct List<ty> \
        { \
            ty*           data; \
            isize         count; \
            isize         capacity; \
            MEM_Allocator allocator; \
            \
            List<ty>() = default; \
            List<ty>(const ty* d, isize cn, isize cp, MEM_Allocator a) : data(const_cast<ty*>(d)), count(cn), capacity(cp), allocator(a) { } \
            List<ty>(const List_(ty)& other) : data(other.data), count(other.count), capacity(other.capacity), allocator(other.allocator) { } \
            explicit List<ty>(const COL_RawList& other) { *this = *reinterpret_cast<const List<ty>*>(&other); } \
            operator List_(ty)() const { return List_(ty) {data, count, capacity, allocator}; } \
            operator Slice_(ty)() const { return Slice_(ty) {data, count}; } \
            operator Slice<ty>() const { return Slice<ty>(data, count); } \
            List_(ty) AsCList() const { return List_(ty) {data, count, capacity, allocator}; } \
        }; \
        \
        static inline Slice<ty> COL_AsSlice(const Slice_(ty)& s) { return Slice<ty>(s); } \
        static inline Slice<ty> COL_AsSlice(const Slice<ty>& s) { return s; } \
        static inline Slice<ty> COL_AsSlice(const List_(ty)& l) { return Slice<ty>(l); } \
        static inline Slice<ty> COL_AsSlice(const List<ty>& l) { return (Slice<ty>) l; } \
        static inline COL_RawSlice* COL_CreateRawSlicePtr(Slice_(ty)* slice) { return (COL_RawSlice*) (slice); } \
        static inline COL_RawList* COL_CreateRawListPtr(List_(ty)* list) { return (COL_RawList*) (list); } \
        EXTERN_C_BEGIN

    #define COL_SLICE_FROM_RAW(ty, raw) (Slice<ty>(raw).AsCSlice())

    #define COL_LIST_FROM_RAW(ty, raw) (List<ty>(raw).AsCList())

    #define COL_RAW_PTR_FROM_SLICE_PTR(sl) (COL_CreateRawSlicePtr(sl))

    #define COL_RAW_PTR_FROM_LIST_PTR(ls) (COL_CreateRawListPtr(ls))

    EXTERN_C_BEGIN

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

COL_DECLARE_FOR( rawptr);
COL_DECLARE_FOR(cstring);

COL_DECLARE_FOR(MEM_Allocator);

// allocation/deallocation -----------------------------------------------------------------------------------------------------

// internal function; creates a new slice (without any type info)
COL_RawSlice COL_NewRawSlice(usize tySize, usize tyAlign, MEM_Allocator, isize count, b8 skipInit OPT_ARG);

// internal function; clones an existing slice (without any type info)
COL_RawSlice COL_CloneRawSlice(usize tySize, usize tyAlign, MEM_Allocator, COL_RawSlice slice);

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

#ifdef __cplusplus
    #define COL_CloneSliceInternal(slice, allocator) \
        ((decltype(slice) \
        { \
            COL_CloneRawSlice( \
                sizeof((slice).data[0]), \
                alignof(MSR_TYPEOF((slice).data[0])), \
                (allocator), \
                COL_RawSlice {(rawptr) (slice).data, (slice).count} \
            ) \
        }).AsCSlice())
#else
    #define COL_CloneSliceInternal(slice, allocator) \
        (MSR_TYPEOF(slice)) \
        { \
            .raw = COL_CloneSlice( \
                sizeof((slice).data[0]), \
                alignof(MSR_TYPEOF((slice).data[0])), \
                (allocator), \
                slice.raw \
            ) \
        }
#endif

#define COL_CloneSlice(slice, allocator) COL_CloneSliceInternal(slice, allocator)

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

// internal function; ensures that the list has enough capacity to hold a given number of elements (without any type info)
void COL_EnsureRawListCapacity(usize tySize, usize tyAlign, COL_RawList* list, isize additionalCap);

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

#ifdef __cplusplus

    #define COL_SLICE_INTERNAL(ty, ...) \
        ((Slice_(ty)) Slice<ty>({__VA_ARGS__}))

#else

    #define COL_SLICE_INTERNAL(ty, ...) \
        ((Slice_(ty)) \
        { \
            .data = (ty[]) {__VA_ARGS__}, \
            .count = sizeof((ty[]) {__VA_ARGS__}) / sizeof(ty), \
        })

#endif

/**
 * Declare an inline slice literal. Note that this will not be allocated using any specific
 * allocator, and is embedded in the application binary.
 */
#define SLICE(ty, ...) COL_SLICE_INTERNAL(ty, __VA_ARGS__)

#ifdef __cplusplus

    EXTERN_C_END

    template <typename T>
    static inline Slice<T> COL_SubSliceInternalCxx(Slice<T> sl, isize st, isize cn)
    {
        if (st < 0 || cn < 0 || (st + cn) > sl.count)
            return Slice<T>();

        sl.data = &(sl.data[st]);
        sl.count = cn;
        return sl;
    }

    EXTERN_C_BEGIN

    #define COL_SubSliceInternal(sl, st, cn) \
        (COL_SubSliceInternalCxx(COL_AsSlice(sl), (isize) (st), (isize) (cn)).AsCSlice())

#else

    #define COL_SubSliceInternal(sl, st, cn) \
        (MSR_TYPEOF(_Generic((sl.raw), COL_RawSlice: (sl), COL_RawList: (sl).slice))) \
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
