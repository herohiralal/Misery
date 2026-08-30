#pragma once
#include <GPU_Dx12/GPU_Dx12.h>

#if GPU_DX12
EXTERN_C_BEGIN

D3D12MessageFunc GPU_GetDx12DebugCallback(void);
void GPU_LogDx12ResultOnFailure(HRESULT result, utf8str fnCall, SrcLoc loc);
void GPU_LogDx12ErrorBlobAndRelease(ID3DBlob* blob, utf8str objName, SrcLoc loc);
void GPU_SetDx12ObjDebugName(const GPU_Dx12Instance* renderer, ID3D12Object* obj, utf8str fmtStr, FMT_Args fmtArgs);

#define GPU_DX12_CHECKED_CALL(call) \
    GPU_LogDx12ResultOnFailure((call), UTF8STR(#call), SRC_LOC())

#define GPU_DX12_LOG_BLOB_AND_RELEASE(blob, objName) \
    GPU_LogDx12ErrorBlobAndRelease((blob), UTF8STR(#objName), SRC_LOC())

#define GPU_DX12_SET_OBJ_DEBUG_NAME(renderer, obj, fmt, ...) \
    GPU_SetDx12ObjDebugName((renderer), (obj), UTF8STR(fmt), FMTARGS(__VA_ARGS__))

DXGI_FORMAT GPU_BreakDx12TextureFormat(GPU_TextureFormat);
GPU_TextureFormat GPU_MakeDx12TextureFormat(DXGI_FORMAT);

GPU_Dx12DescriptorHeap* GPU_CreateDx12DescriptorHeap(ID3D12Device2* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 capacity, MEM_Allocator allocator);
void GPU_DestroyDx12DescriptorHeap(GPU_Dx12DescriptorHeap* heap);
GPU_Dx12DescriptorData GPU_AllocateFromDx12DescriptorHeap(GPU_Dx12DescriptorHeap* heap);
void GPU_DestroyDx12DescriptorData(GPU_Dx12DescriptorData* data);

EXTERN_C_END
#endif
