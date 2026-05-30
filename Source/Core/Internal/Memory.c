#include <Core/Memory.h>

void MEM_Copy(rawptr dst, const void* src, usize num) { memcpy(dst, src, (size_t) num); }
void MEM_Move(rawptr dst, const void* src, usize num) { memmove(dst, src, (size_t) num); }
void MEM_Set(rawptr dst, i32 value, usize num) { memset(dst, (int) value, num); }

static usize MEM_Internal_FixAlign(usize inAlign)
{
    if (inAlign == 0)
        return 1;

    if ((inAlign & (inAlign - 1)) == 0)
        return inAlign;

    #if MSR_MSVC
    {
        unsigned long idx;
        #if MSR_PTR_SIZE == 8
            _BitScanReverse64(&idx, inAlign);
        #else
            _BitScanReverse(  &idx, inAlign);
        #endif
        return ((usize) 1) << (usize) (idx + 1);
    }
    #elif MSR_GCC || MSR_CLANG
    {
        #if MSR_PTR_SIZE == 8
            return ((usize) 1) << (usize) (64 - __builtin_clzll(inAlign));
        #else
            return ((usize) 1) << (usize) (32 - __builtin_clz(  inAlign));
        #endif
    }
    #else
    {
        #error "unsupported platform"
    }
    #endif
}

rawptr MEM_Allocate(
    MEM_Allocator allocator,
    b8    zeroed,
    usize size,
    usize alignment
)
{
    if (!allocator.procedure)
        return nil;

    MEM_AllocatorMode mode = zeroed ? MEM_AllocatorMode_Allocate : MEM_AllocatorMode_AllocateUninitialised;
    return allocator.procedure(mode, allocator.data, size, alignment, nil, 0);
}

rawptr MEM_Reallocate(
    MEM_Allocator allocator,
    b8     zeroed,
    rawptr oldMem,
    usize  oldSize,
    usize  newSize,
    usize  alignment
)
{
    if (!allocator.procedure)
        return nil;

    MEM_AllocatorMode mode = zeroed ? MEM_AllocatorMode_Reallocate : MEM_AllocatorMode_ReallocateUninitialised;
    return allocator.procedure(mode, allocator.data, newSize, alignment, oldMem, oldSize);
}

rawptr MEM_DefaultReallocate(
    MEM_Allocator allocator,
    b8     zeroed,
    rawptr oldMem,
    usize  oldSize,
    usize  newSize,
    usize  alignment
)
{
    if (!allocator.procedure)
        return nil;

    if (newSize == oldSize)
        return oldMem;

    if (newSize == 0)
    {
        MEM_Deallocate(allocator, oldMem);
        return nil;
    }

    rawptr newMem = MEM_Allocate(allocator, zeroed, newSize, alignment);
    MSR_ASSERT(newMem && "failed to allocate more memory");

    if (oldMem)
    {
        usize copySize = (oldSize < newSize) ? oldSize : newSize;
        MEM_Copy(newMem, oldMem, copySize);
        MEM_Deallocate(allocator, oldMem);
    }

    return newMem;
}

void MEM_Deallocate(MEM_Allocator allocator, rawptr memory)
{
    if (!allocator.procedure)
        return;

    allocator.procedure(MEM_AllocatorMode_Deallocate, allocator.data, 0, 0, memory, 0);
}

b8 MEM_DeallocateAll(MEM_Allocator allocator)
{
    if (!allocator.procedure)
        return false;

    rawptr result = allocator.procedure(MEM_AllocatorMode_DeallocateAll, allocator.data, 0, 0, nil, 0);
    return !result; // if the allocator returns some value, it successfully deallocated all memory
}

MEM_ArenaAllocator MEM_CreateArenaAllocator(usize defaultBlockSize, MEM_Allocator backingAllocator)
{
    MEM_ArenaAllocator arena = {0};
    arena.defaultBlockSize = defaultBlockSize;
    arena.backingAllocator = backingAllocator;
    return arena;
}

