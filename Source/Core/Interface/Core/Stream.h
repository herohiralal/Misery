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

/**
 * Get the size of the stream in bytes. Returns -1 if the stream is null or if an error occurs.
 */
isize IO_GetSize(IO_Stream);

/**
 * Get the current position in the stream in bytes. Returns -1 if the stream is null or if an error occurs.
 */
isize IO_GetCurrentPosition(IO_Stream);

/**
 * Seek to a specific position in the stream. If relative is false, the position is absolute from the
 * start of the stream. If relative is true, the position is relative to the current position.
 * Returns true on success, false on failure or if the stream is null.
 */
b8 IO_Seek(IO_Stream, isize position, b8 relative OPT_ARG);

/**
 * Read data from the stream into the provided buffer. The buffer is specified as a Slice of bytes,
 * which contains a pointer to the data and the size of the buffer. Returns the number of bytes read, or 0 on failure
 * or if the stream is null.
 */
isize IO_Read(IO_Stream, Slice_(u8) dst);

/**
 * Read the entire contents of the stream into a newly allocated buffer, and return it as a Slice of bytes. The memory
 * for the buffer is allocated using the provided allocator. Returns an empty slice on failure or if the stream is null.
 */
Slice_(u8) IO_ReadAll(IO_Stream, MEM_Allocator, b8 keepOpen OPT_ARG);

/**
 * Write data to the stream from the provided buffer. The buffer is specified as a Slice of bytes, which
 * contains a pointer to the data and the size of the buffer. Returns the number of bytes written, or 0 on failure
 * or if the stream is null.
 */
isize IO_Write(IO_Stream, Slice_(u8) src);

/**
 * Truncate the stream at the current position. Returns true on success, false on failure or if the stream is null.
 */
b8 IO_TruncateAtCursor(IO_Stream);

/**
 * Truncate the stream to a specific size. Returns true on success, false on failure or if the stream is null.
 */
b8 IO_TruncateToSize(IO_Stream, isize newSize);

/**
 * Flush any buffered data to the stream. Returns true on success, false on failure or if the stream is null.
 */
b8 IO_Flush(IO_Stream);

/**
 * Close the stream and release any associated resources. After calling this function, the stream should not
 * be used again. Returns without doing anything if the stream is null.
 */
void IO_Close(IO_Stream);

// stream implementations ------------------------------------------------------------------------------------------------------

/**
 * Open a file for reading and return it as a stream. If allowWrite is true, the file will be opened with write
 * permissions as well. Returns an empty stream if the file cannot be opened.
 */
IO_Stream IO_OpenFileToRead(utf8str path, b8 allowWrite OPT_ARG);

/**
 * Open a file for writing and return it as a stream. If append is true, the file will be opened in append mode
 * (data will be written at the end of the file). * If allowRead is true, the file will be opened with read permissions
 * as well. Returns an empty stream if the file cannot be opened.
 */
IO_Stream IO_OpenFileToWrite(utf8str path, b8 append OPT_ARG, b8 allowRead OPT_ARG);

/**
 * Read the entire contents of the file at the given path into a newly allocated buffer, and return it as a Slice of bytes.
 * The memory for the buffer is allocated using the provided allocator. Returns an empty slice on failure or if
 * the file cannot be opened.
 */
Slice_(u8) IO_ReadEntireFile(utf8str path, MEM_Allocator);

/**
 * Write the provided data to the file at the given path. If append is true, the file will be opened in append mode
 * (data will be written at the end of the file). Returns true on success, false on failure or if the file cannot be opened.
 */
b8 IO_WriteAllToFile(utf8str path, Slice_(u8) data, b8 append OPT_ARG);

EXTERN_C_END
