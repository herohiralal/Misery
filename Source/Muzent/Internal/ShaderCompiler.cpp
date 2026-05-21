#include "Muzent/ShaderCompiler.h"

struct MZNT_Internal_ShaderCompiler
{
    PNSLR_DynamicLibrary lib;
    PNSLR_Allocator      allocator;
    IDxcUtils*           dxcUtils;
    IDxcCompiler3*       dxcCompiler;
    IDxcIncludeHandler*  dxcIncludeHandler;
};

MZNT_ShaderCompiler MZNT_CreateShaderCompiler(MZNT_ShaderCompilerConfiguration cfg, PNSLR_Allocator tempAllocator)
{
    #if PNSLR_WINDOWS
    utf8str libName = PNSLR_StringLiteral("dxcompiler.dll");
    #elif PNSLR_LINUX
    utf8str libName = PNSLR_StringLiteral("libdxcompiler.so");
    #else
    {
        libName = libName;
        PNSLR_LogW(PNSLR_StringLiteral("shader compiler unsupported on the current platform"), PNSLR_GET_LOC());
        return MZNT_ShaderCompiler { };
    }
    #endif

    #if PNSLR_WINDOWS || PNSLR_LINUX
    {
        if (!cfg.libSearchDir.path.count || !cfg.libSearchDir.path.data)
            return MZNT_ShaderCompiler { };

        PNSLR_Path library = PNSLR_GetPathForChildFile(cfg.libSearchDir, libName, tempAllocator);

        MZNT_Internal_ShaderCompiler* outputPtr = Panshilar::NewT<MZNT_Internal_ShaderCompiler>(cfg.allocator, PNSLR_GET_LOC());
        if (!outputPtr) return MZNT_ShaderCompiler { };

        MZNT_Internal_ShaderCompiler& output = *outputPtr;

        output.lib = PNSLR_LoadDynamicLibrary(library);
        if (!output.lib.handle)
        {
            PNSLR_LogEf(PNSLR_StringLiteral("could not load $"), PNSLR_FmtArgs(PNSLR_FmtString(library.path)), PNSLR_GET_LOC());
            Panshilar::DeleteT(outputPtr, cfg.allocator, PNSLR_GET_LOC());
            return MZNT_ShaderCompiler { };
        }

        DxcCreateInstanceProc createInstance = (DxcCreateInstanceProc) PNSLR_GetDynamicLibraryFunction(output.lib, PNSLR_StringLiteral("DxcCreateInstance"));
        if (!createInstance)
        {
            PNSLR_LogEf(PNSLR_StringLiteral("failed to load shader instance creation function from $"), PNSLR_FmtArgs(PNSLR_FmtString(library.path)), PNSLR_GET_LOC());
            PNSLR_UnloadDynamicLibrary(output.lib);
            Panshilar::DeleteT(outputPtr, cfg.allocator, PNSLR_GET_LOC());
            return MZNT_ShaderCompiler { };
        }

        if (!SUCCEEDED(createInstance(CLSID_DxcUtils, IID_PPV_ARGS(&output.dxcUtils))))
            FORCE_DBG_TRAP;

        if (!SUCCEEDED(createInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&output.dxcCompiler))))
            FORCE_DBG_TRAP;

        output.dxcUtils->CreateDefaultIncludeHandler(&output.dxcIncludeHandler);

        return MZNT_ShaderCompiler { outputPtr };
    }
    #endif
}

void MZNT_DestroyShaderCompiler(MZNT_ShaderCompiler compiler, PNSLR_Allocator tempAllocator)
{
    #if PNSLR_WINDOWS || PNSLR_LINUX
    {
        if (!compiler.handle) return;

        MZNT_Internal_ShaderCompiler& c = *(MZNT_Internal_ShaderCompiler*) compiler.handle;

        if (c.dxcIncludeHandler) c.dxcIncludeHandler->Release();
        if (c.dxcCompiler) c.dxcCompiler->Release();
        if (c.dxcUtils) c.dxcUtils->Release();
        if (c.lib.handle) PNSLR_UnloadDynamicLibrary(c.lib);

        Panshilar::DeleteT(&c, c.allocator, PNSLR_GET_LOC());
    }
    #endif
}

