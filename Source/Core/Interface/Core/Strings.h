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
utf8str STR_StringFromCStr(cstring str);

/**
 * Clones a C-style null-terminated string into a new UTF-8 string, with length info.
 * The new string will be created from the provided allocator.
 */
utf8str STR_NewStringFromCStr(cstring str, MEM_Allocator);

/**
 * Clones a UTF-8 string to a new allocated UTF-8 string.
 * The returned string is allocated using the specified allocator.
 */
utf8str STR_CloneString(utf8str str, MEM_Allocator);

/**
 * Create a substring from an existing string. Will return an empty string, if the bounds check fails.
 */
utf8str STR_SubString(utf8str str, isize start, isize count);

/**
 * Concatenates two UTF-8 strings into a new allocated string.
 * The returned string is allocated using the specified allocator.
 */
utf8str STR_Join(utf8str str1, utf8str str2, MEM_Allocator);

EXTERN_C_END
