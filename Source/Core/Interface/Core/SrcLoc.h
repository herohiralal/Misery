#pragma once
#include <__init.h>

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
