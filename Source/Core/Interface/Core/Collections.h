#pragma once
#include <__init.h>
#include "SrcLoc.h"

template <typename T>
struct Slice;
template <typename T>
struct List;
struct CString;
struct String;

/**
 * General interface for an allocator. This is used to abstract away the details of memory allocation and allow for different
 * allocation strategies to be used in different parts of the codebase.
 */
class IAllocator
{
    friend struct Allocator;

public:
    virtual ~IAllocator() = default;

protected:
    virtual void* Allocate(SrcLoc loc, size_t size, size_t alignment, bool zeroed = false);
    virtual void* Reallocate(SrcLoc loc, void* ptr, size_t oldSize, size_t newSize, size_t alignment, bool zeroed = false);
    virtual void Deallocate(SrcLoc loc, void* ptr);
    virtual bool DeallocateAll(SrcLoc loc);
};

struct Allocator
{
    IAllocator* impl;

    Allocator() = default;
    Allocator(IAllocator* allocatorImpl) : impl(allocatorImpl) { }

    operator bool() const { return impl != nullptr; }
    operator IAllocator*() const { return impl; }

    /**
     * Allocate memory using the specified allocator.
     * If the allocator is null, this function will return nullptr.
     */
    void* Allocate(size_t size, size_t alignment, SrcLoc loc) const
    {
        if (!impl) return nullptr;
        if (!size) return nullptr;
        return impl->Allocate(loc, size, alignment, false);
    }

    /**
     * Allocate zero-initialized memory using the specified allocator.
     * If the allocator is null, this function will return nullptr.
     */
    void* AllocateZeroed(size_t size, size_t alignment, SrcLoc loc) const
    {
        if (!impl) return nullptr;
        if (!size) return nullptr;
        return impl->Allocate(loc, size, alignment, true);
    }

    /**
     * Reallocate memory using the specified allocator. This will attempt to resize the given memory block to the new size, while preserving
     * the existing data. If the allocator does not support in-place reallocation, it will allocate a new block of memory, copy the existing
     * data to it, and free the old block. Optionally, zero-initialize any new memory if the allocator supports it.
     * If the allocator is null, this function will return nullptr, and the original memory block will not be freed.
     */
    void* Reallocate(void* ptr, size_t oldSize, size_t newSize, size_t alignment, SrcLoc loc) const
    {
        if (!impl) return nullptr;
        if (!newSize) { impl->Deallocate(loc, ptr); return nullptr; }
        if (!oldSize) { if (ptr) impl->Deallocate(loc, ptr); return Allocate(newSize, alignment, loc); }
        return impl->Reallocate(loc, ptr, oldSize, newSize, alignment, false);
    }

    /**
     * Reallocate zero-initialized memory using the specified allocator. This is similar to Reallocate, but will zero-initialize any new
     * memory if the allocator supports it. If the allocator is null, this function will return nullptr, and the original memory block will not be freed.
     */
    void* ReallocateZeroed(void* ptr, size_t oldSize, size_t newSize, size_t alignment, SrcLoc loc) const
    {
        if (!impl) return nullptr;
        if (!newSize) { impl->Deallocate(loc, ptr); return nullptr; }
        if (!oldSize) { if (ptr) impl->Deallocate(loc, ptr); return AllocateZeroed(newSize, alignment, loc); }
        return impl->Reallocate(loc, ptr, oldSize, newSize, alignment, true);
    }

    /**
     * Deallocate memory using the specified allocator. This will free the given memory block back to the allocator. If the allocator is null,
     * this function will return without doing anything.
     */
    void Deallocate(void* ptr, SrcLoc loc) const
    {
        if (!impl) return;
        if (!ptr) return;
        impl->Deallocate(loc, ptr);
    }

