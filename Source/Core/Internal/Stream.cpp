#pragma once
#include <Core/Stream.h>

int64_t IStream::GetSize() { return -1; }
int64_t IStream::GetCurrentPosition() { return -1; }
bool IStream::Seek(int64_t position, bool relative) { return false; }
int64_t IStream::Read(Slice<uint8_t> dst) { return 0; }
int64_t IStream::Write(const Slice<uint8_t> src) { return 0; }
bool IStream::Truncate() { return false; }
bool IStream::Truncate(int64_t newSize) { return false; }
bool IStream::Flush() { return false; }
void IStream::Close() { }

Slice<uint8_t> Stream::ReadAll(Allocator allocator, bool keepOpen)
{
    if (!impl) { return Slice<uint8_t>(); }

    int64_t size = impl->GetSize();
    if (size <= 0) { return Slice<uint8_t>(); }

    Slice<uint8_t> buffer = allocator.MakeSlice<uint8_t>((size_t) size, SRC_LOC());
    if (!buffer) { return Slice<uint8_t>(); }

    int64_t bytesRead = impl->Read(buffer);
    if (bytesRead != size)
    {
        allocator.FreeSlice(&buffer, SRC_LOC());
        return Slice<uint8_t>();
    }

    if (!keepOpen) { impl->Close(); }

    return buffer;
}
