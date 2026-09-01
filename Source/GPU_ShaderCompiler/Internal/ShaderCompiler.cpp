#include "GPU_ShaderCompiler/GPU_ShaderCompiler.h"
#include "ExtDeps_GPU.h"

#if DX_SHADER_COMPILER
namespace GPU_ShaderCompiler
{
    static const struct Internals
    {
        Internals() = default;

        Internals(PRC_Library inLib)
            : init(true)
            , library(inLib)
        {
            createInstance = (DxcCreateInstanceProc) PRC_GetLibraryFunction(
                library, UTF8STR("DxcCreateInstance"));

            MSR_ASSERT(createInstance && "Shader compiler library not loaded properly!");
        }

        ~Internals()
        {
            if (!init)
                return;

            PRC_UnloadLibrary(library);
        }

        HRESULT DxcCreateInstance(REFCLSID rclsid, REFIID riid, LPVOID* ppv) const
        {
            if (!init)
                return E_FAIL;

            return createInstance(rclsid, riid, ppv);
        }

        b8                    init;
        PRC_Library           library;
        DxcCreateInstanceProc createInstance;
    } G_Internals = { };

    static thread_local const struct ThreadInternals
    {
        ThreadInternals() = default;
        ThreadInternals(const Internals& globals)
            : init(true)
        {
            HRESULT res1 = globals.DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
            MSR_ASSERT(SUCCEEDED(res1) && "Failed to create `IDxcUtils*`!");

            HRESULT res2 = globals.DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
            MSR_ASSERT(SUCCEEDED(res2) && "Failed to create `IDxcCompiler3*`!");

            HRESULT res3 = utils->CreateDefaultIncludeHandler(&includeHandler);
            MSR_ASSERT(SUCCEEDED(res3) && "Failed to create `IDxcIncludeHandler*`!");
        }

        ~ThreadInternals()
        {
            if (!init)
                return;

            if (includeHandler) includeHandler->Release();
            if (compiler) compiler->Release();
            if (utils) utils->Release();
        }

        b8                  init;
        IDxcUtils*          utils;
        IDxcCompiler3*      compiler;
        IDxcIncludeHandler* includeHandler;
    } G_ThreadInternals = { };

    static void Init()
    {
        static SYN_DoOnce globalsInit = { };
        SYN_ExecuteDoOnce(&globalsInit, []()
        {
            FIL_Path executablePath = PRC_GetCurrentExecutablePath(MEM_temp);
            DIR_Path execContainingDir = DIR_Parent(executablePath);
            FIL_Path libPath = DIR_FileInside(execContainingDir, UTF8STR(DX_SHADER_COMPILER_PATH), MEM_temp);
            PRC_Library library = PRC_LoadLibrary(libPath);
            MSR_ASSERT(library.handle && "Failed to load shader compiler library!");
            *const_cast<Internals*>(&G_Internals) = Internals(library);
        });

        if (!G_ThreadInternals.init)
        {
            *const_cast<ThreadInternals*>(&G_ThreadInternals) = ThreadInternals(G_Internals);
        }
    }

    static wchar_t* ToWideString(utf8str str, MEM_Allocator allocator)
    {
        if (!str.data || !str.count)
            return { };

        #if MSR_WINDOWS
        {
            int requiredSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                (const char*) str.data, (int) str.count, nullptr, 0);

            if (requiredSize <= 0)
                return nil;

            wchar_t* output = (wchar_t*) MEM_Allocate(allocator, false,
                sizeof(wchar_t) * (requiredSize + 1), alignof(wchar_t));

            if (!output)
                return nil;

            int convertedSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                (const char*) str.data, (int) str.count, output, requiredSize + 1);

            if (convertedSize <= 0)
            {
                MEM_Deallocate(allocator, output);
                return nil;
            }

            return output;
        }
        #else
        {
            std::mbstate_t state = { };
            size_t requiredSize = std::mbsrtowcs(nullptr, (const char**) &(str.data),
                0, &state);

            if (requiredSize == (size_t) -1)
                return nil;

            wchar_t* output = (wchar_t*) MEM_Allocate(allocator, false,
                sizeof(wchar_t) * (requiredSize + 1), alignof(wchar_t));

            if (!output)
                return nil;

            state = std::mbstate_t { };
            size_t convertedSize = std::mbsrtowcs(output, (const char**) &(str.data),
                requiredSize + 1, &state);

            if (convertedSize == (size_t) -1)
            {
                MEM_Deallocate(allocator, output);
                return nil;
            }

            return output;
        }
        #endif
    }
}
#endif

EXTERN_C_BEGIN
COL_DECLARE_FOR(LPCWSTR);
EXTERN_C_END

