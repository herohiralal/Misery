#pragma once
#include "MtlTypes.h"

#if GPU_MTL
EXTERN_C_BEGIN

void GPU_MtlCreate(GPU_Instance*, GPU_InstanceCfg);
void GPU_MtlWaitTillIdle(GPU_Instance*);
void GPU_MtlDestroy(GPU_Instance*);

void GPU_MtlCreateSwapChainFromWindow(GPU_SwapChain*, GPU_Instance*, WND_Handle, GPU_SwapChainCfg);
void GPU_MtlReconfigureSwapChain(GPU_SwapChain*, GPU_SwapChainCfg);
void GPU_MtlDestroySwapChain(GPU_SwapChain*);
GPU_TextureFormat GPU_MtlGetSwapChainTextureFormat(GPU_SwapChain*);
void GPU_MtlIterateSwapChain(GPU_SwapChain*);
GPU_CmdBuffer* GPU_MtlGetSwapChainCommandBuffer(GPU_SwapChain*, u8* outImgIdx);
void GPU_MtlPresentSwapChain(GPU_SwapChain*);

EXTERN_C_END
#endif
