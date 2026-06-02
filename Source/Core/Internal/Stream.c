#include "Core/Stream.h"
#include "Core/Strings.h"
#include "__init.h"
#include <Core/Core.h>

isize IO_GetSize(IO_Stream stream)
{
    if (!stream.procedure)
        return -1;

    return stream.procedure(IO_StreamMode_GetSize, stream.data, 0, (Slice_(u8)) {0});
}

isize IO_GetCurrentPosition(IO_Stream stream)
{
    if (!stream.procedure)
        return -1;

    return stream.procedure(IO_StreamMode_GetCurrentPosition, stream.data, 0, (Slice_(u8)) {0});
}

b8 IO_Seek(IO_Stream stream, isize position, b8 relative OPT_ARG)
{
    if (!stream.procedure)
        return false;

    IO_StreamMode mode = relative ? IO_StreamMode_SeekRelative : IO_StreamMode_SeekAbsolute;
    return !!stream.procedure(mode, stream.data, position, (Slice_(u8)) {0});
}

isize IO_Read(IO_Stream stream, Slice_(u8) dst)
{
    if (!stream.procedure)
        return -1;

    return stream.procedure(IO_StreamMode_Read, stream.data, 0, dst);
}

Slice_(u8) IO_ReadAll(IO_Stream stream, MEM_Allocator allocator, b8 keepOpen OPT_ARG)
{
    if (!stream.procedure)
        return (Slice_(u8)) {0};

    isize size = IO_GetSize(stream);
    if (size <= 0)
        return (Slice_(u8)) {0};

    Slice_(u8) buffer = COL_NewSlice(u8, size, true, allocator);

    isize bytesRead = IO_Read(stream, buffer);
    if (bytesRead != size)
    {
        COL_DeleteSlice(&buffer, allocator);
        return (Slice_(u8)) {0};
    }

    if (!keepOpen)
        IO_Close(stream);

    return buffer;
}

isize IO_Write(IO_Stream stream, Slice_(u8) src)
{
    if (!stream.procedure)
        return -1;

    return stream.procedure(IO_StreamMode_Write, stream.data, 0, src);
}

b8 IO_TruncateAtCursor(IO_Stream stream)
{
    if (!stream.procedure)
        return false;

    return !!stream.procedure(IO_StreamMode_TruncateAtCursor, stream.data, 0, (Slice_(u8)) {0});
}

b8 IO_TruncateToSize(IO_Stream stream, isize newSize)
{
    if (!stream.procedure)
        return false;

    return !!stream.procedure(IO_StreamMode_TruncateToSize, stream.data, newSize, (Slice_(u8)) {0});
}

b8 IO_Flush(IO_Stream stream)
{
    if (!stream.procedure)
        return false;

    return !!stream.procedure(IO_StreamMode_Flush, stream.data, 0, (Slice_(u8)) {0});
}

void IO_Close(IO_Stream stream)
{
    if (!stream.procedure)
        return;

    stream.procedure(IO_StreamMode_Close, stream.data, 0, (Slice_(u8)) {0});
}

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

static isize IO_Internal_FileStreamProc(IO_StreamMode, rawptr data, isize position, Slice_(u8) buffer);

IO_Stream IO_OpenFileToRead(utf8str path, b8 allowWrite)
{
    IO_Internal_FileStreamData streamData = {.handle = IO_Internal_InvalidFileStreamData};
    cstring cPath = STR_CloneToCStr(path, MEM_temp);

    if (cPath)
    {
        #if MSR_WINDOWS
        {
            streamData.handle = CreateFileA(
                cPath,
                GENERIC_READ | (allowWrite ? GENERIC_WRITE : 0),
                FILE_SHARE_READ | (allowWrite ? FILE_SHARE_WRITE : 0),
                nil,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nil
            );
        }
        #elif MSR_UNIX
        {
            stream.handle = open(cPath, allowWrite ? O_RDWR : O_RDONLY);
        }
        #else
            #error "unimplemented"
        #endif
    }

    return (IO_Stream) {.procedure = IO_Internal_FileStreamProc, .data = streamData.asPtr};
}

