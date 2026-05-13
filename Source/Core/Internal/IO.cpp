#include <Core/Defer.h>
#include <Core/IO.h>
#include <Core/Allocators/Arena.h>
#include "IO.h"

namespace Misery::IO::Internal
{
#if MSR_WINDOWS

    static int32_t GetVolumeLengthFromPath(String path)
    {
        #define IS_SLASH(c) ((c) == '/' || (c) == '\\')

        if (path.Length() < 2) return 0; // no volume

        if (path[1] == ':' && ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')))
        {
            return 2; // volume is present
        }

        if (path.Length() >= 5 && IS_SLASH(path[0]) && IS_SLASH(path[1]) && !IS_SLASH(path[2]) && (path[2] != '.'))
        {
            for (int32_t i = 3; i < path.Length(); i++)
            {
                if (IS_SLASH(path[i]))
                {
                    i += 1;
                    if (i < path.Length() && !IS_SLASH(path[i]))
                    {
                        if (path[i] == '.')
                        {
                            break;
                        }
                    }

                    for (; i < path.Length(); i++)
                    {
                        if (IS_SLASH(path[i]))
                        {
                            break;
                        }
                    }

                    return i;
                }
            }
        }

        #undef IS_SLASH

        return 0;
    }

    struct LazyPathBuffer
    {
        String originalStr;
        List<uint8_t> writeBuffer;
        size_t writeIdx;
        String volAndPath;
        size_t volumeLength;

        LazyPathBuffer(String inOriginal, String inVolAndPath, size_t inVolumeLength, Allocator allocator)
            : originalStr(inOriginal),
              volAndPath(inVolAndPath),
              volumeLength(inVolumeLength),
              writeBuffer(allocator.MakeList<uint8_t>()),
              writeIdx(0)
        {
        }

        uint8_t GetByte(size_t idx)
        {
            if (writeBuffer)
            {
                return writeBuffer[idx];
            }

            return originalStr[idx];
        }

        bool Append(uint8_t c)
        {
            if (!writeBuffer)
            {
                if (writeIdx < originalStr.Length() && originalStr[writeIdx] == c)
                {
                    writeIdx += 1;
                    return true; // no need to append, just increment the index
                }

                writeBuffer.Reserve(writeIdx + 1, SRC_LOC());
                if (!writeBuffer) { return false; } // failed to reserve memory for the buffer
                memcpy(writeBuffer.Data(), originalStr.Data(), writeIdx);
            }

            writeBuffer.Add(c, SRC_LOC());
            writeIdx++;
            return true;
        }

        String ToString()
        {
            if (!writeBuffer)
            {
                String toClone = String(originalStr.Data(), volumeLength + writeIdx);
                return writeBuffer.GetAllocator().CloneString(toClone, SRC_LOC());
            }

            String x = String(volAndPath.Data(), volumeLength);
            String y = String(writeBuffer.Data(), writeIdx);
            String z = writeBuffer.GetAllocator().MakeString(x.Length() + y.Length(), SRC_LOC());
            if (z.Data())
            {
                memcpy(z.Data(),              x.Data(), x.Length());
                memcpy(z.Data() + x.Length(), y.Data(), y.Length());
            }

            return z;
        }

        ~LazyPathBuffer()
        {
            writeBuffer.Free(SRC_LOC());
        }
    };

#endif

    static String NormalisePath(String path, bool isDir, Allocator allocator)
    {
        CString str = alloc_temp.MakeCString(path, SRC_LOC());

        #if MSR_WINDOWS
        {
            int32_t n = GetFullPathNameA(str.Data(), 0, nullptr, nullptr);
            if (n <= 0) { return String(); }
            CString tempFullPath = alloc_temp.MakeCString(n, SRC_LOC());
            n = GetFullPathNameA(str.Data(), n, tempFullPath.Data(), nullptr);

            #define IS_SEPARATOR(c) ((c) == '/' || (c) == '\\')

            path = String(tempFullPath);
            String originalPath = path;
            int32_t volumeLength = GetVolumeLengthFromPath(path);
            path = String(path.Data() + volumeLength, path.Length() - volumeLength);

            if (!path.Length())
            {
                // path is just a volume, needs a trailing slash and then return
                String resultPath = allocator.MakeString(volumeLength + 1, SRC_LOC());
                memcpy(resultPath.Data(), originalPath.Data(), volumeLength);
                resultPath[volumeLength] = '/'; // add trailing slash
                return resultPath;
            }

            bool isRooted = IS_SEPARATOR(path[0]);
            n = (int32_t) path.Length();
            auto outputBuffer = LazyPathBuffer(path, originalPath, volumeLength, allocator);

            int32_t r = 0, dotDot = 0;
            if (isRooted)
            {
                if (!outputBuffer.Append('/')) { return String(); }
                r = 1;
                dotDot = 1;
            }

            while (r < n)
            {
                if (IS_SEPARATOR(path[r]))
                {
                    r++; // skip the separator
                }
                else if (path[r] == '.' && (((r + 1) == n) || IS_SEPARATOR(path[r + 1])))
                {
                    r++; // skip the "."
                }
                else if (path[r] == '.' && path[r + 1] == '.' && (((r + 2) == n) || IS_SEPARATOR(path[r + 2])))
                {
                    r += 2; // skip the ".."

                    if (outputBuffer.writeIdx > dotDot)
                    {
                        outputBuffer.writeIdx--;
                        while (outputBuffer.writeIdx > dotDot)
                        {
                            uint8_t b = outputBuffer.GetByte(outputBuffer.writeIdx);
                            if (IS_SEPARATOR(b)) { break; }
                            outputBuffer.writeIdx--; // go back until we find a separator
                        }
                    }
                    else if (!isRooted)
                    {
                        if (outputBuffer.writeIdx > 0)
                        {
                            if (!outputBuffer.Append('/')) { return String(); }
                        }

                        if (!outputBuffer.Append('.') || !outputBuffer.Append('.')) { return String(); }
                        dotDot = outputBuffer.writeIdx;
                    }
                }
                else
                {
                    if (isRooted && outputBuffer.writeIdx != 1 || !isRooted && outputBuffer.writeIdx != 0)
                    {
                        if (!outputBuffer.Append('/')) { return String(); }
                    }

                    for (; r < n && !IS_SEPARATOR(path[r]); ++r)
                    {
                        if (!outputBuffer.Append(path[r])) { return String(); }
                    }
                }
            }

            if (!outputBuffer.writeIdx)
            {
                if (!outputBuffer.Append('.')) { return String(); }
            }

            if (isDir)
            {
                uint8_t lastChar = outputBuffer.writeIdx > 0 ? outputBuffer.GetByte(outputBuffer.writeIdx - 1) : 0;
                if (!IS_SEPARATOR(lastChar))
                {
                    if (!outputBuffer.Append('/')) { return String(); }
                }
            }

            String output = outputBuffer.ToString();
            for (int32_t i = 0; i < output.Length(); i++)
            {
                if (output[i] == '\\') { output[i] = '/'; } // normalise path separators
            }

            return output;

            #undef IS_SEPARATOR
        }
        #elif MSR_UNIX
        {
            CString pathPtr = realpath(str, nullptr);
            DEFER { free(pathPtr); };

            if (pathPtr)
            {
                String tempAlias = String(pathPtr);
                size_t tgtLen = tempAlias.Length() + (isDir ? 1 : 0);
                String output = allocator.MakeString(tgtLen, SRC_LOC());

                if (output.Data())
                {
                    memcpy(output.Data(), tempAlias.Data(), tempAlias.Length());
                    if (isDir) { output[tempAlias.Length()] = '/'; }
                    return String(output.Data(), tgtLen);
                }
            }
        }
        #endif

        return String();
    }

    struct DeleteAllContentsPayload
    {
        Allocator allocator;
        bool failedAtSomething;
    };

    static bool DeleteAllContents(String path, bool isDirectory, void* userData, bool* exploreCurrentDirectory)
    {
        DeleteAllContentsPayload* payload = (DeleteAllContentsPayload*) userData;

        if (isDirectory)
        {
            /**
             * The reason for doing things weirdly like this is because of how the directory iterator function works
             * it performs a top-down approach where the callback is given for the directory before its contents.
             * But we cannot delete the directory before its contents. So we kind of hack it by recursing the directory
             * iterator function ourselves.
             */
            *exploreCurrentDirectory = false;

            DirectoryPath(path).IterateDirectory(DeleteAllContents, userData, false);
            CString path2 = payload->allocator.MakeCString(path, SRC_LOC());
            #if MSR_WINDOWS
                payload->failedAtSomething = (RemoveDirectoryA(path2) == 0) || payload->failedAtSomething;
            #elif MSR_UNIX
                payload->failedAtSomething = (rmdir(path2)            != 0) || payload->failedAtSomething;
            #endif
        }
        else
        {
            FilePath(path).Delete();
        }

        return true; // continue iterating
    }
}

DirectoryPath DirectoryPath::Normalise(String path, Allocator allocator)
{
    return DirectoryPath(Misery::IO::Internal::NormalisePath(path ? path : String("."), true, allocator));
}

DirectoryPath DirectoryPath::GetParentDirectory() const
{
    String s = actual;
    if (s.Length() && s[s.Length() - 1] == '/')
         s = actual.SubString(0, actual.Length() - 1); // skip trailing slash

    int32_t lastSlashIdx = s.LastIndexOf("/");
    if (lastSlashIdx == -1)
        return DirectoryPath(String("/"));

    String parentPath = actual.SubString(0, lastSlashIdx + 1); // include the slash
    return DirectoryPath(parentPath);
}

void DirectoryPath::IterateDirectory(VisitorDelegate visitor, void* userData, bool recursive) const
{
    Slice<char> tempBuffer = alloc_temp.MakeSlice<char>(actual.Length() + 3, SRC_LOC()); // +3 for potential wildcard and null terminator
    memcpy(tempBuffer.Data(), actual.Data(), actual.Length());

    #if MSR_WINDOWS
    {
        uint32_t iterator = (uint32_t) actual.Length() - 1;
        if (tempBuffer[iterator] == '/' || tempBuffer[iterator] == '\\') // has trailing slash
        {
            iterator = (uint32_t) actual.Length();
        }
        else
        {
            tempBuffer[actual.Length()] = '/'; // add slash at the end
            iterator = (uint32_t) actual.Length() + 1;
        }

        tempBuffer[iterator    ] = '*';  // add wildcard for file matching
        tempBuffer[iterator + 1] = '\0'; // null-terminate
        tempBuffer = tempBuffer.SubSlice(0, iterator + 1);
    }
    #elif MSR_UNIX
    {
        tempBuffer[actual.Length()] = '\0'; // null-terminate
        tempBuffer = tempBuffer.SubSlice(0, actual.Length());
    }
    #endif

    // 8kb scratch buffer per invocation
    ArenaAllocator tempAllocatorImpl = ArenaAllocator(8 * 1024, alloc_main);
    DEFER { tempAllocatorImpl.Destroy(); };

    Allocator tempAllocator = &tempAllocatorImpl;

    #if MSR_WINDOWS

        WIN32_FIND_DATAA findData;
        HANDLE findHandle = FindFirstFileA(tempBuffer.Data(), &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                CString nextFileName = (const char*) findData.cFileName;

    #elif MSR_UNIX

        DIR* dir = opendir(tempBuffer.Data());
        if (dir != NULL)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL)
            {
                CString nextFileName = (const char*) entry->d_name;

    #endif

                DEFER { tempAllocator.DeallocateAll(SRC_LOC()); }; // free any temp allocs

                int32_t fileNameLen = nextFileName.Length();
                if (fileNameLen == 0                                                    ) { continue; } // skip empty names
                if (fileNameLen == 1 && nextFileName[0] == '.'                          ) { continue; } // skip current directory
                if (fileNameLen == 2 && nextFileName[0] == '.' && nextFileName[1] == '.') { continue; } // skip parent directory

                String foundPath = tempAllocator.MakeString(actual.Length() + fileNameLen + 3, SRC_LOC()); // +3 for potential slash, null terminator, and just in case
                memcpy(foundPath.Data(), actual.Data(), actual.Length());
                uint32_t iterator = (uint32_t) actual.Length() - 1;
                if (foundPath[iterator] == '/' || foundPath[iterator] == '\\')
                {
                    iterator = (uint32_t) actual.Length();
                }
                else
                {
                    foundPath[actual.Length()] = '/'; // add slash at the end
                    iterator = (uint32_t) actual.Length() + 1;
                }

                memcpy(foundPath.Data() + iterator, nextFileName.Data(), fileNameLen);
                iterator += (uint32_t) fileNameLen;
                foundPath[iterator] = '\0'; // null-terminate the string, just in case

                foundPath = String(foundPath.Data(), iterator); // update count

                #if MSR_WINDOWS
                {
                    for (uint32_t i = 0; i < foundPath.Length(); i++)
                    {
                        if (foundPath[i] == '\\') { foundPath[i] = '/'; } // normalise path separators
                    }
                }
                #endif

                bool isDirectory = false;

                #if MSR_WINDOWS
                    isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                #elif MSR_UNIX
                    struct stat statBuf;
                    if (stat((const char*) foundPath.Data(), &statBuf) == 0)
                    {
                        isDirectory = S_ISDIR(statBuf.st_mode);
                    }
                #endif

                if (isDirectory)
                {
                    foundPath[iterator] = '/'; // ensure directory paths end with a slash
                    foundPath[iterator + 1] = '\0'; // null-terminate the string

                    foundPath = String(foundPath.Data(), iterator + 1); // update count
                    iterator++;
                }

                String foundPath2 = foundPath;

                bool exploreCurrentDirectory = recursive;
                bool iterateFurther = visitor(foundPath2, isDirectory, userData, &exploreCurrentDirectory);

                // handle recursion
                if (iterateFurther && isDirectory && exploreCurrentDirectory)
                {
                    DirectoryPath(foundPath2).IterateDirectory(visitor, userData, recursive);
                }

                if (!iterateFurther) break; // stop iteration if the visitor function returns false

    #if MSR_UNIX

            }

            closedir(dir);
        }

