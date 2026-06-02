#include <Core/Core.h>

isize STR_CStrLen(cstring str)
{
    return (isize) strlen(str);
}

utf8str STR_AliasCStr(cstring str)
{
    return (utf8str) {.data = (u8*) str, .count = STR_CStrLen(str)};
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

utf8str STR_ToUpper(utf8str str, MEM_Allocator allocator)
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

utf8str STR_ToLower(utf8str str, MEM_Allocator allocator)
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

isize STR_Find(utf8str str, utf8str subString, b8 ignoreCase)
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

isize STR_FindLast(utf8str str, utf8str subString, b8 ignoreCase)
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

    List_(isize) replacementIndices = COL_NewList(isize, 64, MEM_temp);

    isize searchSpaceOffset = 0;
    while (true)
    {
        utf8str searchSpace = STR_SubString(str, searchSpaceOffset, str.count - searchSpaceOffset);
        if (!searchSpace.data || !searchSpace.count) { break; }

        isize searchIdx = STR_Find(searchSpace, oldSubString, ignoreCase);
        if (searchIdx < 0) { break; }

        isize idx = searchSpaceOffset + searchIdx;
        if (idx >= str.count) { break; }

        COL_AppendToList(&replacementIndices, idx);
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
        isize repIdx = replacementIndices.data[r];
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

#define STR_RUNE_ERROR 0xfffd
#define STR_MAX_RUNE 0x0010ffff

#define STR_SURROGATE_MIN 0xd800
#define STR_SURROGATE_MAX 0xdfff

#define STR_MASKX 0x3f
#define STR_MASK2 0x1f
#define STR_MASK3 0x0f
#define STR_MASK4 0x07

#define STR_RUNE1_MAX 127
#define STR_RUNE2_MAX 2047
#define STR_RUNE3_MAX 65535

#define STR_LOCB 0x80
#define STR_HICB 0xbf

typedef struct { u8 lo, hi; } STR_Internal_AcceptRange;
static STR_Internal_AcceptRange G_STR_Internal_AcceptRanges[5] =
{
    {0x80, 0xbf},
    {0xa0, 0xbf},
    {0x80, 0x9f},
    {0x90, 0xbf},
    {0x80, 0x8f},
};

static u8 STR_Internal_GetAcceptSize(u8 byte)
{
    if (byte <= 0x7f) return 0xf0;
    if (byte <= 0xc1) return 0xf1;
    if (byte <= 0xdf) return 0x02;
    if (byte == 0xe0) return 0x13;
    if (byte <= 0xec) return 0x03;
    if (byte == 0xed) return 0x23;
    if (byte <= 0xef) return 0x03;
    if (byte == 0xf0) return 0x34;
    if (byte <= 0xf3) return 0x04;
    if (byte == 0xf4) return 0x44;
    return 0xf1;
}

i32 STR_GetRuneLength(u32 r)
{
    if (r <= STR_RUNE1_MAX)                               { return 1;  }
    if (r <= STR_RUNE2_MAX)                               { return 2;  }
    if (STR_SURROGATE_MIN <= r && r <= STR_SURROGATE_MAX) { return -1; }
    if (r <= STR_RUNE3_MAX)                               { return 3;  }
    if (r <= STR_MAX_RUNE)                                { return 4;  }
    return -1;
}

STR_EncodedRune STR_EncodeRune(u32 c)
{
    u32 r = c;
    STR_EncodedRune result = {0};
    u8 mask = 0x3f;

    if (r <= (1 << 7) - 1)
    {
        result.data[0] = (u8)r;
        result.len = 1;
        return result;
    }

    if (r <= (1 << 11) - 1)
    {
        result.data[0] = 0xc0 | (u8)(r >> 6);
        result.data[1] = 0x80 | ((u8)r & mask);
        result.len = 2;
        return result;
    }

    if (r > 0x0010ffff || (STR_SURROGATE_MIN <= r && r <= STR_SURROGATE_MAX))
        r = STR_RUNE_ERROR;

    if (r <= (1 << 16) - 1)
    {
        result.data[0] = 0xe0 | (u8)(r >> 12);
        result.data[1] = 0x80 | ((u8)(r >> 6) & mask);
        result.data[2] = 0x80 | ((u8)r & mask);
        result.len = 3;
        return result;
    }

    result.data[0] = 0xf0 | (u8)(r >> 18);
    result.data[1] = 0x80 | ((u8)(r >> 12) & mask);
    result.data[2] = 0x80 | ((u8)(r >> 6) & mask);
    result.data[3] = 0x80 | ((u8)r & mask);
    result.len = 4;
    return result;
}

STR_DecodedRune STR_DecodeRune(utf8str s)
{
    i64 n = s.count;
    STR_DecodedRune result;

    if (n < 1)
    {
        result.rune = STR_RUNE_ERROR;
        result.len = 0;
        return result;
    }

    u8 s0 = s.data[0];
    u8 x = STR_Internal_GetAcceptSize(s0);

    if (x >= 0xf0)
    {
        u32 mask = (u32) x << 31 >> 31;
        result.rune = ((u32) s.data[0] & ~mask) | (STR_RUNE_ERROR & mask);
        result.len = 1;
        return result;
    }

    i32 sz = x & 7;
    STR_Internal_AcceptRange accept = G_STR_Internal_AcceptRanges[x >> 4];

    if (n < sz)
    {
        result.rune = STR_RUNE_ERROR;
        result.len = 1;
        return result;
    }

    u8 b1 = s.data[1];
    if (b1 < accept.lo || accept.hi < b1)
    {
        result.rune = STR_RUNE_ERROR;
        result.len = 1;
        return result;
    }

    if (sz == 2) {
        result.rune = ((u32) (s0 & STR_MASK2) << 6) | (u32) (b1 & STR_MASKX);
        result.len = 2;
        return result;
    }

    u8 b2 = s.data[2];
    if (b2 < STR_LOCB || STR_HICB < b2)
    {
        result.rune = STR_RUNE_ERROR;
        result.len = 1;
        return result;
    }

    if (sz == 3)
    {
        result.rune = ((u32)(s0 & STR_MASK3) << 12) |
                      ((u32)(b1 & STR_MASKX) << 6) |
                      (u32)(b2 & STR_MASKX);
        result.len = 3;
        return result;
    }

    u8 b3 = s.data[3];
    if (b3 < STR_LOCB || STR_HICB < b3)
    {
        result.rune = STR_RUNE_ERROR;
        result.len = 1;
        return result;
    }

    result.rune = ((u32)(s0 & STR_MASK4) << 18) |
                  ((u32)(b1 & STR_MASKX) << 12) |
                  ((u32)(b2 & STR_MASKX) << 6) |
                  (u32)(b3 & STR_MASKX);
    result.len = 4;
    return result;
}

#undef STR_RUNE_ERROR
#undef STR_MAX_RUNE
#undef STR_SURROGATE_MIN
#undef STR_SURROGATE_MAX
#undef STR_MASKX
#undef STR_MASK2
#undef STR_MASK3
#undef STR_MASK4
#undef STR_RUNE1_MAX
#undef STR_RUNE2_MAX
#undef STR_RUNE3_MAX
#undef STR_LOCB
#undef STR_HICB

utf8str STR_FromCStr(cstring str, MEM_Allocator allocator)
{
    utf8str str2 = STR_AliasCStr(str);
    return STR_Clone(str2, allocator);
}