    /**
     * Deallocate all memory allocated by the specified allocator. This is a bulk deallocation function that can be used to free all memory
     * allocated by an allocator in one go. This is particularly useful for temporary allocators that are designed for short-term allocations,
     * as it allows for efficient cleanup without having to track individual allocations.
     * If the allocator is null, this function will return false.
     *
     * Note that this function may not be supported by all allocators, and may return false if the allocator does not support bulk deallocation.
     * In that case, the caller should fall back to manually deallocating individual allocations if necessary.
     */
    bool DeallocateAll(SrcLoc loc) const
    {
        if (!impl) return false;
        return impl->DeallocateAll(loc);
    }

    /**
     * Create a new object of type T using the specified allocator. This function will allocate memory for the object using the allocator, and then
     * construct the object in-place using the provided constructor arguments. If the allocator is null, this function will return nullptr.
     * The allocated memory will be zero-initialized before constructing the object, so that any padding bytes are also zeroed.
     */
    template <typename T, typename... Args>
    T* New(SrcLoc loc, Args&&... args);

    template <typename T>
    T* New(SrcLoc loc);

    /**
     * Delete an object of type T using the specified allocator. This function will call the destructor of the object, and then free the memory
     * back to the allocator. If the allocator is null, this function will return without doing anything.
     */
    template <typename T>
    void Delete(T* obj, SrcLoc loc);

    /**
     * Create a new slice of type T using the specified allocator. This function will allocate memory for the slice using the allocator, and then
     * construct the slice in-place with the given count. The allocated memory will be zero-initialized before constructing the slice, so that any padding bytes are also zeroed.
     * If the allocator is null, this function will return an empty slice.
     */
    template <typename T>
    Slice<T> MakeSlice(size_t count, SrcLoc loc);

    /**
     * Create a new slice of type T using the specified allocator, and initialize it with the given initializer list. This function will allocate memory for the slice using the allocator, and then
     * construct the slice in-place with the given count. The allocated memory will be zero-initialized before constructing the slice, so that any padding bytes are also zeroed.
     * If the allocator is null, this function will return an empty slice.
     */
    template <typename T>
    Slice<T> MakeSlice(std::initializer_list<T> initList, SrcLoc loc);

    /**
     * Delete a slice of type T using the specified allocator. This function will free the memory used by the slice back to the allocator,
     * and then reset the slice to an empty state. If the allocator is null, this function will return without doing anything.
     */
    template <typename T>
    void FreeSlice(Slice<T>* slice, SrcLoc loc);

    /**
     * Resize a slice of type T using the specified allocator. This function will attempt to resize the given slice to the new size, while preserving
     * the existing data. If the allocator does not support in-place reallocation, it will allocate a new block of memory, copy the existing data to it,
     * and free the old block.
     */
    template <typename T>
    void ResizeSlice(Slice<T>* slice, size_t newSize, SrcLoc loc);

    /**
     * Clone a slice of type T using the specified allocator. This function will create a new slice that is a copy of the given slice, with its own
     * memory allocation. The contents of the slice will be copied to the new memory block. If the allocator is null, this function will return an
     * empty slice.
     */
    template <typename T>
    Slice<T> CloneSlice(const Slice<T>& slice, SrcLoc loc);

    /**
     * Create a new list of type T using the specified allocator. This function will allocate memory for the list using the allocator, and then
     * construct the list in-place. The allocated memory will be zero-initialized before constructing the list, so that any padding bytes are also
     * zeroed. If the allocator is null, this function will return an empty list.
     */
    template <typename T>
    List<T> MakeList();

    /**
     * Create a new list of type T using the specified allocator. This function will allocate memory for the list using the allocator, and then
     * construct the list in-place with the given initial capacity. The allocated memory will be zero-initialized before constructing the list,
     * so that any padding bytes are also zeroed.
     */
    template <typename T>
    List<T> MakeList(size_t initialCapacity, SrcLoc loc);

    /**
     * Free a list of type T using the specified allocator. This function will free the memory used by the list back to the allocator, and then reset the
     * list to an empty state. If the allocator is null, this function will return without doing anything.
     */
    template <typename T>
    void FreeList(List<T>* list, SrcLoc loc);

