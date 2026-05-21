#pragma once
#include <__init.h>
#include "Collections.h"
#include "Stream.h"

struct FilePath;
struct DirectoryPath;
struct FileStream;

struct DirectoryPath
{
    String actual;

    explicit DirectoryPath(String p) : actual(p) { }

    static DirectoryPath Normalise(String path, Allocator allocator);

    DirectoryPath GetParentDirectory() const;

    using VisitorDelegate = bool (*)(String path, bool isDirectory, void* userData, bool* exploreCurrentDirectory);
    void IterateDirectory(VisitorDelegate visitor, void* userData = nullptr, bool recursive = false) const;

    DirectoryPath GetSubdirectory(String dirName, Allocator allocator) const;
    FilePath GetFile(String fileNameWithExtension, Allocator allocator) const;

    bool Exists() const;
    bool Ensure() const;

    bool Delete() const;
};

struct FilePath
{
    String actual;

    explicit FilePath(String p) : actual(p) { }

    static FilePath Normalise(String path, Allocator allocator);

    DirectoryPath GetParentDirectory() const;
    String FileNameWithExtension() const;
    String FileNameWithoutExtension() const;
    String Extension() const;

    bool Exists() const;

    // nanoseconds since unix epoch
    int64_t LastUpdated() const;

    bool Delete() const;

    FileStream OpenRead(bool allowWrite = false) const;
    FileStream OpenWrite(bool append = false, bool allowRead = false) const;
    Slice<uint8_t> ReadAll(Allocator allocator) const;
    void WriteAll(Slice<uint8_t> data, bool append = false) const;
};

struct FileStream : public IStream
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

    FileStream() = default;
    explicit FileStream(HandleType h) : handle(h) { }

    bool IsValid() const { return handle != k_InvalidHandle; }
    operator bool() const { return IsValid(); }

    virtual int64_t GetSize() override;
    virtual int64_t GetCurrentPosition() override;
    virtual bool Seek(int64_t position, bool relative = false) override;
    virtual int64_t Read(Slice<uint8_t> dst) override;
    virtual int64_t Write(const Slice<uint8_t> src) override;
    virtual bool Truncate() override;
    virtual bool Truncate(int64_t newSize) override;
    virtual bool Flush() override;
    virtual void Close() override;
};