IO_Stream IO_OpenFileToWrite(utf8str path, b8 append, b8 allowRead)
{
    IO_Internal_FileStreamData streamData = {.handle = IO_Internal_InvalidFileStreamData};
    cstring cPath = STR_CloneToCStr(path, MEM_temp);

    if (cPath)
    {
        #if MSR_WINDOWS
        {
            streamData.handle = CreateFileA(
                cPath,
                GENERIC_WRITE | (allowRead ? GENERIC_READ : 0),
                FILE_SHARE_WRITE | (allowRead ? FILE_SHARE_READ : 0),
                nil,
                append ? OPEN_ALWAYS : CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nil
            );

            if (append && streamData.handle != IO_Internal_InvalidFileStreamData)
            {
                // Move the file pointer to the end for appending
                SetFilePointer(streamData.handle, 0, nil, FILE_END);
            }
        }
        #elif MSR_UNIX
        {
            int flags = allowRead ? O_RDWR : O_WRONLY;
            flags |= O_CREAT;
            if (append) { flags |= O_APPEND; }
            else        { flags |= O_TRUNC;  }

            streamData.handle = open(cPath, flags, 0666);
        }
        #else
            #error "unimplemented"
        #endif
    }

    return (IO_Stream) {.procedure = IO_Internal_FileStreamProc, .data = streamData.asPtr};
}

