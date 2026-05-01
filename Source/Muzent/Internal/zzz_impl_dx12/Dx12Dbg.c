#include "Dx12Fns.h"
#if MZNT_DX12

static void __stdcall MZNT_Internal_Dx12DebugCallback(
    D3D12_MESSAGE_CATEGORY category,
    D3D12_MESSAGE_SEVERITY severity,
    D3D12_MESSAGE_ID id,
    LPCSTR desc,
    void* ctx)
{
    utf8str categoryStr = {0};
    switch (category)
    {
        case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:   categoryStr = PNSLR_StringLiteral("[ApplicationDefined]");   break;
        case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:         categoryStr = PNSLR_StringLiteral("[Miscellaneous]");        break;
        case D3D12_MESSAGE_CATEGORY_INITIALIZATION:        categoryStr = PNSLR_StringLiteral("[Initialization]");       break;
        case D3D12_MESSAGE_CATEGORY_CLEANUP:               categoryStr = PNSLR_StringLiteral("[Cleanup]");              break;
        case D3D12_MESSAGE_CATEGORY_COMPILATION:           categoryStr = PNSLR_StringLiteral("[Compilation]");          break;
        case D3D12_MESSAGE_CATEGORY_STATE_CREATION:        categoryStr = PNSLR_StringLiteral("[StateCreation]");        break;
        case D3D12_MESSAGE_CATEGORY_STATE_SETTING:         categoryStr = PNSLR_StringLiteral("[StateSetting]");         break;
        case D3D12_MESSAGE_CATEGORY_STATE_GETTING:         categoryStr = PNSLR_StringLiteral("[StateGetting]");         break;
        case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION: categoryStr = PNSLR_StringLiteral("[ResourceManipulation]"); break;
        case D3D12_MESSAGE_CATEGORY_EXECUTION:             categoryStr = PNSLR_StringLiteral("[Execution]");            break;
        case D3D12_MESSAGE_CATEGORY_SHADER:                categoryStr = PNSLR_StringLiteral("[Shader]");               break;
        default:                                           categoryStr = PNSLR_StringLiteral("[Unknown]");              break;
    }

    PNSLR_LoggerLevel lvl = 0;
    switch (severity)
    {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION: lvl = PNSLR_LoggerLevel_Critical; break;
        case D3D12_MESSAGE_SEVERITY_ERROR:      lvl = PNSLR_LoggerLevel_Error;    break;
        case D3D12_MESSAGE_SEVERITY_WARNING:    lvl = PNSLR_LoggerLevel_Warn;     break;
        case D3D12_MESSAGE_SEVERITY_INFO:       lvl = PNSLR_LoggerLevel_Info;     break;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:    lvl = PNSLR_LoggerLevel_Debug;    break;
        default:                                lvl = PNSLR_LoggerLevel_Error;    break;
    }

    PNSLR_Logf(lvl, PNSLR_StringLiteral("D3D12 INFO QUEUE: $ $ (message id - $)"),
        PNSLR_FmtArgs(
            PNSLR_FmtCString((cstring) desc),
            PNSLR_FmtString(categoryStr),
            PNSLR_FmtU32((u32) id, 0)
        ),
        PNSLR_GET_LOC()
    );
}

D3D12MessageFunc MZNT_Internal_GetDx12DebugCallback(void) { return MZNT_Internal_Dx12DebugCallback; }

void MZNT_Internal_LogDx12ResultOnFailure(HRESULT result, utf8str fnCall, PNSLR_SourceCodeLocation loc)
{
    if (SUCCEEDED(result)) return;
    utf8str message = {0};
    switch (result)
    {
        case D3D12_ERROR_ADAPTER_NOT_FOUND: message = PNSLR_StringLiteral("\"The specified cached PSO was created on a different adapter and cannot be reused on the current adapter.\""); break;
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH: message = PNSLR_StringLiteral("\"The specified cached PSO was created on a different driver version and cannot be reused on the current adapter.\""); break;
        case DXGI_ERROR_INVALID_CALL: message = PNSLR_StringLiteral("\"The method call is invalid. For example, a method's parameter may not be a valid pointer.\""); break;
        case DXGI_ERROR_WAS_STILL_DRAWING: message = PNSLR_StringLiteral("\"The previous blit operation that is transferring information to or from this surface is incomplete.\""); break;
        case E_FAIL: message = PNSLR_StringLiteral("\"Attempted to create a device with the debug layer enabled and the layer is not installed.\""); break;
        case E_INVALIDARG: message = PNSLR_StringLiteral("\"An invalid parameter was passed to the returning function.\""); break;
        case E_OUTOFMEMORY: message = PNSLR_StringLiteral("\"Direct3D could not allocate sufficient memory to complete the call.\""); break;
        case E_NOTIMPL: message = PNSLR_StringLiteral("\"The method call isn't implemented with the passed parameter combination.\""); break;
        case S_FALSE: message = PNSLR_StringLiteral("\"Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context).\""); break;
        default: message = PNSLR_StringLiteral("\"An unknown error occurred.\""); break;
    }

    if (result != S_OK)
    {
        PNSLR_LogEf(PNSLR_StringLiteral("DirectX 12 error: $ from $."),
                    PNSLR_FmtArgs(
                        PNSLR_FmtString(message),
                        PNSLR_FmtString(fnCall)
                    ),
                    loc);

        FORCE_DBG_TRAP;
    }
}

void MZNT_Internal_LogErrorBlobAndRelease(ID3DBlob* blob, utf8str objName, PNSLR_SourceCodeLocation loc)
{
    if (!blob) return;

    utf8str blobMsg = {0};
    blobMsg.data  = (u8*) ID3D10Blob_GetBufferPointer(blob);
    blobMsg.count = (i64) ID3D10Blob_GetBufferSize(blob);

    if (blobMsg.data && blobMsg.data[blobMsg.count - 1] == '\0')
    {
        // trim null terminator from message for cleaner logging
        blobMsg.count -= 1;
    }

    PNSLR_LogEf(PNSLR_StringLiteral("[$]: $"),
                PNSLR_FmtArgs(
                    PNSLR_FmtString(objName),
                    PNSLR_FmtString(blobMsg)
                ),
                loc);

    ID3D10Blob_Release(blob);
}

void MZNT_Internal_SetDx12ObjDebugName(const MZNT_DirectX12Renderer* renderer, ID3D12Object* obj, utf8str fmtStr, PNSLR_ArraySlice(PNSLR_PrimitiveFmtOptions) fmtArgs, PNSLR_Allocator tempAllocator)
{
    if (!renderer->dbgController) return;

    PNSLR_ArraySlice(u16) name = PNSLR_FormatUTF16StringWindowsOnly(fmtStr, fmtArgs, tempAllocator);
    MZNT_INTERNAL_DX12_CHECKED_CALL(ID3D12Object_SetName(obj, (WCHAR*) name.data));
}

#endif
