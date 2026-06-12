#include "CorePrivate.h"
#include "StreamPrivate.h"

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

static isize IO_Internal_FileStreamProc(IO_StreamMode, rawptr data, isize position, Slice_(u8) buffer);

IO_Stream IO_OpenFileToRead(FIL_Path path, b8 allowWrite)
{
    IO_Internal_FileStreamData streamData = {.handle = IO_Internal_InvalidFileStreamData};
    cstring cPath = STR_CloneToCStr(path.path, MEM_temp);

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
            streamData.handle = open(cPath, allowWrite ? O_RDWR : O_RDONLY);
        }
        #else
            #error "unimplemented"
        #endif
    }

    return (IO_Stream) {.procedure = IO_Internal_FileStreamProc, .data = streamData.asPtr};
}

IO_Stream IO_OpenFileToWrite(FIL_Path path, b8 append, b8 allowRead)
{
    IO_Internal_FileStreamData streamData = {.handle = IO_Internal_InvalidFileStreamData};
    cstring cPath = STR_CloneToCStr(path.path, MEM_temp);

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

static inline isize IO_Internal_InvalidStreamValue(IO_StreamMode mode)
{
    if (false ||
        mode == IO_StreamMode_SeekAbsolute ||
        mode == IO_StreamMode_SeekRelative ||
        mode == IO_StreamMode_TruncateAtCursor ||
        mode == IO_StreamMode_TruncateToSize ||
        mode == IO_StreamMode_Flush ||
        mode == IO_StreamMode_Close ||
        false)
    {
        return (isize) false;
    }

    return -1;
}

static isize IO_Internal_FileStreamProc(IO_StreamMode mode, rawptr data, isize position, Slice_(u8) buffer)
{
    IO_Internal_FileStreamData streamData = {.asPtr = data};

    if (streamData.handle == IO_Internal_InvalidFileStreamData)
    {
        return IO_Internal_InvalidStreamValue(mode);
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
            return (isize) (b8) ftruncate(streamData.handle, cur) == 0;
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
            return (isize) (b8) ftruncate(streamData.handle, position) == 0;
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

        return IO_Internal_InvalidStreamValue(mode);
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

    if (mode == IO_StreamMode_Write)
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
    return IO_Internal_InvalidStreamValue(mode);
}

Slice_(u8) IO_ReadEntireFile(FIL_Path path, MEM_Allocator allocator)
{
    IO_Stream stream = IO_OpenFileToRead(path, false);
    if (!stream.procedure)
        return (Slice_(u8)) {0};

    return IO_ReadAll(stream, allocator, false);
}

b8 IO_WriteAllToFile(FIL_Path path, Slice_(u8) data, b8 append OPT_ARG)
{
    IO_Stream stream = IO_OpenFileToWrite(path, append, false);
    if (!stream.procedure)
        return false;

    isize bytesWritten = IO_Write(stream, data);
    IO_Flush(stream);
    IO_Close(stream);

    return bytesWritten == (isize) data.count;
}

static isize IO_Internal_PipeProc(IO_StreamMode mode, rawptr data, isize position, Slice_(u8) buffer);

b8 IO_CreatePipe(IO_Stream* outR, IO_Stream* outW)
{
    IO_Internal_FileStreamData rStream = {.handle = IO_Internal_InvalidFileStreamData};
    IO_Internal_FileStreamData wStream = rStream;

    if (outR) *outR = (IO_Stream) {.procedure = IO_Internal_PipeProc, .data = rStream.asPtr};
    if (outW) *outW = (IO_Stream) {.procedure = IO_Internal_PipeProc, .data = wStream.asPtr};
    if (!outR || !outW)
        return false;

    #if MSR_WINDOWS
    {
        HANDLE readHandle, writeHandle;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        if (!CreatePipe(&readHandle, &writeHandle, &sa, 0)) { return false; }

        rStream.handle = readHandle;
        wStream.handle = writeHandle;

        outR->data = rStream.asPtr;
        outW->data = wStream.asPtr;
        return true;
    }
    #elif MSR_UNIX
    {
        int fds[2];
        if (pipe(fds) != 0) { return false; }

        rStream.handle = fds[0];
        wStream.handle = fds[1];

        outR->data = rStream.asPtr;
        outW->data = wStream.asPtr;
        return true;
    }
    #else
    {
        #error "unsupported platform"
        return false;
    }
    #endif
}

static isize IO_Internal_PipeProc(IO_StreamMode mode, rawptr data, isize position, Slice_(u8) buffer)
{
    IO_Internal_FileStreamData streamData = {.asPtr = data};

    if (streamData.handle == IO_Internal_InvalidFileStreamData)
    {
        return IO_Internal_InvalidStreamValue(mode);
    }

    if (false ||
        mode == IO_StreamMode_GetCurrentPosition ||
        mode == IO_StreamMode_SeekAbsolute ||
        mode == IO_StreamMode_SeekRelative ||
        mode == IO_StreamMode_TruncateAtCursor ||
        mode == IO_StreamMode_TruncateToSize ||
        mode == IO_StreamMode_Flush ||
        false)
    {
        return IO_Internal_InvalidStreamValue(mode);
    }

    if (mode == IO_StreamMode_GetSize)
    {
        #if MSR_WINDOWS
        {
            DWORD bytesAvailable = 0;
            if (!PeekNamedPipe(streamData.handle, NULL, 0, NULL, &bytesAvailable, NULL))
                return -1;

            return (isize) bytesAvailable;
        }
        #elif MSR_UNIX
        {
            int bytesAvailable = 0;
            if (ioctl(streamData.handle, FIONREAD, &bytesAvailable) != 0)
                return -1;

            return (isize) bytesAvailable;
        }
        #else
        {
            #error "unsupported platform"
            return -1;
        }
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
        {
            #error "unsupported platform"
        }
        #endif

        return IO_Internal_InvalidStreamValue(mode);
    }

    if (mode == IO_StreamMode_Read)
    {
        #if MSR_WINDOWS
        {
            DWORD bytesRead = 0;
            if (!ReadFile(streamData.handle, buffer.data, (DWORD) buffer.count, &bytesRead, NULL))
            {
                DWORD err = GetLastError();
                if (err == ERROR_NO_DATA || err == ERROR_PIPE_NOT_CONNECTED)
                    bytesRead = 0; // no data yet
                else
                    return IO_Internal_InvalidStreamValue(mode); // error
            }

            return (isize) bytesRead;
        }
        #elif MSR_UNIX
        {
            ssize_t bytesRead = read(streamData.handle, buffer.data, (size_t) buffer.count);
            if (bytesRead < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    bytesRead = 0; // no data yet
                else
                    return IO_Internal_InvalidStreamValue(mode); // error
            }

            return (isize) bytesRead;
        }
        #else
        {
            #error "unsupported platform"
            return IO_Internal_InvalidStreamValue(mode);
        }
        #endif
    }

    if (mode == IO_StreamMode_Write)
    {
        #if MSR_WINDOWS
        {
            DWORD bytesWritten = 0;
            if (!WriteFile(streamData.handle, buffer.data, (DWORD) buffer.count, &bytesWritten, NULL))
            {
                DWORD err = GetLastError();
                if (err == ERROR_NO_DATA || err == ERROR_PIPE_NOT_CONNECTED)
                    bytesWritten = 0; // cannot write now
                else
                    return -1; // error
            }

            return (isize) bytesWritten;
        }
        #elif MSR_UNIX
        {
            ssize_t bytesWritten = write(streamData.handle, buffer.data, (size_t) buffer.count);
            if (bytesWritten < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    bytesWritten = 0; // cannot write now
                else
                    return -1; // error
            }

            return (isize) bytesWritten;
        }
        #else
        {
            #error "unsupported platform"
            return IO_Internal_InvalidStreamValue(mode);
        }
        #endif
    }

    MSR_ASSERT(false && "unsupported stream mode");
    return IO_Internal_InvalidStreamValue(mode);
}

IO_Stream IO_GetStdOut(void)
{
    #if MSR_WINDOWS
    {
        HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE)
            return (IO_Stream) {0};

        IO_Internal_FileStreamData streamData = {.handle = stdoutHandle};
        return (IO_Stream) {.procedure = IO_Internal_PipeProc, .data = streamData.asPtr};
    }
    #elif MSR_UNIX
    {
        IO_Internal_FileStreamData streamData = {.handle = STDOUT_FILENO};
        return (IO_Stream) {.procedure = IO_Internal_PipeProc, .data = streamData.asPtr};
    }
    #else
    {
        #error "unsupported platform"
        return (IO_Stream) {0};
    }
    #endif
}

IO_Stream IO_GetStdErr(void)
{
    #if MSR_WINDOWS
    {
        HANDLE stderrHandle = GetStdHandle(STD_ERROR_HANDLE);
        if (stderrHandle == INVALID_HANDLE_VALUE)
            return (IO_Stream) {0};

        IO_Internal_FileStreamData streamData = {.handle = stderrHandle};
        return (IO_Stream) {.procedure = IO_Internal_PipeProc, .data = streamData.asPtr};
    }
    #elif MSR_UNIX
    {
        IO_Internal_FileStreamData streamData = {.handle = STDERR_FILENO};
        return (IO_Stream) {.procedure = IO_Internal_PipeProc, .data = streamData.asPtr};
    }
    #else
    {
        #error "unsupported platform"
        return (IO_Stream) {0};
    }
    #endif
}

#undef IO_Internal_InvalidFileStreamData
