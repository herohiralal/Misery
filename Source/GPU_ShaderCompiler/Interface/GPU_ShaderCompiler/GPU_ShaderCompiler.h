#pragma once
#include <__init.h>
#include <GPU_Base/GPU_Base.h>

EXTERN_C_BEGIN

/**
 * THREAD_SAFE
 * Create a program stage from the given configuration.
 */
b8 GPU_CompileProgramStage(GPU_ProgramStage* stage, GPU_ProgramStageCfg cfg);

/**
 * THREAD_SAFE
 * Destroy the given program stage, freeing up associated resources.
 */
void GPU_FreeProgramStage(GPU_ProgramStage* stage);

EXTERN_C_END