static isize IO_Internal_FileStreamProc(IO_StreamMode mode, rawptr data, isize position, Slice_(u8) buffer)
{
    IO_Internal_FileStreamData streamData = {.asPtr = data};

    isize invalidOutput = -1;
    if (true ||
        mode == IO_StreamMode_SeekAbsolute ||
        mode == IO_StreamMode_SeekRelative ||
        mode == IO_StreamMode_TruncateAtCursor ||
        mode == IO_StreamMode_TruncateToSize ||
        mode == IO_StreamMode_Flush ||
        true)
    {
        invalidOutput = (isize) false;
    }

    if (streamData.handle == IO_Internal_InvalidFileStreamData)
    {
        return invalidOutput;
    }

    if (mode == IO_StreamMode_GetSize)
    {
        #if MSR_WINDOWS
        {
            LARGE_INTEGER out;
            if (!GetFileSizeEx(streamData.handle, &out)) { return -1; }
            return (isize) out.QuadPart;
        }
        #elif MSR_UNIX
        {
            struct stat st;
            if (fstat(streamData.handle, &st) != 0) { return -1; }
            return (isize) st.st_size;
        }
        #else
            #error "unimplemented"
        #endif
    }

    if (mode == IO_StreamMode_GetCurrentPosition)
    {
        #if MSR_WINDOWS
        {
            LARGE_INTEGER zero = {0};
            LARGE_INTEGER out;
            if (!SetFilePointerEx(streamData.handle, zero, &out, FILE_CURRENT)) { return -1; }
            return (isize) out.QuadPart;
        }
        #elif MSR_UNIX
        {
            off_t off = lseek(streamData.handle, 0, SEEK_CUR);
            if (off == (off_t) -1) { return -1; }
            return (isize) off;
        }
        #else
            #error "unimplemented"
        #endif
    }

    if (mode == IO_StreamMode_SeekAbsolute || mode == IO_StreamMode_SeekRelative)
    {
        b8 relative = (mode == IO_StreamMode_SeekRelative);

        #if MSR_WINDOWS
        {
            LARGE_INTEGER li = { };
            li.QuadPart = position;
            if (!SetFilePointerEx(streamData.handle, li, nil, relative ? FILE_CURRENT : FILE_BEGIN)) { return (isize) false; }
            return (isize) true;
        }
        #elif MSR_UNIX
        {
            off_t newPos = lseek(streamData.handle, (off_t) position, relative ? SEEK_CUR : SEEK_SET);
            return (isize) (b8) (newPos != (off_t) -1);
        }
        #else
            #error "unimplemented"
        #endif
    }

    if (mode == IO_StreamMode_TruncateAtCursor)
    {
        #if MSR_WINDOWS
        {
            LARGE_INTEGER zero = { };
            if (!SetFilePointerEx(streamData.handle, zero, nil, FILE_CURRENT)) { return (isize) false; }
            if (!SetEndOfFile(streamData.handle)) { return (isize) false; }
            return (isize) true;
        }
        #elif MSR_UNIX
        {
            off_t cur = lseek(streamData.handle, 0, SEEK_CUR);
            if (cur < 0) { return (isize) false; }
            return (isize) (b8) ftruncate(handle, cur) == 0;
        }
        #else
            #error "unimplemented"
        #endif
    }

    if (mode == IO_StreamMode_TruncateToSize)
    {
        #if MSR_WINDOWS
        {
            LARGE_INTEGER li = { };
            li.QuadPart = position;
            if (!SetFilePointerEx(streamData.handle, li, nil, FILE_BEGIN)) { return (isize) false; }
            if (!SetEndOfFile(streamData.handle)) { return (isize) false; }
            return (isize) true;
        }
        #elif MSR_UNIX
        {
            return (isize) (b8) ftruncate(handle, position) == 0;
        }
        #else
            #error "Unsupported platform"
        #endif
    }

    if (mode == IO_StreamMode_Flush)
    {
        #if MSR_WINDOWS
        {
            return (isize) (b8) (FlushFileBuffers(streamData.handle) != 0);
        }
        #elif MSR_UNIX
        {
            return (isize) (b8) (fsync(streamData.handle) == 0);
        }
        #else
            #error "Unsupported platform"
        #endif
    }

    if (mode == IO_StreamMode_Close)
    {
        #if MSR_WINDOWS
        {
            CloseHandle(streamData.handle);
        }
        #elif MSR_UNIX
        {
            close(streamData.handle);
        }
        #else
            #error "Unsupported platform"
        #endif
    }

    if (mode == IO_StreamMode_Read)
    {
        #if MSR_WINDOWS
        {
            DWORD bytesRead;
            if (!ReadFile(streamData.handle, buffer.data, (DWORD) buffer.count, &bytesRead, nil)) { return 0; }
            return (isize) bytesRead;
        }
        #elif MSR_UNIX
        {
            ssize_t result = read(streamData.handle, buffer.data, (size_t) buffer.count);
            if (result < 0) { return 0; }
            return (isize) result;
        }
        #else
            #error "Unsupported platform"
        #endif
    }

    if (mode == IO_StreamMode_Read)
    {
        #if MSR_WINDOWS
        {
            DWORD bytesWritten;
            if (!WriteFile(streamData.handle, buffer.data, (DWORD) buffer.count, &bytesWritten, nil)) { return 0; }
            return (isize) bytesWritten;
        }
        #elif MSR_UNIX
        {
            ssize_t result = write(streamData.handle, buffer.data, (size_t) buffer.count);
            if (result < 0) { return 0; }
            return (isize) result;
        }
        #else
            #error "Unsupported platform"
        #endif
    }

    MSR_ASSERT(false && "unsupported stream mode");
    return invalidOutput;
}

Slice_(u8) IO_ReadEntireFile(utf8str path, MEM_Allocator allocator)
{
    IO_Stream stream = IO_OpenFileToRead(path, false);
    if (!stream.procedure)
        return (Slice_(u8)) {0};

    return IO_ReadAll(stream, allocator, false);
}

b8 IO_WriteAllToFile(utf8str path, Slice_(u8) data, b8 append OPT_ARG)
{
    IO_Stream stream = IO_OpenFileToWrite(path, append, false);
    if (!stream.procedure)
        return false;

    isize bytesWritten = IO_Write(stream, data);
    IO_Flush(stream);
    IO_Close(stream);

    return bytesWritten == (isize) data.count;
}

#undef IO_Internal_InvalidFileStreamData
