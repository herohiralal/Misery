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

    for (char** var = environ; *var; var++)
        envVarsCount++;

#else
        #error "unsupported platform"
#endif

    envVars = allocator.MakeSlice<EnvVarKVP>(envVarsCount, SRC_LOC());
    if (!envVars) return envVars;

    size_t index = 0;

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

        size_t minLen = 16; // some buffer?
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
        bool success = (bool) CreateProcessA(
            nullptr,
            cmdLine.Data(),
            nullptr,
            nullptr,
            true, // inherit handles
            NORMAL_PRIORITY_CLASS,
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
        List<char> exeBuilder = tempAlloc.MakeList<char>();
        exeBuilder.Reserve(256, SRC_LOC()); // some initial capacity to avoid immediate reallocations

        String exePath = execAndArgs[0];

        bool isSimpleExePath = true;
        for (size_t i = 0; i < exePath.Length(); i++)
        {
            if (exePath[i] == '/' || exePath[i] == '\\')
            {
                isSimpleExePath = false;
                break;
            }
        }

        Slice<EnvVarKVP> currentEnvVars = { };
        if (!isSimpleExePath)
        {
            for (size_t i = 0; i < exePath.Length(); i++)
            {
                char c = (char) exePath[i];
                if (c == '\\') c = '/'; // normalize backslashes to forward slashes
                exeBuilder.Add(c, SRC_LOC());
            }

            exeBuilder.Add('\0', SRC_LOC());

            CString exePathCStr = exeBuilder.Data();

            // check if path is executable
            if (access(exePathCStr, X_OK) != 0)
            {
                return output; // not executable or doesn't exist
            }
        }
        else
        {
            currentEnvVars = GetEnvironmentVariables(tempAlloc); // to ensure PATH is loaded
            String pathVar = {0};
            for (size_t i = 0; i < currentEnvVars.Count(); i++)
            {
                const EnvVarKVP& kvp = currentEnvVars[i];
                if (kvp.key == "PATH")
                {
                    pathVar = kvp.value;
                    break;
                }
            }

            Slice<String> pathDirs = Misery::Internal::Process::SplitUnixPathList(pathVar, tempAlloc);

            bool found = false;
            for (const String& dir : pathDirs)
            {
                exeBuilder.Clear();
                for (size_t i = 0; i < dir.Length(); i++)
                {
                    char c = (char) dir[i];
                    if (c == '\\') c = '/'; // normalize backslashes to forward slashes
                    exeBuilder.Add(c, SRC_LOC());
                }

                exeBuilder.Add('/', SRC_LOC());
                for (size_t i = 0; i < exePath.Length(); i++)
                {
                    char c = (char) exePath[i];
                    if (c == '\\') c = '/'; // normalize backslashes to forward slashes
                    exeBuilder.Add(c, SRC_LOC());
                }

                exeBuilder.Add('\0', SRC_LOC());

                CString fullPathCStr = exeBuilder.Data();

                if (access(fullPathCStr, X_OK) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found) // check in cwd
            {
                exeBuilder.Clear();
                for (size_t i = 0; i < workingDirectory.actual.Length(); i++)
                {
                    char c = (char) workingDirectory.actual[i];
                    if (c == '\\') c = '/'; // normalize backslashes to forward slashes
                    exeBuilder.Add(c, SRC_LOC());
                }

                if (exeBuilder.Count() > 0 && exeBuilder[exeBuilder.Count() - 1] != '/')
                    exeBuilder.Add('/', SRC_LOC());

                exeBuilder.Add('.', SRC_LOC());
                exeBuilder.Add('/', SRC_LOC());
                for (size_t i = 0; i < exePath.Length(); i++)
                {
                    char c = (char) exePath[i];
                    if (c == '\\') c = '/'; // normalize backslashes to forward slashes
                    exeBuilder.Add(c, SRC_LOC());
                }

                exeBuilder.Add('\0', SRC_LOC());

                CString fullPathCStr = exeBuilder.Data();

                if (access(fullPathCStr, X_OK) == 0)
                {
                    found = true;
                }
            }

            if (!found) { return output; } // not found in PATH
        }

        CString cwd = { };
        if (workingDirectory.actual)
            cwd = tempAlloc.MakeCString(workingDirectory.actual, SRC_LOC());

        Slice<char*> cmd = tempAlloc.MakeSlice<char*>(execAndArgs.Count() + 1, SRC_LOC());
        for (size_t i = 0; i < execAndArgs.Count(); i++)
            cmd[i] = tempAlloc.MakeCString(execAndArgs[i], SRC_LOC());
        cmd[execAndArgs.Count()] = nullptr; // null-terminate argv

        char** env;
        char** cenv = nullptr;
        if (!environmentVariables)
        {
            env = environ; // inherit from current process
        }
        else
        {
            cenv = tempAlloc.MakeSlice<char*>(environmentVariables.Count() + 1, SRC_LOC()).Data();
            for (size_t i = 0; i < environmentVariables.Count(); i++)
                cenv[i] = tempAlloc.MakeCString(environmentVariables[i], SRC_LOC());
            cenv[environmentVariables.Count()] = nullptr; // null-terminate envp

            env = cenv;
        }

        static const int32_t READ = 0;
        static const int32_t WRITE = 1;

        int pipeVal[2];
        if (pipe(pipeVal) != 0) { return output; }

        // make read end close-on-exec
        if (fcntl(pipeVal[READ], F_SETFD, FD_CLOEXEC) == -1)
        {
            close(pipeVal[READ]);
            close(pipeVal[WRITE]);
            return output;
        }

        // make write end close-on-exec
        if (fcntl(pipeVal[WRITE], F_SETFD, FD_CLOEXEC) == -1)
        {
            close(pipeVal[READ]);
            close(pipeVal[WRITE]);
            return output;
        }

        pid_t pid = fork();
        switch (pid)
        {
            case -1: // fork failed
            {
                close(pipeVal[WRITE]);
                close(pipeVal[READ]);
                return output;
            }

            case 0: // child
            {
                // Close read end in child; child will write a single byte on failure.
                close(pipeVal[READ]);

                int nullFile = open("/dev/null", O_RDWR);
                if (nullFile == -1)
                {
                    int errNoVal = errno;
                    // write 4 bytes of errno for safety
                    (void)write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                int stdoutFile = stdOutPipe ? stdOutPipe->handle : nullFile;
                int stderrFile = stdErrPipe ? stdErrPipe->handle : nullFile;
                int stdinFile  = nullFile; // we do not support stdin for now

                if (dup2(stdoutFile, STDOUT_FILENO) == -1 ||
                    dup2(stderrFile, STDERR_FILENO) == -1 ||
                    dup2(stdinFile,  STDIN_FILENO)  == -1)
                {
                    int errNoVal = errno;
                    (void)write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                if (cwd.IsValid() && chdir(cwd) != 0)
                {
                    int errNoVal = errno;
                    (void)write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                // exeBuilder contains the full executable path (we ensured '\0' was appended earlier).
                CString fullExePathCStr = exeBuilder.Data();

                execve(fullExePathCStr, cmd.Data(), env);
                // If execve returns, it's an error
                {
                    int errNoVal = errno;
                    (void)write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                break;
            }

            default: // parent
            {
                // Close write end in parent — child writes to it on error.
                close(pipeVal[WRITE]);

                int errNoVal = 0;

                // Read error info from child (child writes errno if it failed before exec).
                // Child writes sizeof(int). Read that (or EOF).
                ssize_t totalRead = 0;
                while (totalRead < (ssize_t)sizeof(int))
                {
                    ssize_t r = read(pipeVal[READ], ((uint8_t*) &errNoVal) + totalRead, (size_t) ((ssize_t) sizeof(int) - totalRead));
                    if (r > 0) { totalRead += r; continue; }
                    if (r == 0) { /* EOF: child closed without writing: treat as no-error */ break; }
                    if (r == -1)
                    {
                        if (errno == EINTR) continue;
                        // read failed; set errNoVal to errno and break
                        errNoVal = errno;
                        break;
                    }
                }

                if (errNoVal != 0)
                {
                    // reported error — wait for the child to avoid zombies (use local pid)
                    while (true)
                    {
                        siginfo_t info;
                        int wpid = waitid(P_PID, (id_t) pid, &info, WEXITED);
                        if (wpid == -1 && errno == EINTR)
                            continue; // interrupted, try again
                        break;
                    }

                    close(pipeVal[READ]);
                    return output;
                }

                // No error reported — child successfully exec'd (or at least didn't fail early).
                output = Process(pid, 0);
                break;
            }
        }

        close(pipeVal[READ]);
    }
    #else
        #error "unsupported platform"
    #endif

    return output;
}

bool Process::Wait(int32_t* outExitCode)
{
    if (!IsValid()) { return false; }

#if MSR_WINDOWS
    {
        DWORD waitResult = WaitForSingleObject(handle, INFINITE);
        if (waitResult != WAIT_OBJECT_0) { return false; }

        if (outExitCode)
        {
            DWORD code;
            if (!GetExitCodeProcess(handle, &code)) { return false; }
            *outExitCode = (int32_t) code;
        }

        return true;
    }
#elif MSR_UNIX
    {
        int status;
        pid_t result = waitpid(pid, &status, 0);
        if (result == -1) { return false; }
        if (outExitCode)
        {
            if (WIFEXITED(status))
                *outExitCode = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                *outExitCode = -WTERMSIG(status); // negative signal number to indicate termination by signal
            else
                *outExitCode = -1; // unknown exit status
        }
        return true;
    }
#else
    {
        #error "unsupported platform"
        return false;
    }
#endif
}

bool Process::Kill()
{
    if (!IsValid()) { return false; }

#if MSR_WINDOWS
    {
        return !!TerminateProcess(handle, 1);
    }
#elif MSR_UNIX
    {
        return kill(pid, SIGKILL) == 0;
    }
#else
    {
        #error "unsupported platform"
        return false;
    }
#endif
}

void Process::Close()
{
    if (!IsValid()) { return; }

#if MSR_WINDOWS
    {
        CloseHandle(handle);
        handle = k_InvalidProcessHandle;
        pid = k_InvalidPID;
    }
#elif MSR_UNIX
    {
        // No handles to close on Unix, just invalidate the PID
        pid = k_InvalidPID;
    }
#else
    {
        #error "unsupported platform"
    }
#endif
}
