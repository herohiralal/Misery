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
 * Checks if the UTF-8 string contains no data (is empty).
 * Returns true if so, false otherwise.
 */
b8 STR_IsEmpty(utf8str str);

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

/**
 * Convert a boolean value to a string ("true" or "false").
 */
utf8str STR_FromB8(b8 val, MEM_Allocator);

/**
 * Convert a 32-bit floating-point number to a string with upto 6 decimal places.
 */
utf8str STR_FromF32(f32 val, MEM_Allocator);

/**
 * Convert a 64-bit floating-point number to a string with upto 6 decimal places.
 */
utf8str STR_FromF64(f64 val, MEM_Allocator);

/**
 * Convert an unsigned 8-bit integer to a string (base-10).
 */
utf8str STR_FromU8(u8 val, MEM_Allocator);

/**
 * Convert an unsigned 16-bit integer to a string (base-10).
 */
utf8str STR_FromU16(u16 val, MEM_Allocator);

/**
 * Convert an unsigned 32-bit integer to a string (base-10).
 */
utf8str STR_FromU32(u32 val, MEM_Allocator);

/**
 * Convert an unsigned 64-bit integer to a string (base-10).
 */
utf8str STR_FromU64(u64 val, MEM_Allocator);

/**
 * Convert a signed 8-bit integer to a string (base-10).
 */
utf8str STR_FromI8(i8 val, MEM_Allocator);

/**
 * Convert a signed 16-bit integer to a string (base-10).
 */
utf8str STR_FromI16(i16 val, MEM_Allocator);

/**
 * Convert a signed 32-bit integer to a string (base-10).
 */
utf8str STR_FromI32(i32 val, MEM_Allocator);

/**
 * Convert a signed 64-bit integer to a string (base-10).
 */
utf8str STR_FromI64(i64 val, MEM_Allocator);

// conversions from string -----------------------------------------------------------------------------------------------------

/**
 * Convert a validstring (case-insensitive "true" or "false", or "1" or "0") to a boolean.
 */
b8 STR_ParseB8(utf8str str, b8* value);

/**
 * Convert a valid string (numbers-only, with zero or one decimal points,
 * optional -/+ sign at the start) to a 32-bit floating-point number.
 */
b8 STR_ParseF32(utf8str str, f32* value);

/**
 * Convert a valid string (numbers-only, with zero or one decimal points,
 * optional -/+ sign at the start) to a 64-bit floating-point number.
 */
b8 STR_ParseF64(utf8str str, f64* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optionally
 * starting with 0b/0o/0x prefix for alternate bases) to an unsigned 8-bit integer.
 * Will be assumed to be hexadecimal if it contains A-F characters but no prefix.
 * By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseU8(utf8str str, u8* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optionally
 * starting with 0b/0o/0x prefix for alternate bases) to an unsigned 16-bit integer.
 * Will be assumed to be hexadecimal if it contains A-F characters but no prefix.
 * By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseU16(utf8str str, u16* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optionally
 * starting with 0b/0o/0x prefix for alternate bases) to an unsigned 32-bit integer.
 * Will be assumed to be hexadecimal if it contains A-F characters but no prefix.
 * By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseU32(utf8str str, u32* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optionally
 * starting with 0b/0o/0x prefix for alternate bases) to an unsigned 64-bit integer.
 * Will be assumed to be hexadecimal if it contains A-F characters but no prefix.
 * By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseU64(utf8str str, u64* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optional -/+ sign
 * at the start, optionally starting with 0b/0o/0x prefix for alternate bases) to
 * a signed 8-bit integer. Will be assumed to be hexadecimal if it contains A-F
 * characters but no prefix. By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseI8(utf8str str, i8* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optional -/+ sign
 * at the start, optionally starting with 0b/0o/0x prefix for alternate bases) to
 * a signed 16-bit integer. Will be assumed to be hexadecimal if it contains A-F
 * characters but no prefix. By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseI16(utf8str str, i16* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optional -/+ sign
 * at the start, optionally starting with 0b/0o/0x prefix for alternate bases) to
 * a signed 32-bit integer. Will be assumed to be hexadecimal if it contains A-F
 * characters but no prefix. By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseI32(utf8str str, i32* value);

/**
 * Convert a valid string (numbers/A-F only, case-insensitive, optional -/+ sign
 * at the start, optionally starting with 0b/0o/0x prefix for alternate bases) to
 * a signed 64-bit integer. Will be assumed to be hexadecimal if it contains A-F
 * characters but no prefix. By default (no prefix), decimal base is assumed.
 */
b8 STR_ParseI64(utf8str str, i64* value);

EXTERN_C_END
