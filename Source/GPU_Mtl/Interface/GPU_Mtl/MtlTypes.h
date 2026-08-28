#pragma once
#include <__init.h>
#include <ExtDeps_GPU.h>
#include <GPU_Base/GPU_Base.h>

#if GPU_MTL
EXTERN_C_BEGIN

GPU_EXTEND_OBJECT(Mtl, Instance,
    APP_Handle          appHandle;

    id<MTLDevice>       device;
    id<MTLCommandQueue> gfxQueue;

    utf8str             appName;

    struct
    {
        u8 appName[32];
    } buffers;
);

GPU_EXTEND_OBJECT(Mtl, CmdBuffer,
    const GPU_MtlInstance* renderer;
    id<MTLCommandBuffer>   actual;
);

GPU_EXTEND_OBJECT(Mtl, SwapChain,
    GPU_MtlInstance* renderer;
    WND_Handle       window;

    // cfg
    b8 vSync;

    // syncing
    b8  allowCmdBuff;
    u32 curFrame;

    struct
    {
        GPU_CmdBuffer cmdBuffers[GPU_FRAMES_IN_FLIGHT];
    } buffers;
);

EXTERN_C_END
#endif
