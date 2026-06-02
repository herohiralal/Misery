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

typedef struct
{
    List_(u8) output;
} FMT_Internal_BufferSink;

static inline b8 FMT_Internal_AppendToBuffer(rawptr userData, u8* data, isize len)
{
    if (!userData || !data || !len)
        return false;

    FMT_Internal_BufferSink* sink = (FMT_Internal_BufferSink*) userData;
    if (!sink->output.data || !sink->output.capacity)
        return false;

    isize spaceLeft = (sink->output.capacity > sink->output.count)
        ? (sink->output.capacity - sink->output.count)
        : 0;

    isize toWrite = (len < spaceLeft) ? len : spaceLeft;

    if (toWrite > 0)
    {
        Slice_(u8) toAppend = {.data = data, .count = len};
        COL_AppendAllToList(&(sink->output), toAppend);
    }

    // return true if we were able to write all data, false if we had to truncate
    return toWrite == len;
}

typedef b8 (*FMT_Internal_AppendProc)(rawptr, u8*, isize);

typedef struct
{
    rawptr userData;
    FMT_Internal_AppendProc append;
    isize totalWritten;
} FMT_Internal_Sink;

static inline b8 FMT_Internal_Append(FMT_Internal_Sink* sink, u8* data, isize len)
{
    if (!sink || !data || !len)
        return false;

    b8 success = true;
    if (sink->append)
        success = sink->append(sink->userData, data, len);

    sink->totalWritten += len;
    return success;
}

static inline b8 FMT_Internal_AppendCh(FMT_Internal_Sink* sink, char c)
{
    u8 c2 = (u8) c;
    return FMT_Internal_Append(sink, &c2, 1);
}

static inline b8 FMT_Internal_AppendStr(FMT_Internal_Sink* sink, utf8str str)
{
    return FMT_Internal_Append(sink, str.data, str.count);
}

#define FMT_Internal_AppendStrLit(sink, str) (FMT_Internal_AppendStr(sink, UTF8STR(str)))

static inline b8 FMT_Internal_AppendU64(FMT_Internal_Sink* sink, u64 value, FMT_IntBase base)
{
    if (!value)
    {
        FMT_Internal_AppendCh(sink, '0');
        return true;
    }

    char digits[64]; // enough for binary representation of 64-bit integer
    isize count = 0;

    u64 baseInt = 10;
    switch (base)
    {
        case FMT_IntBase_Dec: baseInt = 10; break;
        case FMT_IntBase_Hex: baseInt = 16; break;
        case FMT_IntBase_Bin: baseInt = 8;  break;
        case FMT_IntBase_Oct: baseInt = 2;  break;
        default: MSR_ASSERT(false && "Unknown integer format base"); return false;
    }

    u64 tempValue = value;
    while (tempValue > 0)
    {
        u64 digit = (tempValue % baseInt);
        if (digit < 10) { digits[count] = (char) ('0' + digit);        }
        else            { digits[count] = (char) ('a' + (digit - 10)); }

        tempValue /= baseInt;
        count++;
    }

    // Append digits in reverse order
    b8 success = true;
    for (isize i = 0; i < count; ++i)
        success = FMT_Internal_AppendCh(sink, digits[count - i - 1]) && success;

    return success;
}

static inline b8 FMT_Internal_AppendF64(FMT_Internal_Sink* sink, double value, u16 minDecimalPlaces, u16 maxDecimalPlaces)
{
    if (minDecimalPlaces > maxDecimalPlaces)
    {
        MSR_ASSERT(false && "minDecimalPlaces cannot be greater than maxDecimalPlaces");
        return false;
    }

    if (isnan(value))
    {
        return FMT_Internal_AppendStrLit(sink, "NaN");
    }
    else if (isinf(value) && value > 0.0)
    {
        return FMT_Internal_AppendStrLit(sink, "+inf");
    }
    else if (isinf(value) && value < 0.0)
    {
        return FMT_Internal_AppendStrLit(sink, "-inf");
    }
    else
    {
        b8 success = true;
        if (fpclassify(value) == FP_ZERO)
        {
            success = FMT_Internal_AppendCh(sink, '0') && success;
            if (minDecimalPlaces > 0 && maxDecimalPlaces > 0)
            {
                success = FMT_Internal_AppendCh(sink, '.') && success;
                for (u16 i = 0; i < minDecimalPlaces; ++i)
                    success = FMT_Internal_AppendCh(sink, '0') && success;
            }

            return success;
        }

        // handle sign
        double absValue = value;
        if (value < 0)
        {
            success = FMT_Internal_AppendCh(sink, '-') && success;
            absValue = -value;
        }

        // integer part
        u64 intPart = (u64) absValue;
        success = FMT_Internal_AppendU64(sink, intPart, FMT_IntBase_Dec) && success;

        double fractionalPart = absValue - (double) intPart;

        // fractional part
        if ((fractionalPart > 0  && maxDecimalPlaces > 0) || minDecimalPlaces > 0)
        {
            success = FMT_Internal_AppendCh(sink, '.') && success;

            // scale fractional part by 10^maxDecimalPlaces
            double scalingFactor = pow(10.0, maxDecimalPlaces);
            u64 scaledFractional = (u64) (fractionalPart * scalingFactor + 0.5); // round to nearest

            // count digits of scaledFractional
            u64 temp = scaledFractional;
            u16 digitCount = 0;
            if (!temp) { digitCount = 1; }
            else while (temp > 0) { digitCount++; temp /= 10; }

            // trailing zeroes
            for (u16 i = 0; i < minDecimalPlaces - digitCount; ++i)
                success = FMT_Internal_AppendCh(sink, '0') && success;

            // append fractional digits if there are any
            if (scaledFractional > 0)
                success = FMT_Internal_AppendU64(sink, scaledFractional, FMT_IntBase_Dec) && success;
        }

        return success;
    }
}

