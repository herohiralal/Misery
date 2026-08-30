#pragma once
#include <__init.h>
#include "Memory.h"
#include "Strings.h"
EXTERN_C_BEGIN

/**
 * An opaque handle to a thread.
 */
typedef struct THR_Handle { usize handle; } THR_Handle;

/**
 * An opaque identifier for a thread.
 * Do not compare directly, use `THR_Eq` instead.
 */
typedef struct THR_Id { usize id; } THR_Id;

/**
 * Checks if the handle to a thread is valid.
 */
b8 THR_IsValid(THR_Handle handle);

/**
 * Gets a handle to the current thread.
 */
THR_Handle THR_GetCurrent(void);

/**
 * Gets the identifier of a thread.
 */
THR_Id THR_GetId(THR_Handle handle);

/**
 * Checks if two thread identifiers are equal.
 */
b8 THR_Eq(THR_Id a, THR_Id b);

/**
 * Gets the name of a thread.
 * The returned string is allocated using the provided allocator.
 * If the thread has no name, a fallback name of the form "Thread#<handle>" is returned.
 */
utf8str THR_GetName(THR_Handle handle, MEM_Allocator allocator);

/**
 * Sets the name of a thread.
 * The name is copied, so the provided string does not need to be valid after this call.
 * On some platforms, thread names may be truncated to a certain length.
 *
 * Maximum name lengths on platforms (excluding null terminator):
 *     Windows/OSX/iOS - 63 characters
 *     Linux/Android   - 15 characters
 */
void THR_SetName(THR_Handle handle, utf8str name);

/**
 * Gets the name of the current thread.
 * Read more about `THR_GetName`.
 */
utf8str THR_GetCurrentName(MEM_Allocator allocator);

/**
 * Sets the name of the current thread.
 * Read more about `THR_SetName`.
 */
void THR_SetCurrentName(utf8str name);

/**
 * A procedure that can be run on a thread.
 * The `data` parameter is optional user data that can be passed to the thread.
 */
typedef void (*THR_Proc)(rawptr data);

/**
 * Start a new thread with the specified procedure and user data.
 */
THR_Handle THR_Start(THR_Proc procedure, rawptr data OPT_ARG, utf8str name OPT_ARG);

/**
 * Joins a thread, blocking the calling thread until the specified thread has finished.
 */
void THR_Join(THR_Handle handle);

/**
 * Sleeps the current thread for the specified number of milliseconds.
 */
void THR_Sleep(u64 milliseconds);

EXTERN_C_END
