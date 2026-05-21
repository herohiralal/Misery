#pragma once
#include <__init.h>
#include "Collections.h"

class IStream
{
public:
    virtual ~IStream() = default;

    virtual int64_t GetSize();
    virtual int64_t GetCurrentPosition();
    virtual bool Seek(int64_t position, bool relative = false);
    virtual int64_t Read(Slice<uint8_t> dst);
    virtual int64_t Write(const Slice<uint8_t> src);
    virtual bool Truncate();
    virtual bool Truncate(int64_t newSize);
    virtual bool Flush();
    virtual void Close();
};

struct Stream
{
    IStream* impl;

    Stream() = default;
    Stream(IStream* streamImpl) : impl(streamImpl) { }

    bool IsValid() const { return impl != nullptr; }
    operator bool() const { return IsValid(); }
    operator IStream*() const { return impl; }

    /**
     * Get the size of the stream in bytes. Returns -1 if the stream is null or if an error occurs.
     */
    int64_t GetSize() { return impl ? impl->GetSize() : -1; }

    /**
     * Get the current position in the stream in bytes. Returns -1 if the stream is null or if an error occurs.
     */
    int64_t GetCurrentPosition() { return impl ? impl->GetCurrentPosition() : -1; }

    /**
     * Seek to a specific position in the stream. If relative is false, the position is absolute from the
     * start of the stream. If relative is true, the position is relative to the current position.
     * Returns true on success, false on failure or if the stream is null.
     */
    bool Seek(int64_t position, bool relative = false) { return impl ? impl->Seek(position, relative) : false; }

    /**
     * Read data from the stream into the provided buffer. The buffer is specified as a Slice of bytes,
     * which contains a pointer to the data and the size of the buffer. Returns the number of bytes read, or 0 on failure
     * or if the stream is null.
     */
    int64_t Read(Slice<uint8_t> dst) { return impl ? impl->Read(dst) : 0; }

    /**
     * Read the entire contents of the stream into a newly allocated buffer, and return it as a Slice of bytes. The memory
     * for the buffer is allocated using the provided allocator. Returns an empty slice on failure or if the stream is null.
     */
    Slice<uint8_t> ReadAll(Allocator allocator, bool keepOpen = false);

    /**
     * Write data to the stream from the provided buffer. The buffer is specified as a Slice of bytes, which
     * contains a pointer to the data and the size of the buffer. Returns the number of bytes written, or 0 on failure
     * or if the stream is null.
     */
    int64_t Write(const Slice<uint8_t> src) { return impl ? impl->Write(src) : 0; }

    /**
     * Format a string and write to this stream. No intermediate buffer is used, the formatted string is written
     * directly to the stream as it is being formatted. Returns the number of bytes written, or 0 on failure or
     * if the stream is null.
     */
    template <typename... Args>
    int64_t Write(const char* fmt, Args&&... args);

    /**
     * Truncate the stream at the current position. Returns true on success, false on failure or if the stream is null.
     */
    bool Truncate() { return impl ? impl->Truncate() : false; }

    /**
     * Truncate the stream to a specific size. Returns true on success, false on failure or if the stream is null.
     */
    bool Truncate(int64_t newSize) { return impl ? impl->Truncate(newSize) : false; }

    /**
     * Flush any buffered data to the stream. Returns true on success, false on failure or if the stream is null.
     */
    bool Flush() { return impl ? impl->Flush() : false; }

    /**
     * Close the stream and release any associated resources. After calling this function, the stream should not
     * be used again. Returns without doing anything if the stream is null.
     */
    void Close() { if (impl) impl->Close(); }
};
