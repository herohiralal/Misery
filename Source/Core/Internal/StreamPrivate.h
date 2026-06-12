#pragma once
#include "CorePrivate.h"

typedef union
{
    rawptr asPtr;

    #if MSR_WINDOWS
        HANDLE handle;
    #elif MSR_UNIX
        i32 handle;
    #else
        #error "unimplemented"
    #endif
} IO_Internal_FileStreamData;

static_assert( sizeof(IO_Internal_FileStreamData) ==  sizeof(rawptr), "IO_Internal_FileStreamData must be the same size as rawptr.");
static_assert(alignof(IO_Internal_FileStreamData) == alignof(rawptr), "IO_Internal_FileStreamData must have the same alignment as rawptr.");

#if MSR_WINDOWS
    #define IO_Internal_InvalidFileStreamData ((HANDLE) nil)
#elif MSR_UNIX
    #define IO_Internal_InvalidFileStreamData ((i32) -1)
#else
    #error "unimplemented"
#endif
