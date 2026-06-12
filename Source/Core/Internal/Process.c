#include "CorePrivate.h"
#include "StreamPrivate.h"

MSR_NORETURN
void PRC_Exit(i32 exitCode OPT_ARG)
{
    #if MSR_WINDOWS
    {
        ExitProcess((UINT) exitCode);
    }
    #elif MSR_UNIX
    {
        // On Unix-like systems, we can use the exit system call directly.
        _exit(exitCode);
    }
    #else
    {
        #error "unimplemented"
    }
    #endif
}

// in unicode, on windows, GetEnvironmentStringsA is not even declared
// so declaring this here to ensure we can call it regardless of UNICODE being defined or not
#if defined(UNICODE) && defined(_WIN32)
#undef GetEnvironmentStrings
PCH GetEnvironmentStringsA(void) { return GetEnvironmentStrings(); }
#define GetEnvironmentStrings GetEnvironmentStringsW
#endif

List_(PRC_EnvVarKVP) PRC_GetEnvVars(MEM_Allocator allocator)
{
    List_(PRC_EnvVarKVP) output = COL_NewList(PRC_EnvVarKVP, 0, allocator);

    isize envVarsCount = 0;

#if MSR_WINDOWS

    PCH envStringsWindows = GetEnvironmentStringsA();
    if (!envStringsWindows) { return output; }

    for (PCH var = envStringsWindows; *var; var += strlen(var) + 1)
        envVarsCount++;

#elif MSR_UNIX

    for (char** var = environ; *var; var++)
        envVarsCount++;

#else
        #error "unsupported platform"
#endif

    COL_ResizeList(&output, envVarsCount);

    isize fullLen = 0;
#if MSR_WINDOWS
    for (PCH var = envStringsWindows; *var; var += fullLen + 1)
    {
#elif MSR_UNIX
    for (char** varPtr = environ; *varPtr; varPtr++)
    {
        char* var = *varPtr;
#else
        #error "unsupported platform"
#endif

        fullLen = (isize) strlen(var);
        if (fullLen == 0) { continue; }

        char* equalSign = strchr(var, '=');
        if (!equalSign)       { continue; } // malformed entry, skip
        if (equalSign == var) { continue; } // empty key, skip

        utf8str kvp = STR_Clone((utf8str) {.data = (u8*) var, .count = fullLen}, allocator);

        isize keyLen = (isize) (strchr((char*) kvp.data, '=') - (char*) kvp.data);
        isize valLen = kvp.count - keyLen - 1; // -1 for '='

        PRC_EnvVarKVP envVar =
        {
            .kvp   = kvp,
            .key   = STR_SubString(kvp, 0,          keyLen),
            .value = STR_SubString(kvp, keyLen + 1, valLen)
        };

        COL_AppendToList(&output, envVar);

#if MSR_UNIX
    }
#elif MSR_WINDOWS
    }

    FreeEnvironmentStringsA(envStringsWindows);
#else
        #error "unsupported platform"
#endif

    return output;
}

void PRC_FreeEnvVars(List_(PRC_EnvVarKVP)* envVars)
{
    if (!envVars)
        return;

    COL_DeleteList(envVars);
    *envVars = (List_(PRC_EnvVarKVP)) {0};
}

#if MSR_WINDOWS

