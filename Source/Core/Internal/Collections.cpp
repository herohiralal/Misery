#include <Core/Collections.h>
#include <Core/Allocators/Default.h>
#include <Core/Allocators/Arena.h>

void* IAllocator::Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed) { return nullptr; }
void IAllocator::Deallocate(SrcLoc loc, void* ptr) { }
bool IAllocator::DeallocateAll(SrcLoc loc) { return false; }

void* IAllocator::Reallocate(SrcLoc loc, void* ptr, size_t oldSize, size_t newSize, size_t alignment, bool zeroed)
{
    auto newMem = Allocate(loc, newSize, alignment, zeroed);
    if (newMem && ptr)
    {
        size_t copySize = oldSize < newSize ? oldSize : newSize;
        memcpy(newMem, ptr, copySize);
        Deallocate(loc, ptr);
    }

    return newMem;
}

Allocator GetDefaultAllocator()
{
    static DefaultAllocator defaultAllocator = { };
    return &defaultAllocator;
}

Allocator GetTempAllocator()
{
    using CleanupDelegate = void(*)();
    struct StaticCleanup
    {
        StaticCleanup(CleanupDelegate cleanupFunc) : func(cleanupFunc) { }
        ~StaticCleanup() { func(); }
        CleanupDelegate func;
    };

    static thread_local ArenaAllocator tempAllocator = ArenaAllocator(4 * 1024 * 1024, GetDefaultAllocator());
    static thread_local StaticCleanup cleanup = StaticCleanup([]() { tempAllocator.Destroy(); });
    return &tempAllocator;
}

size_t CString::Length() const
{
    return Data() ? strlen(Data()) : 0;
}

String CString::AsString() { return String(*this); }
const String CString::AsString() const { return String(*this); }

Slice<char> CString::AsSlice() { return Slice<char>(const_cast<char*>(data), Length()); }
const Slice<char> CString::AsSlice() const { return Slice<char>(const_cast<char*>(data), Length()); }

String String::SubString(size_t start, size_t subCount)
{
    MSR_ASSERT(start <= Length() && "Start index out of bounds in String::SubString");
    MSR_ASSERT(start + subCount <= Length() && "SubString range out of bounds");
    return String(slice.SubSlice(start, subCount));
}

String Allocator::MakeString(size_t length, SrcLoc loc)
{
    return String(MakeSlice<uint8_t>(length, loc));
}

void Allocator::FreeString(String* str, SrcLoc loc)
{
    FreeSlice(&(str->slice), loc);
}

String Allocator::CloneString(const String& str, SrcLoc loc)
{
    return String(CloneSlice(str.slice, loc));
}

CString Allocator::MakeCString(size_t length, SrcLoc loc)
{
    return CString((char*) Allocate(length + 1, alignof(char), loc));
}

CString Allocator::MakeCString(const String& str, SrcLoc loc)
{
    if (!str) { return CString(); }
    size_t length = str.Length();
    char* newData = (char*) Allocate(length + 1, alignof(char), loc);
    if (!newData) { return CString(); }
    memcpy(newData, str.Data(), length);
    newData[length] = '\0';
    return CString(newData);
}

void Allocator::FreeCString(CString* str, SrcLoc loc)
{
    Deallocate(str->Data(), loc);
    *str = CString();
}

CString Allocator::CloneCString(const CString& str, SrcLoc loc)
{
    return MakeCString(str.AsString(), loc);
}
