#include "Core/Process.h"
#include <Core/Core.h>

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

        PRC_EnvVarKVP var =
        {
            .kvp   = kvp,
            .key   = STR_SubString(kvp, 0,          keyLen),
            .value = STR_SubString(kvp, keyLen + 1, valLen)
        };

        COL_AppendToList(&output, var);

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

static cstring PRC_Internal_BuildWindowsProcessCmdLine(Slice_(utf8str) execAndArgs, MEM_Allocator allocator)
{
    isize minLen = 25; // adjusting for maybe 25 chars of extra backslashes and the null terminator
    for (isize i = 0; i < execAndArgs.count; i++)
        minLen += execAndArgs.data[i].count + 3; // +3 for potential quotes and a space

    List_(char) sb = COL_NewList(char, minLen, allocator);

    for (i64 i = 0; i < execAndArgs.count; i++)
    {
        utf8str arg = execAndArgs.data[i];

        if (i != 0) { COL_AppendToList(&sb, ' '); }

        // Escape and quote the argument as needed
        b8 needsQuotes = false;
        for (i64 j = 0; j < arg.count; j++)
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

        i64 j = 0;
        while (j < arg.count)
        {
            i64 backslashes = 0;

            while (j < arg.count && arg.data[j] == '\\') { backslashes++; j++; }

            if (j == arg.count)
            {
                // Escape all backslashes at the end
                for (i64 k = 0; k < backslashes * 2; k++)
                    COL_AppendToList(&sb, '\\');
                break;
            }
            else if (arg.data[j] == '\"')
            {
                // Escape all backslashes and the quote
                for (i64 k = 0; k < (backslashes * 2) + 1; k++)
                    COL_AppendToList(&sb, '\\');

                COL_AppendToList(&sb, '\"');
            }
            else
            {
                // No special handling needed, just output the backslashes
                for (i64 k = 0; k < backslashes; k++)
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

cstring PRC_Internal_BuildWindowsProcessEnvBlock(List_(utf8str) envVars, MEM_Allocator allocator)
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

        utf8str key   = STR_SubString(kv, 0,         eqIdx);
        utf8str value = STR_SubString(kv, eqIdx + 1, kv.count - eqIdx - 1);

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

    i64 index = 0;
    for (i64 i = 0; i < pathStr.count; i++)
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

    for (i64 i = 0; i < paths.count; i++)
    {
        paths.data[i] = STR_Replace(paths.data[i], UTF8STR("\""), UTF8STR(""), allocator, false);
    }

    return paths;
}

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
    int status = 0;
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

    p.pid = 0;
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
    p.pid = 0;
#endif

    return success;
}
