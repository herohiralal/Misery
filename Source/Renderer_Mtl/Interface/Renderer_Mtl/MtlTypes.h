#pragma once
#include <__init.h>
#include <ExtDeps_Renderer.h>
#include <Renderer_Base/Renderer_Base.h>

#if REN_MTL
EXTERN_C_BEGIN

REN_EXTEND_OBJECT(Mtl, Instance,
    MEM_Allocator       allocator;
    APP_Handle          appHandle;

    id<MTLDevice>       device;
    id<MTLCommandQueue> gfxQueue;

    utf8str             appName;
);

REN_EXTEND_OBJECT(Mtl, CmdBuffer,
    const REN_MtlInstance* renderer;
    id<MTLCommandBuffer>   actual;
);

REN_EXTEND_OBJECT(Mtl, SwapChain,
    const REN_MtlInstance* renderer;
);

EXTERN_C_END
#endif
