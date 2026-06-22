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

DIR_Path DIR_Parent(DIR_Path path)
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

                bool isDirectory = false;

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

                bool exploreCurrentDirectory = recursive;
                bool iterateFurther = visitor(childPath, userData, &exploreCurrentDirectory);

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

FIL_Path FIL_Normalise(utf8str path, MEM_Allocator allocator)
{
    return (FIL_Path) {.path = NormalisePath(path, false, allocator)};
}

DIR_Path FIL_Parent(FIL_Path path)
{
    if (!path.path.data || !path.path.count)
        return (DIR_Path) {0};

    isize lastSlashIdx = STR_FindLast(path.path, UTF8STR("/"), false);
    if (lastSlashIdx == -1)
        return (DIR_Path) {.path = UTF8STR("/")};

    utf8str parentPath = STR_SubString(path.path, 0, lastSlashIdx + 1); // include the slash
    return (DIR_Path) {.path = parentPath};
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
