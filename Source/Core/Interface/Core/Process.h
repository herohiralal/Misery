#pragma once
#include <__init.h>
#include "Stream.h"

EXTERN_C_BEGIN

/**
 * Retrieves the path of the currently running executable.
 * The returned path is allocated using the provided allocator.
 */
FIL_Path PRC_GetCurrentExecutablePath(MEM_Allocator);

/**
 * Exits the current process immediately with the specified exit code.
 */
MSR_NORETURN void PRC_Exit(i32 exitCode OPT_ARG);

/**
 * A key-value pair representing an environment variable.
 * The `kvp` field contains the full "KEY=VALUE" string.
 * The 'key' field contains the key part.
 * The 'value' field contains the value part.
 */
typedef struct
{
    utf8str kvp;
    utf8str key;
    utf8str value;
} PRC_EnvVarKVP;

COL_DECLARE_FOR(PRC_EnvVarKVP)

/**
 * Retrieves all environment variables as a slice of key-value pairs.
 * The returned slice is allocated using the provided allocator.
 * The individual strings within the key-value pairs are also allocated using the same allocator.
 * For the key-value pairs, the `kvp` field contains the full "KEY=VALUE" string,
 * while the `key` and `value` fields are just 'views' into that string.
 */
List_(PRC_EnvVarKVP) PRC_GetEnvVars(MEM_Allocator);

/**
 * Frees the memory associated with the environment variables and its contents.
 * This should be called for any slice returned by `PRC_GetEnvVars` when it is no longer needed.
 */
void PRC_FreeEnvVars(List_(PRC_EnvVarKVP)* envVars);

/**
 * An opaque handle to a process.
 */
typedef struct
{
    u64 handle;
} PRC_Handle;

/**
 * Checks if the given process handle is valid.
 * A handle is considered valid if it represents an active process that can be waited on or killed.
 */
b8 PRC_IsValid(PRC_Handle handle);

/**
 * Starts a new process with the specified executable and arguments.
 * Optionally, environment variables, working directory, and pipes for
 * standard output and error can be provided.
 *
 * If not provided, environment variables and working directory are inherited
 * from the current process. If provided, they must be in a an array of
 * "KEY=VALUE" format.
 *
 * The pipe handles provided must be read ends for stdout and stderr respectively.
 * If null, the respective output is discarded.
 */
PRC_Handle PRC_Run(
    Slice_(utf8str) execAndArgs,
    Slice_(utf8str) environmentVariables OPT_ARG,
    DIR_Path        workingDirectory     OPT_ARG,
    IO_Stream*      stdOutPipe           OPT_ARG,
    IO_Stream*      stdErrPipe           OPT_ARG
);

/**
 * Waits for the given process to exit and retrieves its exit code.
 * Will also free the resources associated with the process handle.
 * Returns true if the process exited cleanly or false on failure.
 * The exit code is stored in *outExitCode if provided.
 */
b8 PRC_Wait(PRC_Handle* process, i32* outExitCode OPT_ARG);

/**
 * Kills the given process immediately.
 * Will also free the resources associated with the process handle.
 * Returns true if the signal/termination request succeeded.
 */
b8 PRC_Kill(PRC_Handle* process);

/**
 * An opaque handle to a dynamically loaded library.
 */
typedef struct { rawptr handle; } PRC_Library;

/**
 * Loads a dynamic library from the given path.
 * Returns a zero-value handle on failure or if the path is empty.
 */
PRC_Library PRC_LoadLibrary(FIL_Path path);

/**
 * Retrieves a function pointer from a loaded dynamic library by name.
 * Returns nil if the library handle is nil, or if the string is empty.
 * Returns nil if the symbol is not found.
 */
rawptr PRC_GetLibraryFunction(PRC_Library lib, utf8str name);

/**
 * Unloads a dynamic library and frees associated resources.
 */
void PRC_UnloadLibrary(PRC_Library lib);

EXTERN_C_END
