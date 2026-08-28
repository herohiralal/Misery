#pragma once
#include "Dx12Types.h"

#if GPU_DX12
EXTERN_C_BEGIN

void GPU_Dx12Create(GPU_Instance*, GPU_InstanceCfg);
void GPU_Dx12WaitTillIdle(GPU_Instance*);
void GPU_Dx12Destroy(GPU_Instance*);

void GPU_Dx12CreateSwapChainFromWindow(GPU_SwapChain*, GPU_Instance*, WND_Handle, GPU_SwapChainCfg);
void GPU_Dx12ReconfigureSwapChain(GPU_SwapChain*, GPU_SwapChainCfg);
void GPU_Dx12DestroySwapChain(GPU_SwapChain*);
GPU_TextureFormat GPU_Dx12GetSwapChainTextureFormat(GPU_SwapChain*);
void GPU_Dx12IterateSwapChain(GPU_SwapChain*);
GPU_CmdBuffer* GPU_Dx12GetSwapChainCommandBuffer(GPU_SwapChain*, u8* outImgIdx);
void GPU_Dx12PresentSwapChain(GPU_SwapChain*);

EXTERN_C_END
#endif
