#include "Dx12Private.h"

#if GPU_DX12

void GPU_Dx12CmdBuffBegin(GPU_CmdBuffer* cb)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    cmdBuffer->cmdList->Reset(cmdBuffer->cmdAllocator, nil);

}

void GPU_Dx12CmdBuffEnd(GPU_CmdBuffer* cb)
{
    GPU_Dx12CmdBuffer* cmdBuffer = GPU_ToDx12CmdBuffer(cb);
    MSR_ASSERT(cmdBuffer && "cmdBuffer must not be null");

    cmdBuffer->cmdList->Close();
}

#endif
