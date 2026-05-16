#include <Core/Process.h>
#include <Core/Defer.h>
#include <Core/Allocators/Arena.h>

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

void Process::Exit(int32_t exitCode)
{
    #if PNSLR_WINDOWS
    {
        ExitProcess((UINT) exitCode);
    }
    #elif PNSLR_UNIX
    {
        // On Unix-like systems, we can use the exit system call directly.
        _exit(exitCode);
    }
    #endif
}

// in unicode, on windows, GetEnvironmentStringsA is not even declared
// so declaring this here to ensure we can call it regardless of UNICODE being defined or not
#if defined(UNICODE) && defined(_WIN32)
#undef GetEnvironmentStrings
LPCH GetEnvironmentStringsA(void) { return GetEnvironmentStrings(); }
#define GetEnvironmentStrings GetEnvironmentStringsW
#endif

Slice<EnvVarKVP> Process::GetEnvironmentVariables(Allocator allocator)
{
    Slice<EnvVarKVP> envVars = Slice<EnvVarKVP>();

    size_t envVarsCount = 0;

#if MSR_WINDOWS

    LPCH envStringsWindows = GetEnvironmentStringsA();
    if (!envStringsWindows) { return envVars; }

    DEFER { FreeEnvironmentStringsA(envStringsWindows); };

    for (LPCH var = envStringsWindows; *var; var += strlen(var) + 1)
        envVarsCount++;

#elif MSR_UNIX

    for (cstring* var = environ; *var; var++)
        envVarsCount++;

#else
        #error "unsupported platform"
#endif

    envVars = allocator.MakeSlice<EnvVarKVP>(envVarsCount, SRC_LOC());
    if (!envVars) return envVars;

    int64_t index = 0;

    size_t fullLen = 0;
#if MSR_WINDOWS
    for (LPCH var = envStringsWindows; *var; var += fullLen + 1)
    {
#elif MSR_UNIX
    for (char** varPtr = environ; *varPtr; varPtr++)
    {
        char* var = *varPtr;
#else
        #error "unsupported platform"
#endif

        fullLen = strlen(var);
        if (fullLen == 0) { continue; }

        char* equalSign = strchr(var, '=');
        if (!equalSign)       { continue; } // malformed entry, skip
        if (equalSign == var) { continue; } // empty key, skip

        String kvp = allocator.CloneString(String(reinterpret_cast<uint8_t*>(var), fullLen), SRC_LOC());

        size_t keyLen = (size_t) (strchr((char*) kvp.Data(), '=') - (char*) kvp.Data());
        size_t valLen = kvp.Length() - keyLen - 1; // -1 for '='

        EnvVarKVP& kvpEntry = envVars[index];
        kvpEntry.kvp   = kvp;
        kvpEntry.key   = kvp.SubString(0,          keyLen);
        kvpEntry.value = kvp.SubString(keyLen + 1, valLen);
        index++;

#if MSR_WINDOWS
    }
#elif MSR_UNIX
    }
#else
        #error "unsupported platform"
#endif

    envVars = envVars.SubSlice(0, index); // in case of malformed entries
    return envVars;
}

namespace Misery::Internal::Process
{
#if MSR_WINDOWS

