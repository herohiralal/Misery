#pragma once
#include <__init.h>
#include <Core/Collections.h>

/**
 * Arena allocator that uses a linked-list of memory blocks to manage allocations.
 * This allocator is designed for short-term allocations that will be freed in bulk, and is not thread-safe.
 */
class ArenaAllocator final : public IAllocator
{
private:
    size_t defaultBlockSize;
    Allocator backingAllocator;
    void* currentBlock;
    size_t currentBlockCapacity;
    size_t currentBlockOffset;

protected:
    void* Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed = false) override;
    void Deallocate(SrcLoc loc, void* ptr) override;
    bool DeallocateAll(SrcLoc loc) override;

public:
    ArenaAllocator() = default;

    // if backing allocator is null, this will use the default allocator for allocating blocks
    // if default block size is less than 16KB, it will be rounded up to 16 KB to avoid fragmentation
    ArenaAllocator(size_t defaultBlockSize, Allocator backingAllocator)
        : defaultBlockSize(defaultBlockSize),
          backingAllocator(backingAllocator),
          currentBlock(nullptr),
          currentBlockCapacity(0),
          currentBlockOffset(0)
    { }

    // completely free all memory allocated by this arena
    void Destroy();
};
