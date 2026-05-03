#pragma once
#include <__init.h>

template <typename T>
struct Slice;
struct CString;
struct String;

namespace DeferInternals
{
    struct Helper
    {
        template <typename Callable>
        struct Defer
        {
            Callable func;
            ~Defer() { func(); }
        };

        template <typename Callable>
        Defer<Callable> operator+(Callable&& func)
        {
            return Defer<Callable>{ std::forward<Callable>(func) };
        }
    };
}

#define DEFER_CONCAT_IMPL(x, y) x##y
#define DEFER_CONCAT(x, y) DEFER_CONCAT_IMPL(x, y)
#define DEFER auto DEFER_CONCAT(defer_, __COUNTER__) = DeferInternals::Helper() + [&]()

/**
 * Defines the source code location for debugging purposes.
 * Primarily used for logging/reporting the location where a call might have been made from.
 * General-purpose.
 */
struct SrcLoc
{
    const char* file;
    int32_t     line;
    int32_t     column;
    const char* function;
};

/**
 * Helper macro to get the current source code location. Used with functions that take a SrcLoc
 * parameter, so that the caller doesn't have to manually specify the file and line number every time.
 */
#define SRC_LOC() (::SrcLoc {.file = __FILE__, .line = __LINE__, .column = 0, .function = __FUNCTION__})

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
        return impl->Allocate(loc, size, alignment, false);
    }

    /**
     * Allocate zero-initialized memory using the specified allocator.
     * If the allocator is null, this function will return nullptr.
     */
    void* AllocateZeroed(size_t size, size_t alignment, SrcLoc loc) const
    {
        if (!impl) return nullptr;
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
        return impl->Reallocate(loc, ptr, oldSize, newSize, alignment, false);
    }

    /**
     * Reallocate zero-initialized memory using the specified allocator. This is similar to Reallocate, but will zero-initialize any new
     * memory if the allocator supports it. If the allocator is null, this function will return nullptr, and the original memory block will not be freed.
     */
    void* ReallocateZeroed(void* ptr, size_t oldSize, size_t newSize, size_t alignment, SrcLoc loc) const
    {
        if (!impl) return nullptr;
        return impl->Reallocate(loc, ptr, oldSize, newSize, alignment, true);
    }

    /**
     * Deallocate memory using the specified allocator. This will free the given memory block back to the allocator. If the allocator is null,
     * this function will return without doing anything.
     */
    void Deallocate(void* ptr, SrcLoc loc) const
    {
        if (!impl) return;
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
    T* New(Args&&... args, SrcLoc loc);

    /**
     * Delete an object of type T using the specified allocator. This function will call the destructor of the object, and then free the memory
     * back to the allocator. If the allocator is null, this function will return without doing anything.
     */
    template <typename T, typename... Args>
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
};

/**
 * Get the default allocator instance. This is a general-purpose thread-safe allocator that can be used for most allocation needs.
 * It is not optimised for any specific use case, but is a good default choice for most situations.
 */
Allocator GetDefaultAllocator();

/**
 * Get a temporary allocator instance. This is a general-purpose allocator that is optimised for short-term allocations that
 * will be freed in bulk. It uses a thread-local arena allocator under the hood, so it is not thread-safe and should only be used
 * for allocations with a small lifetime ( < 1 function / frame).
 */
Allocator GetTempAllocator();

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
    CString() : data(nullptr) { }
    constexpr CString(const char* str) : data(str) { }

    template <size_t N>
    constexpr CString(const char (&str)[N]) : data(str) { }

    char* Data() { return const_cast<char*>(data); }
    const char* Data() const { return data; }

    size_t Length() const;

    operator bool() const { return data != nullptr && data[0] != '\0'; }
    operator char*() { return Data(); }
    operator const char*() const { return Data(); }
};

/**
 * UTF-8 strings (not null-terminated). Simple wrapper around Slice<uint8_t> with some utility functions
 * for convenience.
 */
struct String : public Slice<uint8_t>
{
    String() : Slice() { };

    // converting constructor
    template <size_t N>
    String(const char (&str)[N]) : Slice((uint8_t*) str, N - 1) { }

    String(Slice<uint8_t> slice) : Slice(slice) { }
    String(CString str) : Slice((uint8_t*) str.Data(), str.Length()) { }
};

template <typename T, typename... Args>
T* Allocator::New(Args&&... args, SrcLoc loc)
{
    if (!impl) { return nullptr; }
    void* mem = AllocateZeroed(sizeof(T), alignof(T), loc);
    if (!mem) { return nullptr; }
    return new (mem) T(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
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
