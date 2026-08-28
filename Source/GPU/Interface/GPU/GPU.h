#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>
#include <GPU_Base/GPU_Base.h>
#include <GPU_ShaderCompiler/GPU_ShaderCompiler.h>

EXTERN_C_BEGIN

/**
 * MAIN_THREAD
 * Creates a renderer instance based on the provided configuration.
 */
void GPU_Create(GPU_Instance* outInstance, GPU_InstanceCfg cfg);

/**
 * MAIN_THREAD
 * Destroys the given renderer instance and frees associated resources.
 */
void GPU_Destroy(GPU_Instance* renderer);

/**
 * MAIN_THREAD
 * Halts the thread until all the work that's already submitted to the renderer is done.
 * Return value is insignificant.
 *
 * Warning! - it won't necessarily account for work that gets submitted after this on other threads.
 */
void GPU_WaitTillIdle(GPU_Instance* renderer);

/**
 * MAIN_THREAD
 * Create a swap-chain for the given window, with the given renderer, for a custom number of frames in flight.
 * If width/height are uninitialised, they will be derived from the window.
 */
void GPU_CreateSwapChainFromWindow(GPU_SwapChain* outSwapChain, GPU_Instance* renderer, WND_Handle windowHandle, GPU_SwapChainCfg cfg);

/**
 * MAIN_THREAD
 * Reconfigure a swap-chain with new properties.
 * All configuration values must be initialised appropriately.
 */
void GPU_ReconfigureSwapChain(GPU_SwapChain* swapChain, GPU_SwapChainCfg cfg);

/**
 * MAIN_THREAD
 * Destroy the swap-chain, freeing up associated resources.
 */
void GPU_DestroySwapChain(GPU_SwapChain* swapChain);

/**
 * MAIN_THREAD
 * Get the texture format of the swap-chain.
 */
GPU_TextureFormat GPU_GetSwapChainTextureFormat(GPU_SwapChain* swapChain);

/**
 * MAIN_THREAD
 * Acquire the next image in the swap-chain. Will block if there is something to block
 * over (such as the next image being presented, or the new command buffer not done
 * processing yet from the last time it was used).
 */
void GPU_IterateSwapChain(GPU_SwapChain* swapChain);

/**
 * RENDER_THREAD
 * Acquire the command buffer for the current swap-chain image.
 * Optionally, also acquire the index of the current image in the frames-in-flight buffer.
 * This index can be used to determine external per-frame-in-flight resource usage.
 */
GPU_CmdBuffer* GPU_GetSwapChainCommandBuffer(GPU_SwapChain* swapChain, u8* outImgIdx);

/**
 * RENDER_THREAD
 * Wrap up recording commands for the current frame, for the given swap-chain and
 * submit the current image for presenting.
 */
void GPU_PresentSwapChain(GPU_SwapChain* swapChain);

EXTERN_C_END
