#include <Core/Allocators/Default.h>
#include <Core/Allocators/Arena.h>

void* DefaultAllocator::Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed)
{
    if (size < 0)
        return nullptr; // size cannot be negative

    if ((alignment < 1) || !!(alignment & (alignment - 1)))
        return nullptr; // alignment must be a power of two

    // on apple platforms, we use posix_memalign to ensure proper alignment
    // linux/windows work fine, but apple being apple, it doesn't
    // same shit on android
    alignment = (alignment > sizeof(void*)) ? alignment : sizeof(void*);
    size = (size + (alignment - 1)) & ~(alignment - 1);

    void* memory = nullptr;
    #if defined(_WIN32)
        memory = _aligned_malloc(size, alignment);
    #elif defined(__APPLE__) || defined(__linux__)
        memory = aligned_alloc(alignment, size);
    #else
        #error "unsupported platform"
    #endif

    if (memory && zeroed)
        memset(memory, 0, size);

    return memory;
}

void DefaultAllocator::Deallocate(SrcLoc loc, void* ptr)
{
    if (!ptr)
        return;

    #if defined(_WIN32)
        _aligned_free(ptr);
    #elif defined(__APPLE__) || defined(__linux__)
        free(ptr);
    #else
        #error "unsupported platform"
    #endif
}

struct ArenaAllocatorBlockHeader
{
    void* previous;
    size_t capacity;
};

void* ArenaAllocator::Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed)
{
    MSR_ASSERT(size > 0 && "Size must be greater than 0");

    // if alignment is not a power of 2, find the next power of 2 that is greater than or equal to the alignment
    if (alignment & (alignment - 1))
    {
        #if defined(_MSC_VER)
        {
            unsigned long index;
            _BitScanReverse64(&index, alignment);
            alignment = 1ULL << (index + 1);
        }
        #elif defined(__GNUC__) || defined(__clang__)
        {
            alignment = 1ULL << (64 - __builtin_clzll(alignment));
        }
        #endif
    }

    // make sure the size is aligned by the alignment, to avoid fragmentation
    size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    // align the offset to the required alignment
    size_t alignedPtr = ((size_t) currentBlock) + currentBlockOffset;
    alignedPtr = (alignedPtr + alignment - 1) & ~(alignment - 1);
    size_t alignedOffset = alignedPtr - (size_t) currentBlock;

    if (alignedOffset + alignedSize > currentBlockCapacity)
    {
        size_t newBlockSize = defaultBlockSize;
        if (newBlockSize < (16 * 1024))
            newBlockSize = 16 * 1024; // minimum block size of 16 KB

        size_t newBlockAlignment = sizeof(ArenaAllocatorBlockHeader);

        // if the requested size is larger than the default block size, use it as the new block size
        // plus the sizeof(void*)-sized first element in every allocation is to store the pointer to
        // the previous block
        // so we need to account for that in the block size calculation to ensure we have enough space
        // for the allocation even if the requested size is larger than the default block size
        {
            // our alignment is more than required, so we need to increase the block size to accommodate
            // the alignment padding
            if (newBlockAlignment >= alignment && newBlockSize < (alignedSize + sizeof(ArenaAllocatorBlockHeader)))
            {
                newBlockSize = alignedSize + sizeof(ArenaAllocatorBlockHeader);
            }

            // our alignment is less than required, so we need to increase the
            // block size to accommodate the worst-case alignment padding
            // and we use the requested alignment as the new block alignment
            // to ensure we can satisfy the alignment requirement
            if (newBlockAlignment < alignment && newBlockSize < (alignedSize + alignment))
            {
                newBlockSize = alignedSize + alignment;
                newBlockAlignment = alignment;
            }
        }

        // allocate the new block
        void* newBlock = backingAllocator.Allocate(newBlockSize, newBlockAlignment, loc);
        MSR_ASSERT(newBlock && "Failed to allocate memory for arena block");

        // store the pointer to the previous block at the start of the new block
        ArenaAllocatorBlockHeader* header = (ArenaAllocatorBlockHeader*) newBlock;
        header->previous = currentBlock;
        header->capacity = newBlockSize;
        currentBlock = newBlock;
        currentBlockOffset = sizeof(ArenaAllocatorBlockHeader);
        currentBlockCapacity = newBlockSize;

        // align the offset to the required alignment
        alignedPtr = ((size_t) currentBlock) + currentBlockOffset;
        alignedPtr = (alignedPtr + alignment - 1) & ~(alignment - 1);
        alignedOffset = alignedPtr - (size_t) currentBlock;
    }

    MSR_ASSERT(alignedOffset + alignedSize <= currentBlockCapacity && "Block size is too small to accommodate the allocation");

    void* result = (void*) alignedPtr;
    currentBlockOffset = alignedOffset + alignedSize;

    if (zeroed)
        memset(result, 0, size);

    return result;
}

void ArenaAllocator::Deallocate(SrcLoc loc, void* ptr)
{
    // no op
}

bool ArenaAllocator::DeallocateAll(SrcLoc loc)
{
    // leave 1 block allocated, but reset the offset
    void* block = currentBlock;
    while (block)
    {
        ArenaAllocatorBlockHeader* header = (ArenaAllocatorBlockHeader*) block;
        void* prevBlock = header->previous;
        if (!prevBlock)
        {
            // first block!
            currentBlock = block;
            currentBlockOffset = sizeof(ArenaAllocatorBlockHeader);
            currentBlockCapacity = header->capacity;
            return true;
        }

        backingAllocator.Deallocate(block, loc);
        block = prevBlock;
    }
}

void ArenaAllocator::Destroy()
{
    this->DeallocateAll(SRC_LOC());
    if (currentBlock)
        backingAllocator.Deallocate(currentBlock, SRC_LOC());
    currentBlock = nullptr;
    currentBlockOffset = 0;
    currentBlockCapacity = 0;
}
