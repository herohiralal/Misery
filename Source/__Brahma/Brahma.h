// =============================================================================================================================
// Warnings
#ifndef BRAHMA_NO_WARNINGS

#ifdef _MSC_VER
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #pragma warning(disable: 5045) // spectre mitigation
    #pragma warning(disable: 4324) // structure was padded due to alignment specifier
    #pragma warning(disable: 4820) // bytes padding added after data member
    #pragma warning(disable: 4127) // conditional expression is constant
    #pragma warning(disable: 4514) // unreferenced inline function has been removed
    #pragma warning(disable: 4710) // function not inlined
    #pragma warning(disable: 4711) // function selected for automatic inline expansion
    #pragma warning(disable: 4464) // relative include path contains '..'
    #pragma warning(disable: 5038) // data member will be initialized after base class
    #pragma warning(disable: 4577) // 'noexcept' used with no exception handling mode specified

    #define BRAHMA_SUPPRESS_WARN \
        __pragma(warning(push, 0))

    #define BRAHMA_UNSUPPRESS_WARN \
        __pragma(warning(pop))
#endif

#ifdef __GNUC__
    #pragma GCC diagnostic error   "-Wall"
    #pragma GCC diagnostic error   "-Wextra"
    #pragma GCC diagnostic error   "-Wshadow"
    #pragma GCC diagnostic error   "-Wconversion"
    #pragma GCC diagnostic error   "-Wsign-conversion"
    #pragma GCC diagnostic error   "-Wdouble-promotion"
    #pragma GCC diagnostic error   "-Wfloat-equal"
    #pragma GCC diagnostic error   "-Wundef"
    #pragma GCC diagnostic error   "-Wswitch-enum"
    #ifndef __cplusplus
        #pragma GCC diagnostic error   "-Wstrict-prototypes"
    #endif
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wuninitialized"

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("GCC diagnostic push")  \
        _Pragma("GCC diagnostic ignored \"-Wall\"") \
        _Pragma("GCC diagnostic ignored \"-Wextra\"") \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \

    #define BRAHMA_UNSUPPRESS_WARN \
        _Pragma("GCC diagnostic pop")
#endif

#ifdef __clang__
    #pragma clang diagnostic error   "-Wall"
    #pragma clang diagnostic error   "-Wextra"
    #pragma clang diagnostic error   "-Wshadow"
    #pragma clang diagnostic error   "-Wconversion"
    #pragma clang diagnostic error   "-Wsign-conversion"
    #pragma clang diagnostic error   "-Wdouble-promotion"
    #pragma clang diagnostic error   "-Wfloat-equal"
    #pragma clang diagnostic error   "-Wundef"
    #pragma clang diagnostic error   "-Wswitch-enum"
    #ifndef __cplusplus
        #pragma clang diagnostic error   "-Wstrict-prototypes"
    #endif
    #pragma clang diagnostic ignored "-Wunused-parameter"
    #pragma clang diagnostic ignored "-Wuninitialized"

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("clang diagnostic push")  \
        _Pragma("clang diagnostic ignored \"-Weverything\"")

    #define BRAHMA_UNSUPPRESS_WARN \
        _Pragma("clang diagnostic pop")
#endif

#endif//BRAHMA_NO_WARNINGS

// =============================================================================================================================
// Includes
#if 1

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #define NOMINMAX
#endif

#ifdef __linux__
    #ifndef __cplusplus
        #define _GNU_SOURCE
    #endif
    #define _POSIX_C_SOURCE 200809L
    #define _XOPEN_SOURCE 700
#endif

#ifdef __APPLE__
    #define _DARWIN_C_SOURCE
#endif

BRAHMA_SUPPRESS_WARN
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
    #include <windows.h>
    #include <malloc.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <pthread.h>
    #include <stdlib.h>
    #include <dirent.h>
    #include <sys/stat.h>
#endif
BRAHMA_UNSUPPRESS_WARN

#endif

// =============================================================================================================================
// Main header file.
#ifndef BRAHMA_H
#define BRAHMA_H

