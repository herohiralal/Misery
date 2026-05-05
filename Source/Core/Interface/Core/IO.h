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
};

struct FilePath
{
    String actual;

    explicit FilePath(String p) : actual(p) { }

    static FilePath Normalise(String path, Allocator allocator);
};