static cstring PRC_Internal_BuildWindowsProcessCmdLine(Slice_(utf8str) execAndArgs, MEM_Allocator allocator)
{
    isize minLen = 25; // adjusting for maybe 25 chars of extra backslashes and the null terminator
    for (isize i = 0; i < execAndArgs.count; i++)
        minLen += execAndArgs.data[i].count + 3; // +3 for potential quotes and a space

    List_(char) sb = COL_NewList(char, minLen, allocator);

    for (isize i = 0; i < execAndArgs.count; i++)
    {
        utf8str arg = execAndArgs.data[i];

        if (i != 0) { COL_AppendToList(&sb, ' '); }

        // Escape and quote the argument as needed
        b8 needsQuotes = false;
        for (isize j = 0; j < arg.count; j++)
        {
            if (false ||
                arg.data[j] == '('  ||
                arg.data[j] == ')'  ||
                arg.data[j] == '['  ||
                arg.data[j] == ']'  ||
                arg.data[j] == '{'  ||
                arg.data[j] == '}'  ||
                arg.data[j] == '^'  ||
                arg.data[j] == '='  ||
                arg.data[j] == ';'  ||
                arg.data[j] == '!'  ||
                arg.data[j] == '\'' ||
                arg.data[j] == '+'  ||
                arg.data[j] == ','  ||
                arg.data[j] == '`'  ||
                arg.data[j] == '~'  ||
                arg.data[j] == '\"' ||
                arg.data[j] == ' '  ||
                arg.data[j] == '\t' || // include tab as whitespace
                false)
            {
                needsQuotes = true;
                break;
            }
        }

        if (!needsQuotes)
        {
            COL_AppendAllToList(&sb, arg);
            continue;
        }

        // Argument needs quotes and possibly escaping

        COL_AppendToList(&sb, '\"'); // start

        isize j = 0;
        while (j < arg.count)
        {
            isize backslashes = 0;

            while (j < arg.count && arg.data[j] == '\\') { backslashes++; j++; }

            if (j == arg.count)
            {
                // Escape all backslashes at the end
                for (isize k = 0; k < backslashes * 2; k++)
                    COL_AppendToList(&sb, '\\');
                break;
            }
            else if (arg.data[j] == '\"')
            {
                // Escape all backslashes and the quote
                for (isize k = 0; k < (backslashes * 2) + 1; k++)
                    COL_AppendToList(&sb, '\\');

                COL_AppendToList(&sb, '\"');
            }
            else
            {
                // No special handling needed, just output the backslashes
                for (isize k = 0; k < backslashes; k++)
                    COL_AppendToList(&sb, '\\');

                COL_AppendToList(&sb, arg.data[j]);
            }

            j++;
        }

        COL_AppendToList(&sb, '\"'); // end
    }

    COL_AppendToList(&sb, '\0');
    return sb.data;
}

cstring PRC_Internal_BuildWindowsProcessEnvBlock(Slice_(utf8str) envVars, MEM_Allocator allocator)
{
    isize minLen = 16; // some buffer?
    for (isize i = 0; i < envVars.count; i++)
        minLen += envVars.data[i].count + 1; // +1 for null terminator
    minLen += 1; // for the double null terminator

    List_(char) sb = COL_NewList(char, minLen, allocator);

    for (isize currIdx = envVars.count - 1; currIdx >= 0; --currIdx)
    {
        utf8str kv = envVars.data[currIdx];

        isize eqIdx = STR_Find(kv, UTF8STR("="), false);
        if (eqIdx == -1) { continue; } // malformed, skip

        utf8str key = STR_SubString(kv, 0, eqIdx);
        b8 foundDuplicate = false;
        for (isize prevIdx = (currIdx + 1); prevIdx < envVars.count; prevIdx++)
        {
            isize prevEqIdx = STR_Find(envVars.data[prevIdx], UTF8STR("="), false);
            if (prevEqIdx == -1) { continue; } // malformed, skip

            utf8str prevKey = STR_SubString(envVars.data[prevIdx], 0, prevEqIdx);
            if (STR_Eq(prevKey, key)) { foundDuplicate = true; break; }
        }

        if (foundDuplicate) { continue; } // skip this one, a later one exists
        COL_AppendAllToList(&sb, kv);
        COL_AppendToList(&sb, '\0'); // null terminator
    }
    COL_AppendToList(&sb, '\0'); // final null terminator
    return sb.data;
}

#elif MSR_UNIX

