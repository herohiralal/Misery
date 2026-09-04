#include <GPU/GPU.h>
#include "DynamicDispatchSwitchboard/Enable.h"

void GPU_Create(GPU_Instance* outInstance, GPU_InstanceCfg cfg)
{
    RHI_FN_SWITCH_VOID(cfg.type, Create, outInstance, cfg);
}

void GPU_Destroy(GPU_Instance* renderer)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        Destroy,
        renderer
    );
}

void GPU_WaitTillIdle(GPU_Instance* renderer)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        WaitTillIdle,
        renderer
    );
}

void GPU_CreateSwapChainFromWindow(GPU_SwapChain* outSwapChain, GPU_Instance* renderer, WND_Handle windowHandle, GPU_SwapChainCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        CreateSwapChainFromWindow,
        outSwapChain, renderer, windowHandle, cfg
    );
}

void GPU_ReconfigureSwapChain(GPU_SwapChain* swapChain, GPU_SwapChainCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        ReconfigureSwapChain,
        swapChain, cfg
    );
}

void GPU_DestroySwapChain(GPU_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        DestroySwapChain,
        swapChain
    );
}

GPU_TextureFormat GPU_GetSwapChainTextureFormat(GPU_SwapChain* swapChain)
{
    RHI_FN_SWITCH_RET(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        GPU_TexFmt_Unknown,
        GetSwapChainTextureFormat,
        swapChain
    );
}

void GPU_IterateSwapChain(GPU_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        IterateSwapChain,
        swapChain
    );
}

GPU_SwapChainFrameContext GPU_BeginSwapChainFrame(GPU_SwapChain* swapChain)
{
    GPU_SwapChainFrameContext invalidCtx = {.valid = false};

    RHI_FN_SWITCH_RET(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        invalidCtx,
        BeginSwapChainFrame,
        swapChain
    );
}

void GPU_EndSwapChainFrame(GPU_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        EndSwapChainFrame,
        swapChain
    );
}

void GPU_NewBuffer(GPU_Buffer* outBuffer, GPU_Instance* renderer, GPU_BufferCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        NewBuffer,
        outBuffer, renderer, cfg
    );
}

void GPU_DeleteBuffer(GPU_Buffer* buffer)
{
    RHI_FN_SWITCH_VOID(
        buffer ? buffer->base.type : GPU_GfxAPIType_Null,
        DeleteBuffer,
        buffer
    );
}

void GPU_NewTexture(GPU_Texture* outTexture, GPU_Instance* renderer, GPU_TextureCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        NewTexture,
        outTexture, renderer, cfg
    );
}

void GPU_DeleteTexture(GPU_Texture* texture)
{
    RHI_FN_SWITCH_VOID(
        texture ? texture->base.type : GPU_GfxAPIType_Null,
        DeleteTexture,
        texture
    );
}

void GPU_NewProgramStage(GPU_ProgramStage* outStage, GPU_Instance* renderer, GPU_ProgramStageByteCode bc)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        NewProgramStage,
        outStage, renderer, bc
    );
}

void GPU_DeleteProgramStage(GPU_ProgramStage* stage)
{
    RHI_FN_SWITCH_VOID(
        stage ? stage->base.type : GPU_GfxAPIType_Null,
        DeleteProgramStage,
        stage
    );
}

void GPU_NewProgramArgsLayout(GPU_ProgramArgsLayout* outArgs, GPU_Instance* renderer, GPU_ProgramArgsLayoutCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        NewProgramArgsLayout,
        outArgs, renderer, cfg
    );
}

void GPU_DeleteProgramArgsLayout(GPU_ProgramArgsLayout* args)
{
    RHI_FN_SWITCH_VOID(
        args ? args->base.type : GPU_GfxAPIType_Null,
        DeleteProgramArgsLayout,
        args
    );
}

void GPU_NewProgram(GPU_Program* outProgram, GPU_Instance* renderer, GPU_ProgramCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        NewProgram,
        outProgram, renderer, cfg
    );
}

void GPU_DeleteProgram(GPU_Program* program)
{
    RHI_FN_SWITCH_VOID(
        program ? program->base.type : GPU_GfxAPIType_Null,
        DeleteProgram,
        program
    );
}

void GPU_NewProgramArgsBuffer(GPU_ProgramArgsBuffer* outArgsBuffer, GPU_Instance* renderer, GPU_ProgramArgsBufferCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : GPU_GfxAPIType_Null,
        NewProgramArgsBuffer,
        outArgsBuffer, renderer, cfg
    );
}

void GPU_DeleteProgramArgsBuffer(GPU_ProgramArgsBuffer* argsBuffer)
{
    RHI_FN_SWITCH_VOID(
        argsBuffer ? argsBuffer->base.type : GPU_GfxAPIType_Null,
        DeleteProgramArgsBuffer,
        argsBuffer
    );
}

void GPU_UpdateProgramArgsBuffer(GPU_ProgramArgsBuffer* argsBuffer, Slice_(GPU_ProgramArgBindingCfg) bindings)
{
    RHI_FN_SWITCH_VOID(
        argsBuffer ? argsBuffer->base.type : GPU_GfxAPIType_Null,
        UpdateProgramArgsBuffer,
        argsBuffer, bindings
    );
}

void GPU_CmdsBegin(GPU_CmdBuffer* cb)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdsBegin,
        cb
    );
}

void GPU_CmdsEnd(GPU_CmdBuffer* cb)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdsEnd,
        cb
    );
}

void GPU_CmdBeginPass(GPU_CmdBuffer* cb, GPU_PassCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBeginPass,
        cb, cfg
    );
}

void GPU_CmdEndPass(GPU_CmdBuffer* cb)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdEndPass,
        cb
    );
}

void GPU_CmdBarrier(GPU_CmdBuffer* cb, GPU_BarrierCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBarrier,
        cb, cfg
    );
}

void GPU_CmdBindProgram(GPU_CmdBuffer* cb, GPU_BindProgramCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBindProgram,
        cb, cfg
    );
}

void GPU_CmdBindProgramArgsGroup(GPU_CmdBuffer* cb, GPU_ProgramArgsGroupBindingCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBindProgramArgsGroup,
        cb, cfg
    );
}

void GPU_CmdBindProgramInlineConstants(GPU_CmdBuffer* cb, GPU_ProgramInlineConstantArgBindingCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBindProgramInlineConstants,
        cb, cfg
    );
}

void GPU_CmdDrawBasic(GPU_CmdBuffer* cb, GPU_DrawBasicCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdDrawBasic,
        cb, cfg
    );
}

void GPU_CmdDrawMeshlets(GPU_CmdBuffer* cb, GPU_DrawMeshletsCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdDrawMeshlets,
        cb, cfg
    );
}

#include "DynamicDispatchSwitchboard/Disable.h"