static inline b8 FMT_Internal_AppendStrEx(FMT_Internal_Sink* sink, utf8str str, FMT_StrStyle style)
{
    if (style == FMT_StrStyle_Default)
    {
        return FMT_Internal_AppendStr(sink, str);
    }

    b8 lastCharWasSlash = false;
    b8 success = true;
    for (isize i = 0; i < str.count; ++i)
    {
        char c = (char) str.data[i];

        if (isspace(c))
        {
            if (style & FMT_StrStyle_NoSpaces)
                c = '_'; // replace spaces with underscores if NO_SPACES is set
        }

        if (c >= 'A' && c <= 'Z' && (style & FMT_StrStyle_NoCaps))
            c = (char) tolower(c);

        if (c >= 'a' && c <= 'z' && (style & FMT_StrStyle_AllCaps))
            c = (char) toupper(c);

        if (c == '/' || c == '\\')
        {
            if (lastCharWasSlash && (style & FMT_StrStyle_RemoveMultipleSlashes))
            {
                // skip this character if it's a slash and the previous character
                // was also a slash, and REMOVE_MULTIPLE_SLASHES is set
                continue;
            }

            if (style & FMT_StrStyle_FwdSlashesOnly)
                c = '/';
            else if (style & FMT_StrStyle_BwdSlashesOnly)
                c = '\\';

            lastCharWasSlash = true;
        }
        else
        {
            lastCharWasSlash = false;
        }

        success = FMT_Internal_AppendCh(sink, c) && success;
    }

    return success;
}

static inline b8 FMT_Internal_AppendFmtArg(FMT_Internal_Sink* sink, FMT_Arg arg)
{
    switch (arg.fmtTy)
    {
    case FMT_Ty_Ukn:
        return FMT_Internal_AppendStrLit(sink, "(%UNKNOWN_ARG%)");
    case FMT_Ty_Int:
    {
        b8 success = true;
        if (arg.intVal.isNegative) success = FMT_Internal_AppendCh(sink, '-') && success;
        success = FMT_Internal_AppendU64(sink, arg.intVal.value, arg.intVal.base) && success;
        return success;
    }
    case FMT_Ty_Dbl:
        return FMT_Internal_AppendF64(sink, arg.dblVal.value, arg.dblVal.minDecimalPlaces, arg.dblVal.maxDecimalPlaces);
    case FMT_Ty_Str:
        return FMT_Internal_AppendStrEx(sink, arg.strVal.value, arg.strVal.format);
    case FMT_Ty_Ptr:
    {
        if (!arg.ptrVal.value)
            return FMT_Internal_AppendStrLit(sink, "(nullptr)");

        b8 success = true;
        success = FMT_Internal_AppendStrLit(sink, "0x") && success;
        success = FMT_Internal_AppendU64(sink, (u64) (usize) arg.ptrVal.value, FMT_IntBase_Hex) && success;
        return success;
    }
    default:
        MSR_ASSERT(false && "Unknown format argument type");
        return false;
    }
}

static inline b8 FMT_Internal_Format(FMT_Internal_Sink* sink, utf8str formatStr, FMT_Args args)
{
    // % for formatting the argument
    // %% to escape a literal '%'

    isize argIndex = 0;
    u8* start = formatStr.data;
    isize len = formatStr.count;

    // "bulk flush size"
    isize bulkFlushSize = 0;

    b8 success = true;

    #define FMT_BULK_FLUSH(idx__) \
        do \
        { \
            if (bulkFlushSize) \
            { \
                success = FMT_Internal_Append( \
                    sink, \
                    start + idx__ - bulkFlushSize, \
                    bulkFlushSize \
                ) && success; \
                bulkFlushSize = 0; \
            } \
        } while (0)

    for (isize i = 0; i < len; ++i)
    {
        char c = (char) start[i];
        if (c != '%')
        {
            bulkFlushSize++;
            continue;
        }

        if (i + 1 < len && start[i + 1] == '%')
        {
            bulkFlushSize++; // include the '%' in the bulk flush
            FMT_BULK_FLUSH(i);

            i++; // skip the next '%'
        }
        else
        {
            // flush any pending literals
            FMT_BULK_FLUSH(i);

            // format argument
            if (argIndex >= args.count)
            {
                success = FMT_Internal_AppendStrLit(sink, "(%MISSING_ARG%)") && success;
                continue;
            }

            success = FMT_Internal_AppendFmtArg(sink, args.data[argIndex]) && success;
            argIndex++;
        }

        continue;
    }

    // flush any remaining literals
    FMT_BULK_FLUSH(len);

    return success;
}

#undef FMT_Internal_AppendStrLit

isize FMT_ToBuffer(Slice_(u8) buffer, utf8str formatStr, FMT_Args args)
{
    FMT_Internal_BufferSink bfS = {.output = {.data = buffer.data, .capacity = buffer.count}};
    FMT_Internal_Sink sink = {.userData = &bfS, .append = FMT_Internal_AppendToBuffer};
    return FMT_Internal_Format(&sink, formatStr, args);
}