Slice_(utf8str) PRC_Internal_SplitUnixPathList(utf8str pathStr, MEM_Allocator allocator)
{
    if (!pathStr.data || !pathStr.count) return (Slice_(utf8str)) {0};

    isize start = 0, count = 0;
    b8 quote = false;

    for (isize i = 0; i < pathStr.count; i++)
    {
        u8 c = pathStr.data[i];
        if (c == '\"') { quote = !quote; }
        else if (c == ':' && !quote) { count++; }
    }

    start = 0; quote = false;

    Slice_(utf8str) paths = COL_NewSlice(utf8str, count + 1, false, allocator);

    isize index = 0;
    for (isize i = 0; i < pathStr.count; i++)
    {
        u8 c = pathStr.data[i];
        if (c == '\"') { quote = !quote; }
        else if (c == ':' && !quote)
        {
            paths.data[index] = pathStr; paths.data[index].data += start; paths.data[index].count = i - start;
            index++;
            start = i + 1;
        }
    }

    MSR_ASSERT(index == count && "path splitting logic error");

    paths.data[index] = pathStr; paths.data[index].data += start; paths.data[index].count = pathStr.count - start;

    for (isize i = 0; i < paths.count; i++)
    {
        paths.data[i] = STR_Replace(paths.data[i], UTF8STR("\""), UTF8STR(""), allocator, false);
    }

    return paths;
}

#endif

#if MSR_WINDOWS
    typedef struct
    {
        HANDLE handle;
    } PRC_PlatformHandle;
#elif MSR_UNIX
    typedef struct
    {
        pid_t pid;
    } PRC_PlatformHandle;
#else
    #error "unsupported platform"
#endif

static_assert( sizeof(PRC_Handle) >= sizeof(PRC_PlatformHandle),      "PRC_Handle must be large enough to hold platform-specific process handle data.");
static_assert(alignof(PRC_Handle) % alignof(PRC_PlatformHandle) == 0, "PRC_Handle must be aligned to accommodate platform-specific process handle data.");

static inline PRC_Handle PRC_ToHandle(PRC_PlatformHandle platformHandle)
{
    PRC_Handle handle = {0};
    *((PRC_PlatformHandle*) &handle) = platformHandle;
    return handle;
}

static inline PRC_PlatformHandle PRC_FromHandle(PRC_Handle handle)
{
    return *((PRC_PlatformHandle*) &handle);
}

