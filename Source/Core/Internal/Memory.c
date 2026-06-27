#include "CorePrivate.h"

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

usize MEM_VirtualPageSize(void)
{
    static usize staticPageSize = 0;
    if (staticPageSize)
        return staticPageSize;

    #if MSR_WINDOWS
    {
        SYSTEM_INFO systemInfo = {0};
        GetSystemInfo(&systemInfo);
        staticPageSize = (usize) systemInfo.dwPageSize;
    }
    #elif MSR_UNIX
    {
        long pageSize = sysconf(_SC_PAGESIZE);
        staticPageSize = pageSize < 0 ? 0 : (usize) pageSize;
    }
    #else
    {
        #error "unsupported platform"
    }
    #endif

    if (!staticPageSize)
        staticPageSize = 4096; // fallback to 4KiB if we couldn't get the page size

    MSR_ASSERT((staticPageSize & (staticPageSize - 1)) == 0 && "Virtual page size must be a power of 2");

    return staticPageSize;
}

rawptr MEM_VirtualReserve(usize size)
{
    if (!size)
        return nil;

    usize pageSize = MEM_VirtualPageSize();
    usize alignedSize = (size + (pageSize - 1)) & ~(pageSize - 1);
    MSR_ASSERT(alignedSize && "Virtual reserve size must be a power of 2");
    MSR_ASSERT(alignedSize >= size && "Virtual reserve size overflowed");

    #if MSR_WINDOWS
    {
        return VirtualAlloc(nil, (SIZE_T) alignedSize, MEM_RESERVE, PAGE_NOACCESS);
    }
    #elif MSR_UNIX
    {
        rawptr memory = mmap(nil, (size_t) alignedSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return (memory == MAP_FAILED) ? nil : memory;
    }
    #else
    {
        #error "unsupported platform"
    }
    #endif
}

b8 MEM_VirtualCommit(rawptr memory, usize size)
{
    if (!memory || !size)
        return false;

    usize pageSize = MEM_VirtualPageSize();
    MSR_ASSERT(((usize) memory & (pageSize - 1)) == 0 && "Virtual commit memory must be aligned to the virtual page size");

    usize alignedSize = (size + (pageSize - 1)) & ~(pageSize - 1);
    MSR_ASSERT(alignedSize && "Virtual commit size must be a power of 2");
    MSR_ASSERT(alignedSize >= size && "Virtual commit size overflowed");

    #if MSR_WINDOWS
    {
        rawptr result = VirtualAlloc(memory, (SIZE_T) alignedSize, MEM_COMMIT, PAGE_READWRITE);
        return result == memory;
    }
    #elif MSR_UNIX
    {
        return mprotect(memory, (size_t) alignedSize, PROT_READ | PROT_WRITE) == 0;
    }
    #else
    {
        #error "unsupported platform"
    }
    #endif
}

b8 MEM_VirtualDecommit(rawptr memory, usize size)
{
    if (!memory || !size)
        return false;

    usize pageSize = MEM_VirtualPageSize();
    MSR_ASSERT(((usize) memory & (pageSize - 1)) == 0 && "Virtual decommit memory must be aligned to the virtual page size");

    usize alignedSize = (size + (pageSize - 1)) & ~(pageSize - 1);
    MSR_ASSERT(alignedSize && "Virtual decommit size must be a power of 2");
    MSR_ASSERT(alignedSize >= size && "Virtual decommit size overflowed");

    #if MSR_WINDOWS
    {
        return VirtualFree(memory, (SIZE_T) alignedSize, MEM_DECOMMIT) != 0;
    }
    #elif MSR_UNIX
    {
        if (mprotect(memory, (size_t) alignedSize, PROT_NONE) != 0)
            return false;

        #if defined(MADV_DONTNEED)
            madvise(memory, (size_t) alignedSize, MADV_DONTNEED);
        #endif

        return true;
    }
    #else
    {
        #error "unsupported platform"
    }
    #endif
}

b8 MEM_VirtualRelease(rawptr memory, usize size)
{
    if (!memory || !size)
        return false;

    usize pageSize = MEM_VirtualPageSize();
    MSR_ASSERT(((usize) memory & (pageSize - 1)) == 0 && "Virtual release memory must be aligned to the virtual page size");

    usize alignedSize = (size + (pageSize - 1)) & ~(pageSize - 1);
    MSR_ASSERT(alignedSize && "Virtual release size must be a power of 2");
    MSR_ASSERT(alignedSize >= size && "Virtual release size overflowed");

    #if MSR_WINDOWS
    {
        (void) alignedSize; // unused
        return VirtualFree(memory, 0, MEM_RELEASE) != 0;
    }
    #elif MSR_UNIX
    {
        return munmap(memory, (size_t) alignedSize) == 0;
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

rawptr MEM_Clone(MEM_Allocator allocator, rawptr oldMem, usize size, usize align)
{
    if (!allocator.procedure)
        return nil;

    rawptr mem = MEM_Allocate(allocator, false, size, align);
    if (mem && oldMem)
    {
        MEM_Copy(mem, oldMem, size);
    }

    return mem;
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

MEM_VirtualListAllocator MEM_CreateVirtualListAllocator(usize reserveSize)
{
    if (!reserveSize)
        return (MEM_VirtualListAllocator) {0};

    usize pageSize = MEM_VirtualPageSize();
    usize alignedReserveSize = (reserveSize + (pageSize - 1)) & ~(pageSize - 1);
    if (alignedReserveSize < reserveSize)
        return (MEM_VirtualListAllocator) {0};

    return (MEM_VirtualListAllocator)
    {
        .reservedMemory = nil,
        .reservedSize = alignedReserveSize,
        .committedSize = 0,
        .hasActiveAllocation = false,
    };
}

MEM_Allocator MEM_AllocatorFromVirtualList(MEM_VirtualListAllocator* allocator)
{
    return (MEM_Allocator) {.data = allocator, .procedure = MEM_VirtualListAllocatorProc};
}

void MEM_DestroyVirtualListAllocator(MEM_VirtualListAllocator* allocator)
{
    if (!allocator)
        return;

    MEM_DeallocateAll(MEM_AllocatorFromVirtualList(allocator));
    *allocator = (MEM_VirtualListAllocator) {0};
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
            if (newBlockSize < (1 * 1024))
                newBlockSize = 1 * 1024; // minimum block size of 1 KB

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

rawptr MEM_VirtualListAllocatorProc(
    MEM_AllocatorMode mode,
    rawptr data,
    usize  size,
    usize  align,
    rawptr oldMem,
    usize  oldSize
)
{
    (void) align;
    (void) oldSize;

    MEM_VirtualListAllocator* allocator = (MEM_VirtualListAllocator*) data;
    if (!allocator)
        return nil;

    usize pageSize = MEM_VirtualPageSize();
    if (!pageSize)
        return nil;

    if (!allocator->reservedSize)
        return nil;

    usize alignedSize = (size + (pageSize - 1)) & ~(pageSize - 1);
    if (alignedSize < size)
        return nil;

    if (mode == MEM_AllocatorMode_DeallocateAll)
    {
        if (allocator->reservedMemory)
        {
            MEM_VirtualRelease(allocator->reservedMemory, allocator->reservedSize);
        }

        allocator->reservedMemory = nil;
        allocator->committedSize = 0;
        allocator->hasActiveAllocation = false;
        return nil;
    }

    if (mode == MEM_AllocatorMode_Allocate || mode == MEM_AllocatorMode_AllocateUninitialised)
    {
        // One live allocation at a time. Caller must deallocate before allocating again.
        MSR_ASSERT(!allocator->hasActiveAllocation && "Virtual list allocator supports only one live allocation");
        if (allocator->hasActiveAllocation)
            return nil;

        if (!allocator->reservedMemory)
        {
            allocator->reservedMemory = MEM_VirtualReserve(allocator->reservedSize);
            if (!allocator->reservedMemory)
                return nil;
        }

        if (alignedSize > allocator->reservedSize)
        {
            // out of memory
            return nil;
        }

        if (alignedSize > allocator->committedSize)
        {
            rawptr commitBegin = ((u8*) allocator->reservedMemory) + allocator->committedSize;
            usize commitSize = alignedSize - allocator->committedSize;
            b8 committed = MEM_VirtualCommit(commitBegin, commitSize);
            MSR_ASSERT(committed && "Virtual list allocator failed to commit pages");
            if (!committed)
                return nil;

            allocator->committedSize = alignedSize;
        }

        allocator->hasActiveAllocation = true;
        return allocator->reservedMemory;
    }

    if (mode == MEM_AllocatorMode_Reallocate || mode == MEM_AllocatorMode_ReallocateUninitialised)
    {
        if (!allocator->hasActiveAllocation && oldMem == nil)
        {
            // no active allocation, so we can treat this as an allocate
            MEM_AllocatorMode m2 = MEM_AllocatorMode_AllocateUninitialised;
            if (m2 == MEM_AllocatorMode_Reallocate)
                m2 = MEM_AllocatorMode_Allocate;

            return MEM_VirtualListAllocatorProc(m2, data, size, align, nil, 0);
        }

        MSR_ASSERT(oldMem == allocator->reservedMemory && "Virtual list allocator reallocate must use original address");
        if (oldMem != allocator->reservedMemory)
            return nil;

        if (alignedSize > allocator->reservedSize)
        {
            // out of memory
            return nil;
        }

        if (alignedSize > allocator->committedSize)
        {
            rawptr commitBegin = ((u8*) allocator->reservedMemory) + allocator->committedSize;
            usize commitSize = alignedSize - allocator->committedSize;
            b8 committed = MEM_VirtualCommit(commitBegin, commitSize);
            MSR_ASSERT(committed && "Virtual list allocator failed to commit pages while growing");
            if (!committed)
                return nil;
        }
        else if (alignedSize < allocator->committedSize)
        {
            rawptr decommitBegin = ((u8*) allocator->reservedMemory) + alignedSize;
            usize decommitSize = allocator->committedSize - alignedSize;
            b8 decommitted = MEM_VirtualDecommit(decommitBegin, decommitSize);
            MSR_ASSERT(decommitted && "Virtual list allocator failed to decommit pages while shrinking");
            if (!decommitted)
                return nil;
        }

        allocator->committedSize = alignedSize;
        return allocator->reservedMemory;
    }

    if (mode == MEM_AllocatorMode_Deallocate)
    {
        if (!oldMem || !allocator->hasActiveAllocation)
            return nil;

        MSR_ASSERT(oldMem == allocator->reservedMemory && "Virtual list allocator deallocate must use original address");
        if (oldMem != allocator->reservedMemory)
            return nil;

        if (allocator->committedSize)
        {
            b8 decommitted = MEM_VirtualDecommit(allocator->reservedMemory, allocator->committedSize);
            MSR_ASSERT(decommitted && "Virtual list allocator failed to decommit memory on deallocate");
            if (!decommitted)
                return nil;
        }

        allocator->committedSize = 0;
        allocator->hasActiveAllocation = false;
        return nil;
    }

    MSR_ASSERT(false && "invalid allocator mode for virtual list allocator");
    return nil;
}

MEM_Allocator MEM_GetMainAllocator(void)
{
    return (MEM_Allocator) {.procedure = MEM_DefaultAllocatorProc, .data = nil};
}
