#include <Core/Core.h>

utf8str STR_Substring(utf8str str, isize start, isize count)
{
    return COL_SubSlice(str, start, count);
}
