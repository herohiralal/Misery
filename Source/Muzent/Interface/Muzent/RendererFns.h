#ifndef MZNT_RENDERER_H // =========================================================
#define MZNT_RENDERER_H
#include "__Prelude.h"
#include "RendererObjects.h"
EXTERN_C_BEGIN

/**
 * Configuration structure for renderer creation.
 */
typedef struct MZNT_RendererConfiguration
{
    MZNT_RendererType type;
    MZNT_AppHandle    appHandle;
    PNSLR_Allocator   allocator;
    utf8str           appName;
} MZNT_RendererConfiguration;

/**
 * MAIN_THREAD
 * Creates a renderer instance based on the provided configuration.
 */
MZNT_Renderer* MZNT_CreateRenderer(MZNT_RendererConfiguration config, PNSLR_Allocator tempAllocator);

/**
 * MAIN_THREAD
 * Destroys the given renderer instance and frees associated resources.
 */
b8 MZNT_DestroyRenderer(MZNT_Renderer* renderer, PNSLR_Allocator tempAllocator);

/**
 * MAIN_THREAD
 * Halts the thread until all the work that's already submitted to the renderer is done.
 * Return value is insignificant.
 *
 * Warning! - it won't necessarily account for work that gets submitted after this on other threads.
 */
b8 MZNT_WaitTillRendererIdle(MZNT_Renderer* renderer);

/**
 * Configuration structure for swap-chain.
 */
typedef struct MZNT_SwapChainConfiguration
{
    u16     width;
    u16     height;
    b8      vSync;
    u8      framesInFlight;
    utf8str objectName;
} MZNT_SwapChainConfiguration;

/**
 * MAIN_THREAD
 * Create a swap-chain for the given window, with the given renderer, for a custom number of frames in flight.
 * If width/height are uninitialised, they will be derived from the window.
 */
MZNT_SwapChain* MZNT_CreateSwapChainFromWindow(MZNT_Renderer* renderer, MZNT_WindowHandle windowHandle, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator);

/**
 * MAIN_THREAD
 * Reconfigure a swap-chain with new properties.
 * All configuration values must be initialised appropriately.
 */
b8 MZNT_ReconfigureSwapChain(MZNT_SwapChain* swapChain, MZNT_SwapChainConfiguration cfg, PNSLR_Allocator tempAllocator);

/**
 * MAIN_THREAD
 * Destroy the swap-chain, freeing up associated resources.
 */
b8 MZNT_DestroySwapChain(MZNT_SwapChain* swapChain, PNSLR_Allocator tempAllocator);

/**
 * MAIN_THREAD
 * Get the texture format of the swap-chain.
 */
MZNT_TextureFormat MZNT_GetSwapChainTextureFormat(MZNT_SwapChain* swapChain);

/**
 * MAIN_THREAD
 * Acquire the next image in the swap-chain. Will block if there is something to block
 * over (such as the next image being presented, or the new command buffer not done
 * processing yet from the last time it was used).
 */
b8 MZNT_IterateSwapChain(MZNT_SwapChain* swapChain, PNSLR_Allocator tempAllocator);

/**
 * RENDER_THREAD
 * Acquire the command buffer for the current swap-chain image.
 * Optionally, also acquire the index of the current image in the frames-in-flight buffer.
 * This index can be used to determine external per-frame-in-flight resource usage.
 */
MZNT_RendererCommandBuffer* MZNT_GetSwapChainCommandBuffer(
    MZNT_SwapChain* swapChain,
    u8* outImgIdx,
    PNSLR_Allocator tempAllocator);

/**
 * RENDER_THREAD
 * Wrap up recording commands for the current frame, for the given swap-chain and
 * submit the current image for presenting.
 */
b8 MZNT_PresentSwapChain(MZNT_SwapChain* swapChain, PNSLR_Allocator tempAllocator);

EXTERN_C_END
#endif // MZNT_RENDERER_H ==========================================================
