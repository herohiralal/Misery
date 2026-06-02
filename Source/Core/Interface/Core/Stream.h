#pragma once
#include <__init.h>
#include "Strings.h"

EXTERN_C_BEGIN

// stream ----------------------------------------------------------------------------------------------------------------------

/**
 * Defines the mode to be used when calling the stream function.
 */
typedef u8 IO_StreamMode;
enum IO_StreamModes
{
    IO_StreamMode_GetSize,
    IO_StreamMode_GetCurrentPosition,
    IO_StreamMode_SeekAbsolute,
    IO_StreamMode_SeekRelative,
    IO_StreamMode_Read,
    IO_StreamMode_Write,
    IO_StreamMode_TruncateAtCursor,
    IO_StreamMode_TruncateToSize,
    IO_StreamMode_Flush,
    IO_StreamMode_Close,
};

/**
 * Defines the delegate type for the stream function.
 */
typedef isize (*IO_StreamProc)(IO_StreamMode, rawptr data, isize position, Slice_(u8) buffer);

/**
 * Defines a generic input/output stream that can be used to read/write data from/to various sources.
 */
typedef struct
{
    IO_StreamProc procedure;
    rawptr data; // Optional data for the stream function
} IO_Stream;

// stream functions ------------------------------------------------------------------------------------------------------------

isize IO_GetSize(IO_Stream);

isize IO_GetCurrentPosition(IO_Stream);

b8 IO_Seek(IO_Stream, isize position, b8 relative OPT_ARG);

isize IO_Read(IO_Stream, Slice_(u8) dst);

Slice_(u8) IO_ReadAll(IO_Stream, MEM_Allocator, b8 keepOpen OPT_ARG);

isize IO_Write(IO_Stream, Slice_(u8) src);

b8 IO_TruncateAtCursor(IO_Stream);

b8 IO_TruncateToSize(IO_Stream, isize newSize);

b8 IO_Flush(IO_Stream);

void IO_Close(IO_Stream);

// stream implementations ------------------------------------------------------------------------------------------------------

IO_Stream IO_OpenFileToRead(utf8str path, b8 allowWrite OPT_ARG);

IO_Stream IO_OpenFileToWrite(utf8str path, b8 append OPT_ARG, b8 allowRead OPT_ARG);

EXTERN_C_END
