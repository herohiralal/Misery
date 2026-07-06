#include "CorePrivate.h"

#if MSR_WINDOWS

static inline i32 PTH_Internal_GetVolumeLengthFromPath(utf8str path)
{
    #define IS_SLASH(c) ((c) == '/' || (c) == '\\')

    if (path.count < 2) return 0; // no volume

    if (path.data[1] == ':' && ((path.data[0] >= 'A' && path.data[0] <= 'Z') || (path.data[0] >= 'a' && path.data[0] <= 'z')))
    {
        return 2; // volume is present
    }

    if (path.count >= 5 && IS_SLASH(path.data[0]) && IS_SLASH(path.data[1]) && !IS_SLASH(path.data[2]) && (path.data[2] != '.'))
    {
        for (i32 i = 3; i < path.count; i++)
        {
            if (IS_SLASH(path.data[i]))
            {
                i += 1;
                if (i < path.count && !IS_SLASH(path.data[i]))
                {
                    if (path.data[i] == '.')
                    {
                        break;
                    }
                }

                for (; i < path.count; i++)
                {
                    if (IS_SLASH(path.data[i]))
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

typedef struct
{
    utf8str originalStr;
    List_(u8) writeBuffer;
    isize writeIdx;
    utf8str volAndPath;
    isize volumeLength;
} PTH_Internal_LazyPathBuffer;

static inline PTH_Internal_LazyPathBuffer PTH_Internal_NewLazyPathBuffer(utf8str inOriginal, utf8str inVolAndPath, isize inVolumeLength, MEM_Allocator allocator)
{
    return (PTH_Internal_LazyPathBuffer)
    {
        .originalStr = inOriginal,
        .volAndPath = inVolAndPath,
        .volumeLength = inVolumeLength,
        .writeBuffer = COL_NewList(u8, 0, allocator),
        .writeIdx = 0
    };
}

static inline u8 PTH_Internal_GetByteFromLazyPathBuffer(PTH_Internal_LazyPathBuffer* buffer, isize idx)
{
    if (buffer->writeBuffer.count && buffer->writeBuffer.data)
    {
        return buffer->writeBuffer.data[idx];
    }

    return buffer->originalStr.data[idx];
}

static inline b8 PTH_Internal_AppendToLazyPathBuffer(PTH_Internal_LazyPathBuffer* buffer, u8 c)
{
    if (!buffer->writeBuffer.count || !buffer->writeBuffer.data)
    {
        if (buffer->writeIdx < buffer->originalStr.count && buffer->originalStr.data[buffer->writeIdx] == c)
        {
            buffer->writeIdx += 1;
            return true; // no need to append, just increment the index
        }

        COL_ResizeList(&buffer->writeBuffer, buffer->writeIdx + 1);
        if (!buffer->writeBuffer.capacity) { return false; } // failed to reserve memory for the buffer

        utf8str toAppend = STR_SubString(buffer->originalStr, 0, buffer->writeIdx);
        COL_AppendAllToList(&(buffer->writeBuffer), toAppend);
    }

    COL_AppendToList(&(buffer->writeBuffer), c);
    buffer->writeIdx++;
    return true;
}

static inline utf8str PTH_Internal_StringFromLazyPathBuffer(PTH_Internal_LazyPathBuffer* buffer)
{
    if (!buffer->writeBuffer.count || !buffer->writeBuffer.data)
    {
        utf8str toClone = {.data = buffer->volAndPath.data, .count = buffer->volumeLength + buffer->writeIdx};
        return STR_Clone(toClone, buffer->writeBuffer.allocator);
    }

    utf8str x = {.data = buffer->volAndPath.data, .count = buffer->volumeLength };
    utf8str y = {.data = buffer->writeBuffer.data, .count = buffer->writeIdx };
    utf8str z = COL_NewSlice(u8, x.count + y.count, true, buffer->writeBuffer.allocator);
    if (z.data)
    {
        MEM_Copy(z.data,           x.data, (usize) x.count);
        MEM_Copy(z.data + x.count, y.data, (usize) y.count);
    }

    return z;
}

static inline void PTH_Internal_DeleteLazyPathBuffer(PTH_Internal_LazyPathBuffer* buffer)
{
    COL_DeleteList(&buffer->writeBuffer);
    *buffer = (PTH_Internal_LazyPathBuffer) {0};
}

#endif

static utf8str NormalisePath(utf8str path, b8 isDir, MEM_Allocator allocator)
{
    cstring str = STR_CloneToCStr(path, MEM_temp);

    #if MSR_WINDOWS
    {
        utf8str output = {0};

        i32 n = GetFullPathNameA(str, 0, nil, nil);
        if (n <= 0) { return (utf8str) {0}; }

        cstring tempFullPath = COL_NewSlice(char, n, true, MEM_temp).data;
        n = GetFullPathNameA(str, n, (PSTR) tempFullPath, nil);

        #define IS_SEPARATOR(c) ((c) == '/' || (c) == '\\')

        path = STR_AliasCStr(tempFullPath);
        for (i32 i = 0; i < path.count; i++) if (path.data[i] == '\\') path.data[i] = '/';
        utf8str originalPath = path;
        i32 volumeLength = PTH_Internal_GetVolumeLengthFromPath(path);
        path = (utf8str) {.data = path.data + volumeLength, .count = path.count - volumeLength};

        if (!path.count)
        {
            // path is just a volume, needs a trailing slash and then return
            utf8str resultPath = COL_NewSlice(u8, volumeLength + 1, true, allocator);
            MEM_Copy(resultPath.data, originalPath.data, (usize) volumeLength);
            resultPath.data[volumeLength] = '/'; // add trailing slash
            return resultPath;
        }

        b8 isRooted = IS_SEPARATOR(path.data[0]);
        n = (i32) path.count;
        PTH_Internal_LazyPathBuffer outputBuffer = PTH_Internal_NewLazyPathBuffer(path, originalPath, volumeLength, allocator);
        b8 fail = false;

        isize r = 0, dotDot = 0;
        if (isRooted)
        {
            if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '/'))
            {
                fail = true;
                goto wrapup;
            }

            r = 1;
            dotDot = 1;
        }

        while (r < n)
        {
            if (IS_SEPARATOR(path.data[r]))
            {
                r++; // skip the separator
            }
            else if (path.data[r] == '.' && (((r + 1) == n) || IS_SEPARATOR(path.data[r + 1])))
            {
                r++; // skip the "."
            }
            else if (path.data[r] == '.' && path.data[r + 1] == '.' && (((r + 2) == n) || IS_SEPARATOR(path.data[r + 2])))
            {
                r += 2; // skip the ".."

                if (outputBuffer.writeIdx > dotDot)
                {
                    outputBuffer.writeIdx--;
                    while (outputBuffer.writeIdx > dotDot)
                    {
                        uint8_t b = PTH_Internal_GetByteFromLazyPathBuffer(&outputBuffer, outputBuffer.writeIdx);
                        if (IS_SEPARATOR(b)) { break; }
                        outputBuffer.writeIdx--; // go back until we find a separator
                    }
                }
                else if (!isRooted)
                {
                    if (outputBuffer.writeIdx > 0)
                    {
                        if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '/'))
                        {
                            fail = true;
                            goto wrapup;
                        }
                    }

                    if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '.') ||
                        !PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '.'))
                    {
                        fail = true;
                        goto wrapup;
                    }
                    dotDot = outputBuffer.writeIdx;
                }
            }
            else
            {
                if (isRooted && outputBuffer.writeIdx != 1 || !isRooted && outputBuffer.writeIdx != 0)
                {
                    if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '/'))
                    {
                        fail = true;
                        goto wrapup;
                    }
                }

                for (; r < n && !IS_SEPARATOR(path.data[r]); ++r)
                {
                    if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, path.data[r]))
                    {
                        fail = true;
                        goto wrapup;
                    }
                }
            }
        }

        if (!outputBuffer.writeIdx)
        {
            if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '.'))
            {
                fail = true;
                goto wrapup;
            }
        }

        if (isDir)
        {
            uint8_t lastChar = outputBuffer.writeIdx > 0 ? PTH_Internal_GetByteFromLazyPathBuffer(&outputBuffer, outputBuffer.writeIdx - 1) : 0;
            if (!IS_SEPARATOR(lastChar))
            {
                if (!PTH_Internal_AppendToLazyPathBuffer(&outputBuffer, '/'))
                {
                    fail = true;
                    goto wrapup;
                }
            }
        }

        output = PTH_Internal_StringFromLazyPathBuffer(&outputBuffer);
        for (i32 i = 0; i < output.count; i++)
        {
            if (output.data[i] == '\\') { output.data[i] = '/'; } // normalise path separators
        }

        wrapup:
        PTH_Internal_DeleteLazyPathBuffer(&outputBuffer);
        if (fail) return (utf8str) {0};
        return output;

        #undef IS_SEPARATOR
    }
    #elif MSR_UNIX
    {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) { return (utf8str) {0}; }

        char combined[PATH_MAX];

        int length = 0;
        cstring trailingSlash = isDir ? "/" : "";
        if (path.count && path.data[0] == '/')
            length = snprintf(combined, sizeof(combined), "%s%s", str, trailingSlash);
        else
            length = snprintf(combined, sizeof(combined), "%s/%s%s", cwd, str, trailingSlash);

        if (length < 0 || length >= (int) sizeof(combined)) { return (utf8str) {0}; }

        char* out = COL_NewSlice(char, length + 1, true, MEM_temp).data;
        if (!out) { return (utf8str) {0}; }

        strcpy(out, combined);

        for (char* p = out; *p; p++)
            if (*p == '\\') { *p = '/'; } // normalise path separators

        // remove duplicate-slashes in-place
        char* src = out;
        char* dst = out;
        b8 lastWasSlash = false;

        #define IS_SEPARATOR(c) ((c) == '/')
        while (*src)
        {
            if (IS_SEPARATOR(*src))
            {
                if (!lastWasSlash)
                {
                    *dst++ = '/';
                    lastWasSlash = true;
                }
            }
            else
            {
                *dst++ = *src;
                lastWasSlash = false;
            }
            src++;
        }
        #undef IS_SEPARATOR

        *dst = '\0'; // null-terminate the string, just in case

        return STR_AliasCStr(out);
    }
    #endif
}

