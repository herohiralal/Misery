#define REN_OBJ_SIZE_CHECK(gfxApi, name) \
    static_assert( \
        sizeof(REN_##name) >= sizeof(REN_##gfxAPi##name), \
        "The size of REN_" #name " must be greater than or equal to the size of REN_" #gfxApi #name "." \
        "\n\tPlease resize the opaque padding for REN_" #name "." \
    ); \
    static_assert( \
        alignof(REN_##name) % alignof(REN_##gfxAPi##name) == 0, \
        "The alignment of REN_" #name " must be divisible by the alignment of REN_" #gfxApi #name "." \
        "\n\tPlease ensure that REN_" #gfxApi #name " does not contain any over-aligned types." \
    );

#include <Renderer_Base/Renderer_Base.h>
