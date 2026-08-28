#include "MtlPrivate.h"

#if GPU_MTL

void GPU_MtlCreateSwapChainFromWindow(GPU_SwapChain* outBaseSwapChain, GPU_Instance* baseRenderer, WND_Handle windowHandle, GPU_SwapChainCfg cfg)
{
	GPU_MtlInstance* renderer = GPU_ToMtlInstance(baseRenderer);
	if (!renderer)
		return;

	MSR_ASSERT(!!outBaseSwapChain && "outBaseSwapChain can't be null");
	outBaseSwapChain->base.type = GPU_GfxAPIType_Mtl;

	GPU_MtlSwapChain* output = GPU_ToMtlSwapChain(outBaseSwapChain);
	MSR_ASSERT(output && "output must not be null");

	output->renderer = renderer;
	output->window = windowHandle;
	output->vSync = cfg.vSync;
	output->allowCmdBuff = false;
	output->curFrame = 0;

	for (u32 i = 0; i < GPU_FRAMES_IN_FLIGHT; i++)
	{
		GPU_CmdBuffer* baseCmdBuffer = &(output->buffers.cmdBuffers[i]);
		baseCmdBuffer->base.type = GPU_GfxAPIType_Mtl;

		GPU_MtlCmdBuffer* cmdBuffer = GPU_ToMtlCmdBuffer(baseCmdBuffer);
		cmdBuffer->renderer = renderer;
		cmdBuffer->actual = nil;
	}
}

void GPU_MtlReconfigureSwapChain(GPU_SwapChain* baseSwapChain, GPU_SwapChainCfg cfg)
{
	GPU_MtlSwapChain* swapChain = GPU_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer))
		return;

	swapChain->vSync = cfg.vSync;
}

void GPU_MtlDestroySwapChain(GPU_SwapChain* baseSwapChain)
{
	GPU_MtlSwapChain* swapChain = GPU_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer))
		return;

	for (u32 i = 0; i < GPU_FRAMES_IN_FLIGHT; i++)
	{
		GPU_MtlCmdBuffer* cmdBuffer = GPU_ToMtlCmdBuffer(&(swapChain->buffers.cmdBuffers[i]));
		if (cmdBuffer->actual)
		{
			[cmdBuffer->actual release];
			cmdBuffer->actual = nil;
		}
	}

	swapChain->allowCmdBuff = false;
}

GPU_TextureFormat GPU_MtlGetSwapChainTextureFormat(GPU_SwapChain* baseSwapChain)
{
	GPU_MtlSwapChain* swapChain = GPU_ToMtlSwapChain(baseSwapChain);
	if (!swapChain) return GPU_TexFmt_Unknown;

	return GPU_TexFmt_B8G8R8A8_UNorm;
}

void GPU_MtlIterateSwapChain(GPU_SwapChain* baseSwapChain)
{
	GPU_MtlSwapChain* swapChain = GPU_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer))
		return;

	swapChain->allowCmdBuff = false;
	swapChain->curFrame = (swapChain->curFrame + 1) % GPU_FRAMES_IN_FLIGHT;
	swapChain->allowCmdBuff = true;
}

GPU_CmdBuffer* GPU_MtlGetSwapChainCommandBuffer(GPU_SwapChain* baseSwapChain, u8* outImgIdx)
{
	u8 outImgIdxThrowaway = 0;
	outImgIdx = outImgIdx ? outImgIdx : &outImgIdxThrowaway;
	*outImgIdx = U8_MAX;

	GPU_MtlSwapChain* swapChain = GPU_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !swapChain->allowCmdBuff)
		return nil;

	GPU_MtlCmdBuffer* cmdBuffer = GPU_ToMtlCmdBuffer(&(swapChain->buffers.cmdBuffers[swapChain->curFrame]));
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
	return GPU_FromMtlCmdBuffer(cmdBuffer);
}

void GPU_MtlPresentSwapChain(GPU_SwapChain* baseSwapChain)
{
	GPU_MtlSwapChain* swapChain = GPU_ToMtlSwapChain(baseSwapChain);
	if (!swapChain || !(swapChain->renderer) || !swapChain->allowCmdBuff)
		return;

	GPU_MtlCmdBuffer* cmdBuffer = GPU_ToMtlCmdBuffer(&(swapChain->buffers.cmdBuffers[swapChain->curFrame]));
	if (!cmdBuffer->actual)
		return;

	[cmdBuffer->actual commit];
	[cmdBuffer->actual waitUntilCompleted];

	[cmdBuffer->actual release];
	cmdBuffer->actual = nil;
}

#endif
