#include <__init.h>
#include <TinyObj.h>

#if MSR_BUILD_DEPS
MSR_SUPPRESS_WARN
#define TOBJ_NO_LIBC
#define TOBJ_ENABLE_MULTITHREADING
#include <ExtDeps/tiny_obj_c.c>
#include <ExtDeps/tobj_tess.c>
#undef TOBJ_ENABLE_MULTITHREADING
#undef TOBJ_NO_LIBC
MSR_UNSUPPRESS_WARN
#else

static void* TOBJ_Alloc(void* ud, size_t size, size_t align)
{
    TOBJ_Ctx* ctx = (TOBJ_Ctx*) ud;
    return MEM_Allocate(ctx->allocator, true, size, align);
}

static void* TOBJ_Calloc(void* ud, size_t count, size_t size, size_t align)
{
    TOBJ_Ctx* ctx = (TOBJ_Ctx*) ud;
    return MEM_Allocate(ctx->allocator, true, count * size, align);
}

static void* TOBJ_Realloc(void* ud, void* ptr, size_t oldSize, size_t newSize, size_t align)
{
    TOBJ_Ctx* ctx = (TOBJ_Ctx*) ud;
    return MEM_Reallocate(ctx->allocator, true, ptr, oldSize, newSize, align);
}

static void TOBJ_Free(void* ud, void *ptr, size_t size)
{
    TOBJ_Ctx* ctx = (TOBJ_Ctx*) ud;
    MEM_Deallocate(ctx->allocator, ptr);
}

tobj_allocator TOBJ_GetAllocator(TOBJ_Ctx* ctx)
{
    return (tobj_allocator)
    {
        .alloc          = TOBJ_Alloc,
        .calloc         = TOBJ_Calloc,
        .realloc        = TOBJ_Realloc,
        .free           = TOBJ_Free,
        .user_data      = ctx,
        .max_alloc_size = 0,
    };
}

static void TOBJ_OnMessage(void* ud, tobj_severity sev, size_t lineNo, const char* msg, size_t msgLen)
{
    TOBJ_Diagnostics* ctx = (TOBJ_Diagnostics*) ud;
    utf8str obj = ctx->objectName;
    u64 line = (size_t) lineNo;
    utf8str message = {.data = (u8*) msg, .count = (isize) msgLen};

    switch (sev)
    {
        case TOBJ_SEV_WARNING:
            LOG_Wrn(TNYOBJ, "[\"%\"] while parsing %:%", FMT(message), FMT(obj), FMT(line));
            break;
        case TOBJ_SEV_ERROR:
            LOG_Err(TNYOBJ, "[\"%\"] while parsing %:%", FMT(message), FMT(obj), FMT(line));
            break;
        default:
            MSR_ASSERT(false && "unknown severity");
            break;
    }
}

tobj_diag TOBJ_GetDiagnostics(TOBJ_Diagnostics* ctx)
{
    return (tobj_diag) {.on_message = TOBJ_OnMessage, .user_data = ctx};
}

static tobj_result TOBJ_Read(void* ud, uint8_t* dst, size_t dstSize, size_t* bytesRead)
{
    TOBJ_Stream* stream = (TOBJ_Stream*) ud;

    Slice_(u8) output = {.data = dst, .count = (isize) dstSize};
    isize bytesRead2 = IO_Read(stream->stream, output);
    if (bytesRead) *bytesRead = (size_t) bytesRead2;
    return TOBJ_OK;
}

tobj_io_callbacks TOBJ_GetStream(TOBJ_Stream* stream)
{
    return (tobj_io_callbacks)
    {
        .read      = TOBJ_Read,
        .close     = nil,
        .user_data = stream,
    };
}

tobj_load_config TOBJ_GetDefaultLoadConfig(TOBJ_Ctx* ctx)
{
    return (tobj_load_config)
    {
        .allocator = TOBJ_GetAllocator(ctx),
        .triangulate = true,
        .store_vertex_colors = true,
        .vertex_color_fallback = true,
        .parse_freeform = false,
        .use_arena = false,
        .num_threads = -1,
        .max_vertices = 0,
        .max_indices = 0,
        .max_faces = 0,
        .max_face_arity = 0,
        .max_materials = 0,
        .max_shapes = 0,
        .max_line_bytes = 0,
        .max_input_bytes = 0,
        .mtl_resolver = nil,
        .mtl_resolver_user_data = nil,
    };
}

tobj_result TOBJ_LoadObjFromMemory(tobj_scene* output, MEM_Allocator allocator, utf8str objName, Slice_(u8) data)
{
    TOBJ_Ctx ctx = {.allocator = allocator};
    TOBJ_Diagnostics diag = {.objectName = objName};
    tobj_diag diagSink = TOBJ_GetDiagnostics(&diag);
    tobj_load_config cfg = TOBJ_GetDefaultLoadConfig(&ctx);
    return tobj_load_obj_from_memory(output, (uint8_t*) data.data, (size_t) data.count, &cfg, &diagSink);
}

tobj_result TOBJ_LoadObjFromFile(tobj_scene* output, MEM_Allocator allocator, FIL_Path path)
{
    TOBJ_Ctx ctx = {.allocator = allocator};
    TOBJ_Diagnostics diag = {.objectName = path.path};
    tobj_diag diagSink = TOBJ_GetDiagnostics(&diag);

    TOBJ_Stream stream = {.ctx = &ctx, .stream = IO_OpenFileToRead(path, false)};
    if (!stream.stream.procedure)
        return TOBJ_ERR_NOT_FOUND;

    tobj_io_callbacks ioCallbacks = TOBJ_GetStream(&stream);

    tobj_load_config cfg = TOBJ_GetDefaultLoadConfig(&ctx);
    tobj_result result = tobj_load_obj_from_io(output, &ioCallbacks, &cfg, &diagSink);

    IO_Close(stream.stream);
    return result;
}

tobj_result TOBJ_LoadObjFromStream(tobj_scene* output, MEM_Allocator allocator, utf8str objName, IO_Stream stream)
{
    TOBJ_Ctx ctx = {.allocator = allocator};
    TOBJ_Diagnostics diag = {.objectName = objName};
    tobj_diag diagSink = TOBJ_GetDiagnostics(&diag);

    TOBJ_Stream streamWrapper = {.ctx = &ctx, .stream = stream};
    tobj_io_callbacks ioCallbacks = TOBJ_GetStream(&streamWrapper);

    tobj_load_config cfg = TOBJ_GetDefaultLoadConfig(&ctx);
    return tobj_load_obj_from_io(output, &ioCallbacks, &cfg, &diagSink);
}

#endif
