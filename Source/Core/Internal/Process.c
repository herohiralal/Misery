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
