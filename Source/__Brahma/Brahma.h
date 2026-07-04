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
    #pragma GCC diagnostic error   "-Wdouble-promotion"
    #pragma GCC diagnostic error   "-Wfloat-equal"
    #pragma GCC diagnostic error   "-Wundef"
    #pragma GCC diagnostic error   "-Wswitch-enum"
    #ifndef __cplusplus
        #pragma GCC diagnostic error   "-Wstrict-prototypes"
    #endif
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wunused-result"
    #pragma GCC diagnostic ignored "-Wuninitialized"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
    #pragma GCC diagnostic ignored "-Wmissing-braces"

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("GCC diagnostic push")  \
        _Pragma("GCC diagnostic ignored \"-Wall\"") \
        _Pragma("GCC diagnostic ignored \"-Wextra\"") \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \

    #define BRAHMA_UNSUPPRESS_WARN \
        _Pragma("GCC diagnostic pop")
#endif

#if defined(__clang__) && !defined(_MSC_VER) && !defined(__GNUC__)
    #pragma clang diagnostic error   "-Wall"
    #pragma clang diagnostic error   "-Wextra"
    #pragma clang diagnostic error   "-Wshadow"
    #pragma clang diagnostic error   "-Wconversion"
    #pragma clang diagnostic error   "-Wdouble-promotion"
    #pragma clang diagnostic error   "-Wfloat-equal"
    #pragma clang diagnostic error   "-Wundef"
    #pragma clang diagnostic error   "-Wswitch-enum"
    #ifndef __cplusplus
        #pragma clang diagnostic error   "-Wstrict-prototypes"
    #endif
    #pragma clang diagnostic ignored "-Wunused-parameter"
    #pragma clang diagnostic ignored "-Wunused-result"
    #pragma clang diagnostic ignored "-Wuninitialized"
    #pragma clang diagnostic ignored "-Wsign-conversion"
    #pragma clang diagnostic ignored "-Wmissing-braces"

    // could've come from msvc; like when using clangd
    #undef BRAHMA_SUPPRESS_WARN
    #undef BRAHMA_UNSUPPRESS_WARN

    #define BRAHMA_SUPPRESS_WARN \
        _Pragma("clang diagnostic push")  \
        _Pragma("clang diagnostic ignored \"-Weverything\"")

    #define BRAHMA_UNSUPPRESS_WARN \
        _Pragma("clang diagnostic pop")
#endif

#endif//BRAHMA_NO_WARNINGS

// =============================================================================================================================
// Includes / Links
#if 1

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #define NOMINMAX
    #define _CRT_SECURE_NO_WARNINGS
    #pragma comment(lib, "advapi32.lib") // for registry access
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
#include <stdio.h>
#ifdef _WIN32
    #include <windows.h>
    #include <malloc.h>
    #include <intrin.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <pthread.h>
    #include <stdlib.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <time.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/wait.h>
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
    static inline void brahma_reserve_##convNameFn##_array_list_capacity(Brahma_##convNameSt##_Array_List* list, size_t requiredCapacity) \
    { \
        if (requiredCapacity <= list->capacity) return; \
        /* the internal allocator allocates in blocks of at least 64 bytes, this ensures we don't waste space */ \
        size_t newCapacity = list->capacity ? list->capacity : 16; \
        while (newCapacity < requiredCapacity) newCapacity *= 2; \
        type* newData = BRAHMA_PUSH_STRUCT_ARRAY(type, newCapacity); \
        if (list->data) { memcpy(newData, list->data, sizeof(type) * list->count); } \
        list->data = newData; \
        list->capacity = newCapacity; \
    } \
    static inline void brahma_append_##convNameFn##_to_array_list(Brahma_##convNameSt##_Array_List* list, type item) \
    { \
        brahma_reserve_##convNameFn##_array_list_capacity(list, list->count + 1); \
        memcpy(&(list->data[list->count]), &item, sizeof(type)); \
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
    static inline type* brahma_index_##convNameFn##_paged_list(const Brahma_##convNameSt##_Paged_List* list, size_t index) \
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
        size_t pageIndex = list->count / itemsPerPage; \
        size_t itemIndex = list->count % itemsPerPage; \
        Brahma_##convNameSt##_Paged_List_Page* page = list->firstPage; \
        for (size_t i = 0; i < pageIndex; i++) { page = page->nextPage; } \
        memcpy(&(page->items[itemIndex]), &item, sizeof(type)); \
        list->count++; \
    }

BRAHMA_DECLARE_ARRAY_LIST(String, string, const char*)
BRAHMA_DECLARE_PAGED_LIST(String, string, const char*, 15)

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
 * Key-Value pairs of strings.
 * Used for a variety of things.
 */
typedef struct
{
    const char* key;
    const char* value;
} Brahma_String_KVP;

/**
 * This is used to store the definitions that a library (or a package) declares.
 * The key is the name of the definition, and the value is the value.
 * The value can include new lines, but will need to add backslashes for escaping, since the value is stored as a single string.
 */
BRAHMA_DECLARE_PAGED_LIST(Definition, definition, Brahma_String_KVP, 7)

/**
 * This is used to store the files that need to be copied from one location to another.
 * The key is the source path, and the value is the destination path.
 */
BRAHMA_DECLARE_PAGED_LIST(Files_To_Copy, files_to_copy, Brahma_String_KVP, 7)

/**
 * The platforms that can be targeted.
 */
enum Brahma_Platforms
{
    BRAHMA_PLATFORM_UNKNOWN,
    BRAHMA_PLATFORM_WINDOWS,
    BRAHMA_PLATFORM_LINUX,
    BRAHMA_PLATFORM_OSX,
    BRAHMA_PLATFORM_ANDROID,
    BRAHMA_PLATFORM_IOS,

    BRAHMA_PLATFORM__COUNT,
};

/**
 * The architectures that can be targeted.
 */
enum Brahma_Architectures
{
    BRAHMA_ARCHITECTURE_UNKNOWN,
    BRAHMA_ARCHITECTURE_X86,
    BRAHMA_ARCHITECTURE_X64,
    BRAHMA_ARCHITECTURE_ARM32,
    BRAHMA_ARCHITECTURE_ARM64,

    BRAHMA_ARCHITECTURE__COUNT,
};

/**
 * The string names of the platforms, for logging purposes.
 */
static const char* BRAHMA_PLATFORM_NAMES[BRAHMA_PLATFORM__COUNT] =
{
    "unknown",
    "windows",
    "linux",
    "osx",
    "android",
    "ios",
};

/**
 * The string names of the architectures, for logging purposes.
 */
static const char* BRAHMA_ARCHITECTURE_NAMES[BRAHMA_ARCHITECTURE__COUNT] =
{
    "unknown",
    "x86",
    "x64",
    "arm32",
    "arm64",
};

/**
 * The platforms that can be targeted; see also: Brahma_Platforms
 */
typedef uint8_t Brahma_Platform;

/**
 * The architectures that can be targeted; see also: Brahma_Architectures
 */
typedef uint8_t Brahma_Architecture;

typedef enum
{
    BRAHMA_PACKAGE_OUTPUT_TYPE_EXECUTABLE,
    BRAHMA_PACKAGE_OUTPUT_TYPE_DYNAMIC_LIBRARY,
    BRAHMA_PACKAGE_OUTPUT_TYPE_STATIC_LIBRARY,
} Brahma_Package_Output_Types;

typedef uint8_t Brahma_Package_Output_Type;

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
     * The name of the package. This should ideally be filled by some kind of codegen, and not manually.
     */
    const char* name;

    /**
     * The path to the file that declared this package. This should ideally be filled by some kind of codegen, and not manually.
     */
    const char* owningFile;

    /**
     * The platform that this package is targeting. This should ideally be filled by some kind of codegen, and not manually.
     */
    Brahma_Platform platform;

    /**
    * The architecture that this package is targeting. This should ideally be filled by some kind of codegen, and not manually.
    */
    Brahma_Architecture architecture;

    /**
     * Whether or not debug info is (to be) included for this package.
     */
    bool includeDebugInfo;

    /**
     * The type of the package.
     */
    Brahma_Package_Output_Type outputType;

    /**
     * Package-level definitions to use for compilation.
     */
    Brahma_Definition_Paged_List definitions;

    /**
     * The primary library that this package depends on. All the source files in this library (and its dependencies) will be compiled
     * to build the package.
     */
    const char* primaryLibrary;

    /**
     * If this package fails to be created, for whatever reason, this should be filled with the error message.
     * Otherwise, it should be null. This will fail the build process.
     */
    const char* error;
} Brahma_Package;

BRAHMA_DECLARE_ARRAY_LIST(Package, package, Brahma_Package)

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
     * The name of the library. This should ideally be filled by some kind of codegen, and not manually.
     */
    const char* name;

    /**
     * The path to the file that declared this package. This should ideally be filled by some kind of codegen, and not manually.
     */
    const char* owningFile;

    /**
     * The owning directory of this library. All the items inside this directory will be considered as belonging to this library.
     */
    const char* owningDir;

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
    Brahma_Definition_Paged_List interfaceDefinitions;

    /**
     * Library-level definitions to use for compilation. These definitions will be used when compiling the source files in this
     * library, but not the source files in the libraries that depend on this library.
     */
    Brahma_Definition_Paged_List internalDefinitions;

    /**
     * List of file paths (relative to library's owning directory) that belong to this library.
     * These files will be copied next the output of the package (with the same relative path) when building the package.
     *
     * This can be useful for including prebuilt dynamic libraries or other executables that the library needs to function.
     */
    Brahma_Files_To_Copy_Paged_List filesToCopyNextToOutput;

    /**
     * The static libraries that this library depends on. These will be linked against when building the package that
     * depends on this library.
     *
     * If the library is an apple framework, then prefix the library name with "framework:", e.g. "framework:Metal".
     */
    Brahma_String_Paged_List externalDependencies;

    /**
     * If this library fails to be created, for whatever reason, this should be filled with the error message.
     * Otherwise, it should be null. This will fail the build process.
     */
    const char* error;
} Brahma_Library;

BRAHMA_DECLARE_ARRAY_LIST(Library, library, Brahma_Library)

/**
 * Implement a package.
 *
 * Usage:
```
BRAHMA_IMPLEMENT_PACKAGE(packageName)
{
    package->someVariable = "my_package";
}
```
 */
#define BRAHMA_IMPLEMENT_PACKAGE(packageName) \
    void brahma_implement_package_##packageName(Brahma_Package* package)

/**
 * Implement a library.
 *
 * Usage:
```
BRAHMA_IMPLEMENT_LIBRARY(libraryName)
{
    library->someVariable = "my_library";
}
```
 */
#define BRAHMA_IMPLEMENT_LIBRARY(libraryName) \
    void brahma_implement_library_##libraryName(const Brahma_Package* package, Brahma_Library* library)

// =============================================================================================================================
// Library interface
#if defined(BRAHMA_LIBRARY) || defined(BRAHMA_LIBRARY_IMPL) || defined(BRAHMA_EXEC)

// delegate signature for creating packages
typedef void (*Brahma_Package_Creator_Delegate)(Brahma_Package_Array_List* packages);

// delegate signature for creating libraries
typedef void (*Brahma_Library_Creator_Delegate)(Brahma_Library_Array_List* libraries, const Brahma_Package* package);

// delegate signature for logging to console
typedef void (*Brahma_Log_Delegate)(const char* fmt, ...);

// flags for input args
typedef enum
{
    BRAHMA_ARGS_FLAG_NONE                =       0,
    BRAHMA_ARGS_FLAG_DEBUG               = 1 <<  0,
    BRAHMA_ARGS_FLAG_OPTIMISED           = 1 <<  1,
    BRAHMA_ARGS_FLAG_SHOW_WARNINGS       = 1 <<  2,
    BRAHMA_ARGS_FLAG_WARNINGS_ARE_ERRORS = 1 <<  3,
    BRAHMA_ARGS_FLAG_UNUSED_04           = 1 <<  4,
    BRAHMA_ARGS_FLAG_UNUSED_05           = 1 <<  5,
    BRAHMA_ARGS_FLAG_UNUSED_06           = 1 <<  6,
    BRAHMA_ARGS_FLAG_UNUSED_07           = 1 <<  7,
    BRAHMA_ARGS_FLAG_UNUSED_08           = 1 <<  8,
    BRAHMA_ARGS_FLAG_UNUSED_09           = 1 <<  9,
    BRAHMA_ARGS_FLAG_UNUSED_10           = 1 << 10,
    BRAHMA_ARGS_FLAG_UNUSED_11           = 1 << 11,
    BRAHMA_ARGS_FLAG_UNUSED_12           = 1 << 12,
    BRAHMA_ARGS_FLAG_UNUSED_13           = 1 << 13,
    BRAHMA_ARGS_FLAG_UNUSED_14           = 1 << 14,
    BRAHMA_ARGS_FLAG_UNUSED_15           = 1 << 15,
    // that's all folks!
} Brahma_Args_Flags_Bits;

// flags for input args; see also: Brahma_Args_Flags_Bits
typedef uint16_t Brahma_Args_Flags;

// arguments required for executing brahma
typedef struct
{
    Brahma_Args_Flags   flags;
    Brahma_Platform     platform;
    Brahma_Architecture architecture;
    char*               packageToBuild;

    size_t pkgCount, libCount;

    void (*log)(const char* fmt, ...);

    void (*createPackages)(Brahma_Platform platform, Brahma_Architecture architecture, bool includeDebugInfo, Brahma_Package_Array_List* packages);
    void (*createLibraries)(Brahma_Library_Array_List* libraries, const Brahma_Package* package);

    char* intermediateOutputDir;
    char* outputDir;

    char* clangdOutputDir;
} Brahma_Args;

// main entry point function
bool brahma_execute(Brahma_Args ex);

#endif//defined(BRAHMA_LIBRARY) || defined(BRAHMA_LIBRARY_IMPL) || defined(BRAHMA_EXEC)

// =============================================================================================================================
// Library implementation
#if defined(BRAHMA_LIBRARY_IMPL) || defined(BRAHMA_EXEC)

#define BRAHMA_LOG_PROFILE "\033[35m[PRF]\033[0m " // magenta
#define BRAHMA_LOG_INFO    "\033[34m[INF]\033[0m " // blue
#define BRAHMA_LOG_WARNING "\033[33m[WRN]\033[0m " // yellow
#define BRAHMA_LOG_ERROR   "\033[31m[ERR]\033[0m " // red
#define BRAHMA_LOG_SUCCESS "\033[32m[SCS]\033[0m " // green

// get current time in nanoseconds since unix epoch
int64_t brahma_get_time(void);