    CString BuildWindowsProcessCmdLine(Slice<String> execAndArgs, Allocator allocator)
    {
        List<char> sb = allocator.MakeList<char>();

        size_t minLen = 25; // adjusting for maybe 25 chars of extra backslashes and the null terminator
        for (const auto& arg : execAndArgs)
            minLen += arg.Length() + 3; // +3 for potential quotes and space

        sb.Reserve(minLen, SRC_LOC());

        for (size_t i = 0; i < execAndArgs.Count(); i++)
        {
            String arg = execAndArgs[i];
            if (i != 0) { sb.Add(' ', SRC_LOC()); }

            bool needsQuotes = false;
            for (size_t j = 0; j < arg.Length(); j++)
            {
                if (false ||
                    arg[j] == '('  ||
                    arg[j] == ')'  ||
                    arg[j] == '['  ||
                    arg[j] == ']'  ||
                    arg[j] == '{'  ||
                    arg[j] == '}'  ||
                    arg[j] == '^'  ||
                    arg[j] == '='  ||
                    arg[j] == ';'  ||
                    arg[j] == '!'  ||
                    arg[j] == '\'' ||
                    arg[j] == '+'  ||
                    arg[j] == ','  ||
                    arg[j] == '`'  ||
                    arg[j] == '~'  ||
                    arg[j] == '\"' ||
                    arg[j] == ' '  ||
                    arg[j] == '\t' || // include tab as whitespace
                    false)
                {
                    needsQuotes = true;
                    break;
                }
            }

            if (!needsQuotes)
            {
                for (size_t j = 0; j < arg.Length(); j++)
                    sb.Add(arg[j], SRC_LOC());
                continue;
            }

            sb.Add('\"', SRC_LOC()); // start
            for (size_t j = 0; j < arg.Length(); j++)
            {
                size_t backslashes = 0;

                while (j < arg.Length() && arg[j] == '\\') { backslashes++; j++; }

                if (j == arg.Length())
                {
                    // Escape all backslashes at the end
                    for (size_t k = 0; k < backslashes * 2; k++)
                        sb.Add('\\', SRC_LOC());
                    break;
                }
                else if (arg[j] == '\"')
                {
                    // Escape all backslashes and the quote
                    for (size_t k = 0; k < (backslashes * 2) + 1; k++)
                        sb.Add('\\', SRC_LOC());

                    sb.Add('\"', SRC_LOC());
                }
                else
                {
                    // No special handling needed, just output the backslashes
                    for (size_t k = 0; k < backslashes; k++)
                        sb.Add('\\', SRC_LOC());

                    sb.Add(arg[j], SRC_LOC());
                }
            }
            sb.Add('\"', SRC_LOC()); // end
        }

        sb.Add('\0', SRC_LOC());
        return CString(sb.Data());
    }

    CString BuildWindowsProcessEnvBlock(Slice<String> envVars, Allocator allocator)
    {
        List<char> sb = allocator.MakeList<char>();

        size_t minLen = 0;
        for (const auto& kv : envVars)
            minLen += kv.Length() + 1; // +1 for null terminator
        minLen += 1; // for the double null terminator
        sb.Reserve(minLen, SRC_LOC());

        for (int64_t currIdx = (int64_t) envVars.Count() - 1; currIdx >= 0; --currIdx)
        {
            String kv = envVars[currIdx];

            int64_t eqIdx = kv.FirstIndexOf("=");
            if (eqIdx == -1) { continue; } // malformed, skip

            String key = kv.SubString(0, eqIdx);
            String value = kv.SubString(eqIdx + 1, kv.Length() - eqIdx - 1);

            bool foundDuplicate = false;
            for (int64_t prevIdx = (currIdx + 1); prevIdx < envVars.Count(); prevIdx++)
            {
                int64_t prevEqIdx = envVars[prevIdx].FirstIndexOf("=");
                if (prevEqIdx == -1) { continue; } // malformed, skip

                String prevKey = envVars[prevIdx].SubString(0, prevEqIdx);
                if (prevKey == key) { foundDuplicate = true; break; }
            }

            if (foundDuplicate) { continue; } // skip this one, a later one exists
            for (size_t j = 0; j < kv.Length(); j++)
                sb.Add(kv[j], SRC_LOC());
            sb.Add('\0', SRC_LOC()); // null terminator
        }
        sb.Add('\0', SRC_LOC()); // double-null-terminate the block
        return CString(sb.Data());
    }

#elif MSR_UNIX