b8 CompileShader(MZNT_ShaderCompiler compiler, PNSLR_Path file, MZNT_ShaderByteCode* output, MZNT_ShaderCompilationOptions options)
{
    #if PNSLR_WINDOWS || PNSLR_LINUX
    {
        if (!compiler.handle)
            return false;

        if (!file.path.count || !file.path.data)
            return false;

        MZNT_Internal_ShaderCompiler& c = *(MZNT_Internal_ShaderCompiler*) compiler.handle;

        ArraySlice<u16> fileW = PNSLR_UTF16FromUTF8WindowsOnly(file.path, options.tempAllocator);

        IDxcBlobEncoding* srcBlob = nil;
        {
            u32 codePage = DXC_CP_ACP;
            if (FAILED(c.dxcUtils->LoadFile((WCHAR*) fileW.data, &codePage, &srcBlob)))
            {
                return false;
            }
        }

        ArraySlice<LPCWSTR> args = { };
        Panshilar::ResizeSliceT(&args, 30, true, options.tempAllocator, PNSLR_GET_LOC());
        u32 argsIt = 0;

        args.data[argsIt++] = (LPCWSTR) fileW.data; // must be the first one, this name will be shown in errors

        args.data[argsIt++] = L"-T";
        b8 useMeshShaderExt = false;
        switch (options.shaderType)
        {
            case MZNT_ShaderType_Compute:   args.data[argsIt++] = L"cs_6_0";                          break;
            case MZNT_ShaderType_Task:      args.data[argsIt++] = L"as_6_5"; useMeshShaderExt = true; break;
            case MZNT_ShaderType_Mesh:      args.data[argsIt++] = L"ms_6_5"; useMeshShaderExt = true; break;
            case MZNT_ShaderType_Vertex:    args.data[argsIt++] = L"vs_6_0";                          break;
            case MZNT_ShaderType_Fragment:  args.data[argsIt++] = L"ps_6_0";                          break;
            default:
            {
                PNSLR_LogE(PNSLR_StringLiteral("unknown shader type!"), PNSLR_GET_LOC());
                srcBlob->Release();
                return false;
            }
        }

        if (!options.entryPoint.data || !options.entryPoint.count)
            options.entryPoint = PNSLR_StringLiteral("main");

        args.data[argsIt++] = L"-E";
        args.data[argsIt++] = (const WCHAR*) PNSLR_UTF16FromUTF8WindowsOnly(options.entryPoint, options.tempAllocator).data;

        if (options.rendererType == MZNT_RendererType_Vulkan)
        {
            args.data[argsIt++] = L"-spirv";
            args.data[argsIt++] = L"-fspv-target-env=vulkan1.3";
            if (useMeshShaderExt) { args.data[argsIt++] = L"-fspv-extension=SPV_EXT_mesh_shader"; }
        }

        if (((i64) argsIt) >= args.count)
        {
            FORCE_DBG_TRAP;
        }

        DxcBuffer buffer = DxcBuffer { };
        {
            buffer.Ptr      = srcBlob->GetBufferPointer();
            buffer.Size     = srcBlob->GetBufferSize();
            buffer.Encoding = DXC_CP_ACP;
        }

        IDxcResult* result = nil;
        IDxcBlob* output2 = nil;
        HRESULT tempRes = 0;
        if (FAILED(c.dxcCompiler->Compile(&buffer, args.data, argsIt, c.dxcIncludeHandler, IID_PPV_ARGS(&result))) // compile fn returned fail
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
                if (SUCCEEDED(result->GetErrorBuffer(&error)) && error)
                {
                    cstring errorStr = (cstring) error->GetBufferPointer();
                    PNSLR_LogEf(PNSLR_StringLiteral("Shader compiler error: $"), PNSLR_FmtArgs(PNSLR_FmtCString(errorStr)), PNSLR_GET_LOC());
                }

                if (error) error->Release();
            }

            if (output2) output2->Release();
            if (result) result->Release();
            srcBlob->Release();
            return false;
        }

        PNSLR_ArraySlice(u8) tempOutput = { };
        tempOutput.data  = (u8*) output2->GetBufferPointer();
        tempOutput.count = (i64) output2->GetBufferSize();

        if (output)
        {
            output->type = options.shaderType;
            output->allocator = options.allocator;
            output->byteCode = PNSLR_CloneString(tempOutput, options.allocator);
            output->entryPoint = PNSLR_CloneString(options.entryPoint, options.allocator);
        }

        output2->Release();
        result->Release();
        srcBlob->Release();
        return true;
    }
    #else
    {
        PNSLR_LogE(PNSLR_StringLiteral("unsupported shader compiler platform!"), PNSLR_GET_LOC());
        return false;
    }
    #endif
}

void DestroyShaderByteCode(MZNT_ShaderByteCode* byteCode)
{
    if (!byteCode) return;
    PNSLR_FreeSlice(&(byteCode->byteCode), byteCode->allocator, PNSLR_GET_LOC(), nil);
    PNSLR_FreeString(byteCode->entryPoint, byteCode->allocator, PNSLR_GET_LOC(), nil);
    *byteCode = { };
}