// initialise the internal allocator
void brahma_initialise_internal_allocator(void);

typedef struct
{
    size_t totalAllocated;
    size_t totalUsed;
    size_t totalUnused;
    size_t totalWasted; // because of fragmentation, page sizing, alignment, etc.
} Brahma_Memory_Usage_Report;

// shutdown the internal allocator
Brahma_Memory_Usage_Report brahma_shutdown_internal_allocator(void);

// get an env value, NULL if not found
char* brahma_get_env_var(const char* name);

// check if a directory exists
bool brahma_dir_exists(const char* path);

// ensure that a directory exists, and create it if it doesn't; warning - it'll write to the string buffer, so make sure to provide a mutable string
bool brahma_ensure_dir(char* path);

// copy a file from srcPath to dstPath; returns true if successful, false otherwise
bool brahma_copy_file(const char* srcPath, const char* dstPath);

// visitor function to use for iterating a directory
typedef bool (*Brahma_Directory_Visitor_Delegate)(void* payload, const char* path, bool isDirectory, bool* exploreCurrentDirectory);

// iterate a directory, calling the visitor function for each child file and subdirectory
void brahma_iterate_directory(const char* path, bool recursive, void* visitorPayload, Brahma_Directory_Visitor_Delegate visitor);

// payload to use for brahma_gather_files_by_extension_visitor
typedef struct
{
    const char*               extension; // extension to match, including the dot
    Brahma_String_Paged_List* outFilePaths; // output list to append the matched file paths to
} Brahma_Gather_Files_By_Extension_Payload;

// visitor func to use for gathering files with a specific extension; payload must be of type Brahma_Gather_Files_By_Extension_Payload
bool brahma_gather_files_by_extension_visitor(void* payload, const char* path, bool isDirectory, bool* exploreCurrentDirectory);

// find a package index by its name; -1 if not found
int brahma_find_package_by_name(const Brahma_Package_Array_List* packages, const char* name);

// find a library index by its name; -1 if not found
int brahma_find_library_by_name(const Brahma_Library_Array_List* libraries, const char* name);

// represents a running process
typedef struct Brahma_Process_Impl* Brahma_Process;
BRAHMA_DECLARE_ARRAY_LIST(Process, process, Brahma_Process)

// start a process and return its handle
Brahma_Process brahma_start_process(Brahma_String_Array_List args, const char* workingDir);

// wait for a process to finish and return its exit code; optionally, also output the process's stdout (with a null-terminator at the end)
int brahma_wait_for_process(Brahma_Process process, char** outStdOut);

BRAHMA_DECLARE_PAGED_LIST(Library_Idx, library_idx, uint16_t, ((64 - sizeof(void*)) / sizeof(uint16_t)))
BRAHMA_DECLARE_ARRAY_LIST(Library_Idx, library_idx, uint16_t)

// a chunk of library dependencies, which represents a contiguous range of dependencies in the list of all dependencies of a library
typedef struct { uint16_t start, count; } Brahma_Data_Chunk;
BRAHMA_DECLARE_ARRAY_LIST(Data_Chunk, data_chunk, Brahma_Data_Chunk)

// append all recursive library dependencies to all the indices (view into this via data chunks)
bool brahma_append_all_library_deps(
    const Brahma_Library_Array_List* allLibs,
    const Brahma_Library* library,
    Brahma_Library_Idx_Paged_List* allLibDeps,
    size_t firstLibDepIdx,
    bool ignoreInterfaceDeps,
    bool ignoreInternalDeps,
    bool ignoreInternalDepsOfInternalDeps,
    char** error,
    void* cycleChecker);

