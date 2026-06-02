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

cstring STR_CloneToCStr(utf8str str, MEM_Allocator allocator)
{
    if (!str.data || !str.count)
        return nil;

    char* output = COL_NewSlice(char, str.count + 1, false, allocator).data;
    if (!output)
        return nil;

    MEM_Copy(output, str.data, (usize) str.count);
    output[str.count] = '\0';

    return output;
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

b8 STR_IsEmpty(utf8str str)
{
    return !str.data || !str.count;
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
    return STR_Clone(STR_AliasCStr(str), allocator);
}

utf8str STR_FromB8(  b8 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromF32(f32 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromF64(f64 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromU8(  u8 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromU16(u16 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromU32(u32 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromU64(u64 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromI8(  i8 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromI16(i16 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromI32(i32 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }
utf8str STR_FromI64(i64 v, MEM_Allocator allocator) { return FMT_APrintf(allocator, "%", FMT(v)); }

b8 STR_ParseB8(utf8str str, b8* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (b8) {0}; // zero by default

    if (STR_EqIgnoreCase(str, UTF8STR("true")))
    {
        *value = true;
        return true;
    }
    else if (STR_EqIgnoreCase(str, UTF8STR("false")))
    {
        *value = false;
        return true;
    }
    else if (STR_Eq(str, UTF8STR("1")))
    {
        *value = true;
        return true;
    }
    else if (STR_Eq(str, UTF8STR("0")))
    {
        *value = false;
        return true;
    }

    return false;
}

b8 STR_ParseF32(utf8str str, f32* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (f32) {0}; // zero by default
    f64 tempVal = 0.0;
    if (!STR_ParseF64(str, &tempVal)) return false;
    *value = (f32) tempVal;
    return true;
}

b8 STR_ParseF64(utf8str str, f64* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (f64) {0}; // zero by default

    isize i = 0;
    b8 negative = false;

    // Handle optional sign
    if (str.data[0] == '-')
    {
        negative = true;
        i = 1;
    }
    else if (str.data[0] == '+')
    {
        i = 1;
    }

    // Find decimal point (if any)
    isize dotIndex = -1;
    for (isize j = i; j < str.count; j++)
    {
        if (str.data[j] == '.')
        {
            if (dotIndex != -1) return false; // multiple dots
            dotIndex = j;
        }
    }

    // Integer part slice
    utf8str intStr = STR_SubString(str, i, dotIndex == -1 ? str.count - i : dotIndex - i);

    // Fractional part slice
    utf8str fracStr = {0};
    if (dotIndex != -1)
    {
        fracStr = STR_SubString(str, (dotIndex + 1), str.count - (dotIndex + 1));
    }

    // Parse integer part
    i64 intVal = 0;
    if (intStr.count > 0)
    {
        if (!STR_ParseI64(intStr, &intVal)) return false;
    }

    // Parse fractional part
    f64 fracVal = 0.0;
    if (fracStr.count > 0)
    {
        u64 fracDigits = 0;
        if (!STR_ParseU64(fracStr, &fracDigits)) return false;

        f64 divisor = 1.0;
        for (isize k = 0; k < fracStr.count; k++) divisor *= 10.0;

        fracVal = (f64) fracDigits / divisor;
    }

    f64 result = (f64) intVal + fracVal;
    if (negative) result = -result;

    *value = result;
    return true;
}

b8 STR_ParseU8(utf8str str, u8* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (u8) {0}; // zero by default
    u64 tempVal = 0;
    if (!STR_ParseU64(str, &tempVal)) return false;
    if (tempVal > U8_MAX) return false; // overflow
    *value = (u8) tempVal;
    return true;
}

b8 STR_ParseU16(utf8str str, u16* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (u16) {0}; // zero by default
    u64 tempVal = 0;
    if (!STR_ParseU64(str, &tempVal)) return false;
    if (tempVal > U16_MAX) return false; // overflow
    *value = (u16) tempVal;
    return true;
}

b8 STR_ParseU32(utf8str str, u32* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (u32) {0}; // zero by default
    u64 tempVal = 0;
    if (!STR_ParseU64(str, &tempVal)) return false;
    if (tempVal > U32_MAX) return false; // overflow
    *value = (u32) tempVal;
    return true;
}

b8 STR_ParseU64(utf8str str, u64* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (u64) {0}; // zero by default

    isize i = 0;
    u64 base = 10;

    // Prefix check
    if (str.count > 2 && str.data[0] == '0')
    {
        u8 p = (u8)str.data[1];
        if (p == 'b' || p == 'B') { base = 2;  i = 2; }
        else if (p == 'o' || p == 'O') { base = 8;  i = 2; }
        else if (p == 'x' || p == 'X') { base = 16; i = 2; }
    }

    // If no prefix: check if hex letters appear
    if (base == 10)
    {
        for (isize j = 0; j < str.count; j++)
        {
            u8 c = (u8)str.data[j];
            if ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
            {
                base = 16;
                break;
            }
        }
    }

    u64 result = 0;

    for (; i < str.count; i++)
    {
        u8 c = (u8)str.data[i];
        u64 digit;

        if (c >= '0' && c <= '9')       digit = (u64)(c - '0');
        else if (c >= 'A' && c <= 'F')  digit = (u64)(10 + (c - 'A'));
        else if (c >= 'a' && c <= 'f')  digit = (u64)(10 + (c - 'a'));
        else                            return false; // invalid char

        if (digit >= base) return false; // invalid digit for base

        result = result * base + digit;
    }

    *value = result;
    return true;
}

b8 STR_ParseI8(utf8str str, i8* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (i8) {0}; // zero by default
    i64 tempVal = 0;
    if (!STR_ParseI64(str, &tempVal)) return false;
    if (tempVal < I8_MIN || tempVal > I8_MAX) return false; // overflow
    *value = (i8) tempVal;
    return true;
}

b8 STR_ParseI16(utf8str str, i16* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (i16) {0}; // zero by default
    i64 tempVal = 0;
    if (!STR_ParseI64(str, &tempVal)) return false;
    if (tempVal < I16_MIN || tempVal > I16_MAX) return false; // overflow
    *value = (i16) tempVal;
    return true;
}

b8 STR_ParseI32(utf8str str, i32* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (i32) {0}; // zero by default
    i64 tempVal = 0;
    if (!STR_ParseI64(str, &tempVal)) return false;
    if (tempVal < I32_MIN || tempVal > I32_MAX) return false; // overflow
    *value = (i32) tempVal;
    return true;
}

b8 STR_ParseI64(utf8str str, i64* value)
{
    if (!str.data || !str.count || !value) return false;
    *value = (i64) {0}; // zero by default

    isize i = 0;
    b8 negative = false;

    // Handle optional sign
    if (str.data[0] == '-')
    {
        negative = true;
        i = 1;
    }
    else if (str.data[0] == '+')
    {
        i = 1;
    }

    // Slice after sign
    utf8str unsignedStr = STR_SubString(str, i, str.count - i);

    u64 uval = 0;
    if (!STR_ParseU64(unsignedStr, &uval))
    {
        return false;
    }

    if (negative)
    {
        if (uval > (u64) I64_MAX + 1ULL) return false; // overflow
        *value = -(i64) uval;
    }
    else
    {
        if (uval > (u64) I64_MAX) return false; // overflow
        *value = (i64) uval;
    }

    return true;
}
