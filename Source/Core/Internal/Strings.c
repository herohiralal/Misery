#include <Core/Core.h>

utf8str STR_Substring(utf8str str, isize start, isize count)
{
    return (utf8str) COL_SUB_SLICE(str, start, count);
}
