#pragma once
#include <__init.h>
#include "Collections.h"
#include "Stream.h"
#include "IO.h"

/**
 * Read or Write handle of a pipe.
 * This is used for inter-process communication (IPC).
 */
struct PipeHandle : public IStream
{
    #if MSR_WINDOWS
        using HandleType = HANDLE;
        static constexpr const HandleType k_InvalidHandle = INVALID_HANDLE_VALUE;
    #elif MSR_UNIX
        using HandleType = int;
        static constexpr const HandleType k_InvalidHandle = -1;
    #else
        #error "Unsupported platform"
    #endif

    HandleType handle;

    PipeHandle() = default;
    explicit PipeHandle(HandleType h) : handle(h) { }

    bool IsValid() const { return handle != k_InvalidHandle; }
    operator bool() const { return IsValid(); }

    /**
     * Creates a pipe and returns the read and write ends.
     * The read end is used for reading data from the pipe.
     * The write end is used for writing data to the pipe.
     */
    static bool Create(PipeHandle* outR, PipeHandle* outW);

    virtual int64_t GetSize() override;
    virtual int64_t Read(Slice<uint8_t> dst) override;
    virtual int64_t Write(const Slice<uint8_t> src) override;
    virtual void Close() override;
};

/**
 * A key-value pair representing an environment variable.
 * The `kvp` field contains the full "KEY=VALUE" string.
 * The 'key' field contains the key part.
 * The 'value' field contains the value part.
 */
struct EnvVarKVP
{
    String kvp;
    String key;
    String value;
};

struct Process
{
    /**
     * Exits the current process immediately with the specified exit code.
     */
    static void Exit(int32_t exitCode = 0);

    /**
     * Retrieves all environment variables as a slice of key-value pairs.
     * The returned slice is allocated using the provided allocator.
     * The individual strings within the key-value pairs are also allocated using the same allocator.
     * For the key-value pairs, the `kvp` field contains the full "KEY=VALUE" string,
     * while the `key` and `value` fields are just 'views' into that string.
     */
    static Slice<EnvVarKVP> GetEnvironmentVariables(Allocator allocator);

    #if MSR_WINDOWS
        using PIDType = DWORD;
        using ProcessHandleType = HANDLE;

        static constexpr const PIDType k_InvalidPID = (DWORD) -1;
        static constexpr const ProcessHandleType k_InvalidProcessHandle = INVALID_HANDLE_VALUE;
    #elif MSR_UNIX
        using PIDType = pid_t;
        using ProcessHandleType = pid_t; // on Unix, the PID itself is used as the handle

        static constexpr const PIDType k_InvalidPID = (pid_t) -1;
        static constexpr const ProcessHandleType k_InvalidProcessHandle = (pid_t) -1;
    #else
        #error "Unsupported platform"
    #endif

    PIDType pid;
    ProcessHandleType handle;

    Process() = default;
    Process(PIDType p, ProcessHandleType h) : pid(p), handle(h) { }

    bool IsValid() const { return pid != k_InvalidPID && handle != k_InvalidProcessHandle; }
    operator bool() const { return IsValid(); }

    /**
     * Starts a new process with the specified executable and arguments.
     * Optionally, environment variables, working directory, and pipes for
     * standard output and error can be provided.
     *
     * If not provided, environment variables and working directory are inherited
     * from the current process. If provided, they must be in a 'KEY=VALUE' format.
     *
     * The pipe handles provided must be read ends for stdout and stderr respectively.
     * If null, the respective output is discarded.
     */
    static Process Run(
        Slice<String> execAndArgs,
        Slice<String> environmentVariables = Slice<String>(),
        DirectoryPath workingDirectory = DirectoryPath(String()),
        PipeHandle* stdOutPipe = nullptr,
        PipeHandle* stdErrPipe = nullptr
    );
};
