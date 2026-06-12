#include "CorePrivate.h"

#if MSR_WINDOWS

    static_assert(sizeof  (SYN_Mutex) >= sizeof  (CRITICAL_SECTION),       "SYN_Mutex must be at least as large as CRITICAL_SECTION.");
    static_assert((alignof(SYN_Mutex) %  alignof (CRITICAL_SECTION)) == 0, "SYN_Mutex must be at least as aligned as CRITICAL_SECTION.");

    static_assert(sizeof  (SYN_RWMutex) >= sizeof  (SRWLOCK),       "SYN_RWMutex must be at least as large as SRWLOCK.");
    static_assert((alignof(SYN_RWMutex) %  alignof (SRWLOCK)) == 0, "SYN_RWMutex must be at least as aligned as SRWLOCK.");

    static_assert(sizeof  (SYN_Semaphore) >= sizeof  (HANDLE),       "SYN_Semaphore must be at least as large as HANDLE.");
    static_assert((alignof(SYN_Semaphore) %  alignof (HANDLE)) == 0, "SYN_Semaphore must be at least as aligned as HANDLE.");

    static_assert(sizeof  (SYN_ConditionVariable) >= sizeof  (CONDITION_VARIABLE),       "SYN_ConditionVariable must be at least as large as CONDITION_VARIABLE.");
    static_assert((alignof(SYN_ConditionVariable) %  alignof (CONDITION_VARIABLE)) == 0, "SYN_ConditionVariable must be at least as aligned as CONDITION_VARIABLE.");

    static_assert(sizeof  (SYN_DoOnce) >= sizeof  (INIT_ONCE),       "SYN_DoOnce must be at least as large as INIT_ONCE.");
    static_assert((alignof(SYN_DoOnce) %  alignof (INIT_ONCE)) == 0, "SYN_DoOnce must be at least as aligned as INIT_ONCE.");

    static_assert(sizeof  (SYN_Event) >= sizeof  (HANDLE),       "SYN_Event must be at least as large as HANDLE.");
    static_assert((alignof(SYN_Event) %  alignof (HANDLE)) == 0, "SYN_Event must be at least as aligned as HANDLE.");

#elif MSR_UNIX

    typedef struct SYN_Internal_EventUnix
    {
        pthread_mutex_t mutex;
        pthread_cond_t  cond;
        b8              signaled;
        b8              manualReset;
    } SYN_Internal_EventUnix;

    static_assert(sizeof  (SYN_Mutex) >= sizeof  (pthread_mutex_t),       "SYN_Mutex must be at least as large as pthread_mutex_t.");
    static_assert((alignof(SYN_Mutex) %  alignof (pthread_mutex_t)) == 0, "SYN_Mutex must be at least as aligned as pthread_mutex_t.");

    static_assert(sizeof  (SYN_RWMutex) >= sizeof  (pthread_rwlock_t),       "SYN_RWMutex must be at least as large as pthread_rwlock_t.");
    static_assert((alignof(SYN_RWMutex) %  alignof (pthread_rwlock_t)) == 0, "SYN_RWMutex must be at least as aligned as pthread_rwlock_t.");

    #if MSR_APPLE
        static_assert(sizeof  (SYN_Semaphore) >= sizeof  (dispatch_semaphore_t),       "SYN_Semaphore must be at least as large as dispatch_semaphore_t.");
        static_assert((alignof(SYN_Semaphore) %  alignof (dispatch_semaphore_t)) == 0, "SYN_Semaphore must be at least as aligned as dispatch_semaphore_t.");
    #else
        static_assert(sizeof  (SYN_Semaphore) >= sizeof  (sem_t),       "SYN_Semaphore must be at least as large as sem_t.");
        static_assert((alignof(SYN_Semaphore) %  alignof (sem_t)) == 0, "SYN_Semaphore must be at least as aligned as sem_t.");
    #endif

    static_assert(sizeof  (SYN_ConditionVariable) >= sizeof  (pthread_cond_t),       "SYN_ConditionVariable must be at least as large as pthread_cond_t.");
    static_assert((alignof(SYN_ConditionVariable) %  alignof (pthread_cond_t)) == 0, "SYN_ConditionVariable must be at least as aligned as pthread_cond_t.");

    static_assert(sizeof  (SYN_DoOnce) >= sizeof  (pthread_once_t),       "SYN_DoOnce must be at least as large as pthread_once_t.");
    static_assert((alignof(SYN_DoOnce) %  alignof (pthread_once_t)) == 0, "SYN_DoOnce must be at least as aligned as pthread_once_t.");

    static_assert(sizeof  (SYN_Event) >= sizeof  (SYN_Internal_EventUnix),       "SYN_Event must be at least as large as SYN_Internal_EventUnix.");
    static_assert((alignof(SYN_Event) %  alignof (SYN_Internal_EventUnix)) == 0, "SYN_Event must be at least as aligned as SYN_Internal_EventUnix.");

