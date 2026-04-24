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
    #pragma GCC diagnostic error   "-Wstrict-prototypes"
    #pragma GCC diagnostic ignored "-Wunused-parameter"

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("GCC diagnostic push")  \
        _Pragma("GCC diagnostic ignored \"-Weverything\"")

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
    #pragma clang diagnostic error   "-Wstrict-prototypes"
    #pragma clang diagnostic ignored "-Wunused-parameter"

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
    #define _GNU_SOURCE
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
#ifdef _WIN32
    #include <windows.h>
    #include <malloc.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <pthread.h>
#endif
BRAHMA_UNSUPPRESS_WARN

#endif

// =============================================================================================================================
// Main header file.
#ifndef BRAHMA_LIBRARY_H
#define BRAHMA_LIBRARY_H

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
 * Declare a paged list type.
 * A paged list provides a way to allocate a list of items without needing to reallocate the entire list when it grows.
 * It does this by allocating fixed-size pages of items, and linking them together using a linked-list.
 * The size and alignment of the pages are chosen to be 64 bytes, which is a common cache line size on modern CPUs.
 * This provides good cache performance when iterating, and it also allows for different threads to freely write to different
 * pages, without causing false sharing.
 */
#define BRAHMA_DECLARE_PAGED_LIST(convenientName, type) \
    typedef struct alignas(64) Brahma_Paged_List_Page__##convenientName Brahma_Paged_List_Page__##convenientName; \
    struct Brahma_Paged_List_Page__##convenientName \
    { \
        Brahma_Paged_List_Page__##convenientName* nextPage; \
        type                                 items[(64 - sizeof(void*)) / sizeof(type)]; \
    }; \
    static_assert(sizeof(Brahma_Paged_List_Page__##convenientName) == 64, \
        "Brahma_Paged_List_Page__" #convenientName " must be exactly 64 bytes in size."); \
    typedef struct \
    { \
        Brahma_Paged_List_Page__##convenientName* pages; \
        uint64_t                            count; \
    } Brahma_Paged_List__##convenientName; \
    void brahma_append_to_list__##convenientName(Brahma_Paged_List__##convenientName* list, type item) \
    { \
        /* if no pages, or the current page is full, allocate a new one */ \
        if (!list->pages || list->count % (sizeof(Brahma_Paged_List_Page__##convenientName) / sizeof(type)) == 0) \
        { \
            Brahma_Paged_List_Page__##convenientName* newPage = BRAHMA_PUSH_STRUCT(Brahma_Paged_List_Page__##convenientName); \
            newPage->nextPage = list->pages; \
            list->pages = newPage; \
        } \
        list->pages->items[list->count % (sizeof(Brahma_Paged_List_Page__##convenientName) / sizeof(type))] = item; \
        list->count++; \
    } \
    type* brahma_index_list__##convenientName(Brahma_Paged_List__##convenientName* list, uint64_t index) \
    { \
        if (index >= list->count) return NULL; \
        uint64_t pageIndex = index / (sizeof(Brahma_Paged_List_Page__##convenientName) / sizeof(type)); \
        uint64_t itemIndex = index % (sizeof(Brahma_Paged_List_Page__##convenientName) / sizeof(type)); \
        Brahma_Paged_List_Page__##convenientName* page = list->pages; \
        for (uint64_t i = 0; i < pageIndex; i++) \
        { \
            page = page->nextPage; \
        } \
        return &page->items[itemIndex]; \
    }

BRAHMA_DECLARE_PAGED_LIST(str, char*)

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
    void* _;
} Brahma_Package;

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
    void* _;
} Brahma_Library;

/**
 * Implement a package.
 *
 * Usage:
 ```
 BRAHMA_IMPLEMENT_PACKAGE(packageName)
 {
     Brahma_Package pkg;
     // set up package data...
     return pkg;
 }
 ```
 */
#define BRAHMA_IMPLEMENT_PACKAGE(packageName) \
    Brahma_Package brahma_implement_package_##packageName(void)

/**
 * Implement a library.
 *
 * Usage:
 ```
 BRAHMA_IMPLEMENT_LIBRARY(libraryName)
 {
     Brahma_Library lib;
     // set up library data...
     return lib;
 }
 ```
 */
#define BRAHMA_IMPLEMENT_LIBRARY(libraryName) \
    Brahma_Library brahma_implement_library_##libraryName(const Brahma_Package* package)

BRAHMA_IMPLEMENT_LIBRARY(Brahma)
{
    Brahma_Library lib;
    lib._ = NULL;
    return lib;
}

// =============================================================================================================================
// Execution code (used when CLI is executed).
#ifdef  BRAHMA_EXEC

typedef enum
{
    BRAHMA_INPUT_ARG_FLAGS_NONE      =      0,
    BRAHMA_INPUT_ARG_FLAGS_DEBUG     = 1 << 0,
    BRAHMA_INPUT_ARG_FLAGS_OPTIMISED = 1 << 1,
} Brahma_Input_Args_Flags_Bits;

typedef uint64_t Brahma_Input_Args_Flags;

typedef struct
{
    Brahma_Input_Args_Flags flags;
    char*                   packageToBuild;
} Brahma_Input_Args;

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

// structure storing all the package definitions
typedef struct
{
    char*          names      [BRAHMA_PACKAGE_COUNT];
    char*          owningFiles[BRAHMA_PACKAGE_COUNT];
    Brahma_Package info       [BRAHMA_PACKAGE_COUNT];
} Brahma_Packages;

// global variable that holds all the package definitions
Brahma_Packages g_BrahmaPkgDefs;

// structure storing all the library definitions
typedef struct
{
    char*          names      [BRAHMA_LIBRARY_COUNT];
    char*          owningFiles[BRAHMA_LIBRARY_COUNT];
    Brahma_Library info       [BRAHMA_LIBRARY_COUNT];
} Brahma_Libraries;

void brahma_initialise_internal_allocator(void);
void brahma_create_all_packages(void);

int main(int argc, char* argv[])
{
    // initialise the internal allocator
    brahma_initialise_internal_allocator();

    Brahma_Input_Args inputArgs = {0};
    inputArgs.flags |= BRAHMA_INPUT_ARG_FLAGS_DEBUG; // default to debug mode

    for (int i = 1; i < argc; i++) // skipping first arg because it's gonna be the executable name
    {
        // intermediate stuff - not relevant once this tool has begun executing
        if (!strcmp("-modules_search_dir", argv[i])) { i++; continue; } // module search dirs
        if (!strcmp("-build_tool_path",    argv[i])) { i++; continue; } // the path where the build tool was compiled

        // flags
        if (!strcmp("-nodebuginfo", argv[i])) { inputArgs.flags &= ~BRAHMA_INPUT_ARG_FLAGS_DEBUG;     continue; }
        if (!strcmp("-optimised",   argv[i])) { inputArgs.flags |=  BRAHMA_INPUT_ARG_FLAGS_OPTIMISED; continue; }

        if (!strcmp("-package", argv[i]))
        {
            if (inputArgs.packageToBuild)
            {
                printf("ERROR: Multiple packages specified with -package. Use as: *.exe -package packageName. Press any key to exit...");
                getchar();
                return 1;
            }

            if (++i >= argc)
            {
                printf("ERROR: No package specified after -package. Use as: *.exe -package packageName. Press any key to exit...");
                getchar();
                return 1;
            }

            inputArgs.packageToBuild = argv[i];
        }
    }

    brahma_create_all_packages();

    // find the package to build
    int selectedPkgIdx = -1;
    {
        if (!inputArgs.packageToBuild)
        {
            printf("No package specified. Defaulting to the first package!\n");
            selectedPkgIdx = 0;
        }
        else
        {
            for (int i = 0; i < BRAHMA_PACKAGE_COUNT; i++)
            {
                if (!strcmp(inputArgs.packageToBuild, g_BrahmaPkgDefs.names[i]))
                {
                    selectedPkgIdx = i;
                    break;
                }
            }

            if (selectedPkgIdx == -1)
            {
                printf("ERROR: No package found with the name '%s'. Press any key to exit...", inputArgs.packageToBuild);
                getchar();
                return 1;
            }
        }
    }

    printf("-----------------------------------------\n");
    printf("Brahma Configuration:\n");
    printf("\tSelected package: %s.\n", g_BrahmaPkgDefs.names[selectedPkgIdx]);
    printf("\tDebug info:       %s.\n", (inputArgs.flags & BRAHMA_INPUT_ARG_FLAGS_DEBUG)     ? "on" : "off");
    printf("\tOptimised:        %s.\n", (inputArgs.flags & BRAHMA_INPUT_ARG_FLAGS_OPTIMISED) ? "on" : "off");
    printf("-----------------------------------------\n");

    return 0;
}

#define BRAHMA_BEGIN_LISTING_PACKAGES() \
    void brahma_create_all_packages(void) {

#define BRAHMA_END_LISTING_PACKAGES() \
    }

#define BRAHMA_BEGIN_LISTING_LIBRARIES() \
    void brahma_create_all_libraries(const Brahma_Package* package) {

#define BRAHMA_END_LISTING_LIBRARIES() \
    }

#define BRAHMA_ADD_PACKAGE(idx, path, packageName) \
    brahma_add_package(idx, #packageName, path, brahma_implement_package_##packageName());

#define BRAHMA_ADD_LIBRARY(idx, path, libraryName) \
    brahma_add_library(idx, #libraryName, path, brahma_implement_library_##libraryName(package));

void brahma_add_package(int idx, char* name, char* owningFile, Brahma_Package package)
{
    g_BrahmaPkgDefs.names[idx]       = name;
    g_BrahmaPkgDefs.owningFiles[idx] = owningFile;
    g_BrahmaPkgDefs.info[idx]        = package;
}

void brahma_add_library(int idx, char* name, char* owningFile, Brahma_Library library)
{
    // TOOD
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

#endif//BRAHMA_EXEC

#ifdef __cplusplus
} // extern "C"
#endif

#endif//BRAHMA_LIBRARY_H