    Slice<String> SplitUnixPathList(String pathStr, Allocator allocator)
    {
        if (!pathStr) { return Slice<String>(); }

        size_t start = 0, count = 0;
        bool quote = false;

        for (size_t i = 0; i < pathStr.Length(); i++)
        {
            uint8_t c = pathStr[i];
            if (c == '\"') { quote = !quote; }
            else if (c == ':' && !quote) { count++; }
        }

        start = 0;
        quote = false;

        Slice<String> paths = allocator.MakeSlice<String>(count + 1, SRC_LOC());
        if (!paths) { return Slice<String>(); }

        size_t idx = 0;
        for (size_t i = 0; i < pathStr.Length(); i++)
        {
            uint8_t c = pathStr[i];
            if (c == '\"') { quote = !quote; }
            else if (c == ':' && !quote)
            {
                paths[idx++] = pathStr.SubString(start, i - start);
                start = i + 1;
            }
        }
        MSR_ASSERT(idx == count && "Count mismatch in SplitUnixPathList");

        paths[idx] = pathStr.SubString(start, pathStr.Length() - start);

        for (size_t i = 0; i < paths.Count(); i++)
        {
            String p = paths[i];
            if (p.Length() >= 2 && p[0] == '\"' && p[p.Length() - 1] == '\"')
                paths[i] = p.SubString(1, p.Length() - 2);
        }

        return paths;
    }

#endif
}

Process Process::Run(
    Slice<String> execAndArgs,
    Slice<String> environmentVariables,
    DirectoryPath workingDirectory,
    PipeHandle* stdOutPipe,
    PipeHandle* stdErrPipe)
{
    Process output = Process(k_InvalidPID, k_InvalidProcessHandle);
    if (!execAndArgs) return output;

    ArenaAllocator tempAllocImpl = ArenaAllocator(4096, alloc_main);
    DEFER { tempAllocImpl.Destroy(); };
    Allocator tempAlloc = &tempAllocImpl;

    bool success = false;
    #if MSR_WINDOWS
    {
        CString cmdLine = Misery::Internal::Process::BuildWindowsProcessCmdLine(execAndArgs, tempAlloc);
        if (!environmentVariables)
        {
            Slice<EnvVarKVP> kvps = GetEnvironmentVariables(tempAlloc);
            environmentVariables = tempAlloc.MakeSlice<String>(kvps.Count(), SRC_LOC());
            for (size_t i = 0; i < kvps.Count(); i++)
                environmentVariables[i] = kvps[i].kvp;
        }

        CString envBlock = { };
        if (environmentVariables)
            envBlock = Misery::Internal::Process::BuildWindowsProcessEnvBlock(environmentVariables, tempAlloc);

        HANDLE nullHandle = nullptr;
        if (!stdOutPipe || !stdErrPipe)
        {
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            nullHandle = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            MSR_ASSERT(nullHandle != INVALID_HANDLE_VALUE && "Failed to open NUL device");
        }

        DEFER { if (nullHandle) CloseHandle(nullHandle); };

        HANDLE stdOutHandle = stdOutPipe && stdOutPipe->IsValid() ? stdOutPipe->handle : nullHandle;
        HANDLE stdErrHandle = stdErrPipe && stdErrPipe->IsValid() ? stdErrPipe->handle : nullHandle;
        HANDLE stdInHandle = nullHandle;

        STARTUPINFOA si =
        {
            .cb = sizeof(STARTUPINFOA),
            .dwFlags = STARTF_USESTDHANDLES,
            .hStdInput = stdInHandle,
            .hStdOutput = stdOutHandle,
            .hStdError = stdErrHandle,
        };

        PROCESS_INFORMATION pi = { };
        success = (bool) CreateProcessA(
            nullptr,
            cmdLine.Data(),
            nullptr,
            nullptr,
            true, // inherit handles
            CREATE_UNICODE_ENVIRONMENT | NORMAL_PRIORITY_CLASS,
            envBlock ? envBlock.Data() : nullptr,
            workingDirectory.actual ? tempAlloc.MakeCString(workingDirectory.actual, SRC_LOC()) : nullptr,
            &si,
            &pi
        );

        if (pi.hThread) { CloseHandle(pi.hThread); }

        if (!success)
        {
            if (pi.hProcess) { CloseHandle(pi.hProcess); }
        }
        else
        {
            output.pid = pi.dwProcessId;
            output.handle = pi.hProcess;
        }
    }
    #elif MSR_UNIX
    {
        #error "unimplemented"
    }
    #else
        #error "unsupported platform"
    #endif

    return output;
}
