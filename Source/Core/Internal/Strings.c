#include <Core/Core.h>

isize STR_CStrLen(cstring str)
{
    return (isize) strlen(str);
}

utf8str STR_StringFromCStr(cstring str)
{
    return (utf8str) {.data = (u8*) str, .count = STR_CStrLen(str)};
}

utf8str STR_NewStringFromCStr(cstring str, MEM_Allocator allocator)
{
    utf8str str2 = STR_StringFromCStr(str);
    return STR_CloneString(str2, allocator);
}

utf8str STR_CloneString(utf8str str, MEM_Allocator allocator)
{
    return COL_CloneSlice(str, allocator);
}

utf8str STR_Substring(utf8str str, isize start, isize count)
{
    return COL_SubSlice(str, start, count);
}
