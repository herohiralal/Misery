#pragma once
#include "Dx12Types.h"

#if REN_DX12
EXTERN_C_BEGIN

void REN_Dx12Create(REN_Instance*, REN_InstanceCfg);
void REN_Dx12WaitTillRendererIdle(REN_Instance*);
void REN_Dx12Destroy(REN_Instance*);

void REN_Dx12CreateSwapChainFromWindow(REN_SwapChain*, REN_Instance*, WND_Handle, REN_SwapChainCfg);
void REN_Dx12ReconfigureSwapChain(REN_SwapChain*, REN_SwapChainCfg);
void REN_Dx12DestroySwapChain(REN_SwapChain*);
REN_TextureFormat REN_Dx12GetSwapChainTextureFormat(REN_SwapChain*);
void REN_Dx12IterateSwapChain(REN_SwapChain*);
REN_CmdBuffer* REN_Dx12GetSwapChainCommandBuffer(REN_SwapChain*, u8* outImgIdx);
void REN_Dx12PresentSwapChain(REN_SwapChain*);

EXTERN_C_END
#endif
