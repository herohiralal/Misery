#include "MtlPrivate.h"

MSR_SUPPRESS_WARN
#if REN_MTL

void REN_MtlCreateSwapChainFromWindow(REN_SwapChain* s, REN_Instance* i, WND_Handle h, REN_SwapChainCfg c) { }
void REN_MtlReconfigureSwapChain(REN_SwapChain* s, REN_SwapChainCfg c) { }
void REN_MtlDestroySwapChain(REN_SwapChain* s) { }
REN_TextureFormat REN_MtlGetSwapChainTextureFormat(REN_SwapChain* s) { return REN_TexFmt_Unknown; }
void REN_MtlIterateSwapChain(REN_SwapChain* s) { }
REN_CmdBuffer* REN_MtlGetSwapChainCommandBuffer(REN_SwapChain* s, u8* outImgIdx) { return nil; }
void REN_MtlPresentSwapChain(REN_SwapChain* s) { }

#endif
MSR_UNSUPPRESS_WARN
