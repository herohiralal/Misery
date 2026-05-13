#pragma once
#include <__init.h>
#include "Collections.h"
#include "Stream.h"

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
};
