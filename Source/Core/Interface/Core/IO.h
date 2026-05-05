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
};
