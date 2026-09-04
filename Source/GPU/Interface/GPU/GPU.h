#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>
#include <GPU_Base/GPU_Base.h>
#include <GPU_ShaderCompiler/GPU_ShaderCompiler.h>

EXTERN_C_BEGIN

// Entry point =================================================================================================================

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

// Swap-chain ==================================================================================================================

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
 * Begin rendering a given frame of the swap-chain.
 * Returns the context required to record commands for the current frame, and to present the current image.
 */
GPU_SwapChainFrameContext GPU_BeginSwapChainFrame(GPU_SwapChain* swapChain);

/**
 * RENDER_THREAD
 * End rendering a given frame of the swap-chain, and present the current image to the screen.
 * Internally, submits the command buffer for execution, and signals the swap-chain to present the current image.
 * Note that this requires the command buffer to be in an "executable" state.
 */
void GPU_EndSwapChainFrame(GPU_SwapChain* swapChain);

// Buffers =====================================================================================================================

/**
 * THREAD_SAFE
 * Create a buffer resource with the given configuration.
 */
void GPU_NewBuffer(GPU_Buffer* outBuffer, GPU_Instance* renderer, GPU_BufferCfg cfg);

/**
 * THREAD_SAFE
 * Destroy the given buffer resource, freeing up associated resources.
 */
void GPU_DeleteBuffer(GPU_Buffer* buffer);

// Textures ====================================================================================================================

/**
 * THREAD_SAFE
 * Create a texture resource with the given configuration.
 */
void GPU_NewTexture(GPU_Texture* outTexture, GPU_Instance* renderer, GPU_TextureCfg cfg);

/**
 * THREAD_SAFE
 * Destroy the given texture resource, freeing up associated resources.
 */
void GPU_DeleteTexture(GPU_Texture* texture);

// Programs ====================================================================================================================

/**
 * THREAD_SAFE
 * Create a program stage with the given configuration.
 */
void GPU_NewProgramStage(GPU_ProgramStage* outStage, GPU_Instance* renderer, GPU_ProgramStageByteCode bc);

/**
 * THREAD_SAFE
 * Destroy the given program stage, freeing up associated resources.
 */
void GPU_DeleteProgramStage(GPU_ProgramStage* stage);

/**
 * THREAD_SAFE
 * Create a program arguments layout object with the given configuration.
 */
void GPU_NewProgramArgsLayout(GPU_ProgramArgsLayout* outArgs, GPU_Instance* renderer, GPU_ProgramArgsLayoutCfg cfg);

/**
 * THREAD_SAFE
 * Destroy the given program arguments layout object, freeing up associated resources.
 */
void GPU_DeleteProgramArgsLayout(GPU_ProgramArgsLayout* args);

/**
 * THREAD_SAFE
 * Create a program with the given configuration.
 */
void GPU_NewProgram(GPU_Program* outProgram, GPU_Instance* renderer, GPU_ProgramCfg cfg);

/**
 * THREAD_SAFE
 * Destroy the given program, freeing up associated resources.
 */
void GPU_DeleteProgram(GPU_Program* program);

// Program arguments buffers ===================================================================================================

/**
 * THREAD_SAFE
 * Create a program arguments buffer with the given configuration.
 */
void GPU_NewProgramArgsBuffer(GPU_ProgramArgsBuffer* outArgsBuffer, GPU_Instance* renderer, GPU_ProgramArgsBufferCfg cfg);

/**
 * THREAD_SAFE
 * Destroy the given program arguments buffer, freeing up associated resources.
 */
void GPU_DeleteProgramArgsBuffer(GPU_ProgramArgsBuffer* argsBuffer);

/**
 * THREAD_SAFE
 * Update the given program arguments buffer with the provided bindings.
 * This will update the buffer with the new bindings, and mark it as dirty so that it can be
 * re-uploaded to the GPU.
 */
void GPU_UpdateProgramArgsBuffer(GPU_ProgramArgsBuffer* argsBuffer, Slice_(GPU_ProgramArgBindingCfg) bindings);

// Command-buffers =============================================================================================================

/**
 * OWNED_THREAD
 * Set the command buffer to "recording" state, so that commands can be recorded into it.
 * This is typically called at the start of a frame, before any commands are recorded.
 *
 * The thread ownership semantics work like this - the thread that acquired the command buffer
 * is the only one that can queue commands into it, and the only one that can submit it for
 * execution.
 *
 * Make sure to also call `GPU_CmdsEnd` to set the command buffer to "executable" state when
 * done.
 */
void GPU_CmdsBegin(GPU_CmdBuffer* cb);

/**
 * OWNED_THREAD
 * Set the command buffer to "executable" state, so that it can be submitted for execution.
 * This is typically called at the end of a frame, after all commands have been recorded.
 */
void GPU_CmdsEnd(GPU_CmdBuffer* cb);

/**
 * OWNED_THREAD
 * Begin a render pass. For more info, see `GPU_PassCfg`.
 * The render pass will be ended with `GPU_CmdEndPass`.
 */
void GPU_CmdBeginPass(GPU_CmdBuffer* cb, GPU_PassCfg cfg);

/**
 * OWNED_THREAD
 * End the current render pass.
 */
void GPU_CmdEndPass(GPU_CmdBuffer* cb);

/**
 * OWNED_THREAD
 * Insert a barrier into the command buffer, to synchronize access to resources across different stages.
 */
void GPU_CmdBarrier(GPU_CmdBuffer* cb, GPU_BarrierCfg cfg);

/**
 * OWNED_THREAD
 * Bind a program to the command buffer, so that it can be used for subsequent dispatches.
 */
void GPU_CmdBindProgram(GPU_CmdBuffer* cb, GPU_BindProgramCfg cfg);

/**
 * OWNED_THREAD
 * Bind a program arguments group to the command buffer, so that it can be used for subsequent dispatches.
 */
void GPU_CmdBindProgramArgsGroup(GPU_CmdBuffer* cb, GPU_ProgramArgsGroupBindingCfg cfg);

/**
 * OWNED_THREAD
 * Bind inline constants to the command buffer, so that they can be used for subsequent dispatches.
 */
void GPU_CmdBindProgramInlineConstants(GPU_CmdBuffer* cb, GPU_ProgramInlineConstantArgBindingCfg cfg);

/**
 * OWNED_THREAD
 * Issue a basic draw call with the given configuration.
 */
void GPU_CmdDrawBasic(GPU_CmdBuffer* cb, GPU_DrawBasicCfg cfg);

/**
 * OWNED_THREAD
 * Issue a draw call with the given configuration, using mesh shading.
 */
void GPU_CmdDrawMeshlets(GPU_CmdBuffer* cb, GPU_DrawMeshletsCfg cfg);

EXTERN_C_END
