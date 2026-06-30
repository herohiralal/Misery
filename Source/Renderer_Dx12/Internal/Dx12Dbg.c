#include "Dx12Private.h"

#if REN_DX12

static void __stdcall REN_Dx12DebugCallback(
    D3D12_MESSAGE_CATEGORY category,
    D3D12_MESSAGE_SEVERITY severity,
    D3D12_MESSAGE_ID id,
    LPCSTR desc,
    void* ctx)
{
    (void) ctx;

    utf8str categoryStr = UTF8STR("[Unknown]");
    switch (category)
    {
        case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:   categoryStr = UTF8STR("[ApplicationDefined]");   break;
        case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:         categoryStr = UTF8STR("[Miscellaneous]");        break;
        case D3D12_MESSAGE_CATEGORY_INITIALIZATION:        categoryStr = UTF8STR("[Initialization]");       break;
        case D3D12_MESSAGE_CATEGORY_CLEANUP:               categoryStr = UTF8STR("[Cleanup]");              break;
        case D3D12_MESSAGE_CATEGORY_COMPILATION:           categoryStr = UTF8STR("[Compilation]");          break;
        case D3D12_MESSAGE_CATEGORY_STATE_CREATION:        categoryStr = UTF8STR("[StateCreation]");        break;
        case D3D12_MESSAGE_CATEGORY_STATE_SETTING:         categoryStr = UTF8STR("[StateSetting]");         break;
        case D3D12_MESSAGE_CATEGORY_STATE_GETTING:         categoryStr = UTF8STR("[StateGetting]");         break;
        case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION: categoryStr = UTF8STR("[ResourceManipulation]"); break;
        case D3D12_MESSAGE_CATEGORY_EXECUTION:             categoryStr = UTF8STR("[Execution]");            break;
        case D3D12_MESSAGE_CATEGORY_SHADER:                categoryStr = UTF8STR("[Shader]");               break;
        default: break;
    }

    utf8str msg = STR_AliasCStr(desc ? desc : "");
    if (false) { }
    else if (severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
        LOG_Ftl(DX12, "% % (message id: %).", FMT(categoryStr), FMT(msg), FMT((u32) id));
    else if (severity == D3D12_MESSAGE_SEVERITY_ERROR)
        LOG_Err(DX12, "% % (message id: %).", FMT(categoryStr), FMT(msg), FMT((u32) id));
    else if (severity == D3D12_MESSAGE_SEVERITY_WARNING)
        LOG_Wrn(DX12, "% % (message id: %).", FMT(categoryStr), FMT(msg), FMT((u32) id));
    else if (severity == D3D12_MESSAGE_SEVERITY_INFO)
        LOG_Inf(DX12, "% % (message id: %).", FMT(categoryStr), FMT(msg), FMT((u32) id));
    else
        LOG_Dbg(DX12, "% % (message id: %).", FMT(categoryStr), FMT(msg), FMT((u32) id));
}

D3D12MessageFunc REN_GetDx12DebugCallback(void) { return REN_Dx12DebugCallback; }

void REN_LogDx12ResultOnFailure(HRESULT result, utf8str fnCall, SrcLoc loc)
{
    if (SUCCEEDED(result)) return;

    utf8str message = UTF8STR("Unknown HRESULT.");
    switch (result)
    {
        case D3D12_ERROR_ADAPTER_NOT_FOUND: message = UTF8STR("\"The specified cached PSO was created on a different adapter and cannot be reused on the current adapter"); break;
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH: message = UTF8STR("\"The specified cached PSO was created on a different driver version and cannot be reused on the current adapter"); break;
        case DXGI_ERROR_INVALID_CALL: message = UTF8STR("\"The method call is invalid. For example, a method's parameter may not be a valid pointer"); break;
        case DXGI_ERROR_WAS_STILL_DRAWING: message = UTF8STR("\"The previous blit operation that is transferring information to or from this surface is incomplete"); break;
        case E_FAIL: message = UTF8STR("\"Attempted to create a device with the debug layer enabled and the layer is not installed"); break;
        case E_INVALIDARG: message = UTF8STR("\"An invalid parameter was passed to the returning function"); break;
        case E_OUTOFMEMORY: message = UTF8STR("\"Direct3D could not allocate sufficient memory to complete the call"); break;
        case E_NOTIMPL: message = UTF8STR("\"The method call isn't implemented with the passed parameter combination"); break;
        case S_FALSE: message = UTF8STR("\"Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context)"); break;
        default: message = UTF8STR("\"An unknown error occurred"); break;
    }

    LOG_Internal_AddEntry(&(LOG_Internal_Entry)
    {
        .lvl     = LOG_Lvl_Error,
        .cat[0]  = '-',
        .cat[1]  = '-',
        .cat[2]  = '-',
        .cat[3]  = 'D',
        .cat[4]  = 'X',
        .cat[5]  = '1',
        .cat[6]  = '2',
        .msg     = UTF8STR("ERROR: % from % (0x%)."),
        .fmtArgs = FMTARGS(FMT(message), FMT(fnCall), FMT((u32) result)),
        .loc     = loc,
    });

    MSR_ASSERT(false && "Dx12 error");
}

void REN_LogDx12ErrorBlobAndRelease(ID3DBlob* blob, utf8str objName, SrcLoc loc)
{
    if (!blob) return;

    utf8str blobMsg =
    {
        .data = (u8*) ID3D10Blob_GetBufferPointer(blob),
        .count = (isize) ID3D10Blob_GetBufferSize(blob)
    };

    if (blobMsg.data && blobMsg.data[blobMsg.count - 1] == '\0')
        blobMsg.count -= 1; // remove trailing null terminator

    LOG_Internal_AddEntry(&(LOG_Internal_Entry)
    {
        .lvl     = LOG_Lvl_Error,
        .cat[0]  = '-',
        .cat[1]  = '-',
        .cat[2]  = '-',
        .cat[3]  = 'D',
        .cat[4]  = 'X',
        .cat[5]  = '1',
        .cat[6]  = '2',
        .msg     = UTF8STR("[$]: $"),
        .fmtArgs = FMTARGS(FMT(objName), FMT(blobMsg)),
        .loc     = loc,
    });

    ID3D10Blob_Release(blob);
}

void REN_SetDx12ObjDebugName(const REN_Dx12Instance* renderer, ID3D12Object* obj, utf8str fmtStr, FMT_Args fmtArgs)
{
    if (!renderer || !(renderer->dbgController) || !obj) return;

    cstring utf8Name = FMT_CAPrintf_(MEM_temp, fmtStr, fmtArgs);
    if (!utf8Name) return;

    i32 wideNameCount = MultiByteToWideChar(CP_UTF8, 0, utf8Name, -1, nil, 0);
    if (wideNameCount <= 0) return;

    Slice_(u16) wideName = COL_NewSlice(u16, wideNameCount, true, MEM_temp);
    if (!wideName.data) return;

    i32 wideNameWritten = MultiByteToWideChar(CP_UTF8, 0, utf8Name, -1, (PWSTR) wideName.data, wideNameCount);
    if (wideNameWritten <= 0) return;

    REN_DX12_CHECKED_CALL(ID3D12Object_SetName(obj, (WCHAR*) wideName.data));
}

#endif