FIL_Path FIL_Normalise(utf8str path, MEM_Allocator allocator)
{
    return (FIL_Path) {.path = NormalisePath(path, false, allocator)};
}

utf8str FIL_Name(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return (utf8str) {0};

    utf8str nameWithExtension = FIL_NameWithExtension(path);
    if (!nameWithExtension.data || !nameWithExtension.count)
        return (utf8str) {0};

    isize lastDotIdx = STR_FindLast(nameWithExtension, UTF8STR("."), false);
    if (lastDotIdx == -1)
        return nameWithExtension; // no extension, so name is the whole thing

    return STR_SubString(nameWithExtension, 0, lastDotIdx);
}

utf8str FIL_Extension(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return (utf8str) {0};

    isize lastDotIdx = STR_FindLast(path.path, UTF8STR("."), false);
    if (lastDotIdx == -1)
        return (utf8str) {0}; // no extension

    isize ldiP1 = lastDotIdx + 1;
    return STR_SubString(path.path, ldiP1, path.path.count - ldiP1);
}

utf8str FIL_NameWithExtension(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return (utf8str) {0};

    isize lastSlashIdx = STR_FindLast(path.path, UTF8STR("/"), false);

    // if no slash (lastSlashIdx = -1), name starts at 0, so conveniently
    // nameStartIdx will be lastSlashIdx + 1 in both cases

    isize nameStartIdx = lastSlashIdx + 1;
    return STR_SubString(path.path, nameStartIdx, path.path.count - nameStartIdx);
}

