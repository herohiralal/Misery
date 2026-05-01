#include "RendererPrivate.h"

#include "DynamicDispatchSwitchboard/Enable.h"

MZNT_RHI_FN_TWO_ARG(
    CreateRenderer,
    (config.type),

    MZNT_Renderer*,             /* ret */      DYNA,
    MZNT_RendererConfiguration, config,        STAT,
    PNSLR_Allocator,            tempAllocator, STAT
)

MZNT_RHI_FN_ONE_ARG(
    WaitTillRendererIdle,
    (renderer ? renderer->type : 0),

    b8,       /* ret */ STAT,
    Renderer, renderer, DYNA
)

MZNT_RHI_FN_TWO_ARG(
    DestroyRenderer,
    (renderer ? renderer->type : 0),

    b8,              /* ret */      STAT,
    Renderer,        renderer,      DYNA,
    PNSLR_Allocator, tempAllocator, STAT
)

MZNT_RHI_FN_FOUR_ARG(
    CreateSwapChainFromWindow,
    (renderer ? renderer->type : 0),

    MZNT_SwapChain*,             /* ret */ DYNA,
    Renderer,                    renderer, DYNA,
    MZNT_WindowHandle,           hnd,      STAT,
    MZNT_SwapChainConfiguration, cfg,      STAT,
    PNSLR_Allocator, t, STAT
)

MZNT_RHI_FN_THREE_ARG(
    ReconfigureSwapChain,
    (swapChain ? swapChain->type : 0),

    b8,                          /* ret */  STAT,
    SwapChain,                   swapChain, DYNA,
    MZNT_SwapChainConfiguration, cfg,       STAT,
    PNSLR_Allocator,             t,         STAT
)

MZNT_RHI_FN_TWO_ARG(
    DestroySwapChain,
    (swapChain ? swapChain->type : 0),

    b8,              /* ret */  STAT,
    SwapChain,       swapChain, DYNA,
    PNSLR_Allocator, t,         STAT
)

MZNT_RHI_FN_ONE_ARG(
    GetSwapChainTextureFormat,
    (swapChain ? swapChain->type : 0),

    MZNT_TextureFormat, /* ret */  STAT,
    SwapChain,          swapChain, DYNA
)

MZNT_RHI_FN_TWO_ARG(
    IterateSwapChain,
    (swapChain ? swapChain->type : 0),

    b8,              /* ret */  STAT,
    SwapChain,       swapChain, DYNA,
    PNSLR_Allocator, t,         STAT
)

MZNT_RHI_FN_THREE_ARG(
    GetSwapChainCommandBuffer,
    (swapChain ? swapChain->type : 0),

    MZNT_RendererCommandBuffer*, /* ret */  DYNA,
    SwapChain,                   swapChain, DYNA,
    u8*,                         outImgIdx, STAT,
    PNSLR_Allocator,             t,         STAT
)

MZNT_RHI_FN_TWO_ARG(
    PresentSwapChain,
    (swapChain ? swapChain->type : 0),

    b8,              /* ret */  STAT,
    SwapChain,       swapChain, DYNA,
    PNSLR_Allocator, t,         STAT
)

#include "DynamicDispatchSwitchboard/Disable.h"
