#pragma once
#include <__init.h>
#include "Strings.h"
#include "Time.h"
#include "FilePath.h"

EXTERN_C_BEGIN

/**
 * Represents a directory path.
 * The path is always absolute, normalised (e.g. no "." or ".." components, no redundant separators), uses
 * forward slashes as separators, and always ends with a slash.
 * Always contains a trailing slash, even for the root directory (which is just "/").
 */
typedef struct
{
    utf8str path;
} DIR_Path;

/**
 * Represents a child path inside a directory, which can be either a file or a subdirectory.
 */
typedef enum { DIR_PathTy_File, DIR_PathTy_Directory } DIR_PathTy;

/**
 * Represents a child path inside a directory, which can be either a file or a subdirectory,
 * and includes some type info.
 */
typedef struct
{
    DIR_PathTy ty;

    union
    {
        FIL_Path fil;
        DIR_Path dir;
    } path;
} DIR_ChildPath;

/**
 * Signature for the callback function used in DIR_Iterate.
 * The callback is called for each child path inside the directory being iterated.
 * The callback can return false to stop iterating further, or true to continue.
 * If the child path is a directory, the callback can set *exploreCurrentDirectory to
 * `true` or `false` to indicate whether the contents of that directory should be iterated
 * as well (regardless of whether recursive iteration was requested).
 */
typedef b8 (*DIR_IteratorProc)(DIR_ChildPath childPath, rawptr userData, b8* exploreCurrentDirectory);

/**
 * Normalises the given path and returns it as a DIR_Path.
 * The newly created path will be allocated using the provided allocator.
 */
DIR_Path DIR_Normalise(utf8str, MEM_Allocator);

/**
 * Returns the parent directory of the given directory path.
 * The returned path will be a slice of the original path, so no new allocation is performed.
 */
DIR_Path DIR_DirPathParent(DIR_Path);

/**
 * Returns the parent directory of the given file path.
 * The returned path will be a slice of the original path, so no new allocation is performed.
 */
DIR_Path DIR_FilPathParent(FIL_Path);

#ifdef __cplusplus
    EXTERN_C_END
    static DIR_Path DIR_ParentInternal(DIR_Path p) { return DIR_DirPathParent(p); }
    static DIR_Path DIR_ParentInternal(FIL_Path p) { return DIR_FilPathParent(p); }
    EXTERN_C_BEGIN
#else
    #define DIR_ParentInternal(p) \
        _Generic((p), DIR_Path: DIR_DirPathParent, FIL_Path: DIR_FilPathParent)(p)
#endif

/**
 * Returns the parent directory of the given path, which can be either a directory or a file.
 * The returned path will be a slice of the original path, so no new allocation is performed.
 */
#define DIR_Parent(p) DIR_ParentInternal(p)

/**
 * Returns a new DIR_Path representing a subdirectory with the given name inside the parent directory.
 * The returned path will be allocated using the provided allocator.
 */
DIR_Path DIR_DirectoryInside(DIR_Path parent, utf8str childName, MEM_Allocator);

/**
 * Returns a new FIL_Path representing a file with the given name inside the parent directory.
 * The returned path will be allocated using the provided allocator.
 */
FIL_Path DIR_FileInside(DIR_Path parent, utf8str childName, MEM_Allocator);

/**
 * Iterates over the child paths inside the given directory, calling the provided callback for each child.
 * If `recursive` is true, subdirectories will be iterated as well (performing a depth-first traversal).
 * The `userData` parameter is passed through to the callback and can be used to maintain state across calls.
 */
void DIR_Iterate(DIR_Path, DIR_IteratorProc, rawptr userData OPT_ARG, b8 recursive OPT_ARG);

/**
 * Checks if the given directory path exists in the file system.
 */
b8 DIR_Exists(DIR_Path);

/**
 * Ensures that the given directory path exists in the file system, creating it if necessary.
 */
b8 DIR_Ensure(DIR_Path);

/**
 * Deletes the given directory path from the file system, along with all of its contents if it is not empty.
 */
b8 DIR_Delete(DIR_Path);

/**
 * Opaque handle for a directory watcher.
 */
typedef struct
{
    rawptr internal;
} DIR_Watcher;

/**
 * The type of a directory watcher event.
 * Rename-like changes are intentionally represented as a removal and addition pair.
 */
typedef u8 DIR_WatchEvtTy;
enum DIR_WatchEvtTys
{
    DIR_WatchEvtTy_Added,
    DIR_WatchEvtTy_Removed,
    DIR_WatchEvtTy_Modified,
};

/**
 * An event produced by DIR_PollWatcher.
 */
typedef struct
{
    DIR_WatchEvtTy ty;
    DIR_ChildPath  path;
} DIR_WatchEvt;

COL_DECLARE_FOR(DIR_WatchEvt);

/**
 * Creates a polling-based watcher for the specified directory.
 * Returns nil on failure.
 */
DIR_Watcher DIR_CreateWatcher(DIR_Path path, MEM_Allocator allocator);

/**
 * Destroys a watcher and releases all associated resources.
 */
void DIR_DestroyWatcher(DIR_Watcher watcher);

/**
 * Polls the watcher for any events that have occurred since the last poll.
 * Returns true if an event was available, false otherwise.
 * Use as:
```
DIR_WatchEvt evt;
while (DIR_IterateWatchEvts(watcher, &evt))
{
    // handle evt
}
```
 */
b8 DIR_IterateWatchEvts(DIR_Watcher watcher, DIR_WatchEvt* outEvt);

EXTERN_C_END
