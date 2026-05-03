#include <Core/Primitives.h>
#include <Core/Allocators/Default.h>
#include <Core/Allocators/Arena.h>

void* IAllocator::Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed) { return nullptr; }
void IAllocator::Deallocate(SrcLoc loc, void* ptr) { }
bool IAllocator::DeallocateAll(SrcLoc loc) { return false; }

void* IAllocator::Reallocate(SrcLoc loc, void* ptr, size_t oldSize, size_t newSize, size_t alignment, bool zeroed)
{
    auto newMem = Allocate(loc, newSize, alignment, zeroed);
    if (newMem && ptr)
    {
        size_t copySize = oldSize < newSize ? oldSize : newSize;
        memcpy(newMem, ptr, copySize);
        Deallocate(loc, ptr);
    }

    return newMem;
}

Allocator GetDefaultAllocator()
{
    static DefaultAllocator defaultAllocator = { };
    return &defaultAllocator;
}

Allocator GetTempAllocator()
{
    using CleanupDelegate = void(*)();
    struct StaticCleanup
    {
        StaticCleanup(CleanupDelegate cleanupFunc) : func(cleanupFunc) { }
        ~StaticCleanup() { func(); }
        CleanupDelegate func;
    };

    static thread_local ArenaAllocator tempAllocator = ArenaAllocator(4 * 1024 * 1024, GetDefaultAllocator());
    static thread_local StaticCleanup cleanup = StaticCleanup([]() { tempAllocator.Destroy(); });
    return &tempAllocator;
}

size_t CString::Length() const
{
    return Data() ? strlen(Data()) : 0;
}