PRC_Handle PRC_Run(Slice_(utf8str) execAndArgs, Slice_(utf8str) environmentVariables, DIR_Path workingDirectory, IO_Stream* stdOutPipe, IO_Stream* stdErrPipe)
{
    PRC_PlatformHandle p = {0};
    #if MSR_WINDOWS
        p.handle = nil;
    #elif MSR_UNIX
        p.pid = (pid_t) -1;
    #else
        #error "unsupported platform"
    #endif

    if (execAndArgs.count < 1 || !execAndArgs.data)
        return PRC_ToHandle(p);

    MEM_ArenaAllocator tempArena = MEM_CreateArenaAllocator(8 * 1024, MEM_main);
    MEM_Allocator tempAllocator = MEM_AllocatorFromArena(&tempArena);

    #if MSR_WINDOWS
    {
        cstring cmdLine = PRC_Internal_BuildWindowsProcessCmdLine(execAndArgs, tempAllocator);

        if (!environmentVariables.count || !environmentVariables.data)
        {
            List_(PRC_EnvVarKVP) kvps = PRC_GetEnvVars(tempAllocator);
            environmentVariables = COL_NewSlice(utf8str, kvps.count, true, tempAllocator);
            for (isize i = 0; i < kvps.count; i++)
                environmentVariables.data[i] = kvps.data[i].kvp;
        }

        cstring envBlock = nil;
        if (environmentVariables.count && environmentVariables.data)
            envBlock = PRC_Internal_BuildWindowsProcessEnvBlock(environmentVariables, tempAllocator);

        HANDLE nullHandle = nil;
        if (!stdOutPipe || !stdErrPipe)
        {
            SECURITY_ATTRIBUTES sa = {.nLength = sizeof(SECURITY_ATTRIBUTES), .bInheritHandle = true};
            nullHandle = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nil);
            MSR_ASSERT(nullHandle != INVALID_HANDLE_VALUE && "Failed to open NUL handle");
        }

        HANDLE stdOutHandle = stdOutPipe ? ((IO_Internal_FileStreamData) {.asPtr = stdOutPipe->data}).handle : nullHandle;
        HANDLE stdErrHandle = stdErrPipe ? ((IO_Internal_FileStreamData) {.asPtr = stdErrPipe->data}).handle : nullHandle;
        HANDLE stdInHandle  = nullHandle; // we do not support stdin for now

        cstring workingDir = STR_CloneToCStr(workingDirectory.path, tempAllocator);

        STARTUPINFOA si =
        {
            .cb = sizeof(STARTUPINFOA),
            .hStdError = stdErrHandle,
            .hStdOutput = stdOutHandle,
            .hStdInput = stdInHandle,
            .dwFlags = STARTF_USESTDHANDLES
        };

        PROCESS_INFORMATION pi = {0};
        BOOL ok = CreateProcessA(
            nil,
            (PSTR) cmdLine,
            nil,
            nil,
            true, // inherit handles
            NORMAL_PRIORITY_CLASS,
            (rawptr) envBlock,
            workingDir,
            &si,
            &pi
        );

        if (pi.hThread) { CloseHandle(pi.hThread); }

        if (nullHandle)
        {
            CloseHandle(nullHandle);
            nullHandle = nil;
        }

        if (!ok)
        {
            if (pi.hProcess) { CloseHandle(pi.hProcess); }
            pi.hProcess = nil;
        }
        else
        {
            p.handle = pi.hProcess;
        }
    }
    #elif MSR_UNIX
    {
        List_(char) exeBuilder = COL_NewList(char, 256, tempAllocator);
        utf8str exePath = execAndArgs.data[0];

        b8 isSimpleExePath = true;
        for (isize i = 0; i < exePath.count; i++)
        {
            if (exePath.data[i] == '/' || exePath.data[i] == '\\')
            {
                isSimpleExePath = false;
                break;
            }
        }

        Slice_(PRC_EnvVarKVP) currentEnvVars = {0};
        if (!isSimpleExePath)
        {
            for (isize i = 0; i < exePath.count; i++)
            {
                char c = (char) exePath.data[i];
                if (c == '\\') c = '/';
                COL_AppendToList(&exeBuilder, c);
            }
            COL_AppendToList(&exeBuilder, '\0');

            cstring exePathCStr = (cstring) exeBuilder.data;

            // check if path is executable
            if (access(exePathCStr, X_OK) != 0)
            {
                goto exitFn; // not executable or does not exist
            }
        }
        else
        {
            currentEnvVars = PRC_GetEnvVars(tempAllocator).slice; // to ensure PATH is loaded
            utf8str pathVar = {0};
            for (isize i = 0; i < currentEnvVars.count; i++)
            {
                PRC_EnvVarKVP kvp = currentEnvVars.data[i];
                if (STR_Eq(kvp.key, UTF8STR("PATH")))
                {
                    pathVar = kvp.value;
                    break;
                }
            }


            Slice_(utf8str) pathDirs = PRC_Internal_SplitUnixPathList(pathVar, tempAllocator);

            b8 found = false;
            for (isize pi = 0; pi < pathDirs.count; pi++)
            {
                utf8str dir = pathDirs.data[pi];

                COL_ClearList(&exeBuilder);
                for (isize i = 0; i < dir.count; i++)
                {
                    char c = (char) dir.data[i];
                    if (c == '\\') c = '/';
                    COL_AppendToList(&exeBuilder, c);
                }

                COL_AppendToList(&exeBuilder, '/');
                for (isize i = 0; i < exePath.count; i++)
                {
                    char c = (char) exePath.data[i];
                    if (c == '\\') c = '/';
                    COL_AppendToList(&exeBuilder, c);
                }
                COL_AppendToList(&exeBuilder, '\0');

                cstring fullPathCStr = (cstring) exeBuilder.data;

                if (access(fullPathCStr, X_OK) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found) // check in cwd
            {
                COL_ClearList(&exeBuilder);
                for (isize i = 0; i < exePath.count; i++)
                {
                    char c = (char) exePath.data[i];
                    if (c == '\\') c = '/';
                    COL_AppendToList(&exeBuilder, c);
                }

                if (exeBuilder.count && exeBuilder.data[exeBuilder.count - 1] != '/')
                    COL_AppendToList(&exeBuilder, '/');

                COL_AppendAllToList(&exeBuilder, UTF8STR("./"));
                for (isize i = 0; i < exePath.count; i++)
                {
                    char c = (char) exePath.data[i];
                    if (c == '\\') c = '/';
                    COL_AppendToList(&exeBuilder, c);
                }

                COL_AppendToList(&exeBuilder, '\0');

                cstring fullPathCStr = (cstring) exeBuilder.data;

                if (access(fullPathCStr, X_OK) == 0)
                {
                    found = true;
                }
            }

            if (!found) { goto exitFn; } // not found in PATH
        }

        cstring cwd = nil;
        if (workingDirectory.path.data && workingDirectory.path.count)
            cwd = STR_CloneToCStr(workingDirectory.path, tempAllocator);

        Slice_(cstring) cmd = COL_NewSlice(cstring, execAndArgs.count + 1, true, tempAllocator);
        if (!cmd.data || !cmd.count) goto exitFn;

        for (isize i = 0; i < execAndArgs.count; i++)
            cmd.data[i] = STR_CloneToCStr(execAndArgs.data[i], tempAllocator);
        cmd.data[execAndArgs.count] = nil; // null-terminate argv

        cstring* env;
        cstring* cenv = nil;
        if (!environmentVariables.count || !environmentVariables.data)
        {
            env = (cstring*) environ; // inherit from current process
        }
        else
        {
            cenv = COL_NewSlice(cstring, environmentVariables.count + 1, true, tempAllocator).data;
            if (!cenv) goto exitFn;
            for (isize i = 0; i < environmentVariables.count; i++)
                cenv[i] = STR_CloneToCStr(environmentVariables.data[i], tempAllocator);
            cenv[environmentVariables.count] = nil; // null-terminate envp

            env = cenv;
        }

        const i32 READ = 0;
        const i32 WRITE = 1;

        i32 pipeVal[2];
        if (pipe(pipeVal) != 0) { goto exitFn; }

        // make read end close-on-exec
        if (fcntl(pipeVal[READ], F_SETFD, FD_CLOEXEC) == -1)
        {
            close(pipeVal[READ]);
            close(pipeVal[WRITE]);
            goto exitFn;
        }

        // make write end close-on-exec
        if (fcntl(pipeVal[WRITE], F_SETFD, FD_CLOEXEC) == -1)
        {
            close(pipeVal[READ]);
            close(pipeVal[WRITE]);
            goto exitFn;
        }

        pid_t pid = fork();
        switch (pid)
        {
            case -1: // fork failed
            {
                close(pipeVal[WRITE]);
                close(pipeVal[READ]);
                goto exitFn;
            }

            case 0: // child
            {
                // Close read end in child; child will write a single byte on failure.
                close(pipeVal[READ]);

                i32 nullFile = open("/dev/null", O_RDWR);
                if (nullFile == -1)
                {
                    i32 errNoVal = errno;
                    // write 4 bytes of errno for safety
                    (void) write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                i32 stdoutFile = stdOutPipe ? ((IO_Internal_FileStreamData) {.asPtr = stdOutPipe->data}).handle : nullFile;
                i32 stderrFile = stdErrPipe ? ((IO_Internal_FileStreamData) {.asPtr = stdErrPipe->data}).handle : nullFile;
                i32 stdinFile  = nullFile; // we do not support stdin for now

                if (dup2(stdoutFile, STDOUT_FILENO) == -1 ||
                    dup2(stderrFile, STDERR_FILENO) == -1 ||
                    dup2(stdinFile,  STDIN_FILENO)  == -1)
                {
                    i32 errNoVal = errno;
                    (void) write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                if (cwd != nil && chdir(cwd) != 0)
                {
                    i32 errNoVal = errno;
                    (void) write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                // exeBuilder contains the full executable path (we ensured '\0' was appended earlier).
                cstring fullExePathCStr = (cstring) exeBuilder.data;

                execve(fullExePathCStr, (char* const*) cmd.data, (char* const*) env);
                // If execve returns, it's an error
                {
                    i32 errNoVal = errno;
                    (void) write(pipeVal[WRITE], &errNoVal, sizeof(errNoVal));
                    _exit(126);
                }

                break;
            }

            default: // parent
            {
                // Close write end in parent — child writes to it on error.
                close(pipeVal[WRITE]);

                i32 errNoVal = 0;

                // Read error info from child (child writes errno if it failed before exec).
                // Child writes sizeof(i32). Read that (or EOF).
                ssize_t totalRead = 0;
                while (totalRead < (ssize_t) sizeof(i32))
                {
                    ssize_t r = read(pipeVal[READ], ((u8*) &errNoVal) + totalRead, (size_t) ((ssize_t) sizeof(i32) - totalRead));
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
                        i32 wpid = waitid(P_PID, (id_t) pid, &info, WEXITED);
                        if (wpid == -1 && errno == EINTR)
                            continue; // interrupted, try again
                        break;
                    }

                    close(pipeVal[READ]);
                    goto exitFn;
                }

                // No error reported — child successfully exec'd (or at least didn't fail early).
                p.pid = pid;
                break;
            }
        }

        close(pipeVal[READ]);
    }
    #else
        #error "Process creation not implemented on this platform"
    #endif

exitFn:
    MEM_DestroyArenaAllocator(&tempArena);
    return PRC_ToHandle(p);
}

b8 PRC_Wait(PRC_Handle* process, i32* outExitCode)
{
    if (!process) return false;

    PRC_PlatformHandle p = PRC_FromHandle(*process);

#if MSR_WINDOWS
    if (!p.handle) return false;

    DWORD waitResult = WaitForSingleObject(p.handle, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
        return false;

    if (outExitCode)
    {
        DWORD e;
        *outExitCode = GetExitCodeProcess(p.handle, &e) ? (i32) e : -1;
    }

    CloseHandle(p.handle);
    p.handle = nil;
#else
    i32 status = 0;
    pid_t pidResult = waitpid(p.pid, &status, 0);
    if (pidResult == -1)
        return false;

    if (outExitCode)
    {
        if (WIFEXITED(status))
            *outExitCode = (i32) WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            *outExitCode = (i32) -WTERMSIG(status);
        else
            *outExitCode = (i32) -1;
    }

    p.pid = -1;
#endif

    *process = PRC_ToHandle(p);
    return true;
}

b8 PRC_Kill(PRC_Handle* process)
{
    if (!process) return false;

    PRC_PlatformHandle p = PRC_FromHandle(*process);

    b8 success = false;
#if MSR_WINDOWS
    if (!p.handle) return false;

    success = !!TerminateProcess(p.handle, 1);
    CloseHandle(p.handle);
    p.handle = nil;
#else
    if (p.pid <= 0) return false;

    success = (kill(p.pid, SIGKILL) == 0);
    p.pid = -1;
#endif

    *process = PRC_ToHandle(p);
    return success;
}