    #elif MSR_WINDOWS

            } while (FindNextFileA(findHandle, &findData));

            FindClose(findHandle);
        }

    #endif
}

DirectoryPath DirectoryPath::GetSubdirectory(String dirName, Allocator allocator) const
{
    String combinedPath = allocator.MakeString(actual.Length() + dirName.Length() + 1, SRC_LOC()); // +1 for trailing slash
    if (!combinedPath.Data()) { return DirectoryPath(String()); } // failed to allocate memory for the combined path

    memcpy(combinedPath.Data(), actual.Data(), actual.Length());
    memcpy(combinedPath.Data() + actual.Length(), dirName.Data(), dirName.Length());
    combinedPath[actual.Length() + dirName.Length()] = '/'; // add trailing slash

    return DirectoryPath(combinedPath);
}

FilePath DirectoryPath::GetFile(String fileNameWithExtension, Allocator allocator) const
{
    String combinedPath = allocator.MakeString(actual.Length() + fileNameWithExtension.Length(), SRC_LOC());
    if (!combinedPath.Data()) { return FilePath(String()); } // failed to allocate memory for the combined path

    memcpy(combinedPath.Data(), actual.Data(), actual.Length());
    memcpy(combinedPath.Data() + actual.Length(), fileNameWithExtension.Data(), fileNameWithExtension.Length());

    return FilePath(combinedPath);
}

