#include "CorePrivate.h"

#if MSR_WINDOWS || MSR_OSX || MSR_IOS
    #define THR_Internal_MaxNameLen 64
#elif MSR_LINUX || MSR_ANDROID
    #define THR_Internal_MaxNameLen 16
#else
    #error "Unimplemented."
#endif

#if MSR_WINDOWS
    static_assert( sizeof(THR_Handle) ==  sizeof(HANDLE), "HANDLE size mismatch");
    static_assert(alignof(THR_Handle) == alignof(HANDLE), "HANDLE alignment mismatch");
#elif MSR_UNIX
    static_assert( sizeof(THR_Handle) ==  sizeof(pthread_t), "pthread_t size mismatch");
    static_assert(alignof(THR_Handle) == alignof(pthread_t), "pthread_t alignment mismatch");
#endif

b8 THR_IsValid(THR_Handle handle)
{
    #if MSR_WINDOWS
        DWORD exitCode = 0;
        return (b8) GetExitCodeThread((HANDLE) handle.handle, &exitCode);
    #elif MSR_UNIX
        return (b8) (0 == pthread_kill((pthread_t) handle.handle, 0));
    #endif
}

THR_Handle THR_GetCurrent(void)
{
    #if MSR_WINDOWS
        return (THR_Handle) { .handle = (usize) GetCurrentThread() };
    #elif MSR_UNIX
        return (THR_Handle) { .handle = (usize) pthread_self() };
    #endif
}

THR_Id THR_GetId(THR_Handle handle)
{
    #if MSR_WINDOWS
        return (THR_Id) { .id = (usize) GetThreadId((HANDLE) handle.handle) };
    #elif MSR_UNIX
        return (THR_Id) { .id = (usize) pthread_self() };
    #endif
}

b8 THR_Eq(THR_Id a, THR_Id b)
{
    #if MSR_WINDOWS
        return a.id == b.id;
    #elif MSR_UNIX
        return pthread_equal((pthread_t) a.id, (pthread_t) b.id);
    #endif
}

utf8str THR_GetName(THR_Handle handle, MEM_Allocator allocator)
{
    #if MSR_WINDOWS
    {
        PWSTR threadName = nil;
        if (GetThreadDescription((HANDLE) handle.handle, &threadName) != S_OK) goto failedToFindName;
        if (!threadName) goto failedToFindName;

        i32 wideLen = 0;
        while (threadName[wideLen] != 0) { ++wideLen; }
        if (!wideLen) { LocalFree(threadName); goto failedToFindName; }

        char nameBuffer[THR_Internal_MaxNameLen] = {0};
        i32 len = WideCharToMultiByte(CP_UTF8, 0, threadName, wideLen, nameBuffer, THR_Internal_MaxNameLen - 1, nil, nil);
        LocalFree(threadName);

        if (len <= 0) goto failedToFindName;

        return STR_Clone(STR_AliasCStr(nameBuffer), allocator);
    }
    #elif MSR_OSX || MSR_IOS || (MSR_LINUX && defined(_GNU_SOURCE)) || (MSR_ANDROID && defined(_GNU_SOURCE))
    {
        char nameBuffer[THR_Internal_MaxNameLen] = {0};
        if (pthread_getname_np((pthread_t) handle.handle, nameBuffer, THR_Internal_MaxNameLen) != 0)
            goto failedToFindName;

        isize len = 0;
        while (len < THR_Internal_MaxNameLen && nameBuffer[len] != 0) { ++len; }
        if (!len) goto failedToFindName;

        return STR_Clone(STR_AliasCStr(nameBuffer), allocator);
    }
    #endif

    failedToFindName:
    {
        return FMT_APrintf(allocator, "Thread#%", FMT(handle.handle));
    }
}

void THR_SetName(THR_Handle handle, utf8str name)
{
    #if !MSR_APPLE
        if (name.count > THR_Internal_MaxNameLen - 1)
            name.count = THR_Internal_MaxNameLen - 1;

        #if MSR_WINDOWS
        {
            WCHAR nameBuffer[THR_Internal_MaxNameLen] = {0};
            MultiByteToWideChar(CP_UTF8, 0, (cstring) name.data, (i32) name.count, nameBuffer, THR_Internal_MaxNameLen);
            SetThreadDescription((HANDLE) handle.handle, nameBuffer);
        }
        #elif (MSR_LINUX && defined(_GNU_SOURCE)) || (MSR_ANDROID && defined(_GNU_SOURCE))
        {
            char nameBuffer[THR_Internal_MaxNameLen] = {0};
            MEM_Copy(nameBuffer, name.data, (usize) name.count);
            nameBuffer[name.count] = '\0';
            pthread_setname_np((pthread_t) handle.handle, nameBuffer);
        }
        #endif
    #else
        (void) handle;
        (void) name;
        return; // setting thread name on Apple platforms is not supported for non-current threads
    #endif
}