MEM_Allocator MEM_AllocatorFromArena(MEM_ArenaAllocator* allocator)
{
    return (MEM_Allocator) {.data  = allocator, .procedure = MEM_ArenaAllocatorProc};
}

void MEM_DestroyArenaAllocator(MEM_ArenaAllocator* allocator)
{
    if (!allocator)
        return;

    MEM_DeallocateAll(MEM_AllocatorFromArena(allocator));

    if (allocator->currentBlock)
        MEM_Deallocate(allocator->backingAllocator, allocator->currentBlock);

    allocator->currentBlock = nil;
    allocator->currentBlockOffset = 0;
    allocator->currentBlockCapacity = 0;
}

rawptr MEM_DefaultAllocatorProc(
    MEM_AllocatorMode mode,
    rawptr data,
    usize  size,
    usize  align,
    rawptr oldMem,
    usize  oldSize
)
{
    if (mode == MEM_AllocatorMode_DeallocateAll)
        return nil; // fail

    if (mode == MEM_AllocatorMode_Reallocate ||
        mode == MEM_AllocatorMode_ReallocateUninitialised)
    {
        b8 zeroed = (mode == MEM_AllocatorMode_Reallocate);
        return MEM_DefaultReallocate(MEM_main, zeroed, oldMem, oldSize, size, align);
    }

    if (mode == MEM_AllocatorMode_Deallocate)
    {
        if (!oldMem)
            return nil; // no-op

        #if MSR_WINDOWS
            _aligned_free(oldMem);
        #elif MSR_UNIX
            free(oldMem);
        #else
            #error "Unknown platform. Cannot implement default allocator."
        #endif

        return nil;
    }

    if (mode == MEM_AllocatorMode_Allocate ||
        mode == MEM_AllocatorMode_AllocateUninitialised)
    {
        align = MEM_Internal_FixAlign(align);

        // do not remove
        // android, ios and osx will crash if we don't do these 2 things
        align = (align > sizeof(rawptr)) ? align : sizeof(rawptr);
        size = (size + (align - 1)) & ~(align - 1);

        rawptr memory = nil;
        #if MSR_WINDOWS
            memory = _aligned_malloc(size, align);
        #elif MSR_UNIX
            memory = aligned_alloc(align, size);
        #else
            #error "unsupported platform"
        #endif

        if (memory && (mode == MEM_AllocatorMode_Allocate))
            MEM_Set(memory, 0, size);

        return memory;
    }

    MSR_ASSERT(false);
    return nil; // invalid mode
}