TIM_Value FIL_LastModified(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return (TIM_Value) {0};

    cstring str = STR_CloneToCStr(path.path, MEM_temp);

    #if MSR_WINDOWS
    {
        WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
        if (!GetFileAttributesExA(str, GetFileExInfoStandard, &fileAttrData)) { return (TIM_Value) {0}; }

        // convert FILETIME to nanoseconds since unix epoch
        u64 fileTime = ((u64) fileAttrData.ftLastWriteTime.dwHighDateTime << 32) | fileAttrData.ftLastWriteTime.dwLowDateTime;
        return (TIM_Value){.ns = (i64) (fileTime * 100)}; // FILETIME is in 100-nanosecond intervals
    }
    #elif MSR_UNIX
    {
        struct stat statBuf;
        if (stat(str, &statBuf) != 0) { return (TIM_Value) {0}; }

        #ifdef __APPLE__
            #define st_mtim st_mtimespec
        #endif

        // convert seconds to nanoseconds and add the nanosecond part
        return (TIM_Value){.ns = (i64) statBuf.st_mtime * 1000000000 + statBuf.st_mtim.tv_nsec};

        #ifdef __APPLE__
            #undef st_mtim
        #endif
    }
    #endif
}

b8 FIL_Exists(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return false;

    cstring str = STR_CloneToCStr(path.path, MEM_temp);

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

b8 FIL_Delete(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return false;

    cstring str = STR_CloneToCStr(path.path, MEM_temp);

    #if MSR_WINDOWS
        return DeleteFileA(str) != 0;
    #elif MSR_UNIX
        return unlink(str) == 0;
    #endif
}

typedef struct
{
    MEM_Allocator allocator;
    b8 failedAtSomething;
} PTH_Internal_DeleteAllContentsPayload;

static b8 PTH_Internal_DeleteAllContents(DIR_ChildPath childPath, rawptr userData, b8* exploreCurrentDirectory)
{
    PTH_Internal_DeleteAllContentsPayload* payload = (PTH_Internal_DeleteAllContentsPayload*) userData;

    if (childPath.ty == DIR_PathTy_Directory)
    {
        /**
         * The reason for doing things weirdly like this is because of how the directory iterator function works
         * it performs a top-down approach where the callback is given for the directory before its contents.
         * But we cannot delete the directory before its contents. So we kind of hack it by recursing the directory
         * iterator function ourselves.
         */
        *exploreCurrentDirectory = false;

        DIR_Iterate(childPath.path.dir, PTH_Internal_DeleteAllContents, userData, false);
        cstring path2 = STR_CloneToCStr(childPath.path.dir.path, payload->allocator);
        #if MSR_WINDOWS
            payload->failedAtSomething = (RemoveDirectoryA(path2) == 0) || payload->failedAtSomething;
        #elif MSR_UNIX
            payload->failedAtSomething = (rmdir(path2)            != 0) || payload->failedAtSomething;
        #endif
    }
    else
    {
        FIL_Delete(childPath.path.fil);
    }

    return true; // continue iterating
}

DIR_Path DIR_Normalise(utf8str path, MEM_Allocator allocator)
{
    return (DIR_Path) {.path = NormalisePath(path, true, allocator)};
}

DIR_Path DIR_DirPathParent(DIR_Path path)
{
    if (!path.path.data || !path.path.count)
        return (DIR_Path) {0};

    utf8str s = path.path;
    if (s.count && s.data[s.count - 1] == '/')
        s = STR_SubString(path.path, 0, path.path.count - 1); // skip trailing slash

    isize lastSlashIdx = STR_FindLast(s, UTF8STR("/"), false);
    if (lastSlashIdx == -1)
        return (DIR_Path) {.path = UTF8STR("/")};

    utf8str parentPath = STR_SubString(path.path, 0, lastSlashIdx + 1); // include the slash
    return (DIR_Path) {.path = parentPath};
}

DIR_Path DIR_FilPathParent(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return (DIR_Path) {0};

    isize lastSlashIdx = STR_FindLast(path.path, UTF8STR("/"), false);
    if (lastSlashIdx == -1)
        return (DIR_Path) {.path = UTF8STR("/")};

    utf8str parentPath = STR_SubString(path.path, 0, lastSlashIdx + 1); // include the slash
    return (DIR_Path) {.path = parentPath};
}

DIR_Path DIR_DirectoryInside(DIR_Path parent, utf8str childName, MEM_Allocator allocator)
{
    if (!parent.path.data || !parent.path.count || !childName.data || !childName.count)
        return (DIR_Path) {0};

    utf8str output = COL_NewSlice(u8, parent.path.count + childName.count + 1, true, allocator); // +1 for trailing slash
    if (!output.data || !output.count)
        return (DIR_Path) {0};

    MEM_Copy(output.data,                     parent.path.data, (usize) parent.path.count);
    MEM_Copy(output.data + parent.path.count, childName.data,   (usize) childName.count  );
    output.data[output.count - 1] = '/';
    return (DIR_Path) {.path = output};
}

FIL_Path DIR_FileInside(DIR_Path parent, utf8str childName, MEM_Allocator allocator)
{
    if (!parent.path.data || !parent.path.count || !childName.data || !childName.count)
        return (FIL_Path) {0};

    utf8str output = COL_NewSlice(u8, parent.path.count + childName.count, true, allocator);
    if (!output.data || !output.count)
        return (FIL_Path) {0};

    MEM_Copy(output.data,                     parent.path.data, (usize) parent.path.count);
    MEM_Copy(output.data + parent.path.count, childName.data,   (usize) childName.count  );
    return (FIL_Path) {.path = output};
}

void DIR_Iterate(DIR_Path path, DIR_IteratorProc visitor, rawptr userData, b8 recursive)
{
    if (!visitor || !path.path.data || !path.path.count)
        return;

    Slice_(char) tempBuffer = COL_NewSlice(char, path.path.count + 3, true, MEM_temp); // +3 for potential wildcard and null terminator
    MEM_Copy(tempBuffer.data, path.path.data, (usize) path.path.count);

    #if MSR_WINDOWS
    {
        isize iterator = path.path.count - 1;
        if (tempBuffer.data[iterator] == '/' || tempBuffer.data[iterator] == '\\') // has trailing slash
        {
            iterator = path.path.count;
        }
        else
        {
            tempBuffer.data[path.path.count] = '/'; // add slash at the end
            iterator = path.path.count + 1;
        }

        tempBuffer.data[iterator    ] = '*';  // add wildcard for file matching
        tempBuffer.data[iterator + 1] = '\0'; // null-terminate
        tempBuffer = COL_SubSlice(tempBuffer, 0, iterator + 1);
    }
    #elif MSR_UNIX
    {
        tempBuffer.data[path.path.count] = '\0'; // null-terminate
        tempBuffer = COL_SubSlice(tempBuffer, 0, path.path.count);
    }
    #endif

    // 4kb scratch buffer per invocation
    MEM_ArenaAllocator fnScratch = MEM_CreateArenaAllocator(4 * 1024, MEM_main);
    MEM_Allocator tempAllocator = MEM_AllocatorFromArena(&fnScratch);

    #if MSR_WINDOWS

        WIN32_FIND_DATAA findData;
        HANDLE findHandle = FindFirstFileA(tempBuffer.data, &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                cstring nextFileName = (cstring) findData.cFileName;

    #elif MSR_UNIX

        DIR* dir = opendir(tempBuffer.data);
        if (dir != NULL)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL)
            {
                cstring nextFileName = (cstring) entry->d_name;

    #endif

                isize fileNameLen = STR_CStrLen(nextFileName);
                if (fileNameLen == 0                                                    ) { continue; } // skip empty names
                if (fileNameLen == 1 && nextFileName[0] == '.'                          ) { continue; } // skip current directory
                if (fileNameLen == 2 && nextFileName[0] == '.' && nextFileName[1] == '.') { continue; } // skip parent directory

                // +3 for potential slash, null terminator, and just in case
                utf8str foundPath = COL_NewSlice(u8, path.path.count + fileNameLen, true, tempAllocator);
                MEM_Copy(foundPath.data, path.path.data, (usize) path.path.count);
                isize iterator = path.path.count - 1;
                if (foundPath.data[iterator] == '/' || foundPath.data[iterator] == '\\')
                {
                    iterator = path.path.count;
                }
                else
                {
                    foundPath.data[path.path.count] = '/'; // add slash at the end
                    iterator = path.path.count + 1;
                }

                MEM_Copy(foundPath.data + iterator, nextFileName, (usize) fileNameLen);
                iterator += fileNameLen;
                foundPath.data[iterator] = '\0'; // null-terminate the string, just in case

                foundPath = (utf8str) {.data = foundPath.data, .count = iterator}; // update count

                #if MSR_WINDOWS
                {
                    for (u32 i = 0; i < foundPath.count; i++)
                    {
                        if (foundPath.data[i] == '\\') { foundPath.data[i] = '/'; } // normalise path separators
                    }
                }
                #endif

                b8 isDirectory = false;

                #if MSR_WINDOWS
                    isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                #elif MSR_UNIX
                    struct stat statBuf;
                    if (stat((cstring) foundPath.data, &statBuf) == 0)
                    {
                        isDirectory = S_ISDIR(statBuf.st_mode);
                    }
                #endif

                if (isDirectory)
                {
                    foundPath.data[iterator] = '/'; // ensure directory paths end with a slash
                    foundPath.data[iterator + 1] = '\0'; // null-terminate the string

                    foundPath = (utf8str) {.data = foundPath.data, .count = iterator + 1}; // update count
                    iterator++;
                }

                utf8str foundPath2 = foundPath;

                DIR_ChildPath childPath = {.ty = isDirectory ? DIR_PathTy_Directory : DIR_PathTy_File, .path.fil.path = foundPath2};

                b8 exploreCurrentDirectory = recursive;
                b8 iterateFurther = visitor(childPath, userData, &exploreCurrentDirectory);

                // handle recursion
                if (iterateFurther && isDirectory && exploreCurrentDirectory)
                {
                    DIR_Iterate((DIR_Path) {.path = foundPath2}, visitor, userData, recursive);
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

    MEM_DestroyArenaAllocator(&fnScratch);
}

b8 DIR_Exists(DIR_Path path)
{
    if (!path.path.data || !path.path.count)
        return false;

    cstring str = STR_CloneToCStr(path.path, MEM_temp);

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

b8 DIR_Ensure(DIR_Path path)
{
    if (!path.path.data || !path.path.count)
        return false;

    if (DIR_Exists(path))
        return true;

    cstring alt = STR_CloneToCStr(path.path, MEM_temp);

    b8 success = true;
    for (isize i = 1; success && i < path.path.count; i++)
    {
        if (alt[i] == '/')
        {
            ((char*) alt)[i] = '\0';
            #if MSR_WINDOWS
                success = CreateDirectoryA(alt, nil) || GetLastError() == ERROR_ALREADY_EXISTS;
            #elif MSR_UNIX
                success = mkdir(alt, 0755) == 0 || errno == EEXIST;
            #endif
            ((char*) alt)[i] = '/';
        }
    }

    return success;
}

b8 DIR_Delete(DIR_Path path)
{
    if (!path.path.data || !path.path.count)
        return false;

    MEM_ArenaAllocator arena = MEM_CreateArenaAllocator(8 * 1024, MEM_main);
    MEM_Allocator allocator = MEM_AllocatorFromArena(&arena);

    b8 throwaway = false;
    PTH_Internal_DeleteAllContentsPayload payload = {.failedAtSomething = false, .allocator = allocator};
    PTH_Internal_DeleteAllContents((DIR_ChildPath){.ty = DIR_PathTy_Directory, .path.dir = path}, &payload, &throwaway);

    MEM_DestroyArenaAllocator(&arena);
    return !payload.failedAtSomething;
}

// directory watcher -------------------------------------------------------------------------------------------------

#if MSR_WINDOWS || MSR_APPLE

/**
 * A tracked child entry of the watched directory.
 * On Windows this is only used to resolve the type (file/directory) of removed children,
 * since ReadDirectoryChangesW does not report it.
 * On macOS this is the snapshot that kqueue events get diffed against, and it also owns
 * the per-file kqueue vnode fd used to detect content modifications.
 */
typedef struct
{
    utf8str name; // owned; allocated with the watcher's allocator
    b8      isDir;

    #if MSR_APPLE
        i64 mtimeNs;
        i64 sizeBytes;
        i32 fd; // O_EVTONLY fd registered with kqueue (files only); -1 if none
        b8  pendingModified;
        b8  pendingReopen;
    #endif
} DIR_Internal_WatchEntry;

COL_DECLARE_FOR(DIR_Internal_WatchEntry);

#endif

typedef struct
{
    MEM_Allocator       allocator;
    utf8str             dirPath;  // owned clone; normalised, with trailing slash
    MEM_ArenaAllocator  evtArena; // owns event path strings; reset on every refill
    List_(DIR_WatchEvt) pendingEvts;
    isize               nextEvtIdx;

    #if MSR_WINDOWS
        HANDLE     dirHandle;
        OVERLAPPED overlapped; // hEvent is owned by the watcher
        b8         readPending;
        List_(DIR_Internal_WatchEntry) entries;
        alignas(8) u8 buffer[32 * 1024]; // ReadDirectoryChangesW requires DWORD alignment
    #elif MSR_LINUX
        i32 inotifyFd;
        i32 watchFd;
    #elif MSR_APPLE
        i32 kq;
        i32 dirFd;
        List_(DIR_Internal_WatchEntry) entries;
    #endif
} DIR_Internal_Watcher;

static void DIR_Internal_PushWatchEvt(DIR_Internal_Watcher* w, DIR_WatchEvtTy ty, utf8str name, b8 isDir)
{
    MEM_Allocator evtAllocator = MEM_AllocatorFromArena(&w->evtArena);

    isize fullCount = w->dirPath.count + name.count + (isDir ? 1 : 0); // directories get a trailing slash
    utf8str full = COL_NewSlice(u8, fullCount + 1, true, evtAllocator); // +1 for a safety null terminator
    if (!full.data) { return; }

    MEM_Copy(full.data,                    w->dirPath.data, (usize) w->dirPath.count);
    MEM_Copy(full.data + w->dirPath.count, name.data,       (usize) name.count      );
    if (isDir) { full.data[fullCount - 1] = '/'; }
    full.data[fullCount] = '\0';
    full.count = fullCount;

    DIR_WatchEvt evt = {.ty = ty};
    if (isDir)
    {
        evt.path.ty = DIR_PathTy_Directory;
        evt.path.path.dir = (DIR_Path) {.path = full};
    }
    else
    {
        evt.path.ty = DIR_PathTy_File;
        evt.path.path.fil = (FIL_Path) {.path = full};
    }

    COL_AppendToList(&w->pendingEvts, evt);
}

#if MSR_WINDOWS || MSR_APPLE

// builds a null-terminated full path for a child of the watched directory, allocated from MEM_temp
static cstring DIR_Internal_WatchChildCStr(DIR_Internal_Watcher* w, utf8str name)
{
    Slice_(char) buf = COL_NewSlice(char, w->dirPath.count + name.count + 1, true, MEM_temp);
    MEM_Copy(buf.data,                    w->dirPath.data, (usize) w->dirPath.count);
    MEM_Copy(buf.data + w->dirPath.count, name.data,       (usize) name.count      );
    buf.data[w->dirPath.count + name.count] = '\0';
    return buf.data;
}

// isDirFilter: -1 = match any type, 0 = files only, 1 = directories only
static isize DIR_Internal_FindWatchEntry(const List_(DIR_Internal_WatchEntry)* entries, utf8str name, i32 isDirFilter)
{
    for (isize i = 0; i < entries->count; i++)
    {
        if (isDirFilter >= 0 && entries->data[i].isDir != (b8) (isDirFilter != 0)) { continue; }
        if (STR_Eq(entries->data[i].name, name)) { return i; }
    }

    return -1;
}

typedef struct
{
    DIR_Internal_Watcher* w;
    List_(DIR_Internal_WatchEntry)* out;
} DIR_Internal_WatcherScanPayload;

static b8 DIR_Internal_WatcherScanVisitor(DIR_ChildPath childPath, rawptr userData, b8* exploreCurrentDirectory)
{
    *exploreCurrentDirectory = false; // non-recursive watch

    DIR_Internal_WatcherScanPayload* payload = (DIR_Internal_WatcherScanPayload*) userData;
    DIR_Internal_Watcher* w = payload->w;

    b8 isDir = childPath.ty == DIR_PathTy_Directory;
    utf8str full = isDir ? childPath.path.dir.path : childPath.path.fil.path;

    isize nameCount = full.count - w->dirPath.count - (isDir ? 1 : 0); // directories have a trailing slash
    if (nameCount <= 0) { return true; }

    utf8str name = STR_SubString(full, w->dirPath.count, nameCount);

    DIR_Internal_WatchEntry entry = {.name = STR_Clone(name, w->allocator), .isDir = isDir};
    if (!entry.name.data) { return true; }

    #if MSR_APPLE
    {
        entry.fd = -1;

        struct stat statBuf;
        if (stat(STR_CloneToCStr(full, MEM_temp), &statBuf) == 0)
        {
            entry.mtimeNs   = (i64) statBuf.st_mtimespec.tv_sec * 1000000000 + (i64) statBuf.st_mtimespec.tv_nsec;
            entry.sizeBytes = (i64) statBuf.st_size;
        }
    }
    #endif

    COL_AppendToList(payload->out, entry);
    return true;
}

#endif

#if MSR_WINDOWS

static b8 DIR_Internal_IssueWatcherRead(DIR_Internal_Watcher* w)
{
    ResetEvent(w->overlapped.hEvent);

    DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
                   FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

    w->readPending = ReadDirectoryChangesW(w->dirHandle, w->buffer, sizeof(w->buffer), FALSE, filter, nil, &w->overlapped, nil) != 0;
    return w->readPending;
}

static void DIR_Internal_HandleWatchAction(DIR_Internal_Watcher* w, DWORD action, utf8str name)
{
    switch (action)
    {
        case FILE_ACTION_ADDED:
        case FILE_ACTION_RENAMED_NEW_NAME: // renames are treated as removal + addition
        {
            DWORD attrs = GetFileAttributesA(DIR_Internal_WatchChildCStr(w, name));
            b8 isDir = (attrs != INVALID_FILE_ATTRIBUTES) && ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);

            if (DIR_Internal_FindWatchEntry(&w->entries, name, -1) < 0)
            {
                DIR_Internal_WatchEntry entry = {.name = STR_Clone(name, w->allocator), .isDir = isDir};
                if (entry.name.data) { COL_AppendToList(&w->entries, entry); }
            }

            DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Added, name, isDir);
            break;
        }

        case FILE_ACTION_REMOVED:
        case FILE_ACTION_RENAMED_OLD_NAME: // renames are treated as removal + addition
        {
            isize idx = DIR_Internal_FindWatchEntry(&w->entries, name, -1);
            b8 isDir = idx >= 0 && w->entries.data[idx].isDir;
            if (idx >= 0)
            {
                MEM_Deallocate(w->allocator, w->entries.data[idx].name.data);
                COL_RemoveIdxFromList(&w->entries, idx);
            }

            DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Removed, name, isDir);
            break;
        }

        case FILE_ACTION_MODIFIED:
        {
            b8 isDir;

            isize idx = DIR_Internal_FindWatchEntry(&w->entries, name, -1);
            if (idx >= 0)
            {
                isDir = w->entries.data[idx].isDir;
            }
            else
            {
                DWORD attrs = GetFileAttributesA(DIR_Internal_WatchChildCStr(w, name));
                isDir = (attrs != INVALID_FILE_ATTRIBUTES) && ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);
            }

            DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Modified, name, isDir);
            break;
        }

        default: break;
    }
}

