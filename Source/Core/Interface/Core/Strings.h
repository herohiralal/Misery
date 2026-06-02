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

// formatting ------------------------------------------------------------------------------------------------------------------

/**
 * The possible primitive types that can be formatted.
 */
typedef u8 FMT_Ty;
enum FMT_Tys
{
    FMT_Ty_Ukn,
    FMT_Ty_Int,
    FMT_Ty_Dbl,
    FMT_Ty_Str,
    FMT_Ty_Ptr,
};

/**
 * The base to use when formatting an integer argument.
 */
typedef u8 FMT_IntBase;
enum FMT_IntBases
{
    FMT_IntBase_Dec,
    FMT_IntBase_Hex,
    FMT_IntBase_Bin,
    FMT_IntBase_Oct,
};

/**
 * Different modifiers to use when formatitng a string argument.
 */
typedef u8 FMT_StrStyle;
enum FMT_StrStyles
{
    FMT_StrStyle_Default               =      0,
    FMT_StrStyle_AllCaps               = 1 << 0,
    FMT_StrStyle_NoCaps                = 1 << 1,
    FMT_StrStyle_NoSpaces              = 1 << 2,
    FMT_StrStyle_FwdSlashesOnly        = 1 << 3, // replace backslashes with forward slashes
    FMT_StrStyle_BwdSlashesOnly        = 1 << 4, // replace forward slashes with backslashes
    FMT_StrStyle_RemoveMultipleSlashes = 1 << 5, // replace multiple consecutive slashes with a single slash
};

/**
 * The internal encoding of a type-unspecific format specifier.
 */
typedef union
{
    FMT_Ty fmtTy;

    struct
    {
        FMT_Ty fmtTy;
        b8 isNegative;
        FMT_IntBase base;
        u64 value;
    } intVal;

    struct
    {
        FMT_Ty fmtTy;
        u16 minDecimalPlaces;
        u16 maxDecimalPlaces;
        double value;
    } dblVal;

    struct
    {
        FMT_Ty fmtTy;
        FMT_StrStyle format;
        utf8str value;
    } strVal;

    struct
    {
        FMT_Ty fmtTy;
        const void* value;
    } ptrVal;
} FMT_Arg;

COL_DECLARE_FOR(FMT_Arg)

FMT_Arg FMT_B8(b8 v);

FMT_Arg FMT_U64(u64 v, FMT_IntBase base OPT_ARG);
FMT_Arg FMT_U32(u32 v, FMT_IntBase base OPT_ARG);
FMT_Arg FMT_U16(u16 v, FMT_IntBase base OPT_ARG);
FMT_Arg FMT_U8(u8 v, FMT_IntBase base OPT_ARG);

FMT_Arg FMT_I64(i64 v, FMT_IntBase base OPT_ARG);
FMT_Arg FMT_I32(i32 v, FMT_IntBase base OPT_ARG);
FMT_Arg FMT_I16(i16 v, FMT_IntBase base OPT_ARG);
FMT_Arg FMT_I8(i8 v, FMT_IntBase base OPT_ARG);

#ifdef __cplusplus
FMT_Arg FMT_F64(f64 v, u16 minDecimalPlaces = 0, u16 maxDecimalPlaces = 6);
FMT_Arg FMT_F32(f32 v, u16 minDecimalPlaces = 0, u16 maxDecimalPlaces = 6);
#else
FMT_Arg FMT_F64(f64 v, u16 minDecimalPlaces, u16 maxDecimalPlaces);
FMT_Arg FMT_F32(f32 v, u16 minDecimalPlaces, u16 maxDecimalPlaces);
#endif

FMT_Arg FMT_Str(utf8str str, FMT_StrStyle style OPT_ARG);
FMT_Arg FMT_CStr(cstring str, FMT_StrStyle style OPT_ARG);

FMT_Arg FMT_Ptr(const void* ptr);

