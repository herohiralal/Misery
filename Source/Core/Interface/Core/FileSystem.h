#pragma once
#include <__init.h>
#include "Strings.h"
#include "Time.h"

EXTERN_C_BEGIN

/**
 * Represents a file path.
 * The path is always absolute, normalised (e.g. no "." or ".." components, no redundant separators), and uses
 * forward slashes as separators.
 */
typedef struct
{
    utf8str path;
} FIL_Path;

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
DIR_Path DIR_Parent(DIR_Path);

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
 * Normalises the given path and returns it as a FIL_Path.
 * The newly created path will be allocated using the provided allocator.
 */
FIL_Path FIL_Normalise(utf8str, MEM_Allocator);

/**
 * Returns the parent directory of the given file path.
 * The returned path will be a slice of the original path, so no new allocation is performed.
 */
DIR_Path FIL_Parent(FIL_Path);

/**
 * Returns the name of the file without the extension.
 * For example, for the path "/foo/bar/baz.txt", this function would return "baz".
 * The returned name will be a slice of the original path, so no new allocation is performed.
 */
utf8str FIL_Name(FIL_Path);

/**
 * Returns the extension of the file, without the dot.
 * For example, for the path "/foo/bar/baz.txt", this function would return "txt".
 * If the file has no extension, an empty string is returned.
 * The returned extension will be a slice of the original path, so no new allocation is performed.
 */
utf8str FIL_Extension(FIL_Path);

/**
 * Returns the name of the file with the extension.
 * For example, for the path "/foo/bar/baz.txt", this function would return "baz.txt".
 * The returned name will be a slice of the original path, so no new allocation is performed.
 */
utf8str FIL_NameWithExtension(FIL_Path);

/**
 * Returns the last modified time of the file at the given path.
 * If the file does not exist, a zero-value is returned.
 * (What are the chances that a file was last modified _exactly_ at the Unix Epoch?)
 */
TIM_Value FIL_LastModified(FIL_Path);

/**
 * Checks if the given file path exists in the file system.
 */
b8 FIL_Exists(FIL_Path);

/**
 * Deletes the file at the given path from the file system.
 */
b8 FIL_Delete(FIL_Path);

EXTERN_C_END
