#ifndef MZNT_DX12_FNS_H
#define MZNT_DX12_FNS_H
#include "Muzent/__Prelude.h"
#include "Muzent/Renderer.h"
#include "Dx12Types.h"
EXTERN_C_BEGIN
#if defined(MZNT_IMPLEMENTATION) && MZNT_DX12

D3D12MessageFunc MZNT_Internal_GetDx12DebugCallback(void);
void MZNT_Internal_LogDx12ResultOnFailure(HRESULT result, utf8str fnCall, PNSLR_SourceCodeLocation loc);
void MZNT_Internal_LogErrorBlobAndRelease(ID3DBlob* blob, utf8str objName, PNSLR_SourceCodeLocation loc);
void MZNT_Internal_SetDx12ObjDebugName(const MZNT_DirectX12Renderer* renderer, ID3D12Object* obj, utf8str fmtStr, PNSLR_ArraySlice(PNSLR_PrimitiveFmtOptions) fmtArgs, PNSLR_Allocator tempAllocator);

#define MZNT_INTERNAL_DX12_CHECKED_CALL(call) \
    MZNT_Internal_LogDx12ResultOnFailure((call), PNSLR_StringLiteral(#call), PNSLR_GET_LOC())

#define MZNT_INTERNAL_DX12_LOG_BLOB_AND_RELEASE(blob) \
    MZNT_Internal_LogErrorBlobAndRelease((blob), PNSLR_StringLiteral(#blob), PNSLR_GET_LOC())

DXGI_FORMAT MZNT_Internal_BreakDx12TextureFormat(MZNT_TextureFormat fmt);
MZNT_TextureFormat MZNT_Internal_MakeDx12TextureFormat(DXGI_FORMAT fmt);

MZNT_DirectX12Renderer* MZNT_CreateRenderer_DirectX12(MZNT_RendererConfiguration config, PNSLR_Allocator tempAllocator);
b8 MZNT_WaitTillRendererIdle_DirectX12(const MZNT_DirectX12Renderer* renderer);
b8 MZNT_DestroyRenderer_DirectX12(MZNT_DirectX12Renderer* renderer, PNSLR_Allocator tempAllocator);

MZNT_DirectX12SwapChain* MZNT_CreateSwapChainFromWindow_DirectX12(MZNT_DirectX12Renderer* renderer, MZNT_WindowHandle windowHandle, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator);
b8 MZNT_ReconfigureSwapChain_DirectX12(MZNT_DirectX12SwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator);
b8 MZNT_DestroySwapChain_DirectX12(MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator);
MZNT_TextureFormat MZNT_GetSwapChainTextureFormat_DirectX12(const MZNT_DirectX12SwapChain* swapChain);
b8 MZNT_IterateSwapChain_DirectX12(MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator);
MZNT_DirectX12RendererCommandBuffer* MZNT_GetSwapChainCommandBuffer_DirectX12(const MZNT_DirectX12SwapChain* swapChain, u8* outImgIdx, PNSLR_Allocator tempAllocator);
b8 MZNT_PresentSwapChain_DirectX12(const MZNT_DirectX12SwapChain* swapChain, PNSLR_Allocator tempAllocator);

#endif
EXTERN_C_END
#endif
