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

GPU_CmdBuffer* GPU_GetSwapChainCommandBuffer(GPU_SwapChain* swapChain, u8* outImgIdx)
{
    RHI_FN_SWITCH_RET(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        nil,
        GetSwapChainCommandBuffer,
        swapChain, outImgIdx
    );
}

void GPU_PresentSwapChain(GPU_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : GPU_GfxAPIType_Null,
        PresentSwapChain,
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

void GPU_CmdBuffBegin(GPU_CmdBuffer* cb)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBuffBegin,
        cb
    );
}

void GPU_CmdBuffEnd(GPU_CmdBuffer* cb)
{
    RHI_FN_SWITCH_VOID(
        cb ? cb->base.type : GPU_GfxAPIType_Null,
        CmdBuffEnd,
        cb
    );
}

#include "DynamicDispatchSwitchboard/Disable.h"