static void DIR_Internal_RefillWatchEvts(DIR_Internal_Watcher* w)
{
    if (w->dirHandle == INVALID_HANDLE_VALUE) { return; }

    if (!w->readPending && !DIR_Internal_IssueWatcherRead(w)) { return; }

    DWORD bytes = 0;
    if (!GetOverlappedResult(w->dirHandle, &w->overlapped, &bytes, FALSE))
    {
        if (GetLastError() != ERROR_IO_INCOMPLETE) { w->readPending = false; } // failed; reissue on the next poll
        return;
    }

    w->readPending = false;

    if (!bytes)
    {
        // the notification buffer overflowed and events were lost;
        // resync the tracked entries so type lookups stay correct
        for (isize i = 0; i < w->entries.count; i++)
        {
            MEM_Deallocate(w->allocator, w->entries.data[i].name.data);
        }
        COL_ClearList(&w->entries);

        DIR_Internal_WatcherScanPayload payload = {.w = w, .out = &w->entries};
        DIR_Iterate((DIR_Path) {.path = w->dirPath}, DIR_Internal_WatcherScanVisitor, &payload, false);
    }
    else
    {
        u8* base = w->buffer;
        for (;;)
        {
            FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*) base;

            i32 nameChars = (i32) (fni->FileNameLength / sizeof(WCHAR));
            i32 utf8Count = nameChars ? WideCharToMultiByte(CP_UTF8, 0, fni->FileName, nameChars, nil, 0, nil, nil) : 0;
            if (utf8Count > 0)
            {
                utf8str name = COL_NewSlice(u8, utf8Count, true, MEM_temp);
                WideCharToMultiByte(CP_UTF8, 0, fni->FileName, nameChars, (char*) name.data, utf8Count, nil, nil);

                for (isize i = 0; i < name.count; i++)
                {
                    if (name.data[i] == '\\') { name.data[i] = '/'; } // normalise path separators
                }

                DIR_Internal_HandleWatchAction(w, fni->Action, name);
            }

            if (!fni->NextEntryOffset) { break; }
            base += fni->NextEntryOffset;
        }
    }

    DIR_Internal_IssueWatcherRead(w);
}

