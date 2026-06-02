#include <Core/Core.h>

FMT_Arg FMT_B8(b8 v)
{
    return (FMT_Arg)
    {
        .strVal =
        {
            .fmtTy  = FMT_Ty_Str,
            .value  = v ? UTF8STR("true") : UTF8STR("false"),
            .format = FMT_StrStyle_Default,
        },
    };
}

FMT_Arg FMT_U64(u64 v, FMT_IntBase base)
{
    return (FMT_Arg)
    {
        .intVal =
        {
            .fmtTy = FMT_Ty_Int,
            .value = v,
            .isNegative = false,
            .base  = base,
        },
    };
}

FMT_Arg FMT_U32(u32 v, FMT_IntBase base)
{
    return FMT_U64((u64) v, base);
}

FMT_Arg FMT_U16(u16 v, FMT_IntBase base)
{
    return FMT_U64((u64) v, base);
}

FMT_Arg FMT_U8(u8 v, FMT_IntBase base)
{
    return FMT_U64((u64) v, base);
}

FMT_Arg FMT_I64(i64 v, FMT_IntBase base)
{
    return (FMT_Arg)
    {
        .intVal =
        {
            .fmtTy = FMT_Ty_Int,
            .value = (v == I64_MIN) ? (1LL + (u64) I64_MAX) : (u64) ((v < 0) ? -v : v),
            .isNegative = (v < 0),
            .base  = base,
        },
    };
}

FMT_Arg FMT_I32(i32 v, FMT_IntBase base)
{
    return FMT_I64((i64) v, base);
}

FMT_Arg FMT_I16(i16 v, FMT_IntBase base)
{
    return FMT_I64((i64) v, base);
}

FMT_Arg FMT_I8(i8 v, FMT_IntBase base)
{
    return FMT_I64((i64) v, base);
}

FMT_Arg FMT_F64(f64 v, u16 minDecimalPlaces, u16 maxDecimalPlaces)
{
    return (FMT_Arg)
    {
        .dblVal =
        {
            .fmtTy = FMT_Ty_Dbl,
            .value = v,
            .minDecimalPlaces = minDecimalPlaces,
            .maxDecimalPlaces = maxDecimalPlaces,
        },
    };
}

FMT_Arg FMT_F32(f32 v, u16 minDecimalPlaces, u16 maxDecimalPlaces)
{
    return FMT_F64((f64) v, minDecimalPlaces, maxDecimalPlaces);
}

FMT_Arg FMT_Str(utf8str str, FMT_StrStyle style)
{
    return (FMT_Arg)
    {
        .strVal =
        {
            .fmtTy  = FMT_Ty_Str,
            .value  = str,
            .format = style,
        },
    };
}

FMT_Arg FMT_CStr(cstring str, FMT_StrStyle style)
{
    return FMT_Str(STR_AliasCStr(str), style);
}

FMT_Arg FMT_Ptr(const void* ptr)
{
    return (FMT_Arg)
    {
        .ptrVal =
        {
            .fmtTy  = FMT_Ty_Ptr,
            .value  = ptr,
        },
    };
}
