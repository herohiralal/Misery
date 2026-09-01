#include "Dx12Private.h"

#if GPU_DX12

void GPU_Dx12NewProgramStage(GPU_ProgramStage*, GPU_Instance*, GPU_ProgramStageByteCode) { MSR_ASSERT(false && "not implemented"); }
void GPU_Dx12DeleteProgramStage(GPU_ProgramStage*) { MSR_ASSERT(false && "not implemented"); }

void GPU_Dx12NewProgram(GPU_Program*, GPU_Instance*, GPU_ProgramCfg) { MSR_ASSERT(false && "not implemented"); }
void GPU_Dx12DeleteProgram(GPU_Program*) { MSR_ASSERT(false && "not implemented"); }

#endif