#else
    #error "Unsupported platform."
#endif

// Mutex ===========================================================================

SYN_Mutex SYN_CreateMutex(void)
{
    SYN_Mutex  output;
    SYN_Mutex* mutex = &output;

    #if MSR_WINDOWS
        InitializeCriticalSection((CRITICAL_SECTION*) mutex);
        SetCriticalSectionSpinCount((CRITICAL_SECTION*) mutex, 4000);
    #elif MSR_UNIX
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init((pthread_mutex_t*) mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    #else
        #error "Unknown platform."
    #endif

    return output;
}

void SYN_DestroyMutex(SYN_Mutex* mutex)
{
    #if MSR_WINDOWS
        DeleteCriticalSection((CRITICAL_SECTION*) mutex);
    #elif MSR_UNIX
        pthread_mutex_destroy((pthread_mutex_t*) mutex);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_LockMutex(SYN_Mutex* mutex)
{
    #if MSR_WINDOWS
        EnterCriticalSection((CRITICAL_SECTION*) mutex);
    #elif MSR_UNIX
        pthread_mutex_lock((pthread_mutex_t*) mutex);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_UnlockMutex(SYN_Mutex* mutex)
{
    #if MSR_WINDOWS
        LeaveCriticalSection((CRITICAL_SECTION*) mutex);
    #elif MSR_UNIX
        pthread_mutex_unlock((pthread_mutex_t*) mutex);
    #else
        #error "Unknown platform."
    #endif
}

b8 SYN_TryLockMutex(SYN_Mutex* mutex)
{
    #if MSR_WINDOWS
        return (b8) !!TryEnterCriticalSection((CRITICAL_SECTION*) mutex);
    #elif MSR_UNIX
        return (b8) (0 == pthread_mutex_trylock((pthread_mutex_t*) mutex));
    #else
        #error "Unknown platform."
    #endif
}

// Read-Write Mutex ================================================================

SYN_RWMutex SYN_CreateRWMutex(void)
{
    SYN_RWMutex  output;
    SYN_RWMutex* rwmutex = &output;

    #if MSR_WINDOWS
        InitializeSRWLock((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        pthread_rwlock_init((pthread_rwlock_t*) rwmutex, nil);
    #else
        #error "Unknown platform."
    #endif

    return output;
}

void SYN_DestroyRWMutex(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        (void) rwmutex; // SRWLOCK has no destruction API
    #elif MSR_UNIX
        pthread_rwlock_destroy((pthread_rwlock_t*) rwmutex);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_LockRWMutexShared(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        AcquireSRWLockShared((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        pthread_rwlock_rdlock((pthread_rwlock_t*) rwmutex);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_LockRWMutexExclusive(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        AcquireSRWLockExclusive((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        pthread_rwlock_wrlock((pthread_rwlock_t*) rwmutex);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_UnlockRWMutexShared(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        ReleaseSRWLockShared((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        pthread_rwlock_unlock((pthread_rwlock_t*) rwmutex);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_UnlockRWMutexExclusive(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        ReleaseSRWLockExclusive((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        pthread_rwlock_unlock((pthread_rwlock_t*) rwmutex);
    #else
        #error "Unknown platform."
    #endif
}

b8 SYN_TryLockRWMutexShared(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        return (b8) !!TryAcquireSRWLockShared((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        return (b8) (0 == pthread_rwlock_tryrdlock((pthread_rwlock_t*) rwmutex));
    #else
        #error "Unknown platform."
    #endif
}

b8 SYN_TryLockRWMutexExclusive(SYN_RWMutex* rwmutex)
{
    #if MSR_WINDOWS
        return (b8) !!TryAcquireSRWLockExclusive((SRWLOCK*) rwmutex);
    #elif MSR_UNIX
        return (b8) (0 == pthread_rwlock_trywrlock((pthread_rwlock_t*) rwmutex));
    #else
        #error "Unknown platform."
    #endif
}

// Semaphore =======================================================================

SYN_Semaphore SYN_CreateSemaphore(i32 initialCount)
{
    SYN_Semaphore  output;
    SYN_Semaphore* semaphore = &output;

    #if MSR_WINDOWS
        HANDLE h = CreateSemaphoreExA(nil, initialCount, I32_MAX, nil, 0, SEMAPHORE_ALL_ACCESS);
        *((HANDLE*) semaphore) = h;
    #elif MSR_APPLE
        dispatch_semaphore_t s = dispatch_semaphore_create(initialCount);
        *((dispatch_semaphore_t*) semaphore) = s;
    #elif MSR_UNIX
        sem_init((sem_t*) semaphore, 0, (u32) initialCount);
    #else
        #error "Unknown platform."
    #endif

    return output;
}

void SYN_DestroySemaphore(SYN_Semaphore* semaphore)
{
    #if MSR_WINDOWS
        CloseHandle(*(HANDLE*) semaphore);
    #elif MSR_APPLE
        dispatch_release(*(dispatch_semaphore_t*) semaphore);
    #elif MSR_UNIX
        sem_destroy((sem_t*) semaphore);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_WaitSemaphore(SYN_Semaphore* semaphore)
{
    #if MSR_WINDOWS
        WaitForSingleObject(*(HANDLE*) semaphore, INFINITE);
    #elif MSR_APPLE
        dispatch_semaphore_wait(*(dispatch_semaphore_t*) semaphore, DISPATCH_TIME_FOREVER);
    #elif MSR_UNIX
        sem_wait((sem_t*) semaphore);
    #else
        #error "Unknown platform."
    #endif
}

b8 SYN_WaitSemaphoreTimeout(SYN_Semaphore* semaphore, i32 timeoutNs)
{
    #if MSR_WINDOWS
        DWORD result = WaitForSingleObject(*(HANDLE*) semaphore, (DWORD) (timeoutNs / 1000000));
        return result == WAIT_OBJECT_0;
    #elif MSR_APPLE
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, timeoutNs);
        return dispatch_semaphore_wait(*(dispatch_semaphore_t*) semaphore, timeout) == 0;
    #elif MSR_UNIX
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timeoutNs / 1000000000;
        ts.tv_nsec += timeoutNs % 1000000000;
        if (sem_timedwait((sem_t*) semaphore, &ts) == 0) return true;
        return false;
    #else
        #error "Unknown platform."
    #endif
}

void SYN_SignalSemaphore(SYN_Semaphore* semaphore, i32 count)
{
    #if MSR_WINDOWS
        LONG previousCount;
        ReleaseSemaphore(*(HANDLE*) semaphore, (LONG) count, &previousCount);
    #elif MSR_APPLE
        for (i32 i = 0; i < count; ++i)
            dispatch_semaphore_signal(*(dispatch_semaphore_t*) semaphore);
    #elif MSR_UNIX
        for (i32 i = 0; i < count; ++i)
            sem_post((sem_t*) semaphore);
    #else
        #error "Unknown platform."
    #endif
}

// Condition Variable ==============================================================

SYN_ConditionVariable SYN_CreateConditionVariable(void)
{
    SYN_ConditionVariable  output;
    SYN_ConditionVariable* condvar = &output;

    #if MSR_WINDOWS
        InitializeConditionVariable((CONDITION_VARIABLE*) condvar);
    #elif MSR_UNIX
        pthread_cond_init((pthread_cond_t*) condvar, nil);
    #else
        #error "Unknown platform."
    #endif

    return output;
}

void SYN_DestroyConditionVariable(SYN_ConditionVariable* condvar)
{
    #if MSR_WINDOWS
        (void) condvar; // CONDITION_VARIABLE has no destruction API
    #elif MSR_UNIX
        pthread_cond_destroy((pthread_cond_t*) condvar);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_WaitConditionVariable(SYN_ConditionVariable* condvar, SYN_Mutex* mutex)
{
    #if MSR_WINDOWS
        SleepConditionVariableCS((CONDITION_VARIABLE*) condvar, (CRITICAL_SECTION*) mutex, INFINITE);
    #elif MSR_UNIX
        pthread_cond_wait((pthread_cond_t*) condvar, (pthread_mutex_t*) mutex);
    #else
        #error "Unknown platform."
    #endif
}

b8 SYN_WaitConditionVariableTimeout(SYN_ConditionVariable* condvar, SYN_Mutex* mutex, i32 timeoutNs)
{
    #if MSR_WINDOWS
        DWORD result = SleepConditionVariableCS(
            (CONDITION_VARIABLE*) condvar,
            (CRITICAL_SECTION*)   mutex,
            (DWORD) (timeoutNs / 1000000)
        );
        if (result) return true;
        return false;
    #elif MSR_UNIX
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timeoutNs / 1000000000;
        ts.tv_nsec += timeoutNs % 1000000000;
        if (pthread_cond_timedwait((pthread_cond_t*) condvar, (pthread_mutex_t*) mutex, &ts) == 0)
            return true;
        return false;
    #else
        #error "Unknown platform."
    #endif
}

void SYN_SignalConditionVariable(SYN_ConditionVariable* condvar)
{
    #if MSR_WINDOWS
        WakeConditionVariable((CONDITION_VARIABLE*) condvar);
    #elif MSR_UNIX
        pthread_cond_signal((pthread_cond_t*) condvar);
    #else
        #error "Unknown platform."
    #endif
}

void SYN_BroadcastConditionVariable(SYN_ConditionVariable* condvar)
{
    #if MSR_WINDOWS
        WakeAllConditionVariable((CONDITION_VARIABLE*) condvar);
    #elif MSR_UNIX
        pthread_cond_broadcast((pthread_cond_t*) condvar);
    #else
        #error "Unknown platform."
    #endif
}

// Do Once =========================================================================

#if MSR_WINDOWS
    static BOOL CALLBACK SYN_Internal_InitOnceImpl(PINIT_ONCE initOnce, PVOID parameter, PVOID* lpCtx)
    {
        (void) initOnce;
        (void) lpCtx;
        ((SYN_DoOnceCallback) parameter)();
        return TRUE;
    }
#endif

void SYN_ExecuteDoOnce(SYN_DoOnce* once, SYN_DoOnceCallback callback)
{
    #if MSR_WINDOWS
        InitOnceExecuteOnce((PINIT_ONCE) once, SYN_Internal_InitOnceImpl, (PVOID) callback, nil);
    #elif MSR_UNIX
        pthread_once((pthread_once_t*) once, callback);
    #else
        #error "Unknown platform."
    #endif
}

// Event ===========================================================================

SYN_Event SYN_CreateEvent(b8 manualReset)
{
    SYN_Event output;

    #if MSR_WINDOWS
        HANDLE h = CreateEventA(nil, (BOOL) manualReset, FALSE, nil);
        *((HANDLE*) &output) = h;
    #elif MSR_UNIX
        SYN_Internal_EventUnix* e = (SYN_Internal_EventUnix*) &output;
        pthread_mutex_init(&e->mutex, nil);
        pthread_cond_init(&e->cond, nil);
        e->signaled    = false;
        e->manualReset = manualReset;
    #endif

    return output;
}

void SYN_DestroyEvent(SYN_Event* event)
{
    #if MSR_WINDOWS
        CloseHandle(*(HANDLE*) event);
    #elif MSR_UNIX
        SYN_Internal_EventUnix* e = (SYN_Internal_EventUnix*) event;
        pthread_mutex_destroy(&e->mutex);
        pthread_cond_destroy(&e->cond);
    #endif
}

void SYN_WaitEvent(SYN_Event* event)
{
    #if MSR_WINDOWS
        WaitForSingleObject(*(HANDLE*) event, INFINITE);
    #elif MSR_UNIX
        SYN_Internal_EventUnix* e = (SYN_Internal_EventUnix*) event;
        pthread_mutex_lock(&e->mutex);
        while (!e->signaled)
            pthread_cond_wait(&e->cond, &e->mutex);
        if (!e->manualReset)
            e->signaled = false;
        pthread_mutex_unlock(&e->mutex);
    #endif
}

b8 SYN_WaitEventTimeout(SYN_Event* event, i32 timeoutNs)
{
    #if MSR_WINDOWS
        DWORD result = WaitForSingleObject(*(HANDLE*) event, (DWORD) (timeoutNs / 1000000));
        return result == WAIT_OBJECT_0;
    #elif MSR_UNIX
        SYN_Internal_EventUnix* e = (SYN_Internal_EventUnix*) event;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timeoutNs / 1000000000;
        ts.tv_nsec += timeoutNs % 1000000000;
        pthread_mutex_lock(&e->mutex);
        while (!e->signaled)
        {
            if (pthread_cond_timedwait(&e->cond, &e->mutex, &ts) == ETIMEDOUT)
            {
                pthread_mutex_unlock(&e->mutex);
                return false;
            }
        }
        if (!e->manualReset)
            e->signaled = false;
        pthread_mutex_unlock(&e->mutex);
        return true;
    #endif
}

void SYN_SignalEvent(SYN_Event* event)
{
    #if MSR_WINDOWS
        SetEvent(*(HANDLE*) event);
    #elif MSR_UNIX
        SYN_Internal_EventUnix* e = (SYN_Internal_EventUnix*) event;
        pthread_mutex_lock(&e->mutex);
        e->signaled = true;
        if (e->manualReset)
            pthread_cond_broadcast(&e->cond);
        else
            pthread_cond_signal(&e->cond);
        pthread_mutex_unlock(&e->mutex);
    #endif
}

void SYN_ResetEvent(SYN_Event* event)
{
    #if MSR_WINDOWS
        ResetEvent(*(HANDLE*) event);
    #elif MSR_UNIX
        SYN_Internal_EventUnix* e = (SYN_Internal_EventUnix*) event;
        pthread_mutex_lock(&e->mutex);
        e->signaled = false;
        pthread_mutex_unlock(&e->mutex);
    #endif
}