#elif MSR_LINUX

static void DIR_Internal_RefillWatchEvts(DIR_Internal_Watcher* w)
{
    if (w->inotifyFd < 0) { return; }

    for (;;)
    {
        alignas(8) u8 buf[4096];
        isize n = read(w->inotifyFd, buf, sizeof(buf));
        if (n <= 0) { break; } // EAGAIN (nothing pending) or error

        for (isize offset = 0; offset < n;)
        {
            const struct inotify_event* ev = (const struct inotify_event*) (buf + offset);
            offset += (isize) sizeof(struct inotify_event) + (isize) ev->len;

            if (ev->wd != w->watchFd || !ev->len) { continue; } // also skips IN_Q_OVERFLOW/IN_IGNORED

            utf8str name = STR_AliasCStr((cstring) ev->name);
            if (!name.count) { continue; }

            b8 isDir = (ev->mask & IN_ISDIR) != 0;

            // renames are treated as removal + addition (IN_MOVED_FROM arrives before IN_MOVED_TO)
            if      (ev->mask & (IN_CREATE | IN_MOVED_TO  )) { DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Added,    name, isDir); }
            else if (ev->mask & (IN_DELETE | IN_MOVED_FROM)) { DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Removed,  name, isDir); }
            else if (ev->mask & (IN_MODIFY | IN_ATTRIB    )) { DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Modified, name, isDir); }
        }
    }
}

