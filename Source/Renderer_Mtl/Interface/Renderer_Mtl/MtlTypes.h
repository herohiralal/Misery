#pragma once
#include <__init.h>
#include <ExtDeps_Renderer.h>
#include <Renderer_Base/Renderer_Base.h>

#if REN_MTL
EXTERN_C_BEGIN

REN_EXTEND_OBJECT(Mtl, Instance,
    APP_Handle          appHandle;

    id<MTLDevice>       device;
    id<MTLCommandQueue> gfxQueue;

    utf8str             appName;

    struct
    {
        u8 appName[32];
    } buffers;
);

REN_EXTEND_OBJECT(Mtl, CmdBuffer,
    const REN_MtlInstance* renderer;
    id<MTLCommandBuffer>   actual;
);

REN_EXTEND_OBJECT(Mtl, SwapChain,
    REN_MtlInstance* renderer;
    WND_Handle       window;

    // cfg
    b8 vSync;

    // syncing
    b8  allowCmdBuff;
    u32 curFrame;

    struct
    {
        REN_CmdBuffer cmdBuffers[REN_FRAMES_IN_FLIGHT];
    } buffers;
);

EXTERN_C_END
#endif
