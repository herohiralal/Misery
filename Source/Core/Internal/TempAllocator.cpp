#include "CorePrivate.h"

MEM_Allocator MEM_GetTempAllocator(void)
{
    class TempAllocator final
    {
        MEM_ArenaAllocator actual;
    public:
        TempAllocator(usize s, MEM_Allocator a) { actual = MEM_CreateArenaAllocator(s, a); }
        ~TempAllocator() { MEM_DestroyArenaAllocator(&actual); }
        operator MEM_Allocator() { return {MEM_ArenaAllocatorProc, &(this->actual)}; }
    };

    static thread_local auto t = TempAllocator(4 * 1024 * 1024, MEM_main);
    return t;
}