bool brahma_execute(Brahma_Args ex)
{
    if (!ex.log || !ex.createPackages || !ex.createLibraries || !ex.outputDir || !ex.outputDir[0])
    {
        ex.log(BRAHMA_LOG_ERROR "Invalid arguments passed to brahma_execute.\n");
        return false;
    }

    if (!ex.platform)
    {
        #if defined(_WIN32)
            ex.platform = BRAHMA_PLATFORM_WINDOWS;
        #elif defined(__linux__)
            ex.platform = BRAHMA_PLATFORM_LINUX;
        #elif defined(__APPLE__)
            ex.platform = BRAHMA_PLATFORM_OSX;
        #else
            #error "Unsupported platform."
        #endif

        ex.log(BRAHMA_LOG_WARNING "No target platform specified. Defaulting to the host platform: '%s'.\n",
            BRAHMA_PLATFORM_NAMES[ex.platform]);
    }

    if (!ex.architecture)
    {
        #if defined(_M_IX86) || defined(__i386__)
            ex.architecture = BRAHMA_ARCHITECTURE_X86;
        #elif defined(_M_X64) || defined(__x86_64__)
            ex.architecture = BRAHMA_ARCHITECTURE_X64;
        #elif defined(_M_ARM) || defined(__arm__)
            ex.architecture = BRAHMA_ARCHITECTURE_ARM32;
        #elif defined(_M_ARM64) || defined(__aarch64__)
            ex.architecture = BRAHMA_ARCHITECTURE_ARM64;
        #else
            #error "Unsupported architecture."
        #endif

        if (ex.platform == BRAHMA_PLATFORM_ANDROID || ex.platform == BRAHMA_PLATFORM_IOS)
        {
            ex.architecture = BRAHMA_ARCHITECTURE_ARM64; // better default
        }

        ex.log(BRAHMA_LOG_WARNING "No target architecture specified. Defaulting to the host architecture: '%s'.\n",
            BRAHMA_ARCHITECTURE_NAMES[ex.architecture]);
    }

    if (false
        || (ex.architecture == BRAHMA_ARCHITECTURE_ARM32 && ex.platform != BRAHMA_PLATFORM_ANDROID) // only android supports arm32
        || (ex.architecture == BRAHMA_ARCHITECTURE_X86 && ex.platform == BRAHMA_PLATFORM_OSX) // no 32-bit support on osx
        || (ex.architecture == BRAHMA_ARCHITECTURE_X86 && ex.platform == BRAHMA_PLATFORM_IOS) // no 32-bit support on ios
        || false)
    {
        ex.log(BRAHMA_LOG_ERROR "The architecture '%s' is not supported on the platform '%s'.\n",
            BRAHMA_ARCHITECTURE_NAMES[ex.architecture], BRAHMA_PLATFORM_NAMES[ex.platform]);
        return false;
    }

    int64_t startTime = brahma_get_time(), time = 0, lastTime = startTime;

    #define PROFILE_SECTION_END(sectionName) \
        do \
        { \
            time = brahma_get_time(); \
            ex.log(BRAHMA_LOG_PROFILE "'" sectionName "': DONE (%.2f ms).\n", (double) (time - lastTime) / 1000000.0); \
            lastTime = time; \
        } while (false)
    bool failed = false;

    brahma_initialise_internal_allocator();

    PROFILE_SECTION_END("initialise");

    char* toolchainPath = NULL;
    char* cCompilerPath = NULL;
    char* cxxCompilerPath = NULL;
    char* staticLinkerPath = NULL;
    char* windowsKitPath = NULL;
    char* windowsKitVersion = NULL;
    if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
    {
        char* programFilesX86 = NULL;
        if (!failed)
        {
            programFilesX86 = brahma_get_env_var(sizeof(void*) == 8 ? "ProgramFiles(x86)" : "ProgramFiles");
            if (!programFilesX86)
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to get 'Program Files' directory from environment variables.\n");
                failed = true;
            }
        }

        // using this file to search, because vswhere can only "-find" files, and not directories
        // but what we actually want is the full directory (up till msvc version), so we get stuff till
        // this temp search file and then trim the path back to get the compiler directory
        #define BRAHMA_TEMP_SEARCH_FILE "\\modules\\modules.json"

        Brahma_Process vswhereProcess = NULL;
        if (!failed)
        {
            // use vswhere to find latest visual studio
            Brahma_String_Array_List vswhereArgs = { NULL, 0, 0 };
            brahma_append_string_to_array_list(&vswhereArgs, brahma_sprintf("%s\\Microsoft Visual Studio\\Installer\\vswhere.exe", programFilesX86));
            brahma_append_string_to_array_list(&vswhereArgs, "-latest");
            brahma_append_string_to_array_list(&vswhereArgs, "-products");
            brahma_append_string_to_array_list(&vswhereArgs, "*");
            brahma_append_string_to_array_list(&vswhereArgs, "-requires");
            brahma_append_string_to_array_list(&vswhereArgs, brahma_sprintf("Microsoft.VisualStudio.Component.VC.Tools.%s",
                (ex.architecture == BRAHMA_ARCHITECTURE_ARM64 ? "ARM64" : "x86.x64")));
            brahma_append_string_to_array_list(&vswhereArgs, "-find");

            char* clFindPath = brahma_sprintf("VC\\Tools\\MSVC\\**" BRAHMA_TEMP_SEARCH_FILE);

            brahma_append_string_to_array_list(&vswhereArgs, clFindPath);

            vswhereProcess = brahma_start_process(vswhereArgs, NULL);
            if (!vswhereProcess)
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to start vswhere process!.\n");
                failed = true;
            }
        }

        char* vswhereStdOut = NULL;
        if (!failed)
        {
            if (0 != brahma_wait_for_process(vswhereProcess, &vswhereStdOut) || !vswhereStdOut || !vswhereStdOut[0])
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to execute vswhere to find Visual Studio installation path. Make sure you have Visual Studio with C++ workload installed.\n");
                if (vswhereStdOut && vswhereStdOut[0])
                {
                    ex.log(BRAHMA_LOG_ERROR "vswhere output: %s\n", vswhereStdOut);
                }

                failed = true;
            }
        }

        if (!failed)
        {
            // remove trailing newlines from vswhere output
            size_t len = strlen(vswhereStdOut);
            while (len > 0 && (vswhereStdOut[len - 1] == '\n' || vswhereStdOut[len - 1] == '\r'))
            {
                vswhereStdOut[--len] = '\0';
            }

            // remove the temp search file to get the toolchain directory
            {
                size_t vswhereOutputLen = strlen(vswhereStdOut);
                size_t searchFileLen = sizeof(BRAHMA_TEMP_SEARCH_FILE) - 1;
                if (vswhereOutputLen <= searchFileLen ||
                    strcmp(vswhereStdOut + vswhereOutputLen - searchFileLen, BRAHMA_TEMP_SEARCH_FILE) != 0)
                {
                    ex.log(BRAHMA_LOG_ERROR "Unexpected vswhere output format. Expected path to end with '" BRAHMA_TEMP_SEARCH_FILE "'. Actual output: %s\n", vswhereStdOut);
                    failed = true;
                }
                else
                {
                    vswhereStdOut[vswhereOutputLen - searchFileLen] = '\0'; // trim the search file from the end
                    toolchainPath = vswhereStdOut;
                }
            }
        }

        if (!failed)
        {
            char* vsBinaries = brahma_sprintf("%s\\bin\\"
            #if defined(_M_ARM64) || defined(__aarch64__)
                "Hostarm64"
            #elif defined(_M_X64) || defined(__x86_64__)
                "Hostx64"
            #elif defined(_M_IX86) || defined(__i386__)
                "Hostx86"
            #else
                #error "Unsupported architecture."
            #endif
            "\\%s",
            toolchainPath,
            (false ? ""
                : ex.architecture == BRAHMA_ARCHITECTURE_ARM64 ? "arm64"
                : ex.architecture == BRAHMA_ARCHITECTURE_X64 ? "x64"
                : ex.architecture == BRAHMA_ARCHITECTURE_X86 ? "x86"
                : "unknown"));

            cCompilerPath = brahma_sprintf("%s\\cl.exe", vsBinaries);
            cxxCompilerPath = cCompilerPath;
            staticLinkerPath = brahma_sprintf("%s\\lib.exe", vsBinaries);
        }

        #ifdef _WIN32
        {
            HKEY hKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
                            0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                char* installDir = (char*) brahma_push_memory(MAX_PATH, 1);
                DWORD installDirSize = MAX_PATH;

                if (RegQueryValueExA(hKey, "KitsRoot10", NULL, NULL,
                                    (LPBYTE) installDir, &installDirSize) == ERROR_SUCCESS)
                {
                    char bestVer[64]; memset(bestVer, 0, sizeof(bestVer));

                    for (DWORD i = 0; ; i++)
                    {
                        char subkeyName[64] = {0};
                        DWORD subkeyNameSize = sizeof(subkeyName);
                        if (RegEnumKeyExA(hKey, i, subkeyName, &subkeyNameSize,
                                        NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                            break;

                        // Only consider version-like subkeys (start with "10.")
                        if (strncmp(subkeyName, "10.", 3) != 0)
                            continue;

                        // no existing version or newer version found
                        if (bestVer[0] == '\0' || strcmp(subkeyName, bestVer) > 0)
                            strncpy(bestVer, subkeyName, sizeof(bestVer) - 1);
                    }

                    if (bestVer[0] != '\0')
                    {
                        windowsKitVersion = brahma_sprintf("%s", bestVer);
                        windowsKitPath = installDir;

                        // change slashes
                        if (windowsKitPath)
                            for (char* i = windowsKitPath; *i; i++)
                                if (*i == '\\')
                                    *i = '/';

                        if (windowsKitPath && windowsKitPath[0] && windowsKitPath[strlen(windowsKitPath) - 1] == '/')
                            windowsKitPath[strlen(windowsKitPath) - 1] = '\0'; // remove trailing slash
                    }
                }
                RegCloseKey(hKey);
            }
        }
        #endif

        #undef BRAHMA_TEMP_SEARCH_FILE

        if (!windowsKitPath || !windowsKitVersion)
        {
            ex.log(BRAHMA_LOG_WARNING "Failed to find Windows SDK installation path. Dependencies to it will not be resolved.");
        }
    }
    else if (ex.platform == BRAHMA_PLATFORM_LINUX)
    {
        #if defined(__linux__)
        {
            toolchainPath = "/usr/bin";
            cCompilerPath = "gcc";
            cxxCompilerPath = "g++";
            staticLinkerPath = "ld";
        }
        #else
        {
            // TODO - cross compilation
        }
        #endif
    }
    else if (ex.platform == BRAHMA_PLATFORM_OSX)
    {
        #if defined(__APPLE__)
        {
            toolchainPath    = "/usr/bin";
            cCompilerPath    = "clang";
            cxxCompilerPath  = "clang++";
            staticLinkerPath = "ar";
        }
        #else
        {
            // TODO - cross compilation
        }
        #endif
    }

    PROFILE_SECTION_END("get toolchains");

    // clean up output dir - forward slashes only, no trailing slash
    {
        size_t outputDirLen = strlen(ex.outputDir);
        bool hasTrailingSlash = (outputDirLen > 0) && (ex.outputDir[outputDirLen - 1] == '/' || ex.outputDir[outputDirLen - 1] == '\\');
        if (hasTrailingSlash) { outputDirLen--; }

        char* outputDirCleanPath = (char*) brahma_push_memory(outputDirLen + 1, 1);
        for (size_t i = 0; i < outputDirLen; i++)
        {
            char c = ex.outputDir[i];
            if (c == '\\') { c = '/'; }
            outputDirCleanPath[i] = c;
        }

        outputDirCleanPath[outputDirLen] = '\0';
        ex.outputDir = outputDirCleanPath;
    }

    if (!ex.intermediateOutputDir || !ex.intermediateOutputDir[0])
    {
        ex.log(BRAHMA_LOG_WARNING "No intermediate output directory specified. Defaulting \"%s/Temp/\".\n", ex.outputDir);
        ex.intermediateOutputDir = brahma_sprintf("%s/Temp", ex.outputDir);
    }

    // clean up intermediate output dir - forward slashes only, no trailing slash
    {
        size_t intermediateOutputDirLen = strlen(ex.intermediateOutputDir);
        bool hasTrailingSlash = (intermediateOutputDirLen > 0) &&
            (ex.intermediateOutputDir[intermediateOutputDirLen - 1] == '/' ||
                ex.intermediateOutputDir[intermediateOutputDirLen - 1] == '\\');

        if (hasTrailingSlash) { intermediateOutputDirLen--; }

        char* intermediateOutputDirCleanPath = (char*) brahma_push_memory(intermediateOutputDirLen + 1, 1);
        for (size_t i = 0; i < intermediateOutputDirLen; i++)
        {
            char c = ex.intermediateOutputDir[i];
            if (c == '\\') { c = '/'; }
            intermediateOutputDirCleanPath[i] = c;
        }

        intermediateOutputDirCleanPath[intermediateOutputDirLen] = '\0';
        ex.intermediateOutputDir = intermediateOutputDirCleanPath;
    }

    PROFILE_SECTION_END("input cleanup");

    // find the package to build
    Brahma_Package* selectedPkg = NULL;
    if (!failed)
    {
        Brahma_Package_Array_List pkgDefs = { NULL, 0, 0 };
        brahma_reserve_package_array_list_capacity(&pkgDefs, ex.pkgCount);
        ex.createPackages(ex.platform, ex.architecture, !!(ex.flags && BRAHMA_ARGS_FLAG_DEBUG), &pkgDefs);

        if (pkgDefs.count == 0)
        {
            ex.log(BRAHMA_LOG_ERROR "No packages were created.\n");
            failed = true;
        }
        else if (!ex.packageToBuild)
        {
            ex.log(BRAHMA_LOG_WARNING "No package specified. Defaulting to the first package: '%s'!\n", pkgDefs.data[0].name);
            selectedPkg = &pkgDefs.data[0];
        }
        else
        {
            int selectedPkgIdx = brahma_find_package_by_name(&pkgDefs, ex.packageToBuild);
            if (selectedPkgIdx >= 0) { selectedPkg = &pkgDefs.data[selectedPkgIdx]; }

            if (!selectedPkg)
            {
                ex.log(BRAHMA_LOG_ERROR "No package found with the name '%s'.\n", ex.packageToBuild);
                failed = true;
            }
        }

        if (selectedPkg && selectedPkg->error && selectedPkg->error[0])
        {
            ex.log(BRAHMA_LOG_ERROR "The package '%s' has an error: %s\n", selectedPkg->name, selectedPkg->error);
            failed = true;
        }

        PROFILE_SECTION_END("create packages");
    }

    // make output dirs
    if (!failed)
    {
        ex.outputDir = brahma_sprintf("%s/%s-%s-%s",
            ex.outputDir,
            BRAHMA_PLATFORM_NAMES[selectedPkg->platform], BRAHMA_ARCHITECTURE_NAMES[selectedPkg->architecture],
            ((ex.flags & BRAHMA_ARGS_FLAG_DEBUG) ? "dbg" : "rel")
        );

        ex.intermediateOutputDir = brahma_sprintf("%s/%s-%s-%s-%s",
            ex.intermediateOutputDir, selectedPkg->name,
            BRAHMA_PLATFORM_NAMES[selectedPkg->platform], BRAHMA_ARCHITECTURE_NAMES[selectedPkg->architecture],
            ((ex.flags & BRAHMA_ARGS_FLAG_DEBUG) ? "dbg" : "rel")
        );

        if (!brahma_ensure_dir(brahma_sprintf("%s", ex.outputDir)))
        {
            ex.log(BRAHMA_LOG_ERROR "Failed to create output directory at '%s'.\n", ex.outputDir);
            failed = true;
        }

        if (!brahma_ensure_dir(brahma_sprintf("%s", ex.intermediateOutputDir)))
        {
            ex.log(BRAHMA_LOG_ERROR "Failed to create intermediate output directory at '%s'.\n", ex.intermediateOutputDir);
            failed = true;
        }

        PROFILE_SECTION_END("ensure output dirs");
    }

    // find libraries to build
    Brahma_Library_Array_List libDefs = { NULL, 0, 0 };
    if (!failed)
    {
        size_t primaryLibIdx = SIZE_MAX;

        brahma_reserve_library_array_list_capacity(&libDefs, ex.libCount);
        ex.createLibraries(&libDefs, selectedPkg);

        if (!selectedPkg->primaryLibrary || !selectedPkg->primaryLibrary[0])
        {
            ex.log(BRAHMA_LOG_ERROR "The selected package '%s' does not have a primary library specified.\n", selectedPkg->name);
            failed = true;
        }
        else
        {
            primaryLibIdx = (size_t) brahma_find_library_by_name(&libDefs, selectedPkg->primaryLibrary);

            if (primaryLibIdx >= libDefs.count)
            {
                ex.log(BRAHMA_LOG_ERROR "No library found with the name '%s', which is the primary library of the package '%s'.\n", selectedPkg->primaryLibrary, selectedPkg->name);
                failed = true;
            }
        }

        if (!failed)
        {
            Brahma_Library_Array_List libsToProcess = { NULL, 0, 0 };
            Brahma_Library* primaryLib = &(libDefs.data[primaryLibIdx]);

            Brahma_Library_Idx_Paged_List libIdxs; memset(&libIdxs, 0, sizeof(libIdxs));
            char* error = NULL;
            if (!brahma_append_all_library_deps(&libDefs, primaryLib, &libIdxs, 0, false, false, false, &error, NULL))
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to resolve dependencies for primary library '%s'.\n\tDetails: %s.\n", primaryLib->name, error ? error : "<unknown>");
                failed = true;
            }

            // add self
            if (!failed)
            {
                bool alreadyExists = false;
                for (size_t i = 0; i < libIdxs.count; i++)
                {
                    uint16_t existingIdx = *brahma_index_library_idx_paged_list(&libIdxs, i);
                    if (existingIdx == primaryLibIdx)
                    {
                        alreadyExists = true;
                        break;
                    }
                }

                if (!alreadyExists)
                {
                    brahma_append_library_idx_to_paged_list(&libIdxs, (uint16_t) primaryLibIdx);
                }
                else
                {
                    ex.log(BRAHMA_LOG_ERROR "Circular dependency detected for primary library '%s'.\n", primaryLib->name);
                    failed = true;
                }
            }

            if (!failed)
            {
                brahma_reserve_library_array_list_capacity(&libsToProcess, libIdxs.count);
                for (size_t i = 0; i < libIdxs.count; i++)
                {
                    uint16_t libIdx = *brahma_index_library_idx_paged_list(&libIdxs, i);
                    brahma_append_library_to_array_list(&libsToProcess, libDefs.data[libIdx]);
                }
            }

            if (!failed)
            {
                /*
                * by this point, the libraries to process are in libsToProcess, and so we shouldn't be using libDefs at all
                * this should minimise the number of libraries that we actually process
                *
                * conveniently, the files are also in a linear order where the dependencies of a library will appear before the library itself
                *
                * so we'll just do a quick switcharoooo
                */
                libDefs = libsToProcess;
                primaryLibIdx = (libsToProcess.count - 1);
            }
        }

        if (!failed)
        {
            for (size_t i = 0; i < libDefs.count; i++)
            {
                Brahma_Library* lib = &(libDefs.data[i]);
                if (lib->error && lib->error[0])
                {
                    ex.log(BRAHMA_LOG_ERROR "The library '%s' has an error: %s\n", lib->name, lib->error);
                    failed = true;
                }
            }
        }

        PROFILE_SECTION_END("create (& sort) libraries");
    }

    // print input summary
    if (!failed)
    {
        ex.log(BRAHMA_LOG_INFO "-----------------------------------------\n");
        ex.log(BRAHMA_LOG_INFO "Brahma Configuration:\n");
        ex.log(BRAHMA_LOG_INFO "\tToolchain:        %s.\n", toolchainPath);
        ex.log(BRAHMA_LOG_INFO "\tC compiler:       %s.\n", cCompilerPath);
        ex.log(BRAHMA_LOG_INFO "\tC++ compiler:     %s.\n", cxxCompilerPath);
        ex.log(BRAHMA_LOG_INFO "\tStatic linker:    %s.\n", staticLinkerPath);
        ex.log(BRAHMA_LOG_INFO "\tSelected package: %s.\n", selectedPkg->name);
        ex.log(BRAHMA_LOG_INFO "\tPlatform:         %s.\n", BRAHMA_PLATFORM_NAMES[selectedPkg->platform]);
        ex.log(BRAHMA_LOG_INFO "\tArch:             %s.\n", BRAHMA_ARCHITECTURE_NAMES[selectedPkg->architecture]);
        ex.log(BRAHMA_LOG_INFO "\tDebug info:       %s.\n", (ex.flags & BRAHMA_ARGS_FLAG_DEBUG)     ? "on" : "off");
        ex.log(BRAHMA_LOG_INFO "\tOptimised:        %s.\n", (ex.flags & BRAHMA_ARGS_FLAG_OPTIMISED) ? "on" : "off");
        ex.log(BRAHMA_LOG_INFO "\tOutput dir:       %s.\n", ex.outputDir);
        ex.log(BRAHMA_LOG_INFO "\tIntermediate dir: %s.\n", ex.intermediateOutputDir);

        if (windowsKitPath)
            ex.log(BRAHMA_LOG_INFO "\tWindows SDK:      %s.\n", windowsKitPath);

        if (windowsKitVersion)
            ex.log(BRAHMA_LOG_INFO "\tWindows SDK ver:  %s.\n", windowsKitVersion);

        ex.log(BRAHMA_LOG_INFO "-----------------------------------------\n");
        ex.log(BRAHMA_LOG_INFO "Libraries to build (in order):\n");
        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);
            ex.log(BRAHMA_LOG_INFO "\t- %s (owning dir: %s)\n", lib->name, lib->owningDir);
        }
        ex.log(BRAHMA_LOG_INFO "-----------------------------------------\n");

        PROFILE_SECTION_END("input summary");
    }

    // gather all interface paths (if they exist)
    Brahma_String_Array_List allInterfacePaths = { NULL, 0, 0 };
    if (!failed)
    {
        brahma_reserve_string_array_list_capacity(&allInterfacePaths, libDefs.count);
        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);
            char* interfaceDir = brahma_sprintf("%s/Interface", lib->owningDir);
            if (brahma_dir_exists(interfaceDir))
            {
                brahma_append_string_to_array_list(&allInterfacePaths, interfaceDir);
            }
            else
            {
                brahma_append_string_to_array_list(&allInterfacePaths, NULL);
            }
        }

        PROFILE_SECTION_END("gather interface dirs");
    }

    // gather all interface deps
    Brahma_Library_Idx_Paged_List allLibInterfaceDeps = { NULL, 0, 0 };
    Brahma_Data_Chunk_Array_List libInterfaceDepChunks = { NULL, 0, 0 };
    if (!failed)
    {
        brahma_reserve_data_chunk_array_list_capacity(&libInterfaceDepChunks, libDefs.count);

        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);

            Brahma_Data_Chunk chunk;
            size_t startCount =  allLibInterfaceDeps.count;

            char* error = NULL;
            if (!brahma_append_all_library_deps(&libDefs, lib, &allLibInterfaceDeps, startCount, false, true, true, &error, NULL))
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to resolve dependencies for library '%s'.\n\tDetails: %s.\n", lib->name, error ? error : "<unknown>");
                failed = true;
            }

            chunk.start = (uint16_t) startCount;
            chunk.count = (uint16_t) (allLibInterfaceDeps.count - startCount);
            brahma_append_data_chunk_to_array_list(&libInterfaceDepChunks, chunk);
        }

        PROFILE_SECTION_END("gather interface dependencies");
    }

    // gather all interface+internal deps
    Brahma_Library_Idx_Paged_List allLibAllDeps = { NULL, 0, 0 };
    Brahma_Data_Chunk_Array_List libAllDepChunks = { NULL, 0, 0 };
    if (!failed)
    {
        brahma_reserve_data_chunk_array_list_capacity(&libAllDepChunks, libDefs.count);

        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);

            Brahma_Data_Chunk chunk;
            size_t startCount =  allLibAllDeps.count;

            char* error = NULL;
            if (!brahma_append_all_library_deps(&libDefs, lib, &allLibAllDeps, startCount, false, false, true, &error, NULL))
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to resolve dependencies for library '%s'.\n\tDetails: %s.\n", lib->name, error ? error : "<unknown>");
                failed = true;
            }

            chunk.start = (uint16_t) startCount;
            chunk.count = (uint16_t) (allLibAllDeps.count - startCount);
            brahma_append_data_chunk_to_array_list(&libAllDepChunks, chunk);
        }

        PROFILE_SECTION_END("gather interface+internal dependencies");
    }

    // files gather
    Brahma_Data_Chunk_Array_List internalCFileChunks = { NULL, 0, 0 };
    Brahma_String_Paged_List internalCFilePaths; memset(&internalCFilePaths, 0, sizeof(internalCFilePaths));
    Brahma_Data_Chunk_Array_List internalCxxFileChunks = { NULL, 0, 0 };
    Brahma_String_Paged_List internalCxxFilePaths; memset(&internalCxxFilePaths, 0, sizeof(internalCxxFilePaths));
    if (!failed)
    {
        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);

            struct
            {
                char* searchPath;
                const char* extension;
                Brahma_Data_Chunk_Array_List* fileChunks;
                Brahma_String_Paged_List* filePaths;
            } toProcess[2];

            toProcess[0].searchPath = brahma_sprintf("%s/Internal", lib->owningDir);
            toProcess[0].extension = ".c";
            toProcess[0].fileChunks = &internalCFileChunks;
            toProcess[0].filePaths = &internalCFilePaths;

            toProcess[1].searchPath = toProcess[0].searchPath; // same search path as internal C files
            toProcess[1].extension = ".cpp";
            toProcess[1].fileChunks = &internalCxxFileChunks;
            toProcess[1].filePaths = &internalCxxFilePaths;

            // performance: technically all three of these can be done on separate threads
            // they're technically writing to different arrays
            for (size_t i = 0; i < (sizeof(toProcess) / sizeof(toProcess[0])); i++)
            {
                brahma_reserve_data_chunk_array_list_capacity(toProcess[i].fileChunks, libDefs.count);

                Brahma_Data_Chunk chunk;
                chunk.start = (uint16_t) toProcess[i].filePaths->count;
                if (brahma_dir_exists(toProcess[i].searchPath))
                {
                    Brahma_Gather_Files_By_Extension_Payload payload = { toProcess[i].extension, toProcess[i].filePaths };
                    brahma_iterate_directory(toProcess[i].searchPath, true, &payload, brahma_gather_files_by_extension_visitor);
                }
                chunk.count = (uint16_t) (toProcess[i].filePaths->count - chunk.start);
                brahma_append_data_chunk_to_array_list(toProcess[i].fileChunks, chunk);
            }
        }

        PROFILE_SECTION_END("gather source files");
    }

    // artifact generation - definitions
    if (!failed)
    {
        char* packageDefinitions = brahma_sprintf("%s/%sPkgDef.h", ex.intermediateOutputDir, selectedPkg->name);
        {
            FILE* packageDefinitionsFile = fopen(packageDefinitions, "w");
            if (packageDefinitionsFile)
            {
                fprintf(packageDefinitionsFile, "// This file is auto-generated by Brahma. Do not edit manually.\n");
                fprintf(packageDefinitionsFile, "#pragma once\n\n");

                fprintf(packageDefinitionsFile, "#define BRAHMA_PACKAGE_NAME \"%s\"\n", selectedPkg->name);

                Brahma_Definition_Paged_List* pkgDefinitions = &selectedPkg->definitions;
                for (size_t j = 0; j < pkgDefinitions->count; j++)
                {
                    const Brahma_String_KVP* define = brahma_index_definition_paged_list(pkgDefinitions, j);
                    fprintf(packageDefinitionsFile, "\n#undef %s\n", define->key);
                    fprintf(packageDefinitionsFile, "#define %s %s\n", define->key, define->value);
                }

                fclose(packageDefinitionsFile);
            }
        }

        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);

            struct
            {
                const char* fileName;
                const Brahma_Definition_Paged_List* definitions;
                bool isInternal;
            } toProcess[2];

            toProcess[0].fileName = "Interface";
            toProcess[0].definitions = &lib->interfaceDefinitions;
            toProcess[0].isInternal = false;

            toProcess[1].fileName = "Internal";
            toProcess[1].definitions = &lib->internalDefinitions;
            toProcess[1].isInternal = true;

            char* allCapsLibName = brahma_sprintf("%s", lib->name);
            for (char* p = allCapsLibName; *p; p++) if (*p >= 'a' && *p <= 'z') { *p = *p - ('a' - 'A'); }

            for (size_t i = 0; i < (sizeof(toProcess) / sizeof(toProcess[0])); i++)
            {
                char* artifactPath = brahma_sprintf("%s/%s%sLibDefinitions.h", ex.intermediateOutputDir, lib->name, toProcess[i].fileName);

                FILE* artifactFile = fopen(artifactPath, "w");
                if (artifactFile)
                {
                    fprintf(artifactFile, "// This file is auto-generated by Brahma. Do not edit manually.\n");
                    fprintf(artifactFile, "#pragma once\n\n");
                    fprintf(artifactFile, "#include \"%s\"\n", packageDefinitions);

                    // write the dependency paths
                    {
                        Brahma_Data_Chunk depChunk = libInterfaceDepChunks.data[libIdx];
                        for (size_t j = depChunk.start; j < (size_t) (depChunk.start + depChunk.count); j++)
                        {
                            uint16_t depLibIdx = *brahma_index_library_idx_paged_list(&allLibInterfaceDeps, j);
                            fprintf(artifactFile, "#include \"%s/%sInterfaceLibDefinitions.h\"\n", ex.intermediateOutputDir, libDefs.data[depLibIdx].name);
                        }
                    }

                    if (!toProcess[i].isInternal)
                    {
                        // include interface definitions in internal definitions, obviously
                        fprintf(artifactFile, "#include \"%s/%sInterfaceLibDefinitions.h\"\n", ex.intermediateOutputDir, lib->name);
                    }

                    const Brahma_Definition_Paged_List* definitions = toProcess[i].definitions;
                    for (size_t j = 0; j < definitions->count; j++)
                    {
                        const Brahma_String_KVP* define = brahma_index_definition_paged_list(definitions, j);
                        fprintf(artifactFile, "\n#undef %s\n", define->key);
                        fprintf(artifactFile, "#define %s %s\n", define->key, define->value);
                    }

                    fclose(artifactFile);
                }
            }
        }

        PROFILE_SECTION_END("generate definitions");
    }

    // artifact generation - unity files
    Brahma_String_Array_List libUnityCFiles = { NULL, 0, 0 };
    Brahma_Library_Idx_Array_List libUnityCFileLibIdxs = { NULL, 0, 0 };
    Brahma_String_Array_List libUnityCxxFiles = { NULL, 0, 0 };
    Brahma_Library_Idx_Array_List libUnityCxxFileLibIdxs = { NULL, 0, 0 };
    if (!failed)
    {
        brahma_reserve_string_array_list_capacity(&libUnityCFiles, libDefs.count);
        brahma_reserve_string_array_list_capacity(&libUnityCxxFiles, libDefs.count);

        brahma_reserve_library_idx_array_list_capacity(&libUnityCFileLibIdxs, libDefs.count);
        brahma_reserve_library_idx_array_list_capacity(&libUnityCxxFileLibIdxs, libDefs.count);

        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);

            struct
            {
                const char* extension;
                Brahma_Data_Chunk filesChunk;
                const Brahma_String_Paged_List* filePaths;
                Brahma_String_Array_List* filePathsOutput;
                Brahma_Library_Idx_Array_List* fileLibIdxsOutput;
            } toProcess[2];

            toProcess[0].extension = ".c";
            toProcess[0].filesChunk = internalCFileChunks.data[libIdx];
            toProcess[0].filePaths = &internalCFilePaths;
            toProcess[0].filePathsOutput = &libUnityCFiles;
            toProcess[0].fileLibIdxsOutput = &libUnityCFileLibIdxs;

            toProcess[1].extension = ".cpp";
            toProcess[1].filesChunk = internalCxxFileChunks.data[libIdx];
            toProcess[1].filePaths = &internalCxxFilePaths;
            toProcess[1].filePathsOutput = &libUnityCxxFiles;
            toProcess[1].fileLibIdxsOutput = &libUnityCxxFileLibIdxs;

            for (size_t i = 0; i < (sizeof(toProcess) / sizeof(toProcess[0])); i++)
            {
                if (!toProcess[i].filesChunk.count) { continue; }

                char* artifactPath = brahma_sprintf("%s/%sUnity%s", ex.intermediateOutputDir, lib->name, toProcess[i].extension);
                brahma_append_string_to_array_list(toProcess[i].filePathsOutput, artifactPath);
                brahma_append_library_idx_to_array_list(toProcess[i].fileLibIdxsOutput, (uint16_t) libIdx);

                FILE* artifactFile = fopen(artifactPath, "w");
                if (artifactFile)
                {
                    fprintf(artifactFile, "// This file is auto-generated by Brahma. Do not edit manually.\n\n");
                    fprintf(artifactFile, "#include \"%s/%sInternalLibDefinitions.h\"\n\n", ex.intermediateOutputDir, lib->name);

                    Brahma_Data_Chunk filesChunk = toProcess[i].filesChunk;
                    for (size_t j = filesChunk.start; j < (size_t) (filesChunk.start + filesChunk.count); j++)
                    {
                        const char* filePath = *brahma_index_string_paged_list(toProcess[i].filePaths, j);
                        fprintf(artifactFile, "#include \"%s\"\n", filePath);
                    }

                    fclose(artifactFile);
                }
            }
        }

        PROFILE_SECTION_END("generate unity files");
    }

    // common compilation args
    Brahma_String_Array_List commonCArgs = { NULL, 0, 0 };
    Brahma_String_Array_List commonCxxArgs = { NULL, 0, 0 };
    if (!failed)
    {
        brahma_reserve_string_array_list_capacity(&commonCArgs, 16);
        brahma_reserve_string_array_list_capacity(&commonCxxArgs, 16);

        // initialise some c-only args
        {
            if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
            {
                brahma_append_string_to_array_list(&commonCArgs, "/Brepro"); // deterministic output
                brahma_append_string_to_array_list(&commonCArgs, "/nologo");
                brahma_append_string_to_array_list(&commonCArgs, "/Zc:preprocessor"); // conformant preprocessor

                brahma_append_string_to_array_list(&commonCArgs, (ex.flags & BRAHMA_ARGS_FLAG_OPTIMISED) ? "/O2" : "/Od"); // optimised vs not

                if (ex.flags & BRAHMA_ARGS_FLAG_DEBUG) // debug
                {
                    brahma_append_string_to_array_list(&commonCArgs, "/Z7"); // debug info
                    brahma_append_string_to_array_list(&commonCArgs, "/D_DEBUG");
                    brahma_append_string_to_array_list(&commonCArgs, "/MTd"); // debug multithreaded runtime
                }
                else // release
                {
                    brahma_append_string_to_array_list(&commonCArgs, "/MT"); // multithreaded runtime
                    brahma_append_string_to_array_list(&commonCArgs, "/DNDEBUG");
                    brahma_append_string_to_array_list(&commonCArgs, "/GL"); // LTCG
                }
            }
            else
            {
                brahma_append_string_to_array_list(&commonCArgs, (ex.flags & BRAHMA_ARGS_FLAG_OPTIMISED) ? "-O2" : "-O0"); // optimised vs not

                if (ex.flags & BRAHMA_ARGS_FLAG_DEBUG) // debug
                {
                    brahma_append_string_to_array_list(&commonCArgs, "-g"); // debug info
                    brahma_append_string_to_array_list(&commonCArgs, "-DDEBUG");
                }
                else // release
                {
                    brahma_append_string_to_array_list(&commonCArgs, "-DNDEBUG");
                    brahma_append_string_to_array_list(&commonCArgs, "-flto");
                }

                brahma_append_string_to_array_list(&commonCArgs, "-fPIC"); // position independent code, for shared libraries
            }
        }

        // copy all c args to c++ args and the nadd some c++ specific args
        {
            for (size_t i = 0; i < commonCArgs.count; i++)
            {
                brahma_append_string_to_array_list(&commonCxxArgs, commonCArgs.data[i]);
            }

            if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
            {
                brahma_append_string_to_array_list(&commonCxxArgs, "/EHs-"); // no exception handling
                brahma_append_string_to_array_list(&commonCxxArgs, "/GR-"); // no rtti
                brahma_append_string_to_array_list(&commonCxxArgs, "/std:c++20");
            }
            else
            {
                brahma_append_string_to_array_list(&commonCxxArgs, "-fno-exceptions");
                brahma_append_string_to_array_list(&commonCxxArgs, "-fno-rtti");
                brahma_append_string_to_array_list(&commonCxxArgs, "-std=c++20");
            }
        }

        // add some c-only args that we don't want for c++
        {
            brahma_append_string_to_array_list(&commonCArgs, (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? "/std:c11" : "-std=c11");
        }

        PROFILE_SECTION_END("create common compile args");
    }

    // begin compile processes
    Brahma_Process_Array_List compileProcesses = { NULL, 0, 0 };
    Brahma_String_Array_List builtObjects = { NULL, 0, 0 };
    Brahma_String_Paged_List allCompileCommands = { NULL, 0, 0 };
    Brahma_Data_Chunk_Array_List compileCommandChunks = { NULL, 0, 0 };
    if (!failed)
    {
        brahma_reserve_process_array_list_capacity(&compileProcesses, (libUnityCFiles.count + libUnityCxxFiles.count));
        brahma_reserve_string_array_list_capacity(&builtObjects, (libUnityCFiles.count + libUnityCxxFiles.count));
        brahma_reserve_data_chunk_array_list_capacity(&compileCommandChunks, (libUnityCFiles.count + libUnityCxxFiles.count));

        struct
        {
            char* compilerPath;
            const Brahma_String_Array_List* files;
            const Brahma_Library_Idx_Array_List* fileLibIdxs;
            const Brahma_String_Array_List* commonArgs;
            const char* objCArg; // for obj-c++ on macOS/iOS
        } toProcess[2];

        toProcess[0].compilerPath = cCompilerPath;
        toProcess[0].files = &libUnityCFiles;
        toProcess[0].fileLibIdxs = &libUnityCFileLibIdxs;
        toProcess[0].commonArgs = &commonCArgs;
        toProcess[0].objCArg = "objective-c";

        toProcess[1].compilerPath = cxxCompilerPath;
        toProcess[1].files = &libUnityCxxFiles;
        toProcess[1].fileLibIdxs = &libUnityCxxFileLibIdxs;
        toProcess[1].commonArgs = &commonCxxArgs;
        toProcess[1].objCArg = "objective-c++";

        for (size_t i = 0; i < (sizeof(toProcess) / sizeof(toProcess[0])); i++)
        {
            const Brahma_String_Array_List* commonArgs = toProcess[i].commonArgs;

            for (size_t j = 0; j < toProcess[i].files->count; j++)
            {
                const char* sourceFilePath = toProcess[i].files->data[j];
                const char* objectFilePath = brahma_sprintf("%s.%s", sourceFilePath, (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? "obj" : "o");

                Brahma_String_Array_List compileArgs = { NULL, 0, 0 };
                brahma_reserve_string_array_list_capacity(&compileArgs, 16);

                brahma_append_string_to_array_list(&compileArgs, toProcess[i].compilerPath);
                for (size_t k = 0; k < commonArgs->count; k++)
                {
                    brahma_append_string_to_array_list(&compileArgs, commonArgs->data[k]);
                }

                if (ex.platform == BRAHMA_PLATFORM_OSX || ex.platform == BRAHMA_PLATFORM_IOS)
                {
                    brahma_append_string_to_array_list(&compileArgs, "-x");
                    brahma_append_string_to_array_list(&compileArgs, toProcess[i].objCArg);
                }

                // compile-only; no link, and on windows, better diagnostics
                if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
                {
                    brahma_append_string_to_array_list(&compileArgs, "/diagnostics:caret");
                    brahma_append_string_to_array_list(&compileArgs, "/c");

                    // warnings
                    if (ex.flags & BRAHMA_ARGS_FLAG_SHOW_WARNINGS)
                        brahma_append_string_to_array_list(&commonCArgs, "/Wall"); // all warnings

                    // w=e
                    if (ex.flags & BRAHMA_ARGS_FLAG_WARNINGS_ARE_ERRORS)
                        brahma_append_string_to_array_list(&commonCArgs, "/WX"); // warnings as errors
                }
                else
                {
                    brahma_append_string_to_array_list(&compileArgs, "-c");

                    // warnings
                    if (ex.flags & BRAHMA_ARGS_FLAG_SHOW_WARNINGS)
                    {
                        brahma_append_string_to_array_list(&commonCArgs, "-Wall");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wextra");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wshadow");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wconversion");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wdouble-promotion");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wfloat-equal");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wundef");
                        brahma_append_string_to_array_list(&commonCArgs, "-Wswitch-enum");
                    }

                    // w=e
                    if (ex.flags & BRAHMA_ARGS_FLAG_WARNINGS_ARE_ERRORS)
                        brahma_append_string_to_array_list(&commonCArgs, "-Werror");
                }

                // include windows sdk dirs
                if (ex.platform == BRAHMA_PLATFORM_WINDOWS && windowsKitPath && windowsKitVersion)
                {
                    brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s\\include", toolchainPath));

                    brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s/Include/%s/um", windowsKitPath, windowsKitVersion));
                    brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s/Include/%s/shared", windowsKitPath, windowsKitVersion));
                    brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s/Include/%s/winrt", windowsKitPath, windowsKitVersion));
                    brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s/Include/%s/ucrt", windowsKitPath, windowsKitVersion));
                }

                // add all includes
                {
                    uint16_t libIdx = toProcess[i].fileLibIdxs->data[j];
                    Brahma_Data_Chunk libAllDepsChunk = libAllDepChunks.data[libIdx];
                    for (size_t k = libAllDepsChunk.start; k < (size_t) (libAllDepsChunk.start + libAllDepsChunk.count); k++)
                    {
                        uint16_t depLibIdx = *brahma_index_library_idx_paged_list(&allLibAllDeps, k);
                        const char* includePath = allInterfacePaths.data[depLibIdx];
                        if (includePath)
                        {
                            if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
                            {
                                brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s", includePath));
                            }
                            else
                            {
                                brahma_append_string_to_array_list(&compileArgs, "-I");
                                brahma_append_string_to_array_list(&compileArgs, includePath);
                            }
                        }
                    }

                    // add self include
                    const char* selfIncludePath = allInterfacePaths.data[libIdx];
                    if (selfIncludePath)
                    {
                        if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
                        {
                            brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/I%s", selfIncludePath));
                        }
                        else
                        {
                            brahma_append_string_to_array_list(&compileArgs, "-I");
                            brahma_append_string_to_array_list(&compileArgs, selfIncludePath);
                        }
                    }
                }

                brahma_append_string_to_array_list(&compileArgs, sourceFilePath);
                if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
                {
                    brahma_append_string_to_array_list(&compileArgs, brahma_sprintf("/Fo%s", objectFilePath)); // object file output
                }
                else
                {
                    brahma_append_string_to_array_list(&compileArgs, "-o");
                    brahma_append_string_to_array_list(&compileArgs, objectFilePath);
                }

                Brahma_Process compileProcess = brahma_start_process(compileArgs, NULL);

                brahma_append_process_to_array_list(&compileProcesses, compileProcess);
                brahma_append_string_to_array_list(&builtObjects, objectFilePath);

                Brahma_Data_Chunk compileCommandChunk;
                compileCommandChunk.start = (uint16_t) allCompileCommands.count;
                for (size_t k = 0; k < compileArgs.count; k++)
                    brahma_append_string_to_paged_list(&allCompileCommands, compileArgs.data[k]);
                compileCommandChunk.count = (uint16_t) (allCompileCommands.count - compileCommandChunk.start);
                brahma_append_data_chunk_to_array_list(&compileCommandChunks, compileCommandChunk);
            }
        }

        PROFILE_SECTION_END("start compile processes");
    }

    // windows manifest
    char* windowsManifestPath = NULL;
    if (!failed && ex.platform == BRAHMA_PLATFORM_WINDOWS && selectedPkg->outputType == BRAHMA_PACKAGE_OUTPUT_TYPE_EXECUTABLE)
    {
        windowsManifestPath = brahma_sprintf("%s/AppManifest.manifest", ex.intermediateOutputDir);
        FILE* manifestFile = fopen(windowsManifestPath, "w");
        if (manifestFile)
        {
            fprintf(manifestFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
            fprintf(manifestFile, "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n");
            fprintf(manifestFile, "  <assemblyIdentity version=\"1.0.0.0\" name=\"%s\" />\n", selectedPkg->name);
            fprintf(manifestFile, "  <application xmlns=\"urn:schemas-microsoft-com:asm.v3\">\n");
            fprintf(manifestFile, "    <windowsSettings>\n");
            fprintf(manifestFile, "      <activeCodePage xmlns=\"http://schemas.microsoft.com/SMI/2019/WindowsSettings\">UTF-8</activeCodePage>\n");
            fprintf(manifestFile, "    </windowsSettings>\n");
            fprintf(manifestFile, "  </application>\n");
            fprintf(manifestFile, "</assembly>\n");

            fclose(manifestFile);
        }
    }

    // gather all external deps
    char* staticLinkLibDir = NULL;
    Brahma_String_Paged_List externalDeps; memset(&externalDeps, 0, sizeof(externalDeps));
    Brahma_String_Paged_List externalFrameworks; memset(&externalFrameworks, 0, sizeof(externalFrameworks));
    if (!failed)
    {
        staticLinkLibDir = brahma_sprintf("%s/ExternalDeps", ex.intermediateOutputDir);
        if (!brahma_ensure_dir(staticLinkLibDir))
        {
            ex.log(BRAHMA_LOG_ERROR "Failed to create external dependency directory '%s'.\n", staticLinkLibDir);
            failed = true;
        }

        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);
            Brahma_String_Paged_List libExternalDeps = lib->externalDependencies;
            for (size_t depIdx = 0; depIdx < libExternalDeps.count; depIdx++)
            {
                const char* dep = *brahma_index_string_paged_list(&libExternalDeps, depIdx);

                const char relPathPrefix[] = "relpath:";
                const char absPathPrefix[] = "abspath:";
                const char globalPrefix[] = "global:";
                const char frameworkPrefix[] = "framework:";

                const char* toAdd = NULL;
                Brahma_String_Paged_List* addTo = NULL;

                char* absPath = NULL;
                if (strncmp(dep, relPathPrefix, sizeof(relPathPrefix) - 1) == 0)
                {
                    absPath = brahma_sprintf("%s/%s", lib->owningDir, dep + sizeof(relPathPrefix) - 1);
                }
                else if (strncmp(dep, absPathPrefix, sizeof(absPathPrefix) - 1) == 0)
                {
                    absPath = brahma_sprintf("%s", dep + sizeof(absPathPrefix) - 1);
                }

                if (absPath != NULL)
                {
                    char* lastFwdSlash = strrchr(absPath, '/');
                    char* lastBwdSlash = strrchr(absPath, '\\');
                    char* lastSlash = (lastFwdSlash > lastBwdSlash) ? lastFwdSlash : lastBwdSlash;
                    if (!lastSlash)
                    {
                        ex.log(BRAHMA_LOG_ERROR "Invalid external dependency path for '%s' in library '%s'.\n", dep, lib->name);
                        failed = true;
                        continue;
                    }

                    char* fileName = lastSlash + 1;
                    char* dstPath = brahma_sprintf("%s/%s", staticLinkLibDir, fileName);

                    if (!brahma_copy_file(absPath, dstPath))
                    {
                        ex.log(BRAHMA_LOG_ERROR "Failed to copy external dependency '%s' to '%s'.\n", absPath, dstPath);
                        failed = true;
                        continue;
                    }

                    ex.log(BRAHMA_LOG_INFO "Copied external dependency '%s' to '%s'.\n", absPath, dstPath);

                    addTo = &externalDeps;
                    if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
                    {
                        toAdd = fileName;
                    }
                    else
                    {
                        // for non-windows platforms, we wanna add the file name without the extension
                        // or the lib prefix, if it exists, so that the linker can find it

                        if (strncmp(fileName, "lib", sizeof("lib") - 1) == 0)
                        {
                            fileName += sizeof("lib") - 1;
                        }

                        char* lastDotInFileName = strrchr(fileName, '.');
                        if (!lastDotInFileName)
                        {
                            ex.log(BRAHMA_LOG_ERROR "Invalid external dependency file name for '%s' in library '%s'. Unable to parse extension.\n", dep, lib->name);
                            failed = true;
                            continue;
                        }

                        size_t fileNameLen = (size_t) (lastDotInFileName - fileName);
                        char* fileNameWithoutExt = brahma_sprintf("%.*s", (int) fileNameLen, fileName);
                        toAdd = fileNameWithoutExt;
                    }
                }

                if (absPath != NULL) { }
                else if (strncmp(dep, globalPrefix, sizeof(globalPrefix) - 1) == 0)
                {
                    addTo = &externalDeps;
                    toAdd = dep + sizeof(globalPrefix) - 1;
                }
                else if (strncmp(dep, frameworkPrefix, sizeof(frameworkPrefix) - 1) == 0)
                {
                    addTo = &externalFrameworks;
                    toAdd = dep + sizeof(frameworkPrefix) - 1;
                }
                else
                {
                    ex.log(BRAHMA_LOG_ERROR "Invalid external dependency format for '%s' in library '%s'. Unknown dependency type. Add a prefix.\n", dep, lib->name);
                    failed = true;
                    continue;
                }

                // avoid duplicates
                bool found = false;
                for (size_t i = 0; i < addTo->count; i++)
                {
                    if (strcmp(toAdd, *brahma_index_string_paged_list(addTo, i)) == 0)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    brahma_append_string_to_paged_list(addTo, toAdd);
                }
            }
        }
    }

    // wait for compile processes
    if (!failed)
    {
        for (size_t i = 0; i < compileProcesses.count; i++)
        {
            Brahma_Process compileProcess = compileProcesses.data[i];
            if (!compileProcess)
            {
                ex.log(BRAHMA_LOG_ERROR "Failed to start compile process for '%s'.\n", builtObjects.data[i]);
                failed = true;
                continue;
            }

            char* outputFromProcess = NULL;
            if (0 != brahma_wait_for_process(compileProcess, &outputFromProcess))
            {
                ex.log("--------------------------------------------------------\n");
                ex.log(BRAHMA_LOG_ERROR "Failed to compile '%s'.\n", builtObjects.data[i]);
                ex.log("--------------------------------------------------------\n");
                ex.log("Command: ");
                Brahma_Data_Chunk compileCommandChunk = compileCommandChunks.data[i];
                for (size_t k = compileCommandChunk.start; k < (size_t) (compileCommandChunk.start + compileCommandChunk.count); k++)
                    ex.log("%s ", *brahma_index_string_paged_list(&allCompileCommands, k));
                ex.log("\n");
                ex.log("--------------------------------------------------------\n");
                ex.log("%s", outputFromProcess);
                ex.log("--------------------------------------------------------\n");
                failed = true;
                continue;
            }

            ex.log(BRAHMA_LOG_SUCCESS "Compiled '%s' successfully.\n", builtObjects.data[i]);
        }

        PROFILE_SECTION_END("end compile processes");
    }

    // begin linking process
    Brahma_String_Array_List linkArgs = { NULL, 0, 0 };
    Brahma_Process linkProcess = NULL;
    char* output = NULL;
    if (!failed)
    {
        brahma_reserve_string_array_list_capacity(&linkArgs, 16);

        brahma_append_string_to_array_list(&linkArgs,
            false ? "" :
            (selectedPkg->outputType == BRAHMA_PACKAGE_OUTPUT_TYPE_EXECUTABLE) ? cxxCompilerPath :
            (selectedPkg->outputType == BRAHMA_PACKAGE_OUTPUT_TYPE_DYNAMIC_LIBRARY) ? cxxCompilerPath :
            (selectedPkg->outputType == BRAHMA_PACKAGE_OUTPUT_TYPE_STATIC_LIBRARY) ? staticLinkerPath :
            "<UNKNOWN OUTPUT TYPE>"
        );

        const char* outputPrefix = NULL;
        const char* extension = NULL;
        switch (selectedPkg->outputType)
        {
            case BRAHMA_PACKAGE_OUTPUT_TYPE_EXECUTABLE:
                outputPrefix = "";
                extension = (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? ".exe" : "";
                break;
            case BRAHMA_PACKAGE_OUTPUT_TYPE_DYNAMIC_LIBRARY:
                outputPrefix = (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? "" : "lib";
                extension = (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? ".dll" :
                            (ex.platform == BRAHMA_PLATFORM_OSX || ex.platform == BRAHMA_PLATFORM_IOS) ? ".dylib" : ".so";
                break;
            case BRAHMA_PACKAGE_OUTPUT_TYPE_STATIC_LIBRARY:
                outputPrefix = (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? "" : "lib";
                extension = (ex.platform == BRAHMA_PLATFORM_WINDOWS) ? ".lib" : ".a";
                break;
            default:
                extension = ".unknown";
                break;
        }

        output = brahma_sprintf("%s/%s%s%s", ex.outputDir, outputPrefix, selectedPkg->name, extension);

        if (selectedPkg->outputType == BRAHMA_PACKAGE_OUTPUT_TYPE_STATIC_LIBRARY)
        {
            if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
            {
                brahma_append_string_to_array_list(&linkArgs, "/Brepro");
                brahma_append_string_to_array_list(&linkArgs, "/nologo");
                brahma_append_string_to_array_list(&linkArgs, brahma_sprintf("/OUT:%s", output));
            }
            else if (ex.platform == BRAHMA_PLATFORM_OSX || ex.platform == BRAHMA_PLATFORM_IOS)
            {
                brahma_append_string_to_array_list(&linkArgs, "rcs");
                brahma_append_string_to_array_list(&linkArgs, output);
            }
            else
            {
                brahma_append_string_to_array_list(&linkArgs, "rcs");
                brahma_append_string_to_array_list(&linkArgs, output);
            }

            for (size_t i = 0; i < builtObjects.count; i++)
            {
                brahma_append_string_to_array_list(&linkArgs, builtObjects.data[i]);
            }
        }
        else // executable or dynamic library
        {
            for (size_t i = 0; i < commonCxxArgs.count; i++)
            {
                brahma_append_string_to_array_list(&linkArgs, commonCxxArgs.data[i]);
            }

            for (size_t i = 0; i < builtObjects.count; i++)
            {
                brahma_append_string_to_array_list(&linkArgs, builtObjects.data[i]);
            }

            if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
            {
                brahma_append_string_to_array_list(&linkArgs, brahma_sprintf("/Fe%s", output));
                if (ex.flags & BRAHMA_ARGS_FLAG_DEBUG)
                {
                    brahma_append_string_to_array_list(&linkArgs, "/DEBUG");
                    brahma_append_string_to_array_list(&linkArgs,
                        // trim the extension while printf'ing and add .pdb
                        brahma_sprintf("/Fd%s/%.*s.pdb", ex.outputDir, (int) (strlen(output) - strlen(extension)), output));
                }

                brahma_append_string_to_array_list(&linkArgs, brahma_sprintf("/Fo%s/", ex.intermediateOutputDir));

                for (size_t i = 0; i < externalDeps.count; i++)
                {
                    const char* dep = *brahma_index_string_paged_list(&externalDeps, i);
                    brahma_append_string_to_array_list(&linkArgs, dep);
                }
            }
            else
            {
                brahma_append_string_to_array_list(&linkArgs, "-o");
                brahma_append_string_to_array_list(&linkArgs, output);

                brahma_append_string_to_array_list(&linkArgs, brahma_sprintf("-L%s", staticLinkLibDir));
                for (size_t i = 0; i < externalDeps.count; i++)
                {
                    const char* dep = *brahma_index_string_paged_list(&externalDeps, i);
                    const char* arg = brahma_sprintf("-l%s", dep);
                    brahma_append_string_to_array_list(&linkArgs, arg);
                }

                if (ex.platform == BRAHMA_PLATFORM_OSX || ex.platform == BRAHMA_PLATFORM_IOS)
                {
                    for (size_t i = 0; i < externalFrameworks.count; i++)
                    {
                        const char* framework = *brahma_index_string_paged_list(&externalFrameworks, i);
                        brahma_append_string_to_array_list(&linkArgs, "-framework");
                        brahma_append_string_to_array_list(&linkArgs, framework);
                    }
                }
            }

            if (selectedPkg->outputType == BRAHMA_PACKAGE_OUTPUT_TYPE_DYNAMIC_LIBRARY)
            {
                if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
                {
                    brahma_append_string_to_array_list(&linkArgs, (ex.flags & BRAHMA_ARGS_FLAG_DEBUG) ? "/LDd" : "/LD");
                }
                else
                {
                    brahma_append_string_to_array_list(&linkArgs, "-shared");
                    if (ex.platform == BRAHMA_PLATFORM_LINUX || ex.platform == BRAHMA_PLATFORM_ANDROID)
                    {
                        brahma_append_string_to_array_list(&linkArgs, "-fPIC");
                    }
                    if (ex.platform == BRAHMA_PLATFORM_OSX || ex.platform == BRAHMA_PLATFORM_IOS)
                    {
                        brahma_append_string_to_array_list(&linkArgs, "-dynamiclib");
                    }
                }
            }

            if (ex.platform == BRAHMA_PLATFORM_WINDOWS)
            {
                brahma_append_string_to_array_list(&linkArgs, "/link");
                brahma_append_string_to_array_list(&linkArgs, brahma_sprintf("/LIBPATH:%s", staticLinkLibDir));
                brahma_append_string_to_array_list(&linkArgs, "/LTCG");

                if (windowsManifestPath)
                {
                    brahma_append_string_to_array_list(&linkArgs, "/MANIFEST:embed");
                    brahma_append_string_to_array_list(&linkArgs, brahma_sprintf("/MANIFESTINPUT:%s", windowsManifestPath));
                }
            }
        }

        linkProcess = brahma_start_process(linkArgs, NULL);
        PROFILE_SECTION_END("start link process");
    }

    // copy all the "files to copy" as requested
    if (!failed)
    {
        for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
        {
            Brahma_Library* lib = &(libDefs.data[libIdx]);
            for (size_t i = 0; i < lib->filesToCopyNextToOutput.count; i++)
            {
                Brahma_String_KVP* fileToCopy = brahma_index_files_to_copy_paged_list(&lib->filesToCopyNextToOutput, i);
                char* srcPath = brahma_sprintf("%s/%s", lib->owningDir, fileToCopy->key);
                char* dstPath = brahma_sprintf("%s/%s", ex.outputDir, fileToCopy->value);

                if (!brahma_copy_file(srcPath, dstPath))
                {
                    ex.log(BRAHMA_LOG_ERROR "Failed to write to destination file for copying dependency '%s' to '%s'.\n", srcPath, dstPath);
                    failed = true;
                    continue;
                }

                ex.log(BRAHMA_LOG_SUCCESS "Copied dependency '%s' to '%s' successfully.\n", srcPath, dstPath);
            }
        }

        PROFILE_SECTION_END("copy dependencies");
    }

    // output .clangd
    if (ex.clangdOutputDir)
    {
        ex.log("Generating .clangd configuration file for clangd at '%s'.\n", ex.clangdOutputDir);

        char* clangdConfigPath = brahma_sprintf("%s/.clangd", ex.clangdOutputDir);
        FILE* clangdConfigFile = fopen(clangdConfigPath, "w");
        bool fileWriteFailed = false;
        if (clangdConfigFile)
        {
            fprintf(clangdConfigFile, "CompileFlags:\n");
            fprintf(clangdConfigFile, "  Add:\n");
            fprintf(clangdConfigFile, "    - -D__INTELLISENSE__\n");
            fprintf(clangdConfigFile, "    - -DBRAHMA_EXEC\n");

            #define CLANGD_ARCH_SUFFIX ""
            #if defined(_M_IX86) || defined(__i386__)
                #define CLANGD_ARCH "i686"
            #elif defined(_M_X64) || defined(__x86_64__)
                #define CLANGD_ARCH "x86_64"
            #elif defined(_M_ARM) || defined(__arm__)
                #define CLANGD_ARCH "armv7"
                #ifdef __linux__
                    #undef CLANGD_ARCH_SUFFIX
                    #define CLANGD_ARCH_SUFFIX "eabihf" // why?
                #endif
            #elif defined(_M_ARM64) || defined(__aarch64__)
                #ifdef __APPLE__
                    #define CLANGD_ARCH "arm64" // why?
                #else
                    #define CLANGD_ARCH "aarch64"
                #endif
            #else
                #define CLANGD_ARCH "unknown_arch"
            #endif

            #if defined(_WIN32)
            {
                fprintf(clangdConfigFile, "    - -D_DEBUG\n");
                fprintf(clangdConfigFile, "    - --target=" CLANGD_ARCH "-pc-windows-msvc\n");
                fprintf(clangdConfigFile, "    - -fms-compatibility\n");
                fprintf(clangdConfigFile, "    - -fms-extensions\n");
            }
            #elif defined(__linux__)
            {
                fprintf(clangdConfigFile, "    - -DDEBUG\n");
                fprintf(clangdConfigFile, "    - -D_GNU_SOURCE\n");
                fprintf(clangdConfigFile, "    - --target=" CLANGD_ARCH "-pc-linux-gnu" CLANGD_ARCH_SUFFIX "\n");
            }
            #elif defined(__APPLE__)
            {
                fprintf(clangdConfigFile, "    - -DDEBUG\n");
                fprintf(clangdConfigFile, "    - -D_DARWIN_C_SOURCE\n");
                fprintf(clangdConfigFile, "    - --target=" CLANGD_ARCH "-apple-darwin\n");
            }
            #endif

            #undef CLANGD_ARCH_SUFFIX
            #undef CLANGD_ARCH
            fprintf(clangdConfigFile,
                "Diagnostics:\n"
                "  UnusedIncludes: None\n"
            );

            for (size_t libIdx = 0; libIdx < libDefs.count; libIdx++)
            {
                Brahma_Library* lib = &(libDefs.data[libIdx]);

                fprintf(clangdConfigFile,
                    "---\n"
                    "If:\n"
                    "  PathMatch: .*/%s/Internal/.*\n"
                    "CompileFlags:\n"
                    "  Add:\n"
                    "    - -include\n"
                    "    - %s\n"
                    "",
                    lib->name,
                    brahma_sprintf("%s/%sInternalLibDefinitions.h", ex.intermediateOutputDir, lib->name)
                );

                if (allInterfacePaths.data[libIdx])
                {
                    fprintf(clangdConfigFile, "    - -I%s\n", allInterfacePaths.data[libIdx]);
                }

                fprintf(clangdConfigFile,
                    "---\n"
                    "If:\n"
                    "  PathMatch: .*/%s/Interface/.*\n"
                    "CompileFlags:\n"
                    "  Add:\n"
                    "    - -include\n"
                    "    - %s\n"
                    "",
                    lib->name,
                    brahma_sprintf("%s/%sInterfaceLibDefinitions.h", ex.intermediateOutputDir, lib->name)
                );

                Brahma_Data_Chunk interfaceDepsChunk = libInterfaceDepChunks.data[libIdx];
                if (interfaceDepsChunk.count)
                {
                    fprintf(clangdConfigFile,
                        "---\n"
                        "If:\n"
                        "  PathMatch: .*/%s/Interface/.*\n"
                        "CompileFlags:\n"
                        "  Add:\n"
                        "",
                        lib->name
                    );

                    for (size_t i = interfaceDepsChunk.start; i < (size_t) (interfaceDepsChunk.start + interfaceDepsChunk.count); i++)
                    {
                        uint16_t depLibIdx = *brahma_index_library_idx_paged_list(&allLibInterfaceDeps, i);
                        const char* depInterfacePath = allInterfacePaths.data[depLibIdx];
                        if (depInterfacePath)
                        {
                            fprintf(clangdConfigFile, "    - -I%s\n", depInterfacePath);
                        }
                    }
                }

                Brahma_Data_Chunk allDepsChunk = libAllDepChunks.data[libIdx];
                if (allDepsChunk.count)
                {
                    fprintf(clangdConfigFile,
                        "---\n"
                        "If:\n"
                        "  PathMatch: .*/%s/Internal/.*\n"
                        "CompileFlags:\n"
                        "  Add:\n"
                        "",
                        lib->name
                    );

                    for (size_t i = allDepsChunk.start; i < (size_t) (allDepsChunk.start + allDepsChunk.count); i++)
                    {
                        uint16_t depLibIdx = *brahma_index_library_idx_paged_list(&allLibAllDeps, i);
                        const char* depInterfacePath = allInterfacePaths.data[depLibIdx];
                        if (depInterfacePath)
                        {
                            fprintf(clangdConfigFile, "    - -I%s\n", depInterfacePath);
                        }
                    }
                }
            }

            bool objC = (ex.platform == BRAHMA_PLATFORM_OSX || ex.platform == BRAHMA_PLATFORM_IOS);

            fprintf(clangdConfigFile,
                "---\n"
                "If:\n"
                "  PathMatch: .*\\.(cc|cpp|cxx)$\n"
                "CompileFlags:\n"
                "  Add:\n"
                "    - -xc++\n"
                "    - -std=c++20\n"
                "    - -fno-exceptions\n"
                "    - -fno-rtti\n"
            );

            if (objC)
            {
                fprintf(clangdConfigFile,
                    "    - -x\n"
                    "    - objective-c++\n"
                );
            }

            fprintf(clangdConfigFile,
                "---\n"
                "If:\n"
                "  PathMatch: .*\\.(h|c)$\n"
                "CompileFlags:\n"
                "  Add:\n"
                "    - -xc\n"
                "    - -std=c11\n"
            );

            if (objC)
            {
                fprintf(clangdConfigFile,
                    "    - -x\n"
                    "    - objective-c\n"
                );
            }

            fileWriteFailed = (fclose(clangdConfigFile) != 0);
        }

        if (fileWriteFailed)
        {
            ex.log(BRAHMA_LOG_ERROR "Failed to write .clangd config file to '%s'.\n", clangdConfigPath);
            failed = true;
        }
        else
        {
            ex.log(BRAHMA_LOG_SUCCESS "Wrote .clangd config file to '%s' successfully.\n", clangdConfigPath);
        }
    }

    // wait for link process
    if (!failed)
    {
        char* outputFromProcess = NULL;
        if (!linkProcess)
        {
            ex.log(BRAHMA_LOG_ERROR "Failed to start link process for '%s'.\n", output);
            failed = true;
        }
        else if (0 != brahma_wait_for_process(linkProcess, &outputFromProcess))
        {
            ex.log("--------------------------------------------------------\n");
            ex.log(BRAHMA_LOG_ERROR "Failed to link '%s'.\n", output);
            ex.log("--------------------------------------------------------\n");
            ex.log("Command: ");
            for (size_t k = 0; k < linkArgs.count; k++)
                ex.log("%s ", linkArgs.data[k]);
            ex.log("\n");
            ex.log("--------------------------------------------------------\n");
            ex.log("%s", outputFromProcess);
            ex.log("--------------------------------------------------------\n");
            failed = true;
        }
        else
        {
            ex.log(BRAHMA_LOG_SUCCESS "Linked '%s' successfully.\n", output);
        }

        PROFILE_SECTION_END("end link process");
    }

    Brahma_Memory_Usage_Report report = brahma_shutdown_internal_allocator();

    PROFILE_SECTION_END("shutdown");

    #undef PROFILE_SECTION_END

    ex.log(BRAHMA_LOG_PROFILE "Total execution time: %.2f ms.\n", (double) (brahma_get_time() - startTime) / 1000000.0);

    {
        #define BYTE_PRINTER(varName) \
            double varName = (double) report.varName; \
            uint8_t varName##Power = 0; \
            while (varName >= 1024.0) { varName /= 1024.0; varName##Power++; } \
            const char* varName##Unit = sizeUnits[varName##Power];

        static const char* sizeUnits[] = { "B", "KiB", "MiB", "GiB", "TiB" };

        BYTE_PRINTER(totalAllocated);
        BYTE_PRINTER(totalUsed);
        BYTE_PRINTER(totalUnused);
        BYTE_PRINTER(totalWasted);

        ex.log(BRAHMA_LOG_PROFILE "Memory usage: %.2f %s allocated, %.2f %s used, %.2f %s unused, %.2f %s wasted (to alignment, paging, fragmentation, etc.).\n",
            totalAllocated, totalAllocatedUnit, totalUsed, totalUsedUnit, totalUnused, totalUnusedUnit, totalWasted, totalWastedUnit);

        #undef BYTE_PRINTER
    }

    if (failed)
    {
        ex.log(BRAHMA_LOG_ERROR "FAILED!\n");
    }
    else
    {
        ex.log(BRAHMA_LOG_SUCCESS "SUCCESS!\n");
    }

    return !failed;
}

bool brahma_append_all_library_deps(
    const Brahma_Library_Array_List* allLibs,
    const Brahma_Library* library,
    Brahma_Library_Idx_Paged_List* allLibDeps,
    size_t firstLibDepIdx,
    bool ignoreInterfaceDeps,
    bool ignoreInternalDeps,
    bool ignoreInternalDepsOfInternalDeps,
    char** error,
    void* cycleChecker)
{
    typedef struct Cycle_Check_Entry Cycle_Check_Entry;
    struct Cycle_Check_Entry
    {
        Cycle_Check_Entry* previous;
        const Brahma_Library* library;
    } cycleCheckCurrentEntry;
    cycleCheckCurrentEntry.previous = (Cycle_Check_Entry*) cycleChecker;
    cycleCheckCurrentEntry.library = library;

    for (Cycle_Check_Entry* entry = cycleCheckCurrentEntry.previous; entry != NULL; entry = entry->previous)
    {
        if (entry->library == library)
        {
            if (error) *error = brahma_sprintf("(%s: circular dependency)", library->name);
            return false;
        }
    }

    const Brahma_String_Paged_List* lists[2];
    lists[0] = NULL; lists[1] = NULL;

    int listCount = 0;
    if (!ignoreInterfaceDeps) lists[listCount++] = &(library->interfaceDependencies);
    if (!ignoreInternalDeps) lists[listCount++] =  &(library->internalDependencies);

    for (size_t listIdx = 0; listIdx < (size_t) listCount; listIdx++)
    {
        const Brahma_String_Paged_List* directDeps = lists[listIdx];

        for (size_t i = 0; i < directDeps->count; i++)
        {
            char** directDependencyPtr = (char**) brahma_index_string_paged_list(directDeps, i);
            if (!directDependencyPtr)
            {
                if (error) *error = brahma_sprintf("(%s) -> (%d: failed to index direct dependency)", library->name, (int) i);
                return false;
            }

            const char* directDependency = *directDependencyPtr;
            int idxOfDirectDepLib = brahma_find_library_by_name(allLibs, directDependency);
            if (idxOfDirectDepLib < 0)
            {
                if (error) *error = brahma_sprintf("(%s) -> (%s: failed to locate)", library->name, directDependency);
                return false;
            }

            Brahma_Library* directDepLib = &(allLibs->data[idxOfDirectDepLib]);
            if (!brahma_append_all_library_deps(allLibs, directDepLib, allLibDeps, firstLibDepIdx,
                    ignoreInterfaceDeps, ignoreInternalDepsOfInternalDeps ? true : ignoreInternalDeps, false,
                    error, &cycleCheckCurrentEntry))
            {
                if (error) *error = brahma_sprintf("(%s) -> %s", library->name, *error);
                return false;
            }

            // by this point, all the dependencies of the direct dependency should have been added
            // now we just need to add the direct dependency itself, if it's not already in the list of all dependencies

            // check if the direct dependency is already in the list of all dependencies
            bool alreadyInList = false;
            for (size_t j = firstLibDepIdx; j < allLibDeps->count; j++)
            {
                int depIdx = (int) *brahma_index_library_idx_paged_list(allLibDeps, j);
                if (depIdx == idxOfDirectDepLib)
                {
                    alreadyInList = true;
                    break;
                }
            }
            if (alreadyInList) continue;

            // add to the list of all dependencies
            brahma_append_library_idx_to_paged_list(allLibDeps, (uint16_t) idxOfDirectDepLib);
        }
    }

    return true;
}

int64_t brahma_get_time(void)
{
    #if defined(_WIN32)
    {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        uint64_t time = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        return (int64_t) (time * 100); // convert from 100-nanosecond intervals to nanoseconds
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return (int64_t) ts.tv_sec * 1000000000LL + ts.tv_nsec;
    }
    #endif
}

static struct
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

    size_t totalAllocatedMemory;
    size_t usedMemory;
} g_brahmaInternalAllocator;

void brahma_initialise_internal_allocator(void)
{
    #if defined(_WIN32)
    {
        InitializeCriticalSection(&g_brahmaInternalAllocator.mutex);

        // spin for 15 cycles before sleeping, to improve performance when the lock
        // is only held for a short time (which is the case for our allocator)
        SetCriticalSectionSpinCount(&g_brahmaInternalAllocator.mutex, 0x0000000F);
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&g_brahmaInternalAllocator.mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    #endif

    g_brahmaInternalAllocator.currentMemoryPage = NULL;
    g_brahmaInternalAllocator.capacity          = 0;
    g_brahmaInternalAllocator.offset            = 0;

    g_brahmaInternalAllocator.previousMemoryPages         = NULL;
    g_brahmaInternalAllocator.previousMemoryPagesCount    = 0;
    g_brahmaInternalAllocator.previousMemoryPagesCapacity = 0;

    g_brahmaInternalAllocator.totalAllocatedMemory = 0;
    g_brahmaInternalAllocator.usedMemory           = 0;

    brahma_push_memory(1, 1); // ready the allocator for use
}

Brahma_Memory_Usage_Report brahma_shutdown_internal_allocator(void)
{
    #if defined(_WIN32)
    {
        DeleteCriticalSection(&g_brahmaInternalAllocator.mutex);
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        pthread_mutex_destroy(&g_brahmaInternalAllocator.mutex);
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

    Brahma_Memory_Usage_Report report;
    report.totalAllocated = g_brahmaInternalAllocator.totalAllocatedMemory;
    report.totalUsed = g_brahmaInternalAllocator.usedMemory;
    report.totalUnused = g_brahmaInternalAllocator.capacity - g_brahmaInternalAllocator.offset;
    report.totalWasted = report.totalAllocated - report.totalUsed - report.totalUnused;

    g_brahmaInternalAllocator.currentMemoryPage = NULL;
    g_brahmaInternalAllocator.capacity          = 0;
    g_brahmaInternalAllocator.offset            = 0;

    g_brahmaInternalAllocator.previousMemoryPages         = NULL;
    g_brahmaInternalAllocator.previousMemoryPagesCount    = 0;
    g_brahmaInternalAllocator.previousMemoryPagesCapacity = 0;

    g_brahmaInternalAllocator.totalAllocatedMemory = 0;
    g_brahmaInternalAllocator.usedMemory           = 0;

    return report;
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
        EnterCriticalSection(&g_brahmaInternalAllocator.mutex);
    #elif defined(__linux__) || defined(__APPLE__)
        pthread_mutex_lock(&g_brahmaInternalAllocator.mutex);
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

        g_brahmaInternalAllocator.totalAllocatedMemory += newPageSize;
    }

    void* result = g_brahmaInternalAllocator.currentMemoryPage + alignedOffset;
    g_brahmaInternalAllocator.offset = alignedOffset + alignedSize;
    g_brahmaInternalAllocator.usedMemory += size;

    #if defined(_WIN32)
        LeaveCriticalSection(&g_brahmaInternalAllocator.mutex);
    #elif defined(__linux__) || defined(__APPLE__)
        pthread_mutex_unlock(&g_brahmaInternalAllocator.mutex);
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

char* brahma_get_env_var(const char* name)
{
    #if defined(_WIN32)
    {
        DWORD bufferSize = GetEnvironmentVariableA(name, NULL, 0); // get the required buffer size
        if (bufferSize == 0) return NULL; // variable not found

        char* buffer = (char*) brahma_push_memory(bufferSize, 1); // +1 for null terminator
        GetEnvironmentVariableA(name, buffer, bufferSize);
        return buffer;
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        const char* value = getenv(name);
        if (!value) return NULL;

        size_t valueLen = strlen(value);
        char* result = (char*) brahma_push_memory(valueLen + 1, 1); // +1 for null terminator
        memcpy(result, value, valueLen + 1);
        return result;
    }
    #else
        #error "unsupported platform"
    #endif
}

bool brahma_dir_exists(const char* path)
{
    #if defined(_WIN32)
    {
        DWORD attrs = GetFileAttributesA(path);
        return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        struct stat st;
        return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
    }
    #endif
}

bool brahma_ensure_dir(char* path)
{
    size_t pathLenWithNullTerm = strlen(path) + 1;

    bool success = true;
    for (size_t i = 1; success && i < pathLenWithNullTerm; i++)
    {
        char originalChar = path[i];
        if (originalChar == '/' || originalChar == '\\' || originalChar == '\0')
        {
            path[i] = '\0';
            #if defined(_WIN32)
            {
                if (!CreateDirectoryA(path, NULL))
                {
                    if (GetLastError() != ERROR_ALREADY_EXISTS) { success = false; }
                }
            }
            #elif defined(__linux__) || defined(__APPLE__)
            {
                if (mkdir(path, 0777) != 0)
                {
                    if (errno != EEXIST) { success = false; }
                }
                success = mkdir(path, 0755) == 0 || errno == EEXIST;
            }
            #endif
            path[i] = originalChar;
        }
    }

    return success;
}

bool brahma_copy_file(const char* srcPath, const char* dstPath)
{
    FILE* srcFile = fopen(srcPath, "rb");
    if (!srcFile) return false;

    FILE* dstFile = fopen(dstPath, "wb");
    if (!dstFile)
    {
        fclose(srcFile);
        return false;
    }

    bool success = true;
    char buffer[8192]; // 8 KiB buffer for copying
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, dstFile) != bytesRead)
        {
            success = false;
            break;
        }
    }

    fclose(srcFile);
    fclose(dstFile);

    return success;
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

bool brahma_gather_files_by_extension_visitor(void* rawPayload, const char* path, bool isDirectory, bool* exploreCurrentDirectory)
{
    Brahma_Gather_Files_By_Extension_Payload* payload = (Brahma_Gather_Files_By_Extension_Payload*) rawPayload;

    if (!isDirectory)
    {
        size_t pathLen = strlen(path);
        size_t extLen  = strlen(payload->extension);

        // compare in reverse, use case-insensitivity for the extension part
        bool match = true;
        if (pathLen < extLen) match = false;
        else
        {
            for (size_t i = 0; i < extLen; i++)
            {
                char c1 = path[pathLen - extLen + i];
                char c2 = payload->extension[i];

                // convert to lowercase if it's an uppercase letter
                if (c1 >= 'A' && c1 <= 'Z') c1 += ('a' - 'A');
                if (c2 >= 'A' && c2 <= 'Z') c2 += ('a' - 'A');

                if (c1 != c2)
                {
                    match = false;
                    break;
                }
            }
        }

        if (match)
        {
            brahma_append_string_to_paged_list(payload->outFilePaths, (char*) path);
        }
    }

    return true; // continue iterating
}

int brahma_find_package_by_name(const Brahma_Package_Array_List* packages, const char* name)
{
    int selectedPkgIdx = -1;
    for (int i = 0; i < (int) packages->count; i++)
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
    for (int i = 0; i < (int) libraries->count; i++)
    {
        if (!strcmp(name, libraries->data[i].name))
        {
            selectedLibIdx = i;
            break;
        }
    }

    return selectedLibIdx;
}

typedef struct Brahma_Process_Impl
{
#if defined(_WIN32)
    HANDLE processHandle;
    HANDLE stdoutReadPipe;
#elif defined(__linux__) || defined(__APPLE__)
    pid_t  pid;
    int    stdoutReadFd;
#else
    void* _; // dummy on empty
#endif
} Brahma_Process_Impl;

Brahma_Process brahma_start_process(Brahma_String_Array_List args, const char* workingDir)
{
    #if defined(_WIN32)
    {
        // build a single string the way CreateProcessA expects it
        char* cmdLine = (char*) malloc(32768); // create process limit
        {
            char* cursor  = cmdLine;
            char* cmdEnd  = cmdLine + 32768 - 1;

            for (int i = 0; i < (int) args.count; i++)
            {
                const char* arg = args.data[i];

                if (i != 0 && cursor < cmdEnd) *cursor++ = ' ';

                // wrap every argument in double-quotes and escape embedded quotes
                if (cursor < cmdEnd) *cursor++ = '"';
                for (const char* c = arg; *c && cursor < cmdEnd; c++)
                {
                    if (*c == '"' && cursor < cmdEnd) *cursor++ = '\\';
                    if (cursor < cmdEnd) *cursor++ = *c;
                }
                if (cursor < cmdEnd) *cursor++ = '"';
            }
            *cursor = '\0';
        }

        SECURITY_ATTRIBUTES sa;
        sa.nLength              = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle       = TRUE; // pipe handles must be inheritable

        HANDLE pipeRead  = NULL;
        HANDLE pipeWrite = NULL;
        if (!CreatePipe(&pipeRead, &pipeWrite, &sa, 0))
        {
            #if defined(_MSC_VER)
            {
                __debugbreak();
            }
            #else
            {
                __builtin_trap();
            }
            #endif
        }

        // don't inherit parent's read end to child
        SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si;
        memset(&si, 0, sizeof(si));
        si.cb          = sizeof(si);
        si.hStdOutput  = pipeWrite;
        si.hStdError   = pipeWrite; // mmerge stderr into same pipe
        si.hStdInput   = GetStdHandle(STD_INPUT_HANDLE);
        si.dwFlags    |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi;
        memset(&pi, 0, sizeof(pi));

        BOOL ok = CreateProcessA(
            NULL, // app name
            cmdLine,
            NULL, // process security attrs
            NULL, // thread security attrs
            TRUE, // inherit handles
            0,    // creation flags
            NULL, // inherit parent environment
            workingDir,
            &si,
            &pi
        );

        CloseHandle(pipeWrite);
        free(cmdLine);

        if (!ok) { return NULL; }

        CloseHandle(pi.hThread); // we won't be waiting on the thread, only the process

        Brahma_Process proc = BRAHMA_PUSH_STRUCT(Brahma_Process_Impl);
        proc->processHandle  = pi.hProcess;
        proc->stdoutReadPipe = pipeRead;
        return proc;
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        int pipefd[2];
        if (pipe(pipefd) == -1) { __builtin_trap(); }

        pid_t pid = fork();

        if (pid == -1) { __builtin_trap(); }
        else if (pid == 0)
        {
            // child process
            close(pipefd[0]); // close read end

            dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
            dup2(pipefd[1], STDERR_FILENO); // redirect stderr to pipe
            close(pipefd[1]); // close original write end

            char** argv = (char**) malloc((args.count + 1) * sizeof(char*));
            for (size_t i = 0; i < args.count; i++)
            {
                argv[i] = (char*) args.data[i];
            }
            argv[args.count] = NULL;

            if (workingDir) ((void) chdir(workingDir));
            execvp(argv[0], argv);

            // if execvp returns, it means it failed
            _exit(127);
        }
        else
        {
            // parent process
            close(pipefd[1]); // close write end

            Brahma_Process proc = BRAHMA_PUSH_STRUCT(Brahma_Process_Impl);
            proc->pid = pid;
            proc->stdoutReadFd = pipefd[0];
            return proc;
        }
    }
    #endif
}

int brahma_wait_for_process(Brahma_Process proc, char** outStdOut)
{
    // accumulate chunks into a resizable buffer, backed by plain malloc
    // then copy finishhed data into the arena at the end
    // avoid polluting arena with intermediate scratch space

    size_t capturedSize     = 0;
    size_t capturedCapacity = 0;
    char*  capturedBuffer   = NULL;
    int    outExitCode      = 0;

    // drain the pipe concurrently with the child running, otherwise
    // child blocks when pipe buffer fills up and we deadlock waiting on its exit
    // in case the caller doesn't want the output, still drain the pipe, but
    // keep ovverwriting to a dummy buffer instead of accumulating, to save memory and time
    while (true)
    {
        // grow the buffer if needed
        if (outStdOut && (!capturedCapacity || capturedCapacity < (capturedSize + 512)))
        {
            size_t newCapacity = capturedCapacity ? capturedCapacity * 2 : 16384;
            char*  newBuffer   = (char*) malloc(newCapacity);
            memcpy(newBuffer, capturedBuffer, capturedSize);
            free(capturedBuffer);
            capturedBuffer   = newBuffer;
            capturedCapacity = newCapacity;
        }

        char dummyBuffer[512];
        char* bufferToUse = outStdOut ? (capturedBuffer + capturedSize) : &(dummyBuffer[0]);
        size_t bufferCapacity = outStdOut ? (capturedCapacity - capturedSize) : sizeof(dummyBuffer);

        size_t bytesRead = 0;

        #if defined(_WIN32)
        {
            DWORD bytesReadTemp = 0;
            BOOL ok = ReadFile(proc->stdoutReadPipe, bufferToUse, (DWORD) bufferCapacity, &bytesReadTemp, NULL);
            if (!ok || bytesReadTemp == 0) break; // pipe closed or error
            bytesRead = (size_t) bytesReadTemp;
        }
        #elif defined(__linux__) || defined(__APPLE__)
        {
            ssize_t bytesReadTemp = read(proc->stdoutReadFd, bufferToUse, bufferCapacity);
            if (bytesReadTemp <= 0) break; // pipe closed or error
            bytesRead = (size_t) bytesReadTemp;
        }
        #endif

        if (outStdOut) capturedSize += bytesRead;
    }

    #if defined(_WIN32)
    {
        CloseHandle(proc->stdoutReadPipe);

        WaitForSingleObject(proc->processHandle, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(proc->processHandle, &exitCode);
        outExitCode = (int) exitCode;

        CloseHandle(proc->processHandle);
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
        close(proc->stdoutReadFd);

        int status = 0;
        waitpid(proc->pid, &status, 0);

        if      (WIFEXITED(status))   outExitCode = WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) outExitCode = -(int) WTERMSIG(status);
        else                          outExitCode = -1;
    }
    #endif

    // copy data to internal allocator and null-terminate it
    if (outStdOut)
    {
        *outStdOut = (char*) brahma_push_memory(capturedSize + 1, 1);
        if (capturedSize) memcpy(*outStdOut, capturedBuffer, capturedSize);
        (*outStdOut)[capturedSize] = '\0';
    }

    free(capturedBuffer);

    return outExitCode;
}

#endif//defined(BRAHMA_LIBRARY_IMPL) || defined(BRAHMA_EXEC)

// =============================================================================================================================
// Execution code (used when CLI is executed).
#if defined(BRAHMA_EXEC)

size_t brahma_get_package_count(void);
size_t brahma_get_library_count(void);
void brahma_create_all_packages(Brahma_Platform platform, Brahma_Architecture architecture, bool includeDebugInfo, Brahma_Package_Array_List* packages);
void brahma_create_all_libraries(Brahma_Library_Array_List* libraries, const Brahma_Package* package);

void brahma_exec_log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[])
{
    #if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
    setvbuf(stdout, NULL, _IONBF, 0); // unbuffered stdout for better interleaving of logs and subprocess output
    #endif

    Brahma_Args ex;
    ex.flags           = BRAHMA_ARGS_FLAG_NONE;
    ex.platform        = BRAHMA_PLATFORM_UNKNOWN;
    ex.architecture    = BRAHMA_ARCHITECTURE_UNKNOWN;
    ex.packageToBuild  = NULL;
    ex.pkgCount        = brahma_get_package_count();
    ex.libCount        = brahma_get_library_count();
    ex.log             = brahma_exec_log;
    ex.createPackages  = brahma_create_all_packages;
    ex.createLibraries = brahma_create_all_libraries;

    ex.intermediateOutputDir = NULL;
    ex.outputDir             = NULL;
    ex.clangdOutputDir       = NULL;

    ex.flags |= BRAHMA_ARGS_FLAG_DEBUG; // default to debug mode

    #if 0 // uncomment to debug argument parsing
    for (int i = 0; i < argc; i++) { printf("arg %d: %s\n", i, argv[i]); }
    #endif

    for (int i = 1; i < argc; i++) // skipping first arg because it's gonna be the executable name
    {
        // intermediate stuff - not relevant once this tool has begun executing
        if (!strcmp("-cxx",              argv[i])) { i++; continue; } // make build tool in cxx mode
        if (!strcmp("-lib_search_dir",   argv[i])) { i++; continue; } // library search dirs
        if (!strcmp("-debug_build_tool", argv[i])) {      continue; } // whether the build tool itself is a debug build

        // flags
        if (!strcmp("-nodebuginfo", argv[i])) { ex.flags &= (~BRAHMA_ARGS_FLAG_DEBUG & UINT16_MAX); continue; }
        if (!strcmp("-optimised",   argv[i])) { ex.flags |= BRAHMA_ARGS_FLAG_OPTIMISED;             continue; }
        if (!strcmp("-warnings",    argv[i])) { ex.flags |= BRAHMA_ARGS_FLAG_SHOW_WARNINGS;         continue; }
        if (!strcmp("-warn_as_err", argv[i])) { ex.flags |= BRAHMA_ARGS_FLAG_WARNINGS_ARE_ERRORS;   continue; }

        if (!strcmp("-clangd", argv[i]))
        {
            if (ex.clangdOutputDir)
            {
                ex.log(BRAHMA_LOG_ERROR "Multiple clangd output directories specified with -clangd. Use as: *.exe -clangd clangdOutputDirectory.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log(BRAHMA_LOG_ERROR "No clangd output directory specified after -clangd. Use as: *.exe -clangd clangdOutputDirectory.\n");
                return 1;
            }

            ex.clangdOutputDir = argv[i];
            continue;
        }

        if (!strcmp("-package", argv[i]))
        {
            if (ex.packageToBuild)
            {
                ex.log(BRAHMA_LOG_ERROR "Multiple packages specified with -package. Use as: *.exe -package packageName.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log(BRAHMA_LOG_ERROR "No package specified after -package. Use as: *.exe -package packageName.\n");
                return 1;
            }

            ex.packageToBuild = argv[i];
            continue;
        }

        if (!strcmp("-out", argv[i]))
        {
            if (ex.outputDir)
            {
                ex.log(BRAHMA_LOG_ERROR "Multiple output directories specified with -output_dir. Use as: *.exe -output_dir outputDirectory.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log(BRAHMA_LOG_ERROR "No output directory specified after -output_dir. Use as: *.exe -output_dir outputDirectory.\n");
                return 1;
            }

            ex.outputDir = argv[i];
            continue;
        }

        if (!strcmp("-intermediate_output", argv[i]))
        {
            if (ex.intermediateOutputDir)
            {
                ex.log(BRAHMA_LOG_ERROR "Multiple intermediate output directories specified with -intermediate_output. Use as: *.exe -intermediate_output intermediateOutputDirectory.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log(BRAHMA_LOG_ERROR "No intermediate output directory specified after -intermediate_output. Use as: *.exe -intermediate_output intermediateOutputDirectory.\n");
                return 1;
            }

            ex.intermediateOutputDir = argv[i];
            continue;
        }

        if (!strcmp("-platform", argv[i]))
        {
            if (ex.platform)
            {
                ex.log(BRAHMA_LOG_ERROR "Multiple platforms specified with -platform. Use as: *.exe -platform platformName.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log(BRAHMA_LOG_ERROR "No platform specified after -platform. Use as: *.exe -platform platformName.\n");
                return 1;
            }

            const char* platformStr = argv[i];
            if (false) { }
            else if (!strcmp(platformStr, BRAHMA_PLATFORM_NAMES[BRAHMA_PLATFORM_WINDOWS])) ex.platform = BRAHMA_PLATFORM_WINDOWS;
            else if (!strcmp(platformStr, BRAHMA_PLATFORM_NAMES[BRAHMA_PLATFORM_LINUX]))   ex.platform = BRAHMA_PLATFORM_LINUX;
            else if (!strcmp(platformStr, BRAHMA_PLATFORM_NAMES[BRAHMA_PLATFORM_OSX]))     ex.platform = BRAHMA_PLATFORM_OSX;
            else if (!strcmp(platformStr, BRAHMA_PLATFORM_NAMES[BRAHMA_PLATFORM_ANDROID])) ex.platform = BRAHMA_PLATFORM_ANDROID;
            else if (!strcmp(platformStr, BRAHMA_PLATFORM_NAMES[BRAHMA_PLATFORM_IOS]))     ex.platform = BRAHMA_PLATFORM_IOS;
            else
            {
                ex.log(BRAHMA_LOG_ERROR "Unknown platform '%s'. Supported platforms are:\n", platformStr);
                for (int j = 0; j < BRAHMA_PLATFORM__COUNT; j++)
                {
                    ex.log(BRAHMA_LOG_INFO "  - %s\n", BRAHMA_PLATFORM_NAMES[j]);
                }
                return 1;
            }
            continue;
        }

        if (!strcmp("-arch", argv[i]))
        {
            if (ex.architecture)
            {
                ex.log(BRAHMA_LOG_ERROR "Multiple architectures specified with -arch. Use as: *.exe -arch architectureName.\n");
                return 1;
            }

            if (++i >= argc)
            {
                ex.log(BRAHMA_LOG_ERROR "No architecture specified after -arch. Use as: *.exe -arch architectureName.\n");
                return 1;
            }

            const char* archStr = argv[i];
            if (false) { }
            else if (!strcmp(archStr, BRAHMA_ARCHITECTURE_NAMES[BRAHMA_ARCHITECTURE_X86]))   ex.architecture = BRAHMA_ARCHITECTURE_X86;
            else if (!strcmp(archStr, BRAHMA_ARCHITECTURE_NAMES[BRAHMA_ARCHITECTURE_X64]))   ex.architecture = BRAHMA_ARCHITECTURE_X64;
            else if (!strcmp(archStr, BRAHMA_ARCHITECTURE_NAMES[BRAHMA_ARCHITECTURE_ARM32])) ex.architecture = BRAHMA_ARCHITECTURE_ARM32;
            else if (!strcmp(archStr, BRAHMA_ARCHITECTURE_NAMES[BRAHMA_ARCHITECTURE_ARM64])) ex.architecture = BRAHMA_ARCHITECTURE_ARM64;
            else
            {
                ex.log(BRAHMA_LOG_ERROR "Unknown architecture '%s'. Supported architectures are:\n", archStr);
                for (int j = 0; j < BRAHMA_ARCHITECTURE__COUNT; j++)
                {
                    ex.log(BRAHMA_LOG_INFO "  - %s\n", BRAHMA_ARCHITECTURE_NAMES[j]);
                }
                return 1;
            }
            continue;
        }

        // unknown arg
        ex.log(BRAHMA_LOG_ERROR "Unknown argument '%s'.\n", argv[i]);
        return 1;
    }

    bool success = brahma_execute(ex);
    return success ? 0 : 1;
}

#ifdef __cplusplus
extern "C" {
#endif

#define BRAHMA_BEGIN_LISTING_PACKAGES() \
    void brahma_create_all_packages(Brahma_Platform platform, Brahma_Architecture architecture, bool includeDebugInfo, Brahma_Package_Array_List* packages) { \
        Brahma_Package currentPackage; \
        (void) currentPackage;

#define BRAHMA_END_LISTING_PACKAGES() \
    }

#define BRAHMA_BEGIN_LISTING_LIBRARIES() \
    void brahma_create_all_libraries(Brahma_Library_Array_List* libraries, const Brahma_Package* package) { \
        Brahma_Library currentLibrary; \
        (void) currentLibrary;

#define BRAHMA_END_LISTING_LIBRARIES() \
    }

#define BRAHMA_PACKAGE_COUNT(x) \
    size_t brahma_get_package_count(void) { return x; }

#define BRAHMA_LIBRARY_COUNT(x) \
    size_t brahma_get_library_count(void) { return x; }

#define BRAHMA_ADD_PACKAGE(owningFile_, packageName) \
    memset(&currentPackage, 0, sizeof(currentPackage)); \
    currentPackage.name = #packageName; \
    currentPackage.owningFile = owningFile_; \
    currentPackage.platform = platform; \
    currentPackage.architecture = architecture; \
    currentPackage.includeDebugInfo = includeDebugInfo; \
    brahma_implement_package_##packageName(&currentPackage); \
    brahma_append_package_to_array_list(packages, currentPackage);

#define BRAHMA_ADD_LIBRARY(owningDir_, owningFile_, libraryName) \
    memset(&currentLibrary, 0, sizeof(currentLibrary)); \
    currentLibrary.name = #libraryName; \
    currentLibrary.owningFile = owningFile_; \
    currentLibrary.owningDir = owningDir_; \
    brahma_implement_library_##libraryName(package, &currentLibrary); \
    brahma_append_library_to_array_list(libraries, currentLibrary);

#endif//defined(BRAHMA_EXEC)

#ifdef __cplusplus
} // extern "C"
#endif

#endif//BRAHMA_H
