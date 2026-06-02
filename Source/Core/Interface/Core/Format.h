#pragma once
#include <__init.h>
#include "Collections.h"
#include "Memory.h"
#include "Strings.h"

EXTERN_C_BEGIN

// fmt args --------------------------------------------------------------------------------------------------------------------

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
typedef Slice_(FMT_Arg) FMT_Args;

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

    #define DECLARE_FMT_SPEC(fnSuffix, ty) \
        static inline FMT_Arg FMT_Generic(ty v) { return FMT_##fnSuffix(v); }

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
            rawptr:      FMT_Generic(FMT_CTy_Ptr,  (x))  \
        ))

#endif

#define FMTARGS(...) SLICE(FMT_Arg, __VA_ARGS__)

/**
 * Format string to an existing buffer.
 * If the buffer is big enough, returns the number of bytes that were used.
 * If the buffer is not big enough, returns the total number of bytes that would be required.
 */
isize FMT_ToBuffer(Slice_(u8) buffer, utf8str formatStr, FMT_Args, b8 addNullTerm);

// internal function; check FMT_APrintf
utf8str FMT_APrintf_(MEM_Allocator, utf8str formatStr, FMT_Args);

/**
 * Format a new UTF-8 string and using the provided allocator.
 */
#define FMT_APrintf(allocator, fmt, ...) \
    (FMT_APrintf_(allocator, UTF8STR(fmt), FMTARGS(__VA_ARGS__)))

/**
 * Format a new UTF-8 string and using the main allocator.
 */
#define FMT_SPrintf(fmt, ...) \
    (FMT_APrintf_(MEM_main, UTF8STR(fmt), FMTARGS(__VA_ARGS__)))

/**
 * Format a new UTF-8 string and using the temporary allocator.
 */
#define FMT_TPrintf(fmt, ...) \
    (FMT_APrintf_(MEM_temp, UTF8STR(fmt), FMTARGS(__VA_ARGS__)))

// internal function; check FMT_CAPrintf
cstring FMT_CAPrintf_(MEM_Allocator, utf8str formatStr, FMT_Args);

/**
 * Format a new C-style null-terminated string and using the provided allocator.
 */
#define FMT_CAPrintf(allocator, fmt, ...) \
    (FMT_CAPrintf_(allocator, UTF8STR(fmt), FMTARGS(__VA_ARGS__)))

/**
 * Format a new C-style null-terminated string and using the main allocator.
 */
#define FMT_CSPrintf(fmt, ...) \
    (FMT_CAPrintf_(MEM_main, UTF8STR(fmt), FMTARGS(__VA_ARGS__)))

/**
 * Format a new C-style null-terminated string and using the temporary allocator.
 */
#define FMT_CTPrintf(fmt, ...) \
    (FMT_CAPrintf_(MEM_temp, UTF8STR(fmt), FMTARGS(__VA_ARGS__)))

EXTERN_C_END
