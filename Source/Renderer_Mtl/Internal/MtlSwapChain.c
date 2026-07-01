#include "MtlPrivate.h"

MSR_SUPPRESS_WARN
#if REN_MTL

void REN_MtlCreateSwapChainFromWindow(REN_SwapChain* outBaseSwapChain, REN_Instance* baseRenderer, WND_Handle windowHandle, REN_SwapChainCfg cfg)
{
	REN_MtlInstance* renderer = REN_ToMtlInstance(baseRenderer);
	if (!renderer)
		return;

	MSR_ASSERT(!!outBaseSwapChain && "outBaseSwapChain can't be null");
	outBaseSwapChain->base.type = REN_GfxAPIType_Mtl;

	REN_MtlSwapChain* output = REN_ToMtlSwapChain(outBaseSwapChain);
	MSR_ASSERT(output && "output must not be null");

	output->renderer = renderer;
	output->window = windowHandle;
	output->vSync = cfg.vSync;
	output->allowCmdBuff = false;
	output->curFrame = 0;

	for (u32 i = 0; i < REN_FRAMES_IN_FLIGHT; i++)
	{
		REN_CmdBuffer* baseCmdBuffer = &(output->buffers.cmdBuffers[i]);
		baseCmdBuffer->base.type = REN_GfxAPIType_Mtl;

		REN_MtlCmdBuffer* cmdBuffer = REN_ToMtlCmdBuffer(baseCmdBuffer);
		cmdBuffer->renderer = renderer;
		cmdBuffer->actual = nil;
	}
}

void REN_MtlReconfigureSwapChain(REN_SwapChain* baseSwapChain, REN_SwapChainCfg cfg)
{
	REN_MtlSwapChain* swapChain = REN_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer))
		return;

	swapChain->vSync = cfg.vSync;
}

void REN_MtlDestroySwapChain(REN_SwapChain* baseSwapChain)
{
	REN_MtlSwapChain* swapChain = REN_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer))
		return;

	for (u32 i = 0; i < REN_FRAMES_IN_FLIGHT; i++)
	{
		REN_MtlCmdBuffer* cmdBuffer = REN_ToMtlCmdBuffer(&(swapChain->buffers.cmdBuffers[i]));
		if (cmdBuffer->actual)
		{
			[cmdBuffer->actual release];
			cmdBuffer->actual = nil;
		}
	}

	swapChain->allowCmdBuff = false;
}

REN_TextureFormat REN_MtlGetSwapChainTextureFormat(REN_SwapChain* baseSwapChain)
{
	REN_MtlSwapChain* swapChain = REN_ToMtlSwapChain(baseSwapChain);
	if (!swapChain) return REN_TexFmt_Unknown;

	return REN_TexFmt_B8G8R8A8_UNorm;
}

void REN_MtlIterateSwapChain(REN_SwapChain* baseSwapChain)
{
	REN_MtlSwapChain* swapChain = REN_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer))
		return;

	swapChain->allowCmdBuff = false;
	swapChain->curFrame = (swapChain->curFrame + 1) % REN_FRAMES_IN_FLIGHT;
	swapChain->allowCmdBuff = true;
}

REN_CmdBuffer* REN_MtlGetSwapChainCommandBuffer(REN_SwapChain* baseSwapChain, u8* outImgIdx)
{
	u8 outImgIdxThrowaway = 0;
	outImgIdx = outImgIdx ? outImgIdx : &outImgIdxThrowaway;
	*outImgIdx = U8_MAX;

	REN_MtlSwapChain* swapChain = REN_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !swapChain->allowCmdBuff)
		return nil;

	REN_MtlCmdBuffer* cmdBuffer = REN_ToMtlCmdBuffer(&(swapChain->buffers.cmdBuffers[swapChain->curFrame]));
	if (cmdBuffer->actual)
	{
		[cmdBuffer->actual release];
		cmdBuffer->actual = nil;
	}

	cmdBuffer->actual = [swapChain->renderer->gfxQueue commandBuffer];
	if (!cmdBuffer->actual)
		return nil;

	[cmdBuffer->actual retain];

	*outImgIdx = (u8) swapChain->curFrame;
	return REN_FromMtlCmdBuffer(cmdBuffer);
}

void REN_MtlPresentSwapChain(REN_SwapChain* baseSwapChain)
{
	REN_MtlSwapChain* swapChain = REN_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer) || !swapChain->allowCmdBuff)
		return;

	REN_MtlCmdBuffer* cmdBuffer = REN_ToMtlCmdBuffer(&(swapChain->buffers.cmdBuffers[swapChain->curFrame]));
	if (!cmdBuffer->actual)
		return;

	[cmdBuffer->actual commit];
	[cmdBuffer->actual waitUntilCompleted];

	[cmdBuffer->actual release];
	cmdBuffer->actual = nil;
}

#endif
MSR_UNSUPPRESS_WARN
