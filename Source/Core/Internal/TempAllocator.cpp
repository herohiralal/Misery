#include <Core/Memory.h>

namespace Misery::Memory::Internals
{
    class TempAllocator final : public MEM_ArenaAllocator
    {
    public:
        TempAllocator(usize defaultBlockSize, MEM_Allocator backingAllocator)
            : MEM_ArenaAllocator(MEM_CreateArenaAllocator(defaultBlockSize, backingAllocator))
        {
        }

        // no copy/move
        TempAllocator(const TempAllocator&) = delete;
        TempAllocator& operator=(const TempAllocator&) = delete;
        TempAllocator(TempAllocator&&) = delete;
        TempAllocator& operator=(TempAllocator&&) = delete;

        ~TempAllocator()
        {
            MEM_DestroyArenaAllocator(this);
        }
    };
}

MEM_Allocator MEM_GetTempAllocator(void)
{
    static thread_local auto tempAllocator = Misery::Memory::Internals::TempAllocator(4 * 1024 * 1024, MEM_main);
    return MEM_Allocator {.procedure = MEM_ArenaAllocatorProc, .data = &tempAllocator};
}