rawptr MEM_ArenaAllocatorProc(
    MEM_AllocatorMode mode,
    rawptr data,
    usize  size,
    usize  align,
    rawptr oldMem,
    usize  oldSize
)
{
    MEM_ArenaAllocator* arena = (MEM_ArenaAllocator*) data;
    if (!arena)
        return nil;

    if (mode == MEM_AllocatorMode_Deallocate)
        return nil; // no-op

    if (mode == MEM_AllocatorMode_Reallocate ||
        mode == MEM_AllocatorMode_ReallocateUninitialised)
    {
        MEM_Allocator arenaAllocator = {.procedure = MEM_ArenaAllocatorProc, .data = data};
        b8 zeroed = (mode == MEM_AllocatorMode_Reallocate);
        return MEM_DefaultReallocate(arenaAllocator, zeroed, oldMem, oldSize, size, align);
    }

    typedef struct
    {
        rawptr previous;
        usize capacity;
    } MEM_ArenaAllocatorBlockHeader;

    if (mode == MEM_AllocatorMode_DeallocateAll)
    {
        // leave 1 block allocated, but reset the offset
        rawptr block = arena->currentBlock;
        while (block)
        {
            MEM_ArenaAllocatorBlockHeader* header = (MEM_ArenaAllocatorBlockHeader*) block;
            rawptr prevBlock = header->previous;
            if (!prevBlock)
            {
                // first block!
                arena->currentBlock = block;
                arena->currentBlockOffset = sizeof(MEM_ArenaAllocatorBlockHeader);
                arena->currentBlockCapacity = header->capacity;
                return nil;
            }

            MEM_Deallocate(arena->backingAllocator, block);
            block = prevBlock;
        }

        return nil;
    }

    if (mode == MEM_AllocatorMode_Allocate ||
        mode == MEM_AllocatorMode_AllocateUninitialised)
    {
        align = MEM_Internal_FixAlign(align);

        // make sure the size is aligned by the alignment, to avoid fragmentation
        usize alignedSize = (size + align - 1) & ~(align - 1);

        // align the offset to the required alignment
        usize alignedPtr = ((usize) arena->currentBlock) + arena->currentBlockOffset;
        alignedPtr = (alignedPtr + align - 1) & ~(align - 1);
        usize alignedOffset = alignedPtr - (usize) arena->currentBlock;

        if (alignedOffset + alignedSize > arena->currentBlockCapacity)
        {
            usize newBlockSize = arena->defaultBlockSize;
            if (newBlockSize < (16 * 1024))
                newBlockSize = 16 * 1024; // minimum block size of 16 KB

            usize newBlockAlign = sizeof(MEM_ArenaAllocatorBlockHeader);

            // if the requested size is larger than the default block size, use it as the new block size
            // plus the sizeof(void*)-sized first element in every allocation is to store the pointer to
            // the previous block
            // so we need to account for that in the block size calculation to ensure we have enough space
            // for the allocation even if the requested size is larger than the default block size
            {
                // our alignment is more than required, so we need to increase the block size to accommodate
                // the alignment padding
                if (newBlockAlign >= align && newBlockSize < (alignedSize + sizeof(MEM_ArenaAllocatorBlockHeader)))
                {
                    newBlockSize = alignedSize + sizeof(MEM_ArenaAllocatorBlockHeader);
                }

                // our alignment is less than required, so we need to increase the
                // block size to accommodate the worst-case alignment padding
                // and we use the requested alignment as the new block alignment
                // to ensure we can satisfy the alignment requirement
                if (newBlockAlign < align && newBlockSize < (alignedSize + align))
                {
                    newBlockSize = alignedSize + align;
                    newBlockAlign = align;
                }
            }

            // allocate the new block
            rawptr newBlock = MEM_Allocate(arena->backingAllocator, false, newBlockSize, newBlockAlign);
            MSR_ASSERT(newBlock && "Failed to allocate memory for arena block");

            // store the pointer to the previous block at the start of the new block
            MEM_ArenaAllocatorBlockHeader* header = (MEM_ArenaAllocatorBlockHeader*) newBlock;
            header->previous = arena->currentBlock;
            header->capacity = newBlockSize;
            arena->currentBlock = newBlock;
            arena->currentBlockOffset = sizeof(MEM_ArenaAllocatorBlockHeader);
            arena->currentBlockCapacity = newBlockSize;

            // align the offset to the required alignment
            alignedPtr = ((usize) arena->currentBlock) + arena->currentBlockOffset;
            alignedPtr = (alignedPtr + align - 1) & ~(align - 1);
            alignedOffset = alignedPtr - (usize) arena->currentBlock;
        }

        MSR_ASSERT(alignedOffset + alignedSize <= arena->currentBlockCapacity && "Block size is too small to accommodate the allocation");

        rawptr result = (rawptr) alignedPtr;
        arena->currentBlockOffset = alignedOffset + alignedSize;

        b8 zeroed = (mode == MEM_AllocatorMode_Allocate);
        if (zeroed)
            MEM_Set(result, 0, size);

        return result;
    }

    MSR_ASSERT(false);
    return nil; // invalid mode
}

MEM_Allocator MEM_GetMainAllocator()
{
    return (MEM_Allocator) {.procedure = MEM_DefaultAllocatorProc, .data = nil};
}
