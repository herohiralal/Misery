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
GPU_SwapChainFrameContext GPU_Dx12BeginSwapChainFrame(GPU_SwapChain*);
void GPU_Dx12EndSwapChainFrame(GPU_SwapChain*);

void GPU_Dx12NewBuffer(GPU_Buffer*, GPU_Instance*, GPU_BufferCfg);
void GPU_Dx12DeleteBuffer(GPU_Buffer*);

void GPU_Dx12NewTexture(GPU_Texture*, GPU_Instance*, GPU_TextureCfg);
void GPU_Dx12DeleteTexture(GPU_Texture*);

void GPU_Dx12CmdBuffBegin(GPU_CmdBuffer*);
void GPU_Dx12CmdBuffEnd(GPU_CmdBuffer*);
void GPU_Dx12CmdBuffBarrier(GPU_CmdBuffer*, GPU_BarrierCfg);

EXTERN_C_END
#endif
