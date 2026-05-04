#pragma once
#include <__init.h>
#include <Core/Collections.h>

/**
 * Default allocator that uses the standard library's malloc/free for memory management.
 * This is the most basic allocator and should be used as a fallback when no other allocator is available.
 */
class DefaultAllocator final : public IAllocator
{
protected:
    void* Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed = false) override;
    void Deallocate(SrcLoc loc, void* ptr) override;
};