bool DirectoryPath::Exists() const
{
    CString str = alloc_temp.MakeCString(actual, SRC_LOC());

    #if MSR_WINDOWS
    {
        DWORD fileAttrs = GetFileAttributesA(str);
        if (fileAttrs == INVALID_FILE_ATTRIBUTES) { return false; }
        return (fileAttrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    #elif MSR_UNIX
    {
        struct stat statBuf;
        if (stat(str, &statBuf) != 0) { return false; }
        return S_ISDIR(statBuf.st_mode);
    }
    #endif
}

bool DirectoryPath::Ensure() const
{
    if (Exists())
        return true;

    CString alt = alloc_temp.MakeCString(actual, SRC_LOC());

    bool success = true;
    for (int32_t i = 1; success && i < (int32_t) actual.Length(); i++)
    {
        if (alt[i] == '/')
        {
            alt[i] = '\0';
            #if MSR_WINDOWS
                success = CreateDirectoryA(alt, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
            #elif MSR_UNIX
                success = mkdir(alt, 0755) == 0 || errno == EEXIST;
            #endif
            alt[i] = '/';
        }
    }

    return success;
}

bool DirectoryPath::Delete() const
{
    bool throwaway = false;

    ArenaAllocator tempAllocatorImpl = ArenaAllocator(8 * 1024, alloc_main);
    DEFER { tempAllocatorImpl.Destroy(); };

    Misery::IO::Internal::DeleteAllContentsPayload payload =
    {
        .allocator = &tempAllocatorImpl,
        .failedAtSomething = false,
    };

    Misery::IO::Internal::DeleteAllContents(actual, true, &payload, &throwaway);
    return !payload.failedAtSomething;
}

FilePath FilePath::Normalise(String path, Allocator allocator)
{
    return FilePath(Misery::IO::Internal::NormalisePath(path ? path : String("unknown.file"), false, allocator));
}

DirectoryPath FilePath::GetParentDirectory() const
{
    int32_t lastSlashIdx = actual.LastIndexOf("/");
    if (lastSlashIdx == -1)
        return DirectoryPath(String("/"));

    String parentPath = actual.SubString(0, lastSlashIdx + 1); // include the slash
    return DirectoryPath(parentPath);
}

String FilePath::FileNameWithExtension() const
{
    int32_t lastSlashIdx = actual.LastIndexOf("/");
    if (lastSlashIdx == -1)
        return actual;

    return actual.SubString(lastSlashIdx + 1, actual.Length() - lastSlashIdx - 1); // skip the slash
}

String FilePath::FileNameWithoutExtension() const
{
    String fileNameWithExt = FileNameWithExtension();
    int32_t lastDotIdx = fileNameWithExt.LastIndexOf(".");
    if (lastDotIdx == -1)
        return fileNameWithExt;

    return fileNameWithExt.SubString(0, lastDotIdx); // skip the dot and extension
}

String FilePath::Extension() const
{
    int32_t lastDotIdx = actual.LastIndexOf(".");
    if (lastDotIdx == -1)
        return String();

    return actual.SubString(lastDotIdx + 1, actual.Length() - lastDotIdx - 1); // skip the dot
}

bool FilePath::Exists() const
{
    CString str = alloc_temp.MakeCString(actual, SRC_LOC());

    #if MSR_WINDOWS
    {
        DWORD fileAttrs = GetFileAttributesA(str);
        if (fileAttrs == INVALID_FILE_ATTRIBUTES) { return false; }
        return (fileAttrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }
    #elif MSR_UNIX
    {
        struct stat statBuf;
        if (stat(str, &statBuf) != 0) { return false; }
        return S_ISREG(statBuf.st_mode);
    }
    #endif
}

int64_t FilePath::LastUpdated() const
{
    CString str = alloc_temp.MakeCString(actual, SRC_LOC());

    #if MSR_WINDOWS
    {
        WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
        if (!GetFileAttributesExA(str, GetFileExInfoStandard, &fileAttrData)) { return -1; }

        // convert FILETIME to nanoseconds since unix epoch
        uint64_t fileTime = ((uint64_t) fileAttrData.ftLastWriteTime.dwHighDateTime << 32) | fileAttrData.ftLastWriteTime.dwLowDateTime;
        return (int64_t) (fileTime * 100); // FILETIME is in 100-nanosecond intervals
    }
    #elif MSR_UNIX
    {
        struct stat statBuf;
        if (stat(str, &statBuf) != 0) { return -1; }

        // convert seconds to nanoseconds and add the nanosecond part
        return (int64_t) statBuf.st_mtime * 1000000000 + statBuf.st_mtim.tv_nsec;
    }
    #endif

    return -1;
}

bool FilePath::Delete() const
{
    CString str = alloc_temp.MakeCString(actual, SRC_LOC());

    #if MSR_WINDOWS
        return DeleteFileA(str) != 0;
    #elif MSR_UNIX
        return unlink(str) == 0;
    #endif
}

FileStream FilePath::OpenRead(bool allowWrite) const
{
    return FileStream::OpenRead(*this, allowWrite);
}

FileStream FilePath::OpenWrite(bool append, bool allowRead) const
{
    return FileStream::OpenWrite(*this, append, allowRead);
}

Slice<uint8_t> FilePath::ReadAll(Allocator allocator) const
{
    FileStream fs = OpenRead();
    if (!fs) { return Slice<uint8_t>(); }
    DEFER { fs.Close(); };

    Stream s = &fs;
    return s.ReadAll(allocator);
}

void FilePath::WriteAll(Slice<uint8_t> data, bool append) const
{
    FileStream fs = OpenWrite(append);
    if (!fs) { return; }
    DEFER { fs.Close(); };

    Stream s = &fs;
    s.Write(data);
    s.Truncate();
}

FileStream FileStream::OpenRead(const FilePath& path, bool allowWrite)
{
    if (!path.actual) { return FileStream(InvalidHandle); }
    CString str = alloc_temp.MakeCString(path.actual, SRC_LOC());

    #if MSR_WINDOWS
    {
        HANDLE h = CreateFileA(str,
                        GENERIC_READ | (allowWrite ? GENERIC_WRITE : 0),
                        FILE_SHARE_READ | (allowWrite ? FILE_SHARE_WRITE : 0),
                        nullptr,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr);

        return FileStream(h);
    }
    #elif MSR_UNIX
    {
        int fd = open(str, allowWrite ? O_RDWR : O_RDONLY);
        return FileStream(fd);
    }
    #else
        #error "Unsupported platform"
    #endif
}

FileStream FileStream::OpenWrite(const FilePath& path, bool append, bool allowRead)
{
    if (!path.actual) { return FileStream(InvalidHandle); }
    CString str = alloc_temp.MakeCString(path.actual, SRC_LOC());

    #if MSR_WINDOWS
    {
        HANDLE h = CreateFileA(str,
                        GENERIC_WRITE | (allowRead ? GENERIC_READ : 0),
                        FILE_SHARE_WRITE | (allowRead ? FILE_SHARE_READ : 0),
                        nullptr,
                        append ? OPEN_ALWAYS : CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr);

        if (INVALID_HANDLE_VALUE != h && append)
        {
            SetFilePointer(h, 0, nullptr, FILE_END);
        }

        return FileStream(h);
    }
    #elif MSR_UNIX
    {
        int flags = allowRead ? O_RDWR : O_WRONLY;
        flags |= O_CREAT;
        if (append) { flags |= O_APPEND; }
        else        { flags |= O_TRUNC;  }

        int fd = open(str, flags, 0666);
        return FileStream(fd);
    }
    #else
        #error "Unsupported platform"
    #endif
}

int64_t FileStream::GetSize()
{
    if (!IsValid()) { return -1; }

    #if MSR_WINDOWS
    {
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(handle, &fileSize)) { return -1; }
        return fileSize.QuadPart;
    }
    #elif MSR_UNIX
    {
        struct stat st;
        if (fstat(handle, &st) != 0) { return -1; }
        return st.st_size;
    }
    #else
        #error "Unsupported platform"
    #endif
}

int64_t FileStream::GetCurrentPosition()
{
    if (!IsValid()) { return -1; }

    #if MSR_WINDOWS
    {
        LARGE_INTEGER zero = { };
        LARGE_INTEGER out;
        if (!SetFilePointerEx(handle, zero, &out, FILE_CURRENT)) { return -1; }
        return out.QuadPart;
    }
    #elif MSR_UNIX
    {
        off_t off = lseek(handle, 0, SEEK_CUR);
        if (off < 0) { return -1; }
        return (int64_t) off;
    }
    #else
        #error "Unsupported platform"
    #endif
}

bool FileStream::Seek(int64_t position, bool relative)
{
    if (!IsValid()) { return false; }

    #if MSR_WINDOWS
    {
        LARGE_INTEGER li = { };
        li.QuadPart = position;
        if (!SetFilePointerEx(handle, li, nullptr, relative ? FILE_CURRENT : FILE_BEGIN)) { return false; }
        return true;
    }
    #elif MSR_UNIX
    {
        return lseek(handle, position, relative ? SEEK_CUR : SEEK_SET) != -1;
    }
    #else
        #error "Unsupported platform"
    #endif
}

int64_t FileStream::Read(Slice<uint8_t> dst)
{
    if (!IsValid()) { return 0; }
    if (!dst) { return 0; }

    #if MSR_WINDOWS
    {
        DWORD bytesRead;
        if (!ReadFile(handle, dst.Data(), (DWORD) dst.Count(), &bytesRead, nullptr)) { return 0; }
        return (int64_t) bytesRead;
    }
    #elif MSR_UNIX
    {
        ssize_t result = read(handle, dst.Data(), dst.Count());
        if (result < 0) { return 0; }
        return (int64_t) result;
    }
    #else
        #error "Unsupported platform"
    #endif

    return 0;
}

int64_t FileStream::Write(const Slice<uint8_t> src)
{
    if (!IsValid()) { return 0; }

    #if MSR_WINDOWS
    {
        DWORD bytesWritten;
        if (!WriteFile(handle, src.Data(), (DWORD) src.Count(), &bytesWritten, nullptr)) { return 0; }
        return (int64_t) bytesWritten;
    }
    #elif MSR_UNIX
    {
        ssize_t result = write(handle, src.Data(), src.Count());
        if (result < 0) { return 0; }
        return (int64_t) result;
    }
    #else
        #error "Unsupported platform"
    #endif

    return 0;
}

bool FileStream::Truncate()
{
    if (!IsValid()) { return false; }

    #if MSR_WINDOWS
    {
        LARGE_INTEGER zero = { };
        if (!SetFilePointerEx(handle, zero, nullptr, FILE_CURRENT)) { return false; }
        if (!SetEndOfFile(handle)) { return false; }
        return true;
    }
    #elif MSR_UNIX
    {
        off_t cur = lseek(handle, 0, SEEK_CUR);
        if (cur < 0) { return false; }
        return ftruncate(handle, cur) == 0;
    }
    #else
        #error "Unsupported platform"
    #endif
}

bool FileStream::Truncate(int64_t newSize)
{
    if (!IsValid()) { return false; }

    #if MSR_WINDOWS
    {
        LARGE_INTEGER li = { };
        li.QuadPart = newSize;
        if (!SetFilePointerEx(handle, li, nullptr, FILE_BEGIN)) { return false; }
        if (!SetEndOfFile(handle)) { return false; }
        return true;
    }
    #elif MSR_UNIX
    {
        return ftruncate(handle, newSize) == 0;
    }
    #else
        #error "Unsupported platform"
    #endif
}

bool FileStream::Flush()
{
    if (!IsValid()) { return false; }

    #if MSR_WINDOWS
    {
        return FlushFileBuffers(handle) != 0;
    }
    #elif MSR_UNIX
    {
        return fsync(handle) == 0;
    }
    #else
        #error "Unsupported platform"
    #endif
}

void FileStream::Close()
{
    if (!IsValid()) { return; }

    #if MSR_WINDOWS
    {
        CloseHandle(handle);
    }
    #elif MSR_UNIX
    {
        close(handle);
    }
    #else
        #error "Unsupported platform"
    #endif

    handle = InvalidHandle;
}
