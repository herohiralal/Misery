#include <Core/Format.h>

namespace Misery::Format::Internal
{

    struct BufferFormatSink
    {
        char* buffer;
        size_t capacity;
        size_t used;

        template <size_t N>
        BufferFormatSink(char (&buf)[N]) : buffer(buf), capacity(N), used(0) { }

        BufferFormatSink(char* buf, size_t cap) : buffer(buf), capacity(cap), used(0) { }
    };

    bool AppendToBufferFormatSink(void* userData, const char* data, size_t len)
    {
        if (!userData || !data || !len)
            return false;

        BufferFormatSink& sink = *(BufferFormatSink*) userData;
        if (!sink.buffer || !sink.capacity)
            return false;

        size_t spaceLeft = (sink.capacity > sink.used) ? (sink.capacity - sink.used) : 0;
        size_t toWrite = (len < spaceLeft) ? len : spaceLeft;

        if (toWrite > 0)
        {
            memcpy(sink.buffer + sink.used, data, toWrite);
            sink.used += toWrite;
        }

        // return true if we were able to write all data, false if we had to truncate
        return toWrite == len;
    }

    struct FileFormatSink
    {
        FILE* file;

        FileFormatSink(FILE* f) : file(f) { }
    };

    bool AppendToFileFormatSink(void* userData, const char* data, size_t len)
    {
        if (!userData || !data || !len)
            return false;

        FileFormatSink& sink = *(FileFormatSink*) userData;
        if (!sink.file)
            return false;

        size_t written = fwrite(data, 1, len, sink.file);
        return written == len;
    }

    typedef bool (*WriteProc)(void* userData, const char* data, size_t len);

    struct FormatSink
    {
        void* userData;
        WriteProc write;
        size_t totalWritten;

        FormatSink() : userData(nullptr), write(nullptr), totalWritten(0) { }
        FormatSink(BufferFormatSink& bufferSink) : userData(&bufferSink), write(AppendToBufferFormatSink), totalWritten(0) { }
        FormatSink(FileFormatSink& fileSink) : userData(&fileSink), write(AppendToFileFormatSink), totalWritten(0) { }
    };

    bool Append(FormatSink& sink, const char* data, size_t len)
    {
        if (!data || !len)
            return false;

        bool success = true;
        if (sink.write)
            success = sink.write(sink.userData, data, len);

        sink.totalWritten += len;
        return success;
    }

    template <size_t N>
    inline bool Append(FormatSink& sink, const char (&str)[N])
    {
        return Append(sink, str, N - 1);
    }

    inline bool Append(FormatSink& sink, char c)
    {
        return Append(sink, &c, 1);
    }

    inline bool Append(FormatSink& sink, uint64_t value, IntFormatBase base)
    {
        if (!value)
        {
            Append(sink, '0');
            return true;
        }

        char digits[64]; // enough for binary representation of 64-bit integer
        size_t count = 0;

        uint64_t baseInt = 10;
        switch (base)
        {
            case FMT_INT_DECIMAL:     baseInt = 10; break;
            case FMT_INT_HEXADECIMAL: baseInt = 16; break;
            case FMT_INT_OCTAL:       baseInt = 8;  break;
            case FMT_INT_BINARY:      baseInt = 2;  break;
            default: MSR_ASSERT(false && "Unknown integer format base"); return false;
        }

        uint64_t tempValue = value;
        while (tempValue > 0)
        {
            uint64_t digit = (tempValue % baseInt);
            if (digit < 10) { digits[count] = (char) ('0' + digit);        }
            else            { digits[count] = (char) ('a' + (digit - 10)); }

            tempValue /= baseInt;
            count++;
        }

        // Append digits in reverse order
        bool success = true;
        for (size_t i = 0; i < count; ++i)
            success = Append(sink, digits[count - i - 1]) && success;

        return success;
    }

