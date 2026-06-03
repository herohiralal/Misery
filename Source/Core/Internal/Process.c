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

PRC_EnvVars PRC_GetEnvVars(MEM_Allocator allocator)
{
    PRC_EnvVars output = {.allocator = allocator};

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

    output.data = COL_NewSlice(PRC_EnvVarKVP, envVarsCount, true, allocator);
    if (!output.data.data || !output.data.count)
        return (PRC_EnvVars) {0};

    isize index = 0;

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

        PRC_EnvVarKVP* kvpEntry = &(output.data.data[index]);
        kvpEntry->kvp   = kvp;
        kvpEntry->key   = STR_SubString(kvp, 0,          keyLen);
        kvpEntry->value = STR_SubString(kvp, keyLen + 1, valLen);
        index++;

#if MSR_UNIX
    }
#elif MSR_WINDOWS
    }

    FreeEnvironmentStringsA(envStringsWindows);
#else
        #error "unsupported platform"
#endif

    output.data = COL_SubSlice(output.data, 0, index); // in case of malformed entries
    return output;
}

void PRC_FreeEnvVars(PRC_EnvVars* vars)
{
    if (!vars || !vars->data.data || !vars->data.count)
        return;

    for (i64 i = 0; i < vars->data.count; i++)
        COL_DeleteSlice(&(vars->data.data[i].kvp), vars->allocator);

    COL_DeleteSlice(&vars->data, vars->allocator);
    *vars = (PRC_EnvVars) {0};
}
