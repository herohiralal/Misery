#include <Core/Process.h>

bool PipeHandle::Create(PipeHandle* outR, PipeHandle* outW)
{
    if (outR) *outR = PipeHandle(k_InvalidHandle);
    if (outW) *outW = PipeHandle(k_InvalidHandle);
    if (!outR || !outW) return false;

    #if MSR_WINDOWS

        HANDLE readHandle, writeHandle;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        if (!CreatePipe(&readHandle, &writeHandle, &sa, 0)) { return false; }

        outR->handle = readHandle;
        outW->handle = writeHandle;
        return true;

    #elif MSR_UNIX

        int fds[2];
        if (pipe(fds) != 0) { return false; }

        outR->handle = fds[0];
        outW->handle = fds[1];
        return true;

    #else

        #error "unsupported platform"
        return false;

    #endif

    return false;
}

int64_t PipeHandle::GetSize()
{
    #if MSR_WINDOWS

        DWORD bytesAvailable = 0;
        if (!PeekNamedPipe(handle, NULL, 0, NULL, &bytesAvailable, NULL))
            return -1;

        return (int64_t) bytesAvailable;

    #elif MSR_UNIX

        int bytesAvailable = 0;
        if (ioctl(handle, FIONREAD, &bytesAvailable) != 0)
            return -1;

        return (int64_t) bytesAvailable;

    #else

        #error "unsupported platform"
        return -1;

    #endif
}

int64_t PipeHandle::Read(Slice<uint8_t> dst)
{
    if (!IsValid()) { return -1; }
    if (!dst) { return 0; }

    #if MSR_WINDOWS

        DWORD bytesRead = 0;
        if (!ReadFile(handle, dst.Data(), (DWORD) dst.Count(), &bytesRead, NULL))
        {
            DWORD err = GetLastError();
            if (err == ERROR_NO_DATA || err == ERROR_PIPE_NOT_CONNECTED)
                bytesRead = 0; // no data yet
            else
                return -1; // error
        }

    #elif MSR_UNIX

        ssize_t bytesRead = read(handle, dst.Data(), dst.Count());
        if (bytesRead < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                bytesRead = 0; // no data yet
            else
                return -1; // error
        }

    #else

        #error "unsupported platform"
        return 0;

    #endif

    return (int64_t) bytesRead;
}

int64_t PipeHandle::Write(const Slice<uint8_t> src)
{
    if (!IsValid()) { return -1; }
    if (!src) { return 0; }

    #if MSR_WINDOWS

        DWORD bytesWritten = 0;
        if (!WriteFile(handle, src.Data(), (DWORD) src.Count(), &bytesWritten, NULL))
        {
            DWORD err = GetLastError();
            if (err == ERROR_NO_DATA || err == ERROR_PIPE_NOT_CONNECTED)
                bytesWritten = 0; // cannot write now
            else
                return -1; // error
        }

    #elif MSR_UNIX

        ssize_t bytesWritten = write(handle, src.Data(), src.Count());
        if (bytesWritten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                bytesWritten = 0; // cannot write now
            else
                return -1; // error
        }

    #else

        #error "unsupported platform"
        return 0;

    #endif

    return (int64_t) bytesWritten;
}

void PipeHandle::Close()
{
    if (!IsValid()) { return; }

    #if MSR_WINDOWS
        CloseHandle(handle);
    #elif MSR_UNIX
        close(handle);
    #else
        #error "unsupported platform"
    #endif

    handle = k_InvalidHandle;
}
