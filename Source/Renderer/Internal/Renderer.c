#include <Renderer/Renderer.h>
#include "DynamicDispatchSwitchboard/Enable.h"

void REN_CreateRenderer(REN_Instance* outInstance, REN_InstanceCfg cfg)
{
    RHI_FN_SWITCH_VOID(cfg.type, CreateRenderer, outInstance, cfg);
}

void REN_DestroyRenderer(REN_Instance* renderer)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : REN_GfxAPIType_Null,
        DestroyRenderer,
        renderer
    );
}

void REN_WaitTillRendererIdle(REN_Instance* renderer)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : REN_GfxAPIType_Null,
        WaitTillRendererIdle,
        renderer
    );
}

void REN_CreateSwapChainFromWindow(REN_SwapChain* outSwapChain, REN_Instance* renderer, WND_Handle windowHandle, REN_SwapChainCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        renderer ? renderer->base.type : REN_GfxAPIType_Null,
        CreateSwapChainFromWindow,
        outSwapChain, renderer, windowHandle, cfg
    );
}

void REN_ReconfigureSwapChain(REN_SwapChain* swapChain, REN_SwapChainCfg cfg)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : REN_GfxAPIType_Null,
        ReconfigureSwapChain,
        swapChain, cfg
    );
}

void REN_DestroySwapChain(REN_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : REN_GfxAPIType_Null,
        DestroySwapChain,
        swapChain
    );
}

REN_TextureFormat REN_GetSwapChainTextureFormat(REN_SwapChain* swapChain)
{
    RHI_FN_SWITCH_RET(
        swapChain ? swapChain->base.type : REN_GfxAPIType_Null,
        REN_TexFmt_Unknown,
        GetSwapChainTextureFormat,
        swapChain
    );
}

void REN_IterateSwapChain(REN_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : REN_GfxAPIType_Null,
        IterateSwapChain,
        swapChain
    );
}

REN_CmdBuffer* REN_GetSwapChainCommandBuffer(REN_SwapChain* swapChain, u8* outImgIdx)
{
    RHI_FN_SWITCH_RET(
        swapChain ? swapChain->base.type : REN_GfxAPIType_Null,
        nil,
        GetSwapChainCommandBuffer,
        swapChain, outImgIdx
    );
}

void REN_PresentSwapChain(REN_SwapChain* swapChain)
{
    RHI_FN_SWITCH_VOID(
        swapChain ? swapChain->base.type : REN_GfxAPIType_Null,
        PresentSwapChain,
        swapChain
    );
}

#include "DynamicDispatchSwitchboard/Disable.h"
