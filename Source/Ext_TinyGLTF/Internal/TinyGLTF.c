#include <__init.h>
#include <TinyGLTF.h>

#if MSR_BUILD_DEPS
MSR_SUPPRESS_WARN
#define TINYGLTF3_ENABLE_STB_IMAGE
#include <ExtDeps/tiny_gltf_v3.c>
#undef TINYGLTF3_ENABLE_STB_IMAGE
MSR_UNSUPPRESS_WARN
#else

static void* TGLTF_Alloc(size_t size, void* userData)
{
    TGLTF_Ctx* ctx = (TGLTF_Ctx*) userData;
    return MEM_Allocate(ctx->allocator, true, size, 8);
}

static void* TGLTF_Realloc(void* ptr, size_t oldSize, size_t newSize, void* userData)
{
    TGLTF_Ctx* ctx = (TGLTF_Ctx*) userData;
    return MEM_Reallocate(ctx->allocator, true, ptr, oldSize, newSize, 8);
}

static void TGLTF_Free(void* ptr, size_t size, void* userData)
{
    TGLTF_Ctx* ctx = (TGLTF_Ctx*) userData;
    MEM_Deallocate(ctx->allocator, ptr);
}

static int32_t TGLTF_FileExists(const char* path, uint32_t pathLen, void* userData)
{
    utf8str pathStr = {.data = (u8*) path, .count = pathLen};
    FIL_Path pathFr = FIL_Normalise(pathStr, MEM_temp);
    return FIL_Exists(pathFr);
}

static int32_t TGLTF_ReadFile(uint8_t** outData, uint64_t* outSize, const char* path, uint32_t pathLen, void* userData)
{
    TGLTF_Ctx* ctx = (TGLTF_Ctx*) userData;
    utf8str pathStr = {.data = (u8*) path, .count = pathLen};
    FIL_Path pathFr = FIL_Normalise(pathStr, MEM_temp);

    Slice_(u8) output = IO_ReadEntireFile(pathFr, ctx->allocator);

    if (!output.data || !output.count)
        return 0;

    if (outData) *outData = output.data;
    if (outSize) *outSize = output.count;
    return 1;
}

static void TGLTF_FreeFile(uint8_t* data, uint64_t size, void* userData)
{
    TGLTF_Ctx* ctx = (TGLTF_Ctx*) userData;
    Slice_(u8) toFree = {.data = data, .count = size};
    COL_DeleteSlice(&toFree, ctx->allocator);
}

static int32_t TGLTF_WriteFile(const char* path, uint32_t pathLen, const uint8_t* data, uint64_t size, void* userData)
{
    utf8str pathStr = {.data = (u8*) path, .count = pathLen};
    FIL_Path pathFr = FIL_Normalise(pathStr, MEM_temp);
    Slice_(u8) toWrite = {.data = (u8*) data, .count = size};
    return IO_WriteAllToFile(pathFr, toWrite, false) ? 1 : 0;
}

static int32_t TGLTF_GetFileSize(uint64_t* outSize, const char* path, uint32_t pathLen, void* userData)
{
    utf8str pathStr = {.data = (u8*) path, .count = pathLen};
    FIL_Path pathFr = FIL_Normalise(pathStr, MEM_temp);
    IO_Stream stream = IO_OpenFileToRead(pathFr, false);
    if (!stream.procedure)
        return 0;

    if (outSize)
        *outSize = (uint64_t) IO_GetSize(stream);
    IO_Close(stream);
    return 1;
}

tg3_allocator TGLTF_GetAllocator(TGLTF_Ctx* ctx)
{
    return (tg3_allocator)
    {
        .user_data = ctx,
        .alloc     = TGLTF_Alloc,
        .realloc   = TGLTF_Realloc,
        .free      = TGLTF_Free,
    };
}

tg3_fs_callbacks TGLTF_GetFileSystem(TGLTF_Ctx* ctx)
{
    return (tg3_fs_callbacks)
    {
        .user_data   = ctx,
        .file_exists = TGLTF_FileExists,
        .read_file   = TGLTF_ReadFile,
        .free_file   = TGLTF_FreeFile,
        .write_file  = TGLTF_WriteFile,
    };
}

#endif
