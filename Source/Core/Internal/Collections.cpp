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

Allocator GetDefaultAllocator()
{
    static DefaultAllocator defaultAllocator = { };
    return &defaultAllocator;
}

Allocator GetTempAllocator()
{
    struct TempAllocator final
    {
        ArenaAllocator allocator;

        TempAllocator() : allocator(4 * 1024 * 1024, GetDefaultAllocator()) { }
        ~TempAllocator() { allocator.Destroy(); }
    };

    static thread_local TempAllocator tempAllocator;
    return &(tempAllocator.allocator);
}

size_t CString::Length() const
{
    return Data() ? strlen(Data()) : 0;
}

String CString::AsString() { return String(*this); }
const String CString::AsString() const { return String(*this); }

Slice<char> CString::AsSlice() { return Slice<char>(const_cast<char*>(data), Length()); }
const Slice<char> CString::AsSlice() const { return Slice<char>(const_cast<char*>(data), Length()); }

bool CString::operator==(const CString& other) const
{
    if (!Data() && !other.Data()) return true;
    if (!Data() || !other.Data()) return false;
    return strcmp(Data(), other.Data()) == 0;
}

bool CString::operator!=(const CString& other) const
{
    return !(*this == other);
}

String String::SubString(size_t start, size_t subCount)
{
    MSR_ASSERT(start <= Length() && "Start index out of bounds in String::SubString");
    MSR_ASSERT(start + subCount <= Length() && "SubString range out of bounds");
    return String(slice.SubSlice(start, subCount));
}

bool String::operator==(const String& other) const
{
    if (!slice && !other.slice) return true;
    if (!slice || !other.slice) return false;
    if (Length() != other.Length()) return false;
    return memcmp(Data(), other.Data(), Length()) == 0;
}

bool String::operator!=(const String& other) const
{
    return !(*this == other);
}
