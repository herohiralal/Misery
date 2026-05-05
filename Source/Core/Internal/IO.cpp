#include <Core/IO.h>

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
            for (int32_t i = 3; i < path.Length(); ++i)
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

                    for (; i < path.Length(); ++i)
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
            for (int32_t i = 0; i < output.Length(); ++i)
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
}

DirectoryPath DirectoryPath::Normalise(String path, Allocator allocator)
{
    return DirectoryPath(Misery::IO::Internal::NormalisePath(path ? path : String("."), true, allocator));
}

FilePath FilePath::Normalise(String path, Allocator allocator)
{
    return FilePath(Misery::IO::Internal::NormalisePath(path ? path : String("unknown.file"), false, allocator));
}
