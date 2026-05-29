#pragma once
#include <__init.h>
EXTERN_C_BEGIN

// allocators ------------------------------------------------------------------------------------------------------------------

/**
 * Defines the mode to be used when calling the allocator function.
 */
typedef u8 MEM_AllocatorMode;
enum MEM_AllocatorModes
{
    MEM_AllocatorMode_Allocate,
    MEM_AllocatorMode_AllocateUninitialised,
    MEM_AllocatorMode_Reallocate,
    MEM_AllocatorMode_ReallocateUninitialised,
    MEM_AllocatorMode_Deallocate,
    MEM_AllocatorMode_DeallocateAll,
};

/**
 * Defines the delegate type for the allocator function.
 */
typedef rawptr (*MEM_AllocatorProc)(MEM_AllocatorMode, rawptr data, usize size, usize align, rawptr oldMem, usize oldSize);

/**
 * Defines a generic allocator structure that can be used to allocate, resize, and free memory.
 */
typedef struct
{
    MEM_AllocatorProc procedure;
    rawptr            data; // Optional data for the allocator function
} MEM_Allocator;

// collections -----------------------------------------------------------------------------------------------------------------

/**
 * A raw type-unspecific slice.
 */
typedef struct MEM_RawSlice { rawptr data; isize count; } MEM_RawSlice;

/**
 * A raw type-unspecific list.
 */
typedef struct MEM_RawList { rawptr data; isize count; isize capacity; MEM_Allocator allocator; } MEM_RawList;

#define Slice_(ty) MEM_Slice_##ty
#define List_(ty)  MEM_List_##ty

#ifdef __cplusplus

    EXTERN_C_END

    template <typename T>
    struct Slice { T* data; isize count; };

    template <typename T>
    struct List { T* data; isize count; isize capacity; MEM_Allocator allocator; };

    EXTERN_C_BEGIN

    #define MEM_DECLARE_SLICE(ty) \
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
            operator Slice_(ty)() const { return {data, count}; } \
        }; \
        \
        static inline Slice<ty>*  CreateSlicePtr(Slice_(ty)* slice) {  return reinterpret_cast<Slice<ty>*>(slice); } \
        static inline Slice_(ty)* CreateSlicePtr(Slice<ty>*  slice) { return reinterpret_cast<Slice_(ty)*>(slice); } \
        EXTERN_C_BEGIN

    #define MEM_DECLARE_LIST(ty) \
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
            operator List_(ty)() const { return {data, count, capacity, allocator}; } \
        }; \
        \
        static inline List<ty>*  CreateListPtr(List_(ty)* list) {  return reinterpret_cast<List<ty>*>(list); } \
        static inline List_(ty)* CreateListPtr(List<ty>*  list) { return reinterpret_cast<List_(ty)*>(list); } \
        EXTERN_C_BEGIN

#else

    #define MEM_DECLARE_SLICE(ty) \
        typedef union \
        { \
            struct \
            { \
                ty*   data; \
                isize count; \
            }; \
            MEM_RawSlice raw; \
        } Slice_(ty);

    #define MEM_DECLARE_LIST(ty) \
        typedef union \
        { \
            struct \
            { \
                ty*           data; \
                isize         count; \
                isize         capacity; \
                MEM_Allocator allocator; \
            }; \
            MEM_RawList raw; \
        } List_(ty);

#endif

MEM_DECLARE_SLICE(  b8);
MEM_DECLARE_SLICE(  u8);
MEM_DECLARE_SLICE( u16);
MEM_DECLARE_SLICE( u32);
MEM_DECLARE_SLICE( u64);
MEM_DECLARE_SLICE(  i8);
MEM_DECLARE_SLICE( i16);
MEM_DECLARE_SLICE( i32);
MEM_DECLARE_SLICE( i64);
MEM_DECLARE_SLICE( f32);
MEM_DECLARE_SLICE( f64);
MEM_DECLARE_SLICE(char);

MEM_DECLARE_LIST(  b8);
MEM_DECLARE_LIST(  u8);
MEM_DECLARE_LIST( u16);
MEM_DECLARE_LIST( u32);
MEM_DECLARE_LIST( u64);
MEM_DECLARE_LIST(  i8);
MEM_DECLARE_LIST( i16);
MEM_DECLARE_LIST( i32);
MEM_DECLARE_LIST( i64);
MEM_DECLARE_LIST( f32);
MEM_DECLARE_LIST( f64);
MEM_DECLARE_LIST(char);

/**
 * UTF-8 string type, with length info (not necessarily null-terminated).
 */
typedef Slice_(u8) utf8str;
MEM_DECLARE_SLICE(utf8str);
MEM_DECLARE_LIST(utf8str);

