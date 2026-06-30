#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>
#include <Renderer_Base/Renderer_Base.h>

EXTERN_C_BEGIN

/**
 * MAIN_THREAD
 * Creates a renderer instance based on the provided configuration.
 */
void REN_Create(REN_Instance* outInstance, REN_InstanceCfg cfg);

/**
 * MAIN_THREAD
 * Destroys the given renderer instance and frees associated resources.
 */
void REN_Destroy(REN_Instance* renderer);

/**
 * MAIN_THREAD
 * Halts the thread until all the work that's already submitted to the renderer is done.
 * Return value is insignificant.
 *
 * Warning! - it won't necessarily account for work that gets submitted after this on other threads.
 */
void REN_WaitTillRendererIdle(REN_Instance* renderer);

/**
 * MAIN_THREAD
 * Create a swap-chain for the given window, with the given renderer, for a custom number of frames in flight.
 * If width/height are uninitialised, they will be derived from the window.
 */
void REN_CreateSwapChainFromWindow(REN_SwapChain* outSwapChain, REN_Instance* renderer, WND_Handle windowHandle, REN_SwapChainCfg cfg);

/**
 * MAIN_THREAD
 * Reconfigure a swap-chain with new properties.
 * All configuration values must be initialised appropriately.
 */
void REN_ReconfigureSwapChain(REN_SwapChain* swapChain, REN_SwapChainCfg cfg);

/**
 * MAIN_THREAD
 * Destroy the swap-chain, freeing up associated resources.
 */
void REN_DestroySwapChain(REN_SwapChain* swapChain);

/**
 * MAIN_THREAD
 * Get the texture format of the swap-chain.
 */
REN_TextureFormat REN_GetSwapChainTextureFormat(REN_SwapChain* swapChain);

/**
 * MAIN_THREAD
 * Acquire the next image in the swap-chain. Will block if there is something to block
 * over (such as the next image being presented, or the new command buffer not done
 * processing yet from the last time it was used).
 */
void REN_IterateSwapChain(REN_SwapChain* swapChain);

/**
 * RENDER_THREAD
 * Acquire the command buffer for the current swap-chain image.
 * Optionally, also acquire the index of the current image in the frames-in-flight buffer.
 * This index can be used to determine external per-frame-in-flight resource usage.
 */
REN_CmdBuffer* REN_GetSwapChainCommandBuffer(REN_SwapChain* swapChain, u8* outImgIdx);

/**
 * RENDER_THREAD
 * Wrap up recording commands for the current frame, for the given swap-chain and
 * submit the current image for presenting.
 */
void REN_PresentSwapChain(REN_SwapChain* swapChain);

EXTERN_C_END