#ifdef __cplusplus
extern "C" {
#endif

// some important macros
#ifndef __cplusplus
    #define static_assert _Static_assert

    #if defined(_MSC_VER)
        #define alignas(x) __declspec(align(x))
        #define alignof(x) __alignof(x)
        #define thread_local __declspec(thread)
        #define inline __inline
    #elif defined(__GNUC__) || defined(__clang__)
        #define alignas(x) __attribute__((aligned(x)))
        #define alignof(x) __alignof__(x)
        #define thread_local __thread
        #define inline __inline__
    #else
        #error "Unsupported compiler."
    #endif

    typedef _Bool bool;
    #ifndef true
        #define true ((bool) 1)
    #endif
    #ifndef false
        #define false ((bool) 0)
    #endif
#endif

/**
 * Internally, Brahma uses a custom memory allocator, which is a simple linear allocator, that never frees memory.
 * This function is thread-safe to use, but it'll lock the allocator's mutex while it's being used.
 */
void* brahma_push_memory(size_t size, size_t alignment);

/**
 * Helper macro to push a struct of a given type to the internal allocator, and return a pointer to it.
 */
#define BRAHMA_PUSH_STRUCT(type) ((type*) brahma_push_memory(sizeof(type), alignof(type)))

/**
 * Helper macro to push an array of a given type and count to the internal allocator, and return a
 * pointer to the first item.
 */
#define BRAHMA_PUSH_STRUCT_ARRAY(type, count) ((type*) brahma_push_memory(sizeof(type) * (count), alignof(type)))

/**
 * Declare an array list type.
 * An array list is a simple dynamic array that can grow in size. It has a pointer to the data, the count of items,
 * and the capacity of the list.
 * It starts out with a small capacity (64 bytes worth of items), and doubles its capacity as it needs to grow.
 */
#define BRAHMA_DECLARE_ARRAY_LIST(convNameSt, convNameFn, type) \
    typedef struct \
    { \
        type*  data; \
        size_t count; \
        size_t capacity; \
    } Brahma_##convNameSt##_Array_List; \
    static inline type* brahma_index_##convNameFn##_array_list(Brahma_##convNameSt##_Array_List* list, size_t index) \
    { \
        if (index >= list->count) return NULL; \
        return &(list->data[index]); \
    } \
    static inline void brahma_reserve_##convNameFn##_array_list_capacity(Brahma_##convNameSt##_Array_List* list, size_t requiredCapacity) \
    { \
        if (requiredCapacity <= list->capacity) return; \
        /* the internal allocator allocates in blocks of at least 64 bytes, this ensures we don't waste space */ \
        size_t newCapacity = list->capacity ? list->capacity : (64 / sizeof(type)); \
        while (newCapacity < requiredCapacity) newCapacity *= 2; \
        type* newData = BRAHMA_PUSH_STRUCT_ARRAY(type, newCapacity); \
        if (list->data) { memcpy(newData, list->data, sizeof(type) * list->count); } \
        list->data = newData; \
        list->capacity = newCapacity; \
    } \
    static inline void brahma_append_##convNameFn##_to_array_list(Brahma_##convNameSt##_Array_List* list, type item) \
    { \
        brahma_reserve_##convNameFn##_array_list_capacity(list, list->count + 1); \
        list->data[list->count] = item; \
        list->count++; \
    }

/**
 * Declare a paged list type.
 * A paged list provides a way to allocate a list of items without needing to reallocate the entire list when it grows.
 * It does this by allocating fixed-size pages of items, and linking them together using a linked-list.
 * The items per page are configurable, but the first variable in each page is always a pointer to the next page.
 * The alignment of each page is hardcoded to be 64 bytes, which provides good cache performance when iterating, and it
 * also allows for different threads to freely write to different pages, without causing false sharing.
 */
#define BRAHMA_DECLARE_PAGED_LIST(convNameSt, convNameFn, type, itemsPerPage) \
    typedef struct Brahma_##convNameSt##_Paged_List_Page Brahma_##convNameSt##_Paged_List_Page; \
    struct alignas(64) Brahma_##convNameSt##_Paged_List_Page \
    { \
        Brahma_##convNameSt##_Paged_List_Page* nextPage; \
        type                                   items[itemsPerPage]; \
    }; \
    typedef struct \
    { \
        Brahma_##convNameSt##_Paged_List_Page* firstPage; \
        size_t                                 count; \
        size_t                                 numPages; \
    } Brahma_##convNameSt##_Paged_List; \
    static inline type* brahma_index_##convNameFn##_paged_list(Brahma_##convNameSt##_Paged_List* list, size_t index) \
    { \
        if (index >= list->count) return NULL; \
        size_t pageIndex = index / itemsPerPage; \
        size_t itemIndex = index % itemsPerPage; \
        Brahma_##convNameSt##_Paged_List_Page* page = list->firstPage; \
        for (size_t i = 0; i < pageIndex; i++) { page = page->nextPage; } \
        return &(page->items[itemIndex]); \
    } \
    static inline void brahma_reserve_##convNameFn##_paged_list_capacity(Brahma_##convNameSt##_Paged_List* list, size_t capacity) \
    { \
        size_t requiredPages = (capacity + itemsPerPage - 1) / itemsPerPage; \
        if (requiredPages <= list->numPages) return; \
        Brahma_##convNameSt##_Paged_List_Page* pageIt = NULL; \
        for (pageIt = list->firstPage; pageIt && pageIt->nextPage; pageIt = pageIt->nextPage) { } \
        /* pageIt is now at the last page (or NULL if there are no pages) */ \
        size_t pagesToCreate = requiredPages - list->numPages; \
        Brahma_##convNameSt##_Paged_List_Page* createdPages = BRAHMA_PUSH_STRUCT_ARRAY(Brahma_##convNameSt##_Paged_List_Page, pagesToCreate); \
        for (size_t i = 0; i < pagesToCreate; i++) \
        { \
            Brahma_##convNameSt##_Paged_List_Page* newPage = &(createdPages[i]); \
            *(pageIt ? &(pageIt->nextPage) : &(list->firstPage)) = newPage; \
            pageIt = newPage; \
            pageIt->nextPage = NULL; \
        } \
        list->numPages += pagesToCreate; \
    } \
    static inline void brahma_append_##convNameFn##_to_paged_list(Brahma_##convNameSt##_Paged_List* list, type item) \
    { \
        brahma_reserve_##convNameFn##_paged_list_capacity(list, list->count + 1); \
        *(brahma_index_##convNameFn##_paged_list(list, list->count)) = item; \
        list->count++; \
    }

BRAHMA_DECLARE_PAGED_LIST(String, string, char*, 15)

/**
 * Helper function to format a string using the internal allocator.
 *
 * A small thread-local buffer of a few kilobytes is used as an intermediate to format the string.
 * If the formatted string fits in the buffer, it takes a fast path and copies the formatted string to the internal allocator.
 * If the formatted string does not fit in the buffer, it takes a slow path and has to format the string a second time, directly
 * into the internal allocator.
 */
char* brahma_sprintf(const char* format, ...);

/**
 * Key-value pair of a definition.
 * This is used to store the definitions that a library (or a package) declares.
 * The key is the name of the definition, and the value is the value.
 * The value can include new lines, but will need to add backslashes for escaping, since the value is stored as a single string.
 */
typedef struct
{
    const char* key;
    const char* value;
} Brahma_Define;

BRAHMA_DECLARE_PAGED_LIST(Define, define, Brahma_Define, 7)

/**
 * Package definition.
 *
 * A package is a single output that is generated by one or more libraries. It could be an executable, a dynamic library,
 * or a static library.
 *
 * An executable package can also optionally depend on other dynamic library packages.
 *
 * A package must declare at least one library that it is dependent on. All the source files in that library (and its dependencies)
 * will be compiled to build the package. The output will also be linked against the static libraries that that library (and its
 * dependencies) might contain.
 */
typedef struct
{
    /**
     * Package-level definitions to use for compilation.
     */
    Brahma_Define_Paged_List defines;

    /**
     * The primary library that this package depends on. All the source files in this library (and its dependencies) will be compiled
     * to build the package.
     */
    const char* primaryLibrary;
} Brahma_Package_Definition;

/**
 * Library definition.
 *
 * A library is a single unit of code that contains one or more input files to build different packages. It could be code files,
 * or a static library.
 *
 * Libraries can also declare dependencies on other libraries. In this case, they will be able to `#include` the headers of the
 * libraries that they are dependent on.
 */
typedef struct
{
    /**
     * The libraries that this library depends on for its implementation, as well as its interface (headers).
     */
    Brahma_String_Paged_List interfaceDependencies;

    /**
     * The libraries that this library depends on for its implementation, and not its interface (headers).
     */
    Brahma_String_Paged_List internalDependencies;

    /**
     * Library-level definitions to use for compilation. These definitions will be used when compiling the source files in this
     * library, as well as the source files in the libraries that depend on this library (recursively).
     */
    Brahma_Define_Paged_List interfaceDefines;

    /**
     * Library-level definitions to use for compilation. These definitions will be used when compiling the source files in this
     * library, but not the source files in the libraries that depend on this library.
     */
    Brahma_Define_Paged_List internalDefines;
} Brahma_Library_Definition;

/**
 * Implement a package.
 *
 * Usage:
 ```
 BRAHMA_IMPLEMENT_PACKAGE_DEF(packageName)
 {
     Brahma_Package_Definition pkg;
     // set up package data...
     return pkg;
 }
 ```
 */
#define BRAHMA_IMPLEMENT_PACKAGE_DEF(packageName) \
    Brahma_Package_Definition brahma_implement_package_##packageName(void)

/**
 * Implement a library.
 *
 * Usage:
 ```
 BRAHMA_IMPLEMENT_LIBRARY_DEF(libraryName)
 {
     Brahma_Library_Definition lib;
     // set up library data...
     return lib;
 }
 ```
 */
#define BRAHMA_IMPLEMENT_LIBRARY_DEF(libraryName) \
    Brahma_Library_Definition brahma_implement_library_##libraryName(const Brahma_Package_Definition* package)

// =============================================================================================================================
// Library code (used when this header is included as a library, or if in exec mode).
#if defined(BRAHMA_LIBRARY) || defined(BRAHMA_EXEC)

// all the data pertaining to a package, including its definition
typedef struct
{
    const char* name;
    const char* owningFile;
    Brahma_Package_Definition def;
} Brahma_Package;

BRAHMA_DECLARE_ARRAY_LIST(Package, package, Brahma_Package)

// all the data pertaining to a library, including its definition
typedef struct
{
    const char* name;
    const char* owningFile;
    Brahma_Library_Definition def;
} Brahma_Library;

BRAHMA_DECLARE_ARRAY_LIST(Library, library, Brahma_Library)

// delegate signature for creating packages
typedef void (*Brahma_Package_Creator_Delegate)(Brahma_Package_Array_List* packages);

// delegate signature for creating libraries
typedef void (*Brahma_Library_Creator_Delegate)(Brahma_Library_Array_List* libraries, const Brahma_Package_Definition* package);

// delegate signature for logging to console
typedef void (*Brahma_Log_Delegate)(const char* fmt, ...);

// flags for input args
typedef enum
{
    BRAHMA_ARGS_FLAG_NONE      =      0,
    BRAHMA_ARGS_FLAG_DEBUG     = 1 << 0,
    BRAHMA_ARGS_FLAG_OPTIMISED = 1 << 1,
} Brahma_Args_Flags_Bits;

// flags for input args; see also: Brahma_Args_Flags_Bits
typedef uint64_t Brahma_Args_Flags;

// arguments required for executing brahma
typedef struct
{
    Brahma_Args_Flags flags;
    char*             packageToBuild;

    size_t pkgCount, libCount;

    void (*log)(const char* fmt, ...);

    void (*createPackages)(Brahma_Package_Array_List* packages);
    void (*createLibraries)(Brahma_Library_Array_List* libraries, const Brahma_Package_Definition* package);
} Brahma_Args;

// main entry point function
bool brahma_execute(Brahma_Args ex);

#endif//defined(BRAHMA_LIBRARY) || defined(BRAHMA_EXEC)

// =============================================================================================================================
// Library implementation
#if defined(BRAHMA_LIBRARY_IMPL) || defined(BRAHMA_EXEC)

// initialise the internal allocator
void brahma_initialise_internal_allocator(void);

// shutdown the internal allocator
void brahma_shutdown_internal_allocator(void);

// visitor function to use for iterating a directory
typedef bool (*Brahma_Directory_Visitor_Delegate)(void* payload, const char* path, bool isDirectory, bool* exploreCurrentDirectory);

// iterate a directory, calling the visitor function for each child file and subdirectory
void brahma_iterate_directory(const char* path, bool recursive, void* visitorPayload, Brahma_Directory_Visitor_Delegate visitor);

// find a package index by its name; -1 if not found
int brahma_find_package_by_name(const Brahma_Package_Array_List* packages, const char* name);

// find a library index by its name; -1 if not found
int brahma_find_library_by_name(const Brahma_Library_Array_List* libraries, const char* name);

bool brahma_execute(Brahma_Args ex)
{
    Brahma_Package_Array_List pkgDefs = { NULL, 0, 0 };
    int selectedPkgIdx = -1;
    bool failed = false;

    brahma_initialise_internal_allocator();

    brahma_reserve_package_array_list_capacity(&pkgDefs, ex.pkgCount);
    ex.createPackages(&pkgDefs);

    // find the package to build
    {
        if (!ex.packageToBuild)
        {
            ex.log("No package specified. Defaulting to the first package!\n");
            selectedPkgIdx = 0;
        }
        else
        {
            for (int i = 0; i < BRAHMA_PACKAGE_COUNT; i++)
            {
                if (!strcmp(ex.packageToBuild, pkgDefs.data[i].name))
                {
                    selectedPkgIdx = i;
                    break;
                }
            }

            if (selectedPkgIdx == -1)
            {
                ex.log("ERROR: No package found with the name '%s'.\n", ex.packageToBuild);
                failed = true;
            }
        }
    }

    if (!failed)
    {
        ex.log("-----------------------------------------\n");
        ex.log("Brahma Configuration:\n");
        ex.log("\tSelected package: %s.\n", pkgDefs.data[selectedPkgIdx].name);
        ex.log("\tDebug info:       %s.\n", (ex.flags & BRAHMA_ARGS_FLAG_DEBUG)     ? "on" : "off");
        ex.log("\tOptimised:        %s.\n", (ex.flags & BRAHMA_ARGS_FLAG_OPTIMISED) ? "on" : "off");
        ex.log("-----------------------------------------\n");
    }

    brahma_shutdown_internal_allocator();
    return !failed;
}

void brahma_add_package(Brahma_Package_Array_List* packages, int idx, const char* name, const char* owningFile, Brahma_Package_Definition def)
{
    Brahma_Package pkg;
    pkg.name = name;
    pkg.owningFile = owningFile;
    pkg.def = def;
    brahma_append_package_to_array_list(packages, pkg);
}

void brahma_add_library(Brahma_Library_Array_List* libraries, int idx, const char* name, const char* owningFile, Brahma_Library_Definition def)
{
    Brahma_Library lib;
    lib.name = name;
    lib.owningFile = owningFile;
    lib.def = def;
    brahma_append_library_to_array_list(libraries, lib);
}

static volatile struct
{
    #if defined(_WIN32)
        CRITICAL_SECTION mutex;
    #elif defined(__linux__) || defined(__APPLE__)
        pthread_mutex_t mutex;
    #endif

    uint8_t* currentMemoryPage;
    size_t   capacity;
    size_t   offset;

    uint8_t** previousMemoryPages;
    size_t    previousMemoryPagesCount;
    size_t    previousMemoryPagesCapacity;
} g_brahmaInternalAllocator;

void brahma_initialise_internal_allocator(void)
{
    #if defined(_WIN32)
    {
        InitializeCriticalSection((LPCRITICAL_SECTION) &g_brahmaInternalAllocator.mutex);

        // spin for 15 cycles before sleeping, to improve performance when the lock
        // is only held for a short time (which is the case for our allocator)
        SetCriticalSectionSpinCount((LPCRITICAL_SECTION) &g_brahmaInternalAllocator.mutex, 0x0000000F);
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init((pthread_mutex_t*) &g_brahmaInternalAllocator.mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    #endif

    brahma_push_memory(1, 1); // ready the allocator for use
}

void brahma_shutdown_internal_allocator(void)
{
    #if defined(_WIN32)
    {
        DeleteCriticalSection((LPCRITICAL_SECTION) &g_brahmaInternalAllocator.mutex);
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        pthread_mutex_destroy((pthread_mutex_t*) &g_brahmaInternalAllocator.mutex);
    }
    #endif

    // free all the allocated pages
    for (size_t i = 0; i < g_brahmaInternalAllocator.previousMemoryPagesCount; i++)
    {
        #if defined(_WIN32)
        {
            _aligned_free(g_brahmaInternalAllocator.previousMemoryPages[i]);
        }
        #elif defined(__linux__) || defined(__APPLE__)
        {
            free(g_brahmaInternalAllocator.previousMemoryPages[i]);
        }
        #endif
    }

    if (g_brahmaInternalAllocator.currentMemoryPage)
    {
        #if defined(_WIN32)
        {
            _aligned_free(g_brahmaInternalAllocator.currentMemoryPage);
        }
        #elif defined(__linux__) || defined(__APPLE__)
        {
            free(g_brahmaInternalAllocator.currentMemoryPage);
        }
        #endif
    }

    free(g_brahmaInternalAllocator.previousMemoryPages);

    g_brahmaInternalAllocator.currentMemoryPage = NULL;
    g_brahmaInternalAllocator.capacity          = 0;
    g_brahmaInternalAllocator.offset            = 0;

    g_brahmaInternalAllocator.previousMemoryPages         = NULL;
    g_brahmaInternalAllocator.previousMemoryPagesCount    = 0;
    g_brahmaInternalAllocator.previousMemoryPagesCapacity = 0;
}

void* brahma_push_memory(size_t size, size_t alignment)
{
    // make sure that the alignment is at least 64 bytes
    // this way, this allocator can be used from multiple threads without causing false sharing
    // since each allocation will be at least 64 bytes apart
    if (alignment < 64) alignment = 64;

    // if alignment is not a power of 2, find the next power of 2 that is greater than or equal to the alignment
    if (alignment & (alignment - 1))
    {
        #if defined(_MSC_VER)
        {
            unsigned long index;
            _BitScanReverse64(&index, alignment);
            alignment = 1ULL << (index + 1);
        }
        #elif defined(__GNUC__) || defined(__clang__)
        {
            alignment = 1ULL << (64 - __builtin_clzll(alignment));
        }
        #endif
    }

    // make sure the size is aligned by the alignment, to avoid fragmentation
    size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    #if defined(_WIN32)
        EnterCriticalSection((LPCRITICAL_SECTION) &g_brahmaInternalAllocator.mutex);
    #elif defined(__linux__) || defined(__APPLE__)
        pthread_mutex_lock((pthread_mutex_t*) &g_brahmaInternalAllocator.mutex);
    #endif

    // align the offset to the required alignment
    size_t alignedPtr = ((size_t) g_brahmaInternalAllocator.currentMemoryPage) + g_brahmaInternalAllocator.offset;
    alignedPtr = (alignedPtr + alignment - 1) & ~(alignment - 1);
    size_t alignedOffset = alignedPtr - (size_t) g_brahmaInternalAllocator.currentMemoryPage;

    // check if we have enough capacity, if not, allocate a new page
    // no need to worry about keeping links to the old pages, since we never free memory
    if (alignedOffset + alignedSize > g_brahmaInternalAllocator.capacity)
    {
        size_t newPageSize = 4 * 1024 * 1024; // 4 MiB
        size_t newPageAlignment = 64; // 64 bytes, which is a common cache line size on modern CPUs

        // if the requested size is larger than the default page size, allocate a page that can fit it
        if (alignedSize > newPageSize)
            newPageSize = alignedSize;

        // allocate the new page with the required alignment
        void* newPage = NULL;
        #if defined(_WIN32)
        {
            newPage = _aligned_malloc(newPageSize, newPageAlignment);
        }
        #elif defined(__linux__) || defined(__APPLE__)
        {
            newPage = aligned_alloc(newPageAlignment, newPageSize);
        }
        #endif

        // out of memory, trap
        if (!newPage)
        {
            #if defined(_MSC_VER)
                __debugbreak();
            #elif defined(__GNUC__) || defined(__clang__)
                __builtin_trap();
            #endif
        }

        // append current page to previous pages list
        if (g_brahmaInternalAllocator.currentMemoryPage)
        {
            if (g_brahmaInternalAllocator.previousMemoryPagesCount >= g_brahmaInternalAllocator.previousMemoryPagesCapacity)
            {
                size_t newCapacity = g_brahmaInternalAllocator.previousMemoryPagesCapacity * 2;
                if (!newCapacity) newCapacity = 16; // start with a capacity of 16 if it was 0
                uint8_t** newPreviousPages = (uint8_t**) malloc(newCapacity * sizeof(uint8_t*));
                memcpy(newPreviousPages, g_brahmaInternalAllocator.previousMemoryPages, g_brahmaInternalAllocator.previousMemoryPagesCount * sizeof(uint8_t*));
                free(g_brahmaInternalAllocator.previousMemoryPages);

                g_brahmaInternalAllocator.previousMemoryPages = newPreviousPages;
                g_brahmaInternalAllocator.previousMemoryPagesCapacity = newCapacity;
            }

            g_brahmaInternalAllocator.previousMemoryPages[g_brahmaInternalAllocator.previousMemoryPagesCount++] = g_brahmaInternalAllocator.currentMemoryPage;
        }

        g_brahmaInternalAllocator.currentMemoryPage = (uint8_t*) newPage;
        g_brahmaInternalAllocator.capacity          = newPageSize;
        g_brahmaInternalAllocator.offset            = 0;
    }

    void* result = g_brahmaInternalAllocator.currentMemoryPage + alignedOffset;
    g_brahmaInternalAllocator.offset = alignedOffset + alignedSize;

    #if defined(_WIN32)
        LeaveCriticalSection((LPCRITICAL_SECTION) &g_brahmaInternalAllocator.mutex);
    #elif defined(__linux__) || defined(__APPLE__)
        pthread_mutex_unlock((pthread_mutex_t*) &g_brahmaInternalAllocator.mutex);
    #endif

    return result;
}

char* brahma_sprintf(const char* format, ...)
{
    char buffer[4096]; // 4 KiB buffer for formatting strings

    va_list args;
    va_start(args, format);
    int requiredSize = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    size_t requiredBufferSize = (size_t) requiredSize + 1; // +1 for null terminator

    char* output = (char*) brahma_push_memory(requiredBufferSize, 1); // +1 for null terminator

    if (requiredSize < (int) sizeof(buffer))
    {
        // fast path, the formatted string fits in the buffer
        memcpy(output, buffer, requiredBufferSize);
    }
    else
    {
        // slow path, the formatted string doesn't fit in the buffer, we need to format it again
        va_list args2;
        va_start(args2, format);
        vsnprintf(output, requiredBufferSize, format, args2);
        va_end(args2);
    }

    return output;
}

void brahma_iterate_directory(const char* path, bool recursive, void* visitorPayload, Brahma_Directory_Visitor_Delegate visitor)
{
    #if defined(__linux__) || defined(__APPLE__)

        DIR* dir = opendir(path);

        if (dir != NULL)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL)
            {
                const char* nextName = entry->d_name;

    #elif defined(_WIN32)

        char* searchPath = brahma_sprintf("%s\\*", path);
        WIN32_FIND_DATAA findData;
        HANDLE findHandle = FindFirstFileA(searchPath, &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                const char* nextName = findData.cFileName;

    #endif

                if (!nextName
                    || nextName[0] == '\0'
                    || !strcmp(nextName, ".")
                    || !strcmp(nextName, ".."))
                {
                    continue;
                }

                char* nextFilePath = brahma_sprintf("%s/%s", path, nextName);
                #ifdef _WIN32
                {
                    for (char* p = nextFilePath; *p; p++) { if (*p == '\\') *p = '/'; }
                }
                #endif

                bool isDirectory = false;
                #if defined(_WIN32)
                {
                    isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                }
                #elif defined(__linux__) || defined(__APPLE__)
                {
                    if (entry->d_type == DT_UNKNOWN)
                    {
                        struct stat st;
                        if (stat(nextFilePath, &st) == 0)
                        {
                            isDirectory = S_ISDIR(st.st_mode);
                        }
                    }
                    else
                    {
                        isDirectory = entry->d_type == DT_DIR;
                    }
                }
                #endif

                bool exploreCurrentDirectory = recursive;
                bool iterateFurther = visitor(visitorPayload, nextFilePath, isDirectory, &exploreCurrentDirectory);

                if (iterateFurther && isDirectory && exploreCurrentDirectory)
                {
                    brahma_iterate_directory(nextFilePath, recursive, visitorPayload, visitor);
                }

                if (!iterateFurther) break; // visitor function returned false

    #if defined(_WIN32)

            } while (FindNextFileA(findHandle, &findData));

            FindClose(findHandle);
        }

    #elif defined(__linux__) || defined(__APPLE__)

            }

            closedir(dir);
        }

    #endif
}

int brahma_find_package_by_name(const Brahma_Package_Array_List* packages, const char* name)
{
    int selectedPkgIdx = -1;
    for (int i = 0; i < BRAHMA_PACKAGE_COUNT; i++)
    {
        if (!strcmp(name, packages->data[i].name))
        {
            selectedPkgIdx = i;
            break;
        }
    }

    return selectedPkgIdx;
}

int brahma_find_library_by_name(const Brahma_Library_Array_List* libraries, const char* name)
{
    int selectedLibIdx = -1;
    for (int i = 0; i < BRAHMA_LIBRARY_COUNT; i++)
    {
        if (!strcmp(name, libraries->data[i].name))
        {
            selectedLibIdx = i;
            break;
        }
    }

    return selectedLibIdx;
}

#endif//BRAHMA_LIBRARY_IMPL

// =============================================================================================================================
// Execution code (used when CLI is executed).
#ifdef  BRAHMA_EXEC

#if !defined(BRAHMA_PACKAGE_COUNT)
#error "Package count not defined. Brahma build tool has not been built correctly."
#define BRAHMA_PACKAGE_COUNT 1
#elif !BRAHMA_PACKAGE_COUNT
#error "No packages defined."
#undef BRAHMA_PACKAGE_COUNT
#define BRAHMA_PACKAGE_COUNT 1
#endif

#if !defined(BRAHMA_LIBRARY_COUNT)
#error "Library count not defined. Brahma build tool has not been built correctly."
#define BRAHMA_LIBRARY_COUNT 1
#elif !BRAHMA_LIBRARY_COUNT
#error "No libraries defined."
#undef BRAHMA_LIBRARY_COUNT
#define BRAHMA_LIBRARY_COUNT 1
#endif

void brahma_create_all_packages(Brahma_Package_Array_List* packages);
void brahma_create_all_libraries(Brahma_Library_Array_List* libraries, const Brahma_Package_Definition* package);

void brahma_exec_log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

int main(int argc, char* argv[])
{
    Brahma_Args ex;
    ex.flags           = BRAHMA_ARGS_FLAG_NONE;
    ex.packageToBuild  = NULL;
    ex.pkgCount        = BRAHMA_PACKAGE_COUNT;
    ex.libCount        = BRAHMA_LIBRARY_COUNT;
    ex.log             = brahma_exec_log;
    ex.createPackages  = brahma_create_all_packages;
    ex.createLibraries = brahma_create_all_libraries;

    ex.flags |= BRAHMA_ARGS_FLAG_DEBUG; // default to debug mode

    for (int i = 1; i < argc; i++) // skipping first arg because it's gonna be the executable name
    {
        // intermediate stuff - not relevant once this tool has begun executing
        if (!strcmp("-cxx",                argv[i])) { i++; continue; } // make build tool in cxx mode
        if (!strcmp("-lib_search_dir", argv[i]))     { i++; continue; } // library search dirs
        if (!strcmp("-build_tool_path",    argv[i])) { i++; continue; } // the path where the build tool was compiled
        if (!strcmp("-debug_build_tool",   argv[i])) {      continue; } // whether the build tool itself is a debug build

        // flags
        if (!strcmp("-nodebuginfo", argv[i])) { ex.flags &= ~(Brahma_Args_Flags) BRAHMA_ARGS_FLAG_DEBUG;     continue; }
        if (!strcmp("-optimised",   argv[i])) { ex.flags |=  (Brahma_Args_Flags) BRAHMA_ARGS_FLAG_OPTIMISED; continue; }

        if (!strcmp("-package", argv[i]))
        {
            if (ex.packageToBuild)
            {
                ex.log("ERROR: Multiple packages specified with -package. Use as: *.exe -package packageName.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log("ERROR: No package specified after -package. Use as: *.exe -package packageName.\n");
                return 1;
            }

            ex.packageToBuild = argv[i];
            continue;
        }

        // unknown arg
        ex.log("ERROR: Unknown argument '%s'.\n", argv[i]);
        return 1;
    }

    bool success = brahma_execute(ex);
    return success ? 0 : 1;
}

#define BRAHMA_BEGIN_LISTING_PACKAGES() \
    void brahma_create_all_packages(Brahma_Package_Array_List* packages) {

#define BRAHMA_END_LISTING_PACKAGES() \
    }

#define BRAHMA_BEGIN_LISTING_LIBRARIES() \
    void brahma_create_all_libraries(Brahma_Library_Array_List* libraries, const Brahma_Package_Definition* package) {

#define BRAHMA_END_LISTING_LIBRARIES() \
    }

#define BRAHMA_ADD_PACKAGE(idx, path, packageName) \
    brahma_add_package(packages, idx, #packageName, path, brahma_implement_package_##packageName());

#define BRAHMA_ADD_LIBRARY(idx, path, libraryName) \
    brahma_add_library(libraries, idx, #libraryName, path, brahma_implement_library_##libraryName(package));

#endif//BRAHMA_EXEC

#ifdef __cplusplus
} // extern "C"
#endif

#endif//BRAHMA_H
