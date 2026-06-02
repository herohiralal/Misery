#pragma once
#include <__init.h>
#include "Collections.h"
#include "Memory.h"

EXTERN_C_BEGIN

/**
 * UTF-8 string type, with length info (not necessarily null-terminated).
 */
typedef Slice_(u8) utf8str;
COL_DECLARE_FOR(utf8str);

// string basics ---------------------------------------------------------------------------------------------------------------

/**
 * Declare a UTF-8 string literal.
 * Use as:
```
 utf8str str = UTF8STR("some_string");
```
 */
#define UTF8STR(text) \
    MSR_TY_INITIALISER(utf8str) {(u8*) text, (isize) (sizeof(text) - 1)}

/**
 * Returns the length of the given C-style null-terminated string, excluding the null terminator.
 */
isize STR_CStrLen(cstring str);

/**
 * Aliases a C-style null-terminated string into a UTF-8 string, with length info.
 */
utf8str STR_AliasCStr(cstring str);

/**
 * Clones a UTF-8 string to a new allocated UTF-8 string.
 * The returned string is allocated using the specified allocator.
 */
utf8str STR_Clone(utf8str str, MEM_Allocator);

/**
 * Create a substring from an existing string. Will return an empty string, if the bounds check fails.
 */
utf8str STR_SubString(utf8str str, isize start, isize count);

/**
 * Concatenates two UTF-8 strings into a new allocated string.
 * The returned string is allocated using the specified allocator.
 */
utf8str STR_Join(utf8str str1, utf8str str2, MEM_Allocator);

// casing ----------------------------------------------------------------------------------------------------------------------

/**
 * Converts a UTF-8 string to uppercase.
 * The returned string is allocated using the specified allocator.
 * If the specified allocator is zero-value, the string is converted in-place.
 */
utf8str STR_ToUpper(utf8str str, MEM_Allocator allocator OPT_ARG);

/**
 * Converts a UTF-8 string to lowercase.
 * The returned string is allocated using the specified allocator.
 * If the specified allocator is zero-value, the string is converted in-place.
 */
utf8str STR_ToLower(utf8str str, MEM_Allocator allocator OPT_ARG);

// comparisons -----------------------------------------------------------------------------------------------------------------

/**
 * Checks if two UTF-8 strings contain the same data.
 * Returns true if they are equal, false otherwise.
 */
b8 STR_Eq(utf8str str1, utf8str str2);

/**
 * Checks if two UTF-8 strings contain the same data, ignoring case-related differences.
 * Returns true if they are equal, false otherwise.
 */
b8 STR_EqIgnoreCase(utf8str str1, utf8str str2);

/**
 * Checks if a UTF-8 string starts with the specified prefix.
 * Returns true if it does, false otherwise.
 */
b8 STR_HasPrefix(utf8str str, utf8str prefix);

/**
 * Checks if a UTF-8 string starts with the specified prefix, ignoring case-related differences.
 * Returns true if it does, false otherwise.
 */
b8 STR_HasPrefixIgnoreCase(utf8str str, utf8str prefix);

/**
 * Checks if a UTF-8 string ends with the specified suffix.
 * Returns true if it does, false otherwise.
 */
b8 STR_HasSuffix(utf8str str, utf8str suffix);

/**
 * Checks if a UTF-8 string ends with the specified suffix, ignoring case-related differences.
 * Returns true if it does, false otherwise.
 */
b8 STR_HasSuffixIgnoreCase(utf8str str, utf8str suffix);

/**
 * Searches for the first occurrence of a substring within a string.
 * Returns the index of the first occurrence, or -1 if not found.
 */
isize STR_Find(utf8str str, utf8str subString, b8 ignoreCase OPT_ARG);

/**
 * Searches for the last occurrence of a substring within a string.
 * Returns the index of the last occurrence, or -1 if not found.
 */
isize STR_FindLast(utf8str str, utf8str subString, b8 ignoreCase OPT_ARG);

/**
 * Replaces all occurrences of a substring within a string with a new value.
 * The returned string is allocated using the specified allocator.
 */
utf8str STR_Replace(utf8str str, utf8str oldSubString, utf8str newSubString, MEM_Allocator, b8 ignoreCase OPT_ARG);

/**
 * Result structure for UTF-8 rune encoding.
 * Contains the encoded bytes and the number of bytes used.
 */
typedef struct { u8 data[4]; i32 len; } STR_EncodedRune;

/**
 * Result structure for UTF-8 rune decoding.
 * Contains the decoded rune and the number of bytes consumed.
 */
typedef struct { u32 rune; i32 len; } STR_DecodedRune;

/**
 * Returns the number of bytes required to encode the given rune in UTF-8.
 */
i32 STR_GetRuneLength(u32 r);

/**
 * Encodes a rune into UTF-8 byte sequence and returns the structure containing encoded bytes/length.
 * Invalid runes or surrogates are replaced with the error rune (U+FFFD).
 */
STR_EncodedRune STR_EncodeRune(u32 c);

/**
 * Decodes a UTF-8 byte sequence into a rune and returns the structure containing the rune/length.
 * Returns error rune (U+FFFD) for invalid sequences.
 */
STR_DecodedRune STR_DecodeRune(utf8str s);

// conversions to string -------------------------------------------------------------------------------------------------------

/**
 * Clones a C-style null-terminated string into a new UTF-8 string, with length info.
 * The new string will be created from the provided allocator.
 */
utf8str STR_FromCStr(cstring str, MEM_Allocator);

EXTERN_C_END