utf8str THR_GetCurrentName(MEM_Allocator allocator)
{
    return THR_GetName(THR_GetCurrent(), allocator);
}

void THR_SetCurrentName(utf8str name)
{
    #if MSR_APPLE
        if (name.count > THR_Internal_MaxNameLen - 1)
            name.count = THR_Internal_MaxNameLen - 1;

        char nameBuffer[THR_Internal_MaxNameLen] = {0};
        MEM_Copy(nameBuffer, name.data, (usize) name.count);
        nameBuffer[name.count] = '\0';
        pthread_setname_np(nameBuffer);
    #else
        THR_SetName(THR_GetCurrent(), name);
    #endif
}

typedef union alignas(64)
{
    struct
    {
        THR_Proc procedure;
        rawptr   data;
        utf8str  threadName;
    };

    u8 padding[64]; // ensure that the payload is at least 64 bytes
} THR_Internal_ProcPayload;

static void THR_Internal_ProcWrapper(THR_Internal_ProcPayload* payloadPtr)
{
    THR_Internal_ProcPayload payload = *payloadPtr;
    MEM_Delete(payloadPtr, MEM_main);

    THR_SetCurrentName(payload.threadName);
    MEM_Deallocate(MEM_main, payload.threadName.data);
    payload.procedure(payload.data);
}

#if MSR_WINDOWS
    static DWORD WINAPI THR_Internal_WinProcWrapper(LPVOID param)
    {
        THR_Internal_ProcWrapper((THR_Internal_ProcPayload*) param);
        return 0;
    }
#elif MSR_UNIX
    static void* THR_Internal_UnixProcWrapper(void* param)
    {
        THR_Internal_ProcWrapper((THR_Internal_ProcPayload*) param);
        return nil;
    }
#else
    #error "Unknown platform."
#endif

THR_Handle THR_Start(THR_Proc procedure, rawptr data, utf8str name)
{
    THR_Internal_ProcPayload* payloadPtr = MEM_New(THR_Internal_ProcPayload, MEM_main);
    MSR_ASSERT(payloadPtr != nil && "Failed to allocate memory for thread payload.");

    *payloadPtr = (THR_Internal_ProcPayload)
    {
        .procedure  = procedure,
        .data       = data,
        .threadName = STR_Clone(name, MEM_main),
    };

    b8 failed = false;
    THR_Handle handle = {0};

    #if MSR_WINDOWS
    {
        HANDLE threadHandle = CreateThread(
            nil, // default security attributes
            0, // use default stack size
            THR_Internal_WinProcWrapper,
            payloadPtr,
            0, // use default creation flags
            nil // returns the thread identifier
        );

        if (!threadHandle) { failed = true; }
        else               { handle.handle = (usize) threadHandle; }
    }
    #elif MSR_UNIX
    {
        pthread_t thread;
        if (pthread_create(&thread, nil, THR_Internal_UnixProcWrapper, payloadPtr) != 0)
        {
            failed = true;
        }
        else
        {
            handle.handle = (usize) thread;
        }
    }
    #else
        #error "Unknown platform."
    #endif

    if (failed)
    {
        MEM_Deallocate(MEM_main, payloadPtr->threadName.data);
        MEM_Delete(payloadPtr, MEM_main);
        MSR_ASSERT(false && "Failed to create thread.");
        return (THR_Handle) {0};
    }

    return handle;
}

void THR_Join(THR_Handle handle)
{
    #if MSR_WINDOWS
        WaitForSingleObject((HANDLE) handle.handle, INFINITE);
        CloseHandle((HANDLE) handle.handle);
    #elif MSR_UNIX
        pthread_join((pthread_t) handle.handle, nil);
    #else
        #error "Unknown platform."
    #endif
}

void THR_Sleep(u64 milliseconds)
{
    #if MSR_WINDOWS
        Sleep((DWORD) milliseconds);
    #elif MSR_UNIX
        struct timespec req, rem;
        req.tv_sec  = (time_t) (milliseconds / 1000);
        req.tv_nsec = (long)   ((milliseconds % 1000) * 1000000);
        while (nanosleep(&req, &rem) == -1 && errno == EINTR)
        {
            req = rem;
        }
    #else
        #error "Unknown platform."
    #endif
}

#undef THR_Internal_MaxNameLen
