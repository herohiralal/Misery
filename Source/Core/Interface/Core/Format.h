#pragma once
#include <__init.h>
#include "Collections.h"

/**
 * The type of format argument being passed.
 * This is used to determine how to format the argument when processing a format string.
 */
enum FormatArgType : uint8_t
{
    FMT_ARG_UNKNOWN,
    FMT_ARG_INT,
    FMT_ARG_DOUBLE,
    FMT_ARG_STRING,
    FMT_ARG_POINTER,
};

/**
 * The style to format a string argument with.
 * This is used to specify additional formatting options for string arguments when processing a format string.
 * These are different bases for formatting integer arguments.
 */
enum IntFormatBase : uint8_t
{
    FMT_INT_DECIMAL,
    FMT_INT_HEXADECIMAL,
    FMT_INT_OCTAL,
    FMT_INT_BINARY,
};

/**
 * The style to format a string argument with.
 * This is used to specify additional formatting options for string arguments when processing a format string.
 * These are some different common styles for formatting string arguments, such as converting to all caps or removing spaces.
 */
enum StringFormatStyle : uint8_t
{
    FMT_STRING_DEFAULT   = 0,
    FMT_STRING_ALL_CAPS  = 1 << 0,
    FMT_STRING_NO_CAPS   = 1 << 1,
    FMT_STRING_NO_SPACES = 1 << 2,
    FMT_STRING_FWD_SLASHES_ONLY = 1 << 3, // replace backslashes with forward slashes (useful for file paths)
    FMT_STRING_BWD_SLASHES_ONLY = 1 << 4, // replace forward slashes with backslashes (useful for file paths)
    FMT_STRING_REMOVE_MULTIPLE_SLASHES = 1 << 5, // replace multiple consecutive slashes with a single slash (useful for file paths)
};

struct FormatArg
{
    union
    {
        FormatArgType type;

        struct
        {
            FormatArgType type;
            bool isNegative;
            IntFormatBase base;
            uint64_t value;
        } intValue;

        struct
        {
            FormatArgType type;
            uint16_t minDecimalPlaces;
            uint16_t maxDecimalPlaces;
            double value;
        } doubleValue;

        struct
        {
            FormatArgType type;
            StringFormatStyle format;
            String value;
        } stringValue;

        struct
        {
            FormatArgType type;
            const void* value;
        } pointerValue;
    };

    FormatArg() = default;

    FormatArg(uint64_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : intValue {.type = FMT_ARG_INT, .isNegative = false, .base = base, .value = val}
    {
    }

    FormatArg(uint32_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : FormatArg((uint64_t) val, base)
    {
    }

    FormatArg(uint16_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : FormatArg((uint64_t) val, base)
    {
    }

    FormatArg(uint8_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : FormatArg((uint64_t) val, base)
    {
    }

    FormatArg(int64_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : intValue {.type = FMT_ARG_INT, .isNegative = (val < 0), .base = base, .value = (val == INT64_MIN) ? (((uint64_t) INT64_MAX) + 1) : (uint64_t) ((val < 0) ? -val : val)}
    {
    }

    FormatArg(int32_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : FormatArg((int64_t) val, base)
    {
    }

    FormatArg(int16_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : FormatArg((int64_t) val, base)
    {
    }

    FormatArg(int8_t val, IntFormatBase base = FMT_INT_DECIMAL)
        : FormatArg((int64_t) val, base)
    {
    }

    FormatArg(double val, uint16_t minDecimalPlaces = 0, uint16_t maxDecimalPlaces = 6)
        : doubleValue {.type = FMT_ARG_DOUBLE, .minDecimalPlaces = minDecimalPlaces, .maxDecimalPlaces = maxDecimalPlaces, .value = val}
    {
    }

    FormatArg(float val, uint16_t minDecimalPlaces = 0, uint16_t maxDecimalPlaces = 6)
        : doubleValue {.type = FMT_ARG_DOUBLE, .minDecimalPlaces = minDecimalPlaces, .maxDecimalPlaces = maxDecimalPlaces, .value = val}
    {
    }

    FormatArg(String val, StringFormatStyle format = FMT_STRING_DEFAULT)
        : stringValue {.type = FMT_ARG_STRING, .format = format, .value = val}
    {
    }

    FormatArg(CString val, StringFormatStyle format = FMT_STRING_DEFAULT)
        : stringValue {.type = FMT_ARG_STRING, .format = format, .value = val.AsString()}
    {
    }

    template <size_t N>
    FormatArg(const char (&val)[N], StringFormatStyle format = FMT_STRING_DEFAULT)
        : stringValue {.type = FMT_ARG_STRING, .format = format, .value = String(val)}
    {
    }

    FormatArg(const void* val)
        : pointerValue {.type = FMT_ARG_POINTER, .value = val}
    {
    }
};

namespace Misery::Format::Internal
{
    size_t FormatStr(Slice<uint8_t> buffer, String formatStr, Slice<FormatArg> args, bool addNullTerm);
    size_t FormatStr(FILE* stream, String formatStr, Slice<FormatArg> args);
    Slice<uint8_t> FormatSlice(Allocator allocator, String formatStr, Slice<FormatArg> args, bool addNullTerm, SrcLoc loc);
}

template <size_t N, typename... Args>
size_t Format(Slice<uint8_t> buffer, const char (&formatStr)[N], Args&&... args)
{
    FormatArg argArray[] = { FormatArg(std::forward<Args>(args))... };
    return Misery::Format::Internal::FormatStr(buffer, String(formatStr), Slice<FormatArg>(argArray, sizeof...(Args)));
}

template <size_t N, typename... Args>
size_t Format(FILE* stream, const char (&formatStr)[N], Args&&... args)
{
    FormatArg argArray[] = { FormatArg(std::forward<Args>(args))... };
    return Misery::Format::Internal::FormatStr(stream, String(formatStr), Slice<FormatArg>(argArray, sizeof...(Args)));
}

template <typename... Args>
String Allocator::FormatString(SrcLoc loc, const char* fmt, Args&&... args)
{
    if constexpr (!sizeof...(Args))
    {
        return CloneString(CString(fmt), loc);
    }
    else
    {
        FormatArg argArray[] = { FormatArg(std::forward<Args>(args))... };
        Slice<uint8_t> formattedSlice = Misery::Format::Internal::FormatSlice(*this, String(fmt), Slice<FormatArg>(argArray, sizeof...(Args)), false, loc);
        if (!formattedSlice) { return String(); }
        return String(formattedSlice);
    }
}

template <size_t N, typename... Args>
CString Allocator::FormatCString(SrcLoc loc, const char (&fmt)[N], Args&&... args)
{
    if constexpr (!sizeof...(Args))
    {
        return CloneCString(CString(fmt), loc);
    }
    else
    {
        FormatArg argArray[] = { FormatArg(std::forward<Args>(args))... };
        Slice<uint8_t> formattedSlice = Misery::Format::Internal::FormatSlice(*this, fmt, Slice<FormatArg>(argArray, sizeof...(Args)), true, loc);
        if (!formattedSlice) { return CString(); }
        return CString((const char*) formattedSlice.Data());
    }
}