    /**
     * Clone a list of type T using the specified allocator. This function will create a new list that is a copy of the given list, with its own memory
     * allocation. The contents of the list will be copied to the new memory block. If the allocator is null, this function will return an empty list.
     */
    template <typename T>
    List<T> CloneList(const List<T>& list, SrcLoc loc);

    /**
     * Change the allocator used by a list of type T. This function will attempt to change the allocator used by the list to the new allocator, while preserving
     * the existing information.
     * It allocates new memory for the list using the new allocator, copies the existing data to it, and frees the old block of memory back to the old allocator.
     */
    template <typename T>
    void ChangeAllocator(List<T>* list, Allocator newAllocator, SrcLoc loc);

    /**
     * Create a new String with the specified length using the given allocator. This function will allocate memory for the string using the allocator,
     * and then construct the string in-place with the given length. The allocated memory will be zero-initialized before constructing the string, so
     * that any padding bytes are also zeroed. If the allocator is null, this function will return an empty string.
     */
    String MakeString(size_t length, SrcLoc loc);

    /**
     * Free a String using the specified allocator. This function will free the memory used by the string back to the allocator, and then reset the
     * string to an empty state. If the allocator is null, this function will return without doing anything.
     */
    void FreeString(String* str, SrcLoc loc);

    /**
     * Clone a String using the specified allocator. This function will create a new string that is a copy of the given string, with its own memory
     * allocation. The contents of the string will be copied to the new memory block. If the allocator is null, this function will return an empty string.
     */
    String CloneString(const String& str, SrcLoc loc);

    /**
     * Create a new CString with the specified length using the given allocator. This function will allocate memory for the C string using the allocator,
     * and then construct the C string in-place with the given length. The allocated memory will be zero-initialized before constructing the C string, so
     * that any padding bytes are also zeroed. The resulting C string will be null-terminated. If the allocator is null, this function will return a null
     * C string.
     *
     * Note that the length parameter specifies the length of the string excluding the null terminator.
     * The function will allocate length + 1 bytes to accommodate the null terminator.
     */
    CString MakeCString(size_t length, SrcLoc loc);

    /**
     * Create a new CString by cloning the given String using the specified allocator. This function will allocate memory for the C string using the
     * allocator, and then construct the C string in-place by copying the contents of the given String. The allocated memory will be zero-initialized
     * before constructing the C string, so that any padding bytes are also zeroed. The resulting C string will be null-terminated. If the allocator is
     * null, this function will return a null C string.
     */
    CString MakeCString(const String& str, SrcLoc loc);

    /**
     * Free a CString using the specified allocator. This function will free the memory used by the C string back to the allocator, and then reset the
     * C string to a null state. If the allocator is null, this function will return without doing anything.
     */
    void FreeCString(CString* str, SrcLoc loc);

    /**
     * Clone a CString using the specified allocator. This function will create a new C string that is a copy of the given C string, with its own memory
     * allocation. The contents of the C string will be copied to the new memory block, and the new C string will be null-terminated. If the allocator is null,
     * this function will return a null C string.
     */
    CString CloneCString(const CString& str, SrcLoc loc);

    /**
     * Format a string using the specified allocator. This function will create a new String that contains the formatted text, using the given format string
     * and arguments. The allocated memory for the string will be managed by the allocator, and will be freed when the string is destroyed. If the allocator
     * is null, this function will return an empty string.
     */
    template <typename... Args>
    String FormatString(SrcLoc loc, const char* fmt, Args&&... args);

    /**
     * Format a C string using the specified allocator. This function will create a new CString that contains the formatted text, using the given format string
     * and arguments. The allocated memory for the C string will be managed by the allocator, and will be freed when the C string is destroyed. The resulting C
     * string will be null-terminated. If the allocator is null, this function will return a null C string.
     */
    template <size_t N, typename... Args>
    CString FormatCString(SrcLoc loc, const char (&fmt)[N], Args&&... args);
};

