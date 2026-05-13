#pragma once
#include <Core/Stream.h>

int64_t IStream::GetSize() { return -1; }
int64_t IStream::GetCurrentPosition() { return -1; }
bool IStream::Seek(int64_t position, bool relative) { return false; }
int64_t IStream::Read(Slice<uint8_t> dst) { return 0; }
int64_t IStream::Write(const Slice<uint8_t> src) { return 0; }
bool IStream::Truncate(int64_t newSize) { return false; }
bool IStream::Flush() { return false; }
void IStream::Close() { }
