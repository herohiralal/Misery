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
    return STR_Clone(str2, allocator);
}

utf8str STR_Clone(utf8str str, MEM_Allocator allocator)
{
    return COL_CloneSlice(str, allocator);
}

utf8str STR_SubString(utf8str str, isize start, isize count)
{
    return COL_SubSlice(str, start, count);
}

utf8str STR_Join(utf8str str1, utf8str str2, MEM_Allocator allocator)
{
    utf8str str = COL_NewSlice(u8, str1.count + str2.count, false, allocator);
    if (!str.data || !str.count) return (utf8str) {0};

    MEM_Copy(str.data,              str1.data, (usize) str1.count);
    MEM_Copy(str.data + str1.count, str2.data, (usize) str2.count);
    return str;
}

utf8str STR_ToUpper(utf8str str, MEM_Allocator allocator OPT_ARG)
{
    if (!str.data || !str.count) return (utf8str) {0};

    // clone if allocator provided
    if (allocator.procedure) str = COL_CloneSlice(str, allocator);

    // recheck after potential allocation
    if (!str.data || !str.count) return (utf8str) {0};

    for (i32 i = 0; i < str.count; i++)
        if (str.data[i] >= 'a' && str.data[i] <= 'z')
            str.data[i] -= ('a' - 'A'); // convert case

    return str;
}

utf8str STR_ToLower(utf8str str, MEM_Allocator allocator OPT_ARG)
{
    if (!str.data || !str.count) return (utf8str) {0};

    // clone if allocator provided
    if (allocator.procedure) str = COL_CloneSlice(str, allocator);

    // recheck after potential allocation
    if (!str.data || !str.count) return (utf8str) {0};

    for (i32 i = 0; i < str.count; i++)
        if (str.data[i] >= 'A' && str.data[i] <= 'Z')
            str.data[i] += ('a' - 'A'); // convert case

    return str;
}

b8 STR_Eq(utf8str str1, utf8str str2)
{
    if (str1.count != str2.count) { return false; } // different lengths, cannot be equal
    if (str1.count == 0)          { return true;  } // both are empty strings
    if (str1.data == str2.data)   { return true;  } // same pointer, same length

    return 0 == strncmp((cstring) str1.data, (cstring) str2.data, (size_t) str1.count);
}

b8 STR_EqIgnoreCase(utf8str str1, utf8str str2)
{
    if (str1.count != str2.count) { return false; } // different lengths, cannot be equal
    if (str1.count == 0)          { return true;  } // both are empty strings
    if (str1.data == str2.data)   { return true;  } // same pointer, same length

    for (i32 i = 0; i < str1.count; i++)
    {
        char c1 = (char) str1.data[i];
        char c2 = (char) str2.data[i];

        char c1Add = (c1 >= 'A' && c1 <= 'Z') ? ('a' - 'A') : '\0';
        char c2Add = (c2 >= 'A' && c2 <= 'Z') ? ('a' - 'A') : '\0';

        c1 += c1Add;
        c2 += c2Add;

        if (c1 != c2) { return false; }
    }

    return true;
}

b8 STR_HasPrefix(utf8str str, utf8str prefix)
{
    utf8str prefixStr = STR_SubString(str, 0, prefix.count);
    return STR_Eq(prefixStr, prefix);
}

b8 STR_HasPrefixIgnoreCase(utf8str str, utf8str prefix)
{
    utf8str prefixStr = STR_SubString(str, 0, prefix.count);
    return STR_EqIgnoreCase(prefixStr, prefix);
}

b8 STR_HasSuffix(utf8str str, utf8str suffix)
{
    utf8str suffixStr = STR_SubString(str, str.count - suffix.count, suffix.count);
    return STR_Eq(suffixStr, suffix);
}

b8 STR_HasSuffixIgnoreCase(utf8str str, utf8str suffix)
{
    utf8str suffixStr = STR_SubString(str, str.count - suffix.count, suffix.count);
    return STR_EqIgnoreCase(suffixStr, suffix);
}

isize STR_Find(utf8str str, utf8str subString, b8 ignoreCase OPT_ARG)
{
    if (!str.data || !str.count || !subString.data || !subString.count)
        return -1;

    i32 strLen = (i32) str.count;
    i32 subLen = (i32) subString.count;

    b8 (*eq)(utf8str, utf8str) = ignoreCase ? STR_EqIgnoreCase : STR_Eq;
    for (i32 i = 0; i <= strLen - subLen; i++)
    {
        utf8str strPart = STR_SubString(str, i, subString.count);
        if (eq(strPart, str))
            return i;
    }

    return -1;
}

isize STR_FindLast(utf8str str, utf8str subString, b8 ignoreCase OPT_ARG)
{
    if (!str.data || !str.count || !subString.data || !subString.count)
        return -1;

    i32 strLen = (i32) str.count;
    i32 subLen = (i32) subString.count;

    b8 (*eq)(utf8str, utf8str) = ignoreCase ? STR_EqIgnoreCase : STR_Eq;
    for (i32 i = strLen - subLen; i >= 0; i--)
    {
        utf8str strPart = STR_SubString(str, i, subString.count);
        if (eq(strPart, str))
            return i;
    }

    return -1;
}

utf8str STR_Replace(utf8str str, utf8str oldSubString, utf8str newSubString, MEM_Allocator allocator, b8 ignoreCase)
{
    if (!str.data || !str.count || !oldSubString.data || !oldSubString.count)
        return (utf8str) {0};

    List_(i16) replacementIndices = COL_NewList(i16, 64, MEM_temp);

    isize searchSpaceOffset = 0;
    while (true)
    {
        utf8str searchSpace = STR_SubString(str, searchSpaceOffset, str.count - searchSpaceOffset);
        if (!searchSpace.data || !searchSpace.count) { break; }

        isize searchIdx = STR_Find(searchSpace, oldSubString, ignoreCase);
        if (searchIdx < 0) { break; }

        isize idx = searchSpaceOffset + searchIdx;
        if (idx >= str.count) { break; }

        COL_AppendToList(&replacementIndices, (i16) idx);
        searchSpaceOffset += searchIdx + oldSubString.count;
    }

    if (0 == replacementIndices.count)
    {
        // fast path for when no replacements
        return STR_Clone(str, allocator);
    }

    isize newSize = str.count + ((newSubString.count - oldSubString.count) * replacementIndices.count);
    utf8str output = COL_NewSlice(u8, newSize, false, allocator);
    if (!output.data || !output.count)
        return (utf8str) {0};

    isize srcIdx = 0, dstIdx = 0;
    for (isize r = 0; r < replacementIndices.count; r++)
    {
        isize repIdx = (isize) replacementIndices.data[r];
        isize chunkSize = repIdx - srcIdx;
        MEM_Copy(output.data + dstIdx, str.data + srcIdx, (usize) chunkSize);
        dstIdx += chunkSize;
        srcIdx += chunkSize;
        MEM_Copy(output.data + dstIdx, newSubString.data, (usize) newSubString.count);
        dstIdx += newSubString.count;
        srcIdx += oldSubString.count;
    }

    isize remaining = str.count - srcIdx;
    MEM_Copy(output.data + dstIdx, str.data + srcIdx, (usize) remaining);

    return output;
}