#ifdef __cplusplus
    EXTERN_C_END

    template <typename T>
    static inline FMT_Arg FMT_Generic(T v) { return FMT_Arg { }; }

    #define DECLARE_FMT_SPEC(fnSuffix, ty) \
        template <> inline FMT_Arg FMT_Generic(ty v) { return FMT_##fnSuffix(v); }

    DECLARE_FMT_SPEC(B8,   b8)
    DECLARE_FMT_SPEC(U8,   u8)
    DECLARE_FMT_SPEC(U16,  u16)
    DECLARE_FMT_SPEC(U32,  u32)
    DECLARE_FMT_SPEC(U64,  u64)
    DECLARE_FMT_SPEC(I8,   i8)
    DECLARE_FMT_SPEC(I16,  i16)
    DECLARE_FMT_SPEC(I32,  i32)
    DECLARE_FMT_SPEC(I64,  i64)
    DECLARE_FMT_SPEC(F32,  f32)
    DECLARE_FMT_SPEC(F64,  f64)
    DECLARE_FMT_SPEC(Str,  utf8str)
    DECLARE_FMT_SPEC(CStr, cstring)
    DECLARE_FMT_SPEC(Ptr,  const void*)
    DECLARE_FMT_SPEC(Ptr,  rawptr)
    DECLARE_FMT_SPEC(Ptr,  std::nullptr_t)

    #undef DECLARE_FMT_SPEC

    #define FMT(x) (FMT_Generic(x))

    EXTERN_C_BEGIN
#else

    typedef enum
    {
        FMT_CTy_Unknown,
        FMT_CTy_B8,
        FMT_CTy_U8,
        FMT_CTy_U16,
        FMT_CTy_U32,
        FMT_CTy_U64,
        FMT_CTy_I8,
        FMT_CTy_I16,
        FMT_CTy_I32,
        FMT_CTy_I64,
        FMT_CTy_F32,
        FMT_CTy_F64,
        FMT_CTy_Str,
        FMT_CTy_CStr,
        FMT_CTy_Ptr,
    } FMT_CTys;

    static inline FMT_Arg FMT_Generic(FMT_CTys ty, ...)
    {
        FMT_Arg o = {0};

        va_list l;
        va_start(l, ty);
        switch (ty)
        {
            case FMT_CTy_B8:   { i32     v = va_arg(l,     i32); o =   FMT_B8((b8)  v      ); break; }
            case FMT_CTy_U8:   { i32     v = va_arg(l,     i32); o =   FMT_U8((u8)  v, 0   ); break; }
            case FMT_CTy_U16:  { i32     v = va_arg(l,     i32); o =  FMT_U16((u16) v, 0   ); break; }
            case FMT_CTy_U32:  { u32     v = va_arg(l,     u32); o =  FMT_U32(      v, 0   ); break; }
            case FMT_CTy_U64:  { u64     v = va_arg(l,     u64); o =  FMT_U64(      v, 0   ); break; }
            case FMT_CTy_I8:   { i32     v = va_arg(l,     i32); o =   FMT_I8((i8)  v, 0   ); break; }
            case FMT_CTy_I16:  { i32     v = va_arg(l,     i32); o =  FMT_I16((i16) v, 0   ); break; }
            case FMT_CTy_I32:  { i32     v = va_arg(l,     i32); o =  FMT_I32(      v, 0   ); break; }
            case FMT_CTy_I64:  { i64     v = va_arg(l,     i64); o =  FMT_I64(      v, 0   ); break; }
            case FMT_CTy_F32:  { f64     v = va_arg(l,     f64); o =  FMT_F32((f32) v, 0, 6); break; }
            case FMT_CTy_F64:  { f64     v = va_arg(l,     f64); o =  FMT_F64(      v, 0, 6); break; }
            case FMT_CTy_Str:  { utf8str v = va_arg(l, utf8str); o =  FMT_Str(      v, 0   ); break; }
            case FMT_CTy_CStr: { cstring v = va_arg(l, cstring); o = FMT_CStr(      v, 0   ); break; }
            case FMT_CTy_Ptr:  { rawptr  v = va_arg(l,  rawptr); o =  FMT_Ptr(      v      ); break; }
            case FMT_CTy_Unknown:
            default:
                break;
        }
        va_end(l);

        return o;
    }

    #define FMT(x) \
        (_Generic((x), \
            b8:          FMT_Generic(FMT_CTy_B8,   (x)), \
            u8:          FMT_Generic(FMT_CTy_U8,   (x)), \
            u16:         FMT_Generic(FMT_CTy_U16,  (x)), \
            u32:         FMT_Generic(FMT_CTy_U32,  (x)), \
            u64:         FMT_Generic(FMT_CTy_U64,  (x)), \
            i8:          FMT_Generic(FMT_CTy_I8,   (x)), \
            i16:         FMT_Generic(FMT_CTy_I16,  (x)), \
            i32:         FMT_Generic(FMT_CTy_I32,  (x)), \
            i64:         FMT_Generic(FMT_CTy_I64,  (x)), \
            utf8str:     FMT_Generic(FMT_CTy_Str,  (x)), \
            cstring:     FMT_Generic(FMT_CTy_CStr, (x)), \
            char*:       FMT_Generic(FMT_CTy_CStr, (x)), \
            const void*: FMT_Generic(FMT_CTy_Ptr,  (x)), \
            rawptr:      FMT_Generic(FMT_CTy_Ptr,  (x)), \
            default:     FMT_Generic(FMT_CTy_Unknown  )  \
        ))

#endif

#define FMTARGS(...) SLICE(FMT_Arg, __VA_ARGS__)

EXTERN_C_END
