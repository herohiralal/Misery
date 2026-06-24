#pragma once
#include "MtlTypes.h"

#if REN_MTL
EXTERN_C_BEGIN

void REN_MtlCreateRenderer(REN_Instance*, REN_InstanceCfg);
void REN_MtlWaitTillRendererIdle(REN_Instance*);
void REN_MtlDestroyRenderer(REN_Instance*);

void REN_MtlCreateSwapChainFromWindow(REN_SwapChain*, REN_Instance*, WND_Handle, REN_SwapChainCfg);
void REN_MtlReconfigureSwapChain(REN_SwapChain*, REN_SwapChainCfg);
void REN_MtlDestroySwapChain(REN_SwapChain*);
REN_TextureFormat REN_MtlGetSwapChainTextureFormat(REN_SwapChain*);
void REN_MtlIterateSwapChain(REN_SwapChain*);
REN_CmdBuffer* REN_MtlGetSwapChainCommandBuffer(REN_SwapChain*, u8* outImgIdx);
void REN_MtlPresentSwapChain(REN_SwapChain*);

EXTERN_C_END
#endif
