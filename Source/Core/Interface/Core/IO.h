#pragma once
#include <__init.h>
#include "Collections.h"

struct FilePath;
struct DirectoryPath;

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
};