/**
 * Get the default allocator instance. This is a general-purpose thread-safe allocator that can be used for most allocation needs.
 * It is not optimised for any specific use case, but is a good default choice for most situations.
 */
Allocator GetDefaultAllocator();

/**
 * Short-hand for `GetDefaultAllocator()`.
 */
#define alloc_main (GetDefaultAllocator())

/**
 * Get a temporary allocator instance. This is a general-purpose allocator that is optimised for short-term allocations that
 * will be freed in bulk. It uses a thread-local arena allocator under the hood, so it is not thread-safe and should only be used
 * for allocations with a small lifetime ( < 1 function / frame).
 */
Allocator GetTempAllocator();

/**
 * Short-hand for `GetTempAllocator()`.
 */
#define alloc_temp (GetTempAllocator())

/**
 * A simple slice structure that represents a contiguous sequence of elements in memory. It is not responsible for managing the memory
 * itself, but provides utility functions for accessing and manipulating the data. It is a general-purpose structure that can be used
 * for any type of data, and is not specific to any particular use case.
 */
template <typename T>
struct Slice
{
    static_assert(std::is_pod_v<T>, "Slice only supports POD types");
    static_assert(std::is_trivially_copyable_v<T>, "Slice only supports trivially copyable types");
    static_assert(std::is_trivially_destructible_v<T>, "Slice only supports trivially destructible types");
    friend struct Allocator;

private:
    T*     data;
    size_t count;

public:
    Slice() = default;

    Slice(T* inData, size_t inCount) : data(inData), count(inCount) { }

    T& operator[](size_t index)
    {
        MSR_ASSERT(index >= 0 && index < count && "Index out of bounds in Slice");
        return data[index];
    }

    const T& operator[](size_t index) const
    {
        MSR_ASSERT(index >= 0 && index < count && "Index out of bounds in Slice");
        return data[index];
    }

    T* Data() { return data; }
    const T* Data() const { return data; }

    size_t Count() const { return count; }

    Slice<T> SubSlice(size_t start, size_t subCount)
    {
        MSR_ASSERT(start >= 0 && subCount >= 0 && start + subCount <= count && "Invalid subslice range");
        return Slice<T>(data + start, subCount);
    }

    operator bool() const { return data != nullptr && count > 0; }
};

template <typename T>
struct List
{
    static_assert(std::is_pod_v<T>, "List only supports POD types");
    static_assert(std::is_trivially_copyable_v<T>, "List only supports trivially copyable types");
    static_assert(std::is_trivially_destructible_v<T>, "List only supports trivially destructible types");
    friend struct Allocator;

private:
    T*        data;
    size_t    count;
    size_t    capacity;
    Allocator allocator;

public:
    List() = default;

    T& operator[](size_t index)
    {
        MSR_ASSERT(index >= 0 && index < count && "Index out of bounds in List");
        return data[index];
    }

    const T& operator[](size_t index) const
    {
        MSR_ASSERT(index >= 0 && index < count && "Index out of bounds in List");
        return data[index];
    }

    T* Data() { return data; }
    const T* Data() const { return data; }

    size_t Count() const { return count; }
    size_t Capacity() const { return capacity; }

    Slice<T> AsSlice() { return Slice<T>(data, count); }
    const Slice<T> AsSlice() const { return Slice<T>(data, count); }

    Slice<T> SubSlice(size_t start, size_t subCount)
    {
        MSR_ASSERT(start >= 0 && subCount >= 0 && start + subCount <= count && "Invalid subslice range");
        return Slice<T>(data + start, subCount);
    }

    operator bool() const { return data != nullptr && count > 0; }

    void Reserve(size_t newCapacity, SrcLoc loc)
    {
        if (newCapacity <= capacity) return;
        size_t newSize = sizeof(T) * newCapacity;
        size_t oldSize = sizeof(T) * capacity;
        void* newMem = allocator.ReallocateZeroed(data, oldSize, newSize, alignof(T), loc);
        if (!newMem) return;
        data = (T*) newMem;
        capacity = newCapacity;
    }