b8 GPU_CompileProgramStage(GPU_ProgramStageByteCode* stage, GPU_ProgramStageByteCodeCfg cfg)
{
    #if !DX_SHADER_COMPILER
    {
        (void) stage;
        (void) cfg;

        LOG_Err(SHDCMPL, "Shader compiler not implemented for this platform!");
        return false;
    }
    #else
    {
        GPU_ShaderCompiler::Init();
        wchar_t* fileW = GPU_ShaderCompiler::ToWideString(cfg.file.path, MEM_temp);

        IDxcBlobEncoding* srcBlob = nil;
        {
            u32 codePage = DXC_CP_ACP;
            if (FAILED(GPU_ShaderCompiler::G_ThreadInternals.utils->LoadFile(fileW, &codePage, &srcBlob)))
            {
                return false;
            }
        }
        DEFER { if (srcBlob) { srcBlob->Release(); srcBlob = nil; } };

        List_(LPCWSTR) args = COL_NewList(LPCWSTR, 30, MEM_temp);

        COL_AppendToList(&args, fileW);
        COL_AppendToList(&args, L"-T");

        b8 useMeshShaderExt = false;
        {
            const wchar_t* shdTy = nil;
            switch (cfg.stage)
            {
                case GPU_ProgramStageType_Compute:   shdTy = L"cs_6_0";                          break;
                case GPU_ProgramStageType_Task:      shdTy = L"as_6_5"; useMeshShaderExt = true; break;
                case GPU_ProgramStageType_Mesh:      shdTy = L"ms_6_5"; useMeshShaderExt = true; break;
                case GPU_ProgramStageType_Vertex:    shdTy = L"vs_6_0";                          break;
                case GPU_ProgramStageType_Fragment:  shdTy = L"ps_6_0";                          break;
                default:
                {
                    LOG_Err(SHDCMPL, "Unknown shader type!");
                    return false;
                }
            }
            COL_AppendToList(&args, shdTy);
        }

        if (!cfg.entryPoint.data || !cfg.entryPoint.count)
            cfg.entryPoint = UTF8STR("main");

        COL_AppendToList(&args, L"-E");
        wchar_t* entryPointW = GPU_ShaderCompiler::ToWideString(cfg.entryPoint, MEM_temp);
        COL_AppendToList(&args, entryPointW);

        if (cfg.gfxAPI == GPU_GfxAPIType_Vk)
        {
            COL_AppendToList(&args, L"-spirv");
            COL_AppendToList(&args, L"-fspv-target-env=vulkan1.3");
            if (useMeshShaderExt)
                COL_AppendToList(&args, L"-fspv-extension=SPV_EXT_mesh_shader");
        }

        DxcBuffer buffer =
        {
            .Ptr      = srcBlob->GetBufferPointer(),
            .Size     = srcBlob->GetBufferSize(),
            .Encoding = DXC_CP_ACP,
        };

        IDxcResult* result = nil;
        DEFER { if (result) { result->Release(); result = nil; } };

        IDxcBlob* output2 = nil;
        DEFER { if (output2) { output2->Release(); output2 = nil; } };

        HRESULT tempRes = 0;
        if (false
            || FAILED(
                GPU_ShaderCompiler::G_ThreadInternals.compiler->Compile(
                &buffer,
                args.data,
                args.count,
                GPU_ShaderCompiler::G_ThreadInternals.includeHandler,
                IID_PPV_ARGS(&result)
            )) // compile fn returned fail
            || !result // result is still null
            || FAILED(result->GetStatus(&tempRes)) // failed to get the result value of the result... (el stupido)
            || FAILED(tempRes) // the result of the result says failed...
            || FAILED(result->GetResult(&output2)) // failed to get output from result...
            || !output2
            || false)
        {
            if (result)
            {
                IDxcBlobEncoding* error = nil;
                DEFER { if (error) { error->Release(); error = nil; } };

                if (SUCCEEDED(result->GetErrorBuffer(&error)) && error)
                {
                    LOG_Err(SHDCMPL, "Shader compiler error: %", FMT(error->GetBufferPointer()));
                }
            }

            return false;
        }

        Slice_(u8) tempOutput =
        {
            .data  = (u8*)   output2->GetBufferPointer(),
            .count = (isize) output2->GetBufferSize(),
        };

        if (stage)
            *stage = GPU_ProgramStageByteCode
            {
                .stage      = cfg.stage,
                .allocator  = cfg.allocator,
                .code       = COL_CloneSlice(tempOutput, cfg.allocator),
                .entryPoint = STR_Clone(cfg.entryPoint, cfg.allocator),
            };

        // TODO: use spv-reflect here to parse the shader and extract the resources it uses
        // TODO: implement the next step for metal shaders (SPIRV -> MSL)

        return true;
    }
    #endif
}

void GPU_FreeProgramStage(GPU_ProgramStageByteCode* stage)
{
    #if !DX_SHADER_COMPILER
    {
        (void) stage;
        return;
    }
    #else
    {
        if (!stage)
            return;

        COL_DeleteSlice(&(stage->code),       stage->allocator);
        COL_DeleteSlice(&(stage->entryPoint), stage->allocator);
        *stage = { };
    }
    #endif
}
