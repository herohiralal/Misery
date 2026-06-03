#pragma once
#include <__init.h>
#include "Stream.h"

EXTERN_C_BEGIN

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
 * A collection of environment variables represented as a slice of key-value pairs.
 * The `data` field contains the slice of environment variable key-value pairs.
 * The `allocator` field indicates the allocator used for the slice and its contents.
 */
typedef struct
{
    Slice_(PRC_EnvVarKVP) data;
    MEM_Allocator allocator;
} PRC_EnvVars;

/**
 * Retrieves all environment variables as a slice of key-value pairs.
 * The returned slice is allocated using the provided allocator.
 * The individual strings within the key-value pairs are also allocated using the same allocator.
 * For the key-value pairs, the `kvp` field contains the full "KEY=VALUE" string,
 * while the `key` and `value` fields are just 'views' into that string.
 */
PRC_EnvVars PRC_GetEnvVars(MEM_Allocator);

/**
 * Frees the memory associated with the environment variables and its contents.
 * This should be called for any slice returned by `PRC_GetEnvVars` when it is no longer needed.
 */
void PRC_FreeEnvVars(PRC_EnvVars*);

/**
 * A handle to a process.
 * The `pid` field is the process ID.
 * On Windows, this is `dwProcessId`.
 * On Unix-like systems, this is the PID.
 * The `handle` field is a platform-specific handle to the process.
 * On Windows, this is a HANDLE.
 * On Unix-like systems, this is pidfd.
 */
typedef struct
{
    i64 pid;
    u64 handle;
} PRC_Handle;

EXTERN_C_END
