#include <Core/Core.h>
#include <Platform/Platform.h>

extern "C"
int whatever(void)
{
    Slice_(u8) a = SLICE(u8);
    (void) a;

    FMT_Args b = FMTARGS();
    (void) b;

    Slice_(u8) c = SLICE(u8, 1, 2, 3);
    (void) c;

    FMT_Args d = FMTARGS(FMT("a"), FMT(1), FMT(5.0f));
    (void) d;

    LOG_Inf(CXX, "Hello, world!");
    LOG_Inf(CXX, "DATA: [0]: %, [1]: %, [2]: %", FMT(c.data[0]), FMT(c.data[1]), FMT(c.data[2]));

    return 0;
}