    void Add(const T& item, SrcLoc loc)
    {
        if (capacity < count + 1)
        {
            size_t newCap = capacity == 0 ? 16 : capacity * 2;
            Reserve(newCap, loc);
        }

        MSR_ASSERT(count < capacity && "Failed to reserve enough capacity in List");
        data[count++] = item;
    }

    void Add(T&& item, SrcLoc loc)
    {
        if (capacity < count + 1)
        {
            size_t newCap = capacity == 0 ? 16 : capacity * 2;
            Reserve(newCap, loc);
        }

        MSR_ASSERT(count < capacity && "Failed to reserve enough capacity in List");
        data[count++] = std::move(item);
    }

    void Push(const T& item, SrcLoc loc)
    {
        Add(item, loc);
    }

    void Push(T&& item, SrcLoc loc)
    {
        Add(std::move(item), loc);
    }

    void RemoveAt(size_t index, SrcLoc loc)
    {
        MSR_ASSERT(index >= 0 && index < count && "Index out of bounds in List");
        if (index < count - 1)
            memmove(data + index, data + index + 1, sizeof(T) * (count - index - 1));
        count--;
    }

    T Pop(SrcLoc loc)
    {
        MSR_ASSERT(count > 0 && "Cannot pop from an empty List");
        return data[--count];
    }

    void Clear(SrcLoc loc)
    {
        count = 0;
    }
};

/**
 * A simple wrapper around a null-terminated C string. This is used for interoperability with C APIs
 * that expect null-terminated strings, and for convenience when working with C-style strings in C++.
 */
struct CString
{
    friend struct Allocator;

private:
    const char* data;

public:
    CString() = default;

    constexpr CString(const char* str) : data(str) { }

    template <size_t N>
    constexpr CString(const char (&str)[N]) : data(str) { }

    char* Data() { return const_cast<char*>(data); }
    const char* Data() const { return data; }

    size_t Length() const;

    operator bool() const { return data != nullptr && data[0] != '\0'; }
    operator char*() { return Data(); }
    operator const char*() const { return Data(); }

    String AsString();
    const String AsString() const;

    Slice<char> AsSlice();
    const Slice<char> AsSlice() const;
};

/**
 * UTF-8 strings (not null-terminated). Simple wrapper around Slice<uint8_t> with some utility functions
 * for convenience.
 */
struct String
{
    friend struct Allocator;

private:
    Slice<uint8_t> slice;

public:
    String() = default;

    // converting constructor
    template <size_t N>
    String(const char (&str)[N]) : slice((uint8_t*) str, N - 1) { }

    String(uint8_t* data, size_t length) : slice(data, length) { }

    String(Slice<uint8_t> slice) : slice(slice) { }

    String(const CString& str) : slice((uint8_t*) str.Data(), str.Length()) { }

    Slice<uint8_t> AsSlice() { return slice; }
    const Slice<uint8_t> AsSlice() const { return slice; }

    uint8_t* Data() { return slice.Data(); }
    const uint8_t* Data() const { return slice.Data(); }

    size_t Length() const { return slice.Count(); }

    operator bool() const { return slice; }

    String SubString(size_t start, size_t subCount);
};

template <typename T, typename... Args>
T* Allocator::New(SrcLoc loc, Args&&... args)
{
    if (!impl) { return nullptr; }
    void* mem = AllocateZeroed(sizeof(T), alignof(T), loc);
    if (!mem) { return nullptr; }
    return new (mem) T(std::forward<Args>(args)...);
}

template <typename T>
T* Allocator::New(SrcLoc loc)
{
    if (!impl) { return nullptr; }
    void* mem = AllocateZeroed(sizeof(T), alignof(T), loc);
    if (!mem) { return nullptr; }
    return new (mem) T();
}

