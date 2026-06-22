#pragma once
#include <__init.h>
EXTERN_C_BEGIN

// Mutex ===========================================================================

/**
 * The most basic synchronization primitive.
 */
typedef struct alignas(MSR_PTR_SIZE) SYN_Mutex
{
    u8 buffer[8 * MSR_PTR_SIZE];
} SYN_Mutex;

/**
 * Creates a mutex.
 */
SYN_Mutex SYN_CreateMutex(void);

/**
 * Destroys a mutex.
 */
void SYN_DestroyMutex(SYN_Mutex* mutex);

/**
 * Locks a mutex.
 */
void SYN_LockMutex(SYN_Mutex* mutex);

/**
 * Unlocks a mutex.
 */
void SYN_UnlockMutex(SYN_Mutex* mutex);

/**
 * Tries to lock a mutex.
 * Returns true if the mutex was successfully locked, false otherwise.
 */
b8 SYN_TryLockMutex(SYN_Mutex* mutex);

// Read-Write Mutex ================================================================

/**
 * A read-write mutex.
 * This is a synchronization primitive that allows multiple readers or a single writer.
 * It is useful for scenarios where reads are more frequent than writes.
 */
typedef struct alignas(MSR_PTR_SIZE) SYN_RWMutex
{
    u8 buffer[25 * MSR_PTR_SIZE];
} SYN_RWMutex;

/**
 * Creates a read-write mutex.
 */
SYN_RWMutex SYN_CreateRWMutex(void);

/**
 * Destroys a read-write mutex.
 */
void SYN_DestroyRWMutex(SYN_RWMutex* rwmutex);

/**
 * Locks a read-write mutex for reading.
 * Multiple threads can read simultaneously.
 */
void SYN_LockRWMutexShared(SYN_RWMutex* rwmutex);

/**
 * Locks a read-write mutex for writing.
 * Only one thread can write at a time, and no other threads can read while writing.
 */
void SYN_LockRWMutexExclusive(SYN_RWMutex* rwmutex);

/**
 * Unlocks a read-write mutex after reading.
 */
void SYN_UnlockRWMutexShared(SYN_RWMutex* rwmutex);

/**
 * Unlocks a read-write mutex after writing.
 */
void SYN_UnlockRWMutexExclusive(SYN_RWMutex* rwmutex);

/**
 * Tries to lock a read-write mutex for reading.
 * Returns true if the mutex was successfully locked for reading, false otherwise.
 */
b8 SYN_TryLockRWMutexShared(SYN_RWMutex* rwmutex);

/**
 * Tries to lock a read-write mutex for writing.
 * Returns true if the mutex was successfully locked for writing, false otherwise.
 */
b8 SYN_TryLockRWMutexExclusive(SYN_RWMutex* rwmutex);

// Semaphore =======================================================================

/**
 * A semaphore synchronization primitive.
 * It allows a certain number of threads to access a resource concurrently.
 */
typedef struct alignas(MSR_PTR_SIZE) SYN_Semaphore
{
    u8 buffer[4 * MSR_PTR_SIZE];
} SYN_Semaphore;

/**
 * Creates a semaphore.
 * The initial count specifies how many threads can access the resource concurrently.
 */
SYN_Semaphore SYN_CreateSemaphore(i32 initialCount);

/**
 * Destroys a semaphore.
 */
void SYN_DestroySemaphore(SYN_Semaphore* semaphore);

/**
 * Waits on a semaphore.
 * The calling thread will block until the semaphore count is greater than zero.
 */
void SYN_WaitSemaphore(SYN_Semaphore* semaphore);

/**
 * Waits on a semaphore with a timeout.
 * The calling thread will block until the semaphore count is greater than zero or the timeout expires.
 * Returns true if the semaphore was acquired, false if the timeout expired.
 */
b8 SYN_WaitSemaphoreTimeout(SYN_Semaphore* semaphore, i32 timeoutNs);

/**
 * Signals a semaphore, incrementing its count by the specified amount.
 * If the count was zero, this will wake up one or more waiting threads.
 */