    inline bool Append(FormatSink& sink, double value, uint16_t minDecimalPlaces, uint16_t maxDecimalPlaces)
    {
        if (minDecimalPlaces > maxDecimalPlaces)
        {
            MSR_ASSERT(false && "minDecimalPlaces cannot be greater than maxDecimalPlaces");
            return false;
        }

        if (value != value)
        {
            return Append(sink, "NaN");
        }
        else if (value == std::numeric_limits<double>::infinity())
        {
            return Append(sink, "+inf");
        }
        else if (value == -std::numeric_limits<double>::infinity())
        {
            return Append(sink, "-inf");
        }
        else
        {
            bool success = true;
            if (!value)
            {
                success = Append(sink, '0') && success;
                if (minDecimalPlaces > 0 && maxDecimalPlaces > 0)
                {
                    success = Append(sink, '.') && success;
                    for (uint16_t i = 0; i < minDecimalPlaces; ++i)
                        success = Append(sink, '0') && success;
                }

                return success;
            }

            // handle sign
            double absValue = value;
            if (value < 0)
            {
                success = Append(sink, '-') && success;
                absValue = -value;
            }

            // integer part
            uint64_t intPart = (uint64_t) absValue;
            success = Append(sink, intPart, FMT_INT_DECIMAL) && success;

            double fractionalPart = absValue - (double) intPart;

            // fractional part
            if ((fractionalPart > 0  && maxDecimalPlaces > 0) || minDecimalPlaces > 0)
            {
                success = Append(sink, '.') && success;

                // scale fractional part by 10^maxDecimalPlaces
                double scalingFactor = pow(10.0, maxDecimalPlaces);
                uint64_t scaledFractional = (uint64_t) (fractionalPart * scalingFactor + 0.5); // round to nearest

                // count digits of scaledFractional
                uint64_t temp = scaledFractional;
                uint16_t digitCount = 0;
                if (!temp) { digitCount = 1; }
                else while (temp > 0) { digitCount++; temp /= 10; }

                // trailing zeroes
                for (uint16_t i = 0; i < minDecimalPlaces - digitCount; ++i)
                    success = Append(sink, '0') && success;

                // append fractional digits if there are any
                if (scaledFractional > 0)
                    success = Append(sink, scaledFractional, FMT_INT_DECIMAL) && success;
            }

            return success;
        }
    }

    inline bool Append(FormatSink& sink, String str, StringFormatStyle format)
    {
        if (format == FMT_STRING_DEFAULT)
        {
            return Append(sink, (const char*) str.Data(), str.Length());
        }

        bool lastCharWasSlash = false;
        bool success = true;
        for (size_t i = 0; i < str.Length(); ++i)
        {
            char c = (char) str.Data()[i];

            if (isspace(c))
            {
                if (format & FMT_STRING_NO_SPACES)
                    c = '_'; // replace spaces with underscores if NO_SPACES is set
            }

            if (c >= 'A' && c <= 'Z' && (format & FMT_STRING_NO_CAPS))
                c = (char) tolower(c);

            if (c >= 'a' && c <= 'z' && (format & FMT_STRING_ALL_CAPS))
                c = (char) toupper(c);

            if (c == '/' || c == '\\')
            {
                if (lastCharWasSlash && (format & FMT_STRING_REMOVE_MULTIPLE_SLASHES))
                {
                    // skip this character if it's a slash and the previous character
                    // was also a slash, and REMOVE_MULTIPLE_SLASHES is set
                    continue;
                }

                if (format & FMT_STRING_FWD_SLASHES_ONLY)
                    c = '/';
                else if (format & FMT_STRING_BWD_SLASHES_ONLY)
                    c = '\\';

                lastCharWasSlash = true;
            }
            else
            {
                lastCharWasSlash = false;
            }

            success = Append(sink, c) && success;
        }

        return success;
    }