#elif MSR_APPLE

static void DIR_Internal_RegisterWatchEntry(DIR_Internal_Watcher* w, DIR_Internal_WatchEntry* entry)
{
    entry->fd = -1;
    if (entry->isDir) { return; } // only files are watched for content modifications

    int fd = open(DIR_Internal_WatchChildCStr(w, entry->name), O_EVTONLY);
    if (fd < 0) { return; }

    struct kevent kev;
    EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_DELETE | NOTE_RENAME, 0, nil);
    if (kevent(w->kq, &kev, 1, nil, 0, nil) < 0)
    {
        close(fd);
        return;
    }

    entry->fd = fd;
}

static void DIR_Internal_RefillWatchEvts(DIR_Internal_Watcher* w)
{
    if (w->kq < 0) { return; }

    for (isize i = 0; i < w->entries.count; i++)
    {
        w->entries.data[i].pendingModified = false;
        w->entries.data[i].pendingReopen   = false;
    }

    // drain all pending kqueue events without blocking
    b8 dirChanged = false;
    for (;;)
    {
        struct kevent evts[16];
        struct timespec zeroTimeout = {0};
        int n = kevent(w->kq, nil, 0, evts, 16, &zeroTimeout);
        if (n <= 0) { break; }

        for (int i = 0; i < n; i++)
        {
            i32 fd = (i32) evts[i].ident;
            if (fd == w->dirFd)
            {
                dirChanged = true;
                continue;
            }

            for (isize j = 0; j < w->entries.count; j++)
            {
                DIR_Internal_WatchEntry* entry = &w->entries.data[j];
                if (entry->fd != fd) { continue; }

                if (evts[i].fflags & (NOTE_DELETE | NOTE_RENAME))
                {
                    entry->pendingReopen = true;
                    dirChanged = true; // the name may now point at a different inode (or nothing)
                }

                if (evts[i].fflags & (NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB)) { entry->pendingModified = true; }
                break;
            }
        }

        if (n < 16) { break; }
    }

    if (!dirChanged)
    {
        // no structural change; only content modifications
        for (isize i = 0; i < w->entries.count; i++)
        {
            DIR_Internal_WatchEntry* entry = &w->entries.data[i];
            if (!entry->pendingModified || entry->isDir) { continue; }

            struct stat statBuf;
            if (stat(DIR_Internal_WatchChildCStr(w, entry->name), &statBuf) == 0)
            {
                entry->mtimeNs   = (i64) statBuf.st_mtimespec.tv_sec * 1000000000 + (i64) statBuf.st_mtimespec.tv_nsec;
                entry->sizeBytes = (i64) statBuf.st_size;
            }

            DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Modified, entry->name, false);
        }

        return;
    }

    // structural change: rescan the directory and diff against the previous snapshot
    List_(DIR_Internal_WatchEntry) newEntries = COL_NewList(DIR_Internal_WatchEntry, w->entries.count + 4, w->allocator);
    DIR_Internal_WatcherScanPayload payload = {.w = w, .out = &newEntries};
    DIR_Iterate((DIR_Path) {.path = w->dirPath}, DIR_Internal_WatcherScanVisitor, &payload, false);

    // removals first (so renames show up as removal followed by addition)
    for (isize i = 0; i < w->entries.count; i++)
    {
        DIR_Internal_WatchEntry* old = &w->entries.data[i];
        if (DIR_Internal_FindWatchEntry(&newEntries, old->name, old->isDir ? 1 : 0) >= 0) { continue; }

        DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Removed, old->name, old->isDir);
        if (old->fd >= 0)
        {
            close(old->fd);
            old->fd = -1;
        }
    }

    // then additions, carrying over surviving entries
    for (isize i = 0; i < newEntries.count; i++)
    {
        DIR_Internal_WatchEntry* fresh = &newEntries.data[i];

        isize oldIdx = DIR_Internal_FindWatchEntry(&w->entries, fresh->name, fresh->isDir ? 1 : 0);
        if (oldIdx < 0)
        {
            DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Added, fresh->name, fresh->isDir);
            DIR_Internal_RegisterWatchEntry(w, fresh);
            continue;
        }

        DIR_Internal_WatchEntry* old = &w->entries.data[oldIdx];
        fresh->fd = old->fd;
        old->fd = -1; // consumed

        if (old->pendingReopen)
        {
            // the old inode was deleted/renamed but the name still exists; watch the new inode
            if (fresh->fd >= 0)
            {
                close(fresh->fd);
                fresh->fd = -1;
            }

            DIR_Internal_RegisterWatchEntry(w, fresh);
        }

        b8 modified = old->pendingModified || old->mtimeNs != fresh->mtimeNs || old->sizeBytes != fresh->sizeBytes;
        if (modified && !fresh->isDir) { DIR_Internal_PushWatchEvt(w, DIR_WatchEvtTy_Modified, fresh->name, false); }
    }

    // release the old snapshot
    for (isize i = 0; i < w->entries.count; i++)
    {
        DIR_Internal_WatchEntry* old = &w->entries.data[i];
        if (old->fd >= 0) { close(old->fd); }
        MEM_Deallocate(w->allocator, old->name.data);
    }

    COL_DeleteList(&w->entries);
    w->entries = newEntries;
}

