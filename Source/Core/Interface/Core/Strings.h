#pragma once
#include <__init.h>
#include "Collections.h"

EXTERN_C_BEGIN

/**
 * UTF-8 string type, with length info (not necessarily null-terminated).
 */
typedef Slice_(u8) utf8str;
COL_DECLARE_FOR(utf8str);

// string utils ----------------------------------------------------------------------------------------------------------------

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
 * Create a substring from an existing string
 */
utf8str STR_SubString(utf8str str, isize start, isize count);

EXTERN_C_END
