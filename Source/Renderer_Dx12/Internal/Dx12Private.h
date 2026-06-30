#pragma once
#include <Renderer_Dx12/Renderer_Dx12.h>

#if REN_DX12
EXTERN_C_BEGIN

D3D12MessageFunc REN_GetDx12DebugCallback(void);
void REN_LogDx12ResultOnFailure(HRESULT result, utf8str fnCall, SrcLoc loc);
void REN_LogDx12ErrorBlobAndRelease(ID3DBlob* blob, utf8str objName, SrcLoc loc);
void REN_SetDx12ObjDebugName(const REN_Dx12Instance* renderer, ID3D12Object* obj, utf8str fmtStr, FMT_Args fmtArgs);

#define REN_DX12_CHECKED_CALL(call) \
    REN_LogDx12ResultOnFailure((call), UTF8STR(#call), SRC_LOC())

#define REN_DX12_LOG_BLOB_AND_RELEASE(blob, objName) \
    REN_LogDx12ErrorBlobAndRelease((blob), UTF8STR(#objName), SRC_LOC())

#define REN_DX12_SET_OBJ_DEBUG_NAME(renderer, obj, fmt, ...) \
    REN_SetDx12ObjDebugName((renderer), (obj), UTF8STR(fmt), FMTARGS(__VA_ARGS__))

DXGI_FORMAT REN_BreakDx12TextureFormat(REN_TextureFormat);
REN_TextureFormat REN_MakeDx12TextureFormat(DXGI_FORMAT);

EXTERN_C_END
#endif