#endif

DIR_Watcher DIR_CreateWatcher(DIR_Path path, MEM_Allocator allocator)
{
    if (!path.path.data || !path.path.count || !DIR_Exists(path))
        return (DIR_Watcher) {0};

    DIR_Internal_Watcher* w = MEM_New(DIR_Internal_Watcher, allocator);
    if (!w) { return (DIR_Watcher) {0}; }

    w->allocator   = allocator;
    w->dirPath     = STR_Clone(path.path, allocator);
    w->evtArena    = MEM_CreateArenaAllocator(8 * 1024, allocator);
    w->pendingEvts = COL_NewList(DIR_WatchEvt, 16, allocator);
    w->nextEvtIdx  = 0;

    b8 ok = w->dirPath.data != nil;

    #if MSR_WINDOWS
    {
        w->dirHandle   = INVALID_HANDLE_VALUE;
        w->readPending = false;
        w->entries     = COL_NewList(DIR_Internal_WatchEntry, 16, allocator);

        if (ok)
        {
            cstring str = STR_CloneToCStr(w->dirPath, MEM_temp);
            w->dirHandle = CreateFileA(
                str,
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nil,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                nil
            );
            ok = w->dirHandle != INVALID_HANDLE_VALUE;
        }

        if (ok)
        {
            w->overlapped.hEvent = CreateEventA(nil, TRUE, FALSE, nil);
            ok = w->overlapped.hEvent != nil;
        }

        if (ok)
        {
            // snapshot the current children so removals can be typed later
            DIR_Internal_WatcherScanPayload payload = {.w = w, .out = &w->entries};
            DIR_Iterate((DIR_Path) {.path = w->dirPath}, DIR_Internal_WatcherScanVisitor, &payload, false);

            ok = DIR_Internal_IssueWatcherRead(w);
        }
    }
    #elif MSR_LINUX
    {
        w->inotifyFd = -1;
        w->watchFd   = -1;

        if (ok)
        {
            w->inotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
            ok = w->inotifyFd >= 0;
        }

        if (ok)
        {
            cstring str = STR_CloneToCStr(w->dirPath, MEM_temp);
            u32 mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_ATTRIB | IN_MOVED_FROM | IN_MOVED_TO | IN_ONLYDIR;
            w->watchFd = inotify_add_watch(w->inotifyFd, str, mask);
            ok = w->watchFd >= 0;
        }
    }
    #elif MSR_APPLE
    {
        w->kq      = -1;
        w->dirFd   = -1;
        w->entries = COL_NewList(DIR_Internal_WatchEntry, 16, allocator);

        if (ok)
        {
            w->kq = kqueue();
            ok = w->kq >= 0;
        }

        if (ok)
        {
            cstring str = STR_CloneToCStr(w->dirPath, MEM_temp);
            w->dirFd = open(str, O_EVTONLY);
            ok = w->dirFd >= 0;
        }

        if (ok)
        {
            struct kevent kev;
            EV_SET(&kev, w->dirFd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_DELETE | NOTE_RENAME, 0, nil);
            ok = kevent(w->kq, &kev, 1, nil, 0, nil) >= 0;
        }

        if (ok)
        {
            // snapshot the current children and watch each file for content modifications
            DIR_Internal_WatcherScanPayload payload = {.w = w, .out = &w->entries};
            DIR_Iterate((DIR_Path) {.path = w->dirPath}, DIR_Internal_WatcherScanVisitor, &payload, false);

            for (isize i = 0; i < w->entries.count; i++)
            {
                DIR_Internal_RegisterWatchEntry(w, &w->entries.data[i]);
            }
        }
    }
    #endif

    DIR_Watcher result = {.internal = w};
    if (!ok)
    {
        DIR_DestroyWatcher(result);
        return (DIR_Watcher) {0};
    }

    return result;
}