    inline bool Append(FormatSink& sink, const FormatArg& arg)
    {
        switch (arg.type)
        {
        case FMT_ARG_UNKNOWN:
            return Append(sink, "(%UNKNOWN_ARG%)");
        case FMT_ARG_INT:
        {
            bool success = true;
            if (arg.intValue.isNegative) success = Append(sink, '-') && success;
            success = Append(sink, arg.intValue.value, arg.intValue.base) && success;
            return success;
        }
        case FMT_ARG_DOUBLE:
            return Append(sink, arg.doubleValue.value, arg.doubleValue.minDecimalPlaces, arg.doubleValue.maxDecimalPlaces);
        case FMT_ARG_STRING:
            return Append(sink, arg.stringValue.value, arg.stringValue.format);
        case FMT_ARG_POINTER:
        {
            if (!arg.pointerValue.value)
                return Append(sink, "(nullptr)");

            bool success = true;
            success = Append(sink, "0x") && success;
            success = Append(sink, (uint64_t) (uintptr_t) arg.pointerValue.value, FMT_INT_HEXADECIMAL) && success;
            return success;
        }
        default:
            MSR_ASSERT(false && "Unknown format argument type");
            return false;
        }
    }

    inline bool Format(FormatSink& sink, String formatStr, Slice<FormatArg> args)
    {
        // % for formatting the argument
        // %% to escape a literal '%'

        size_t argIndex = 0;
        char* start = (char*) formatStr.Data();
        size_t len = formatStr.Length();

        size_t bulkFlushSize = 0;

        auto bulkFlush = [&](size_t i) -> bool
        {
            if (bulkFlushSize > 0)
            {
                bool success = Append(sink, start + i - bulkFlushSize, bulkFlushSize);
                bulkFlushSize = 0;
                return success;
            }

            return true;
        };

        bool success = true;
        for (size_t i = 0; i < len; ++i)
        {
            char c = start[i];
            if (c != '%')
            {
                bulkFlushSize++;
                continue;
            }

            if (i + 1 < len && start[i + 1] == '%')
            {
                bulkFlushSize++; // include the '%' in the bulk flush
                success = bulkFlush(i) && success;
                i++; // skip the next '%'
            }
            else
            {
                // flush any pending literals
                success = bulkFlush(i) && success;

                // format argument
                if (argIndex >= args.Count())
                {
                    success = Append(sink, "(%MISSING_ARG%)") && success;
                    continue;
                }

                success = Append(sink, args[argIndex]) && success;
                argIndex++;
            }

            continue;
        }

        // flush any remaining literals
        success = bulkFlush(len) && success;

        return success;
    }

    size_t FormatStr(Slice<uint8_t> buffer, String formatStr, Slice<FormatArg> args, bool addNullTerm)
    {
        BufferFormatSink bufferSink = BufferFormatSink((char*) buffer.Data(), buffer.Count());
        FormatSink sink = FormatSink(bufferSink);
        Format(sink, formatStr, args);
        if (addNullTerm) Append(sink, '\0');
        return sink.totalWritten;
    }

    size_t FormatStr(FILE* stream, String formatStr, Slice<FormatArg> args)
    {
        FileFormatSink fileSink = FileFormatSink(stream);
        FormatSink sink = FormatSink(fileSink);
        Format(sink, formatStr, args);
        return sink.totalWritten;
    }

    Slice<uint8_t> FormatSlice(Allocator allocator, String formatStr, Slice<FormatArg> args, bool addNullTerm, SrcLoc loc)
    {
        uint8_t buffer[8192]; // 8KB flash-format buffer

        size_t len = FormatStr(Slice<uint8_t>(buffer, sizeof(buffer)), formatStr, args, addNullTerm);
        if (!len) { return Slice<uint8_t>(); }

        Slice<uint8_t> output = allocator.MakeSlice<uint8_t>(len, loc);
        if (!output) { return Slice<uint8_t>(); }

        if (len <= sizeof(buffer)) // formatting into the buffer was successful, just memcpy
        {
            memcpy(output.Data(), buffer, len);
        }
        else // format again!
        {
            size_t len2 = Misery::Format::Internal::FormatStr(output, formatStr, args, addNullTerm);
            if (len2 != len)
            {
                // this should never happen, but if it does, we can at least return a truncated string instead of crashing or returning garbage
                MSR_ASSERT(false && "Inconsistent formatting length between buffer and output string");
                return output.SubSlice(0, len2 < len ? len2 : len);
            }
        }

        return output;
    }
}
