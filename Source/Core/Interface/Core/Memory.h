#pragma once
#include <__init.h>
EXTERN_C_BEGIN

// basics ----------------------------------------------------------------------------------------------------------------------

/**
 * Copy a block of non-overlapping memory from source to destination.
 */
void MEM_Copy(rawptr dst, const void* src, usize num);

/**
 * Copy a block of memory from source to destination.
 */
void MEM_Move(rawptr dst, const void* src, usize num);

/**
 * Set a block of memory to a specific value.
 */
void MEM_Set(rawptr dst, i32 value, usize num);

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
 * Defines a generic allocator structure that can be used to allocate, reallocate, and deallocate memory.
 */
typedef struct
{
    MEM_AllocatorProc procedure;
    rawptr            data; // Optional data for the allocator function
} MEM_Allocator;

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

/**
 * Duplicate existing memory with the provided allocator.
 */
rawptr MEM_Clone(MEM_Allocator, rawptr oldMem, usize size, usize align);

// new/delete ------------------------------------------------------------------------------------------------------------------

/**
 * Allocate a new object with the provided allocator.
 */
#define MEM_New(ty, allocator) \
    ((ty*) MEM_Allocate((allocator), true, (usize) sizeof(ty), (usize) alignof(ty)))

/**
 * Deallocate an existing object with the provided allocator.
 */
#define MEM_Delete(ptr, allocator) \
    ((void) MEM_Deallocate((allocator), (rawptr) ptr))

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
 * will be freed in bulk. It uses a thread-local arena allocator under the hood, so it is not thread-safe and should only
 * be used for allocations with a small lifetime ( < 1 function / frame).
 */
#define MEM_temp (MEM_GetTempAllocator())

EXTERN_C_END