void DIR_DestroyWatcher(DIR_Watcher watcher)
{
    DIR_Internal_Watcher* w = (DIR_Internal_Watcher*) watcher.internal;
    if (!w) { return; }

    #if MSR_WINDOWS
    {
        if (w->dirHandle != INVALID_HANDLE_VALUE)
        {
            if (w->readPending)
            {
                // make sure the kernel is done with the buffer/OVERLAPPED before freeing them
                CancelIoEx(w->dirHandle, &w->overlapped);
                DWORD bytes = 0;
                GetOverlappedResult(w->dirHandle, &w->overlapped, &bytes, TRUE);
            }

            CloseHandle(w->dirHandle);
        }

        if (w->overlapped.hEvent) { CloseHandle(w->overlapped.hEvent); }
    }
    #elif MSR_LINUX
    {
        if (w->inotifyFd >= 0)
        {
            if (w->watchFd >= 0) { inotify_rm_watch(w->inotifyFd, w->watchFd); }
            close(w->inotifyFd);
        }
    }
    #elif MSR_APPLE
    {
        for (isize i = 0; i < w->entries.count; i++)
        {
            if (w->entries.data[i].fd >= 0) { close(w->entries.data[i].fd); }
        }

        if (w->dirFd >= 0) { close(w->dirFd); }
        if (w->kq    >= 0) { close(w->kq);    }
    }
    #endif

    #if MSR_WINDOWS || MSR_APPLE
    {
        for (isize i = 0; i < w->entries.count; i++)
        {
            MEM_Deallocate(w->allocator, w->entries.data[i].name.data);
        }

        COL_DeleteList(&w->entries);
    }
    #endif

    COL_DeleteList(&w->pendingEvts);
    MEM_DestroyArenaAllocator(&w->evtArena);
    if (w->dirPath.data) { MEM_Deallocate(w->allocator, w->dirPath.data); }
    MEM_Delete(w, w->allocator);
}

b8 DIR_IterateWatchEvts(DIR_Watcher watcher, DIR_WatchEvt* outEvt)
{
    DIR_Internal_Watcher* w = (DIR_Internal_Watcher*) watcher.internal;
    if (!w || !outEvt) { return false; }

    if (w->nextEvtIdx >= w->pendingEvts.count)
    {
        // queue exhausted; recycle event path memory and poll the platform for new events
        COL_ClearList(&w->pendingEvts);
        w->nextEvtIdx = 0;
        MEM_DeallocateAll(MEM_AllocatorFromArena(&w->evtArena));

        DIR_Internal_RefillWatchEvts(w);

        if (!w->pendingEvts.count) { return false; }
    }

    *outEvt = w->pendingEvts.data[w->nextEvtIdx];
    w->nextEvtIdx += 1;
    return true;
}