void SYN_SignalSemaphore(SYN_Semaphore* semaphore, i32 count);

// Condition Variable ==============================================================

/**
 * A condition variable for signaling between threads.
 * It allows threads to wait for a condition to be signaled.
 */
typedef struct alignas(MSR_PTR_SIZE) SYN_ConditionVariable
{
    u8 buffer[6 * MSR_PTR_SIZE];
} SYN_ConditionVariable;

/**
 * Creates a condition variable.
 */
SYN_ConditionVariable SYN_CreateConditionVariable(void);

/**
 * Destroys a condition variable.
 */
void SYN_DestroyConditionVariable(SYN_ConditionVariable* condvar);

/**
 * Waits on a condition variable.
 * The calling thread will block until the condition variable is signaled.
 * The mutex must be locked before calling this function.
 */
void SYN_WaitConditionVariable(SYN_ConditionVariable* condvar, SYN_Mutex* mutex);

/**
 * Waits on a condition variable with a timeout.
 * The calling thread will block until the condition variable is signaled or the timeout expires.
 * The mutex must be locked before calling this function.
 * Returns true if the condition variable was signaled, false if the timeout expired.
 */
b8 SYN_WaitConditionVariableTimeout(SYN_ConditionVariable* condvar, SYN_Mutex* mutex, i32 timeoutNs);

/**
 * Signals a condition variable, waking up one waiting thread.
 * If no threads are waiting, this has no effect.
 */
void SYN_SignalConditionVariable(SYN_ConditionVariable* condvar);

/**
 * Signals a condition variable, waking up all waiting threads.
 * If no threads are waiting, this has no effect.
 */
void SYN_BroadcastConditionVariable(SYN_ConditionVariable* condvar);

// Do Once =========================================================================

/**
 * A "do once" primitive.
 * It ensures that a specified initialization function is executed only once, even
 * if called from multiple threads.
 * This is useful for one-time initialization of shared resources.
 */
typedef struct alignas(MSR_PTR_SIZE) SYN_DoOnce
{
    u8 buffer[2 * MSR_PTR_SIZE];
} SYN_DoOnce;

/**
 * The callback function type for the "do once" primitive.
 */
typedef void (*SYN_DoOnceCallback)(void);

/**
 * Executes the specified callback function only once.
 * If multiple threads call this function simultaneously, only one will execute the callback.
 */
void SYN_ExecuteDoOnce(SYN_DoOnce* once, SYN_DoOnceCallback callback);

// Event ===========================================================================

/**
 * An event synchronization primitive.
 * It allows one or more threads to wait until another thread signals a condition.
 */
typedef struct alignas(MSR_PTR_SIZE) SYN_Event
{
    u8 buffer[15 * MSR_PTR_SIZE];
} SYN_Event;

/**
 * Creates an event.
 * If manualReset is true, the event must be manually reset after being signaled.
 * If manualReset is false, the event automatically resets after releasing one waiting thread.
 */
SYN_Event SYN_CreateEvent(b8 manualReset);

/**
 * Destroys an event.
 */
void SYN_DestroyEvent(SYN_Event* event);

/**
 * Waits on an event.
 * The calling thread will block until the event is signaled.
 */
void SYN_WaitEvent(SYN_Event* event);

/**
 * Waits on an event with a timeout.
 * The calling thread will block until the event is signaled or the timeout expires.
 * Returns true if the event was signaled, false if the timeout expired.
 */
b8 SYN_WaitEventTimeout(SYN_Event* event, i32 timeoutNs);

/**
 * Signals an event.
 * If manualReset is false, wakes up one waiting thread and resets automatically.
 * If manualReset is true, wakes up all waiting threads and remains signaled until reset.
 */
void SYN_SignalEvent(SYN_Event* event);

/**
 * Resets an event, returning it to the unsignaled state.
 * Only meaningful for manual-reset events.
 */
void SYN_ResetEvent(SYN_Event* event);

EXTERN_C_END