/**
 * Get a subslice of the provided slice/list, starting at 'start' and containing 'count' elements.
 * Note: if the bounds check fail, the returned slice will have a null data pointer and a count of 0.
 * Use as:
 * ```
 * Slice_(i32) s = MEM_SUBSLICE(sliceOrList, start, count);
 * ```
 * Note that in c++, the following won't work:
 * ```
 * auto s = MEM_SUBSLICE(sliceOrList, start, count); // WILL NOT COMPILE!!
 * ```
 * Can also be used when passing parameters to functions:
 * ```
 * int some_func(Slice_(i32) s);
 * int main()
 * {
 *     // ...
 *     #ifdef __cplusplus
 *         // c++ supports aggregate initialisation
 *         some_func(MEM_SUBSLICE(sliceOrList, start, count));
 *         // OR
 *         some_func(Slice_(i32) MEM_SUBSLICE(sliceOrList, start, count));
 *         // OR
 *         some_func(Slice<i32> MEM_SUBSLICE(sliceOrList, start, count));
 *     #else
 *         // c requires this weird cast-like syntax
 *         some_func((Slice_(i32)) MEM_SUBSLICE(sliceOrList, start, count));
 *     #endif
 *     // ...
 * }
 * ```
 */
#define MEM_SUBSLICE(sl, st, cnt) \
    { \
        .data  = ((st) < 0 || (cnt) < 0 || ((st) + (cnt)) > (sl).count) ? nil : &((sl).data[(st)]), \
        .count = ((st) < 0 || (cnt) < 0 || ((st) + (cnt)) > (sl).count) ? 0   : (cnt), \
    }

// allocator functions ---------------------------------------------------------------------------------------------------------

/**
 * Allocate memory using the provided allocator.
 */
rawptr MEM_Allocate(MEM_Allocator, b8 zeroed, usize size, usize alignment);

/**
 * Reallocate memory using the provided allocator.
 */
rawptr MEM_Reallocate(MEM_Allocator, b8 zeroed, rawptr oldMem, usize oldSize, usize newSize, usize alignment);

/**
 * Fallback reallocate function that can be used when the allocator does not support any specialised reallocation.
 */
rawptr MEM_DefaultReallocate(MEM_Allocator, b8 zeroed, rawptr oldMem, usize oldSize, usize newSize, usize alignment);

/**
 * Deallocate memory using the provided allocator.
 */
void MEM_Deallocate(MEM_Allocator, rawptr memory);

/**
 * Deallocate all memory allocated by the provided allocator.
 * Returns true if the allocator successfully deallocated all memory, false otherwise.
 * (e.g. if the allocator does not support this operation).
 */
b8 MEM_DeallocateAll(MEM_Allocator);

// allocator implementations ---------------------------------------------------------------------------------------------------

/**
 * Default allocator that uses the standard library's malloc/free for memory management.
 * This is the most basic allocator and should be used as a fallback when no other allocator is available.
 */
rawptr MEM_DefaultAllocatorProc(MEM_AllocatorMode, rawptr data, usize size, usize align, rawptr oldMem, usize oldSize);

/**
 * Arena allocator that uses a linked-list of memory blocks to manage allocations.
 * This allocator is designed for short-term allocations that will be freed in bulk, and is not thread-safe.
 */
rawptr MEM_ArenaAllocatorProc(MEM_AllocatorMode, rawptr data, usize size, usize align, rawptr oldMem, usize oldSize);

/**
 * The payload for an arena allocator.
 */
typedef struct
{
    usize         defaultBlockSize;
    MEM_Allocator backingAllocator;
    rawptr        currentBlock;
    usize         currentBlockCapacity;
    usize         currentBlockOffset;
} MEM_ArenaAllocator;

/**
 * Create a new arena allocator with the specified default block size and backing allocator.
 * If backing allocator is null, this will use the default allocator for allocating blocks.
 * If default block size is less than 16KB, it will be rounded up to 16 KB to avoid fragmentation
 */
MEM_ArenaAllocator MEM_CreateArenaAllocator(usize defaultBlockSize, MEM_Allocator backingAllocator);

/**
 * Cast an arena allocator to a generic allocator that can be used with the allocation functions.
 * The parameter is the arena allocator payload address.
 */
MEM_Allocator MEM_AllocatorFromArena(MEM_ArenaAllocator* allocator);

/**
 * Completely free all memory allocated by the arena allocator.
 */
void MEM_DestroyArenaAllocator(MEM_ArenaAllocator* allocator);

// global allocator instances --------------------------------------------------------------------------------------------------

// see MEM_main
MEM_Allocator MEM_GetMainAllocator(void);

/**
 * The primary/main allocator. This is a general-purpose thread-safe allocator that can be used for most allocation needs.
 * It is not optimised for any specific use case, but is a good default choice for most situations.
 */
#define MEM_main (MEM_GetMainAllocator())

// see MEM_temp
MEM_Allocator MEM_GetTempAllocator(void);

/**
 * The temporary allocator instance. This is a general-purpose allocator that is optimised for short-term allocations that
 * will be freed in bulk. It uses a thread-local arena allocator under the hood, so it is not thread-safe and should only be used
 * for allocations with a small lifetime ( < 1 function / frame).
 */
#define MEM_temp (MEM_GetTempAllocator())

EXTERN_C_END