template <typename T>
void Allocator::Delete(T* obj, SrcLoc loc)
{
    if (!impl) { return; }
    if (!obj) { return; }
    obj->~T();
    Deallocate(obj, loc);
}

template <typename T>
Slice<T> Allocator::MakeSlice(size_t count, SrcLoc loc)
{
    if (!impl) { return Slice<T>(); }
    void* mem = AllocateZeroed(sizeof(T) * count, alignof(T), loc);
    if (!mem) { return Slice<T>(); }
    return Slice<T>((T*) mem, count);
}

template <typename T>
Slice<T> Allocator::MakeSlice(std::initializer_list<T> initList, SrcLoc loc)
{
    if (!impl) { return Slice<T>(); }
    void* mem = Allocate(sizeof(T) * initList.size(), alignof(T), loc);
    if (!mem) { return Slice<T>(); }
    std::uninitialized_copy(initList.begin(), initList.end(), (T*) mem);
    return Slice<T>((T*) mem, initList.size());
}

template <typename T>
void Allocator::FreeSlice(Slice<T>* slice, SrcLoc loc)
{
    if (!impl) { return; }
    if (slice->data) Deallocate(slice->data, loc);
    *slice = Slice<T>();
}

template <typename T>
void Allocator::ResizeSlice(Slice<T>* slice, size_t newSize, SrcLoc loc)
{
    if (!impl) { return; }
    void* newMem = ReallocateZeroed(slice->data, sizeof(T) * slice->count, sizeof(T) * newSize, alignof(T), loc);
    if (!newMem) { return; }
    slice->data = (T*) newMem;
    slice->count = newSize;
}

template <typename T>
Slice<T> Allocator::CloneSlice(const Slice<T>& slice, SrcLoc loc)
{
    if (!impl) { return Slice<T>(); }
    if (!slice) { return Slice<T>(); }
    Slice<T> newSlice = MakeSlice<T>(slice.Count(), loc);
    if (!newSlice) { return Slice<T>(); }
    memcpy(newSlice.Data(), slice.Data(), sizeof(T) * slice.Count());
    return newSlice;
}

template <typename T>
List<T> Allocator::MakeList()
{
    List<T> list = List<T>();
    list.allocator = *this;
    return list;
}

template <typename T>
List<T> Allocator::MakeList(size_t initialCapacity, SrcLoc loc)
{
    List<T> list = MakeList<T>();
    list.Reserve(initialCapacity, loc);
    return list;
}

template <typename T>
void Allocator::FreeList(List<T>* list, SrcLoc loc)
{
    if (!impl) { return; }
    if (list->data) Deallocate(list->data, loc);
    *list = List<T>();
}

template <typename T>
List<T> Allocator::CloneList(const List<T>& list, SrcLoc loc)
{
    if (!impl) { return List<T>(); }
    if (!list) { return List<T>(); }
    List<T> newList = MakeList<T>(list.Capacity(), loc);
    if (!newList) { return List<T>(); }
    newList.count = list.Count();
    memcpy(newList.Data(), list.Data(), sizeof(T) * list.Count());
    return newList;
}

template <typename T>
void Allocator::ChangeAllocator(List<T>* list, Allocator newAllocator, SrcLoc loc)
{
    if (!impl || !list) { return; }
    if (!list->data) // no data at all
    {
        *list = newAllocator.MakeList<T>();
        return;
    }

    if (!list->capacity) // has some data but no capacity (?)
    {
        FreeList(list, loc);
        *list = MakeList<T>();
        return;
    }

    if (!list->count) // has capacity but no info, basically a preallocated buffer
    {
        size_t oldCapacity = list->capacity;
        FreeList(list, loc);
        *list = MakeList<T>(oldCapacity, loc);
        return;
    }

    // has info and capacity
    auto newList = newAllocator.MakeList<T>(list->capacity, loc);
    if (!newList) { return; }

    memcpy(newList.Data(), list->Data(), sizeof(T) * list->Count());
    newList.count = list->count;

    FreeList(list, loc);
    *list = newList;
}
