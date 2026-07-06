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
 * Normalises the given path and returns it as a FIL_Path.
 * The newly created path will be allocated using the provided allocator.
 */
FIL_Path FIL_Normalise(utf8str, MEM_Allocator);

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
