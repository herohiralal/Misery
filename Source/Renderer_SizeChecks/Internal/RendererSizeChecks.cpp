#include <__init.h>

#define REN_OBJ_SIZE_CHECK(gfxApi, name) \
    static_assert( \
        alignof(REN_##name) % alignof(REN_##gfxAPi##name) == 0, \
        "The alignment of REN_" #name " must be divisible by the alignment of REN_" #gfxApi #name "." \
        "\n\tPlease ensure that REN_" #gfxApi #name " does not contain any over-aligned types." \
    ); \
    EXTERN_C_END \
    template <std::size_t N> \
    struct ______________________________________________________________IF_YOU_SEE_THIS_LINE_IN_YOUR_COMPILER_ERROR_THEN_SET_THE_PADDING_OF_TYPE_____REN_##name##_____TO_THE_VALUE_IN_ANGULAR_BRACKETS \
    { \
        static_assert(sizeof(REN_##name) >= sizeof(REN_##gfxApi##name), ""); \
    }; \
    template struct ______________________________________________________________IF_YOU_SEE_THIS_LINE_IN_YOUR_COMPILER_ERROR_THEN_SET_THE_PADDING_OF_TYPE_____REN_##name##_____TO_THE_VALUE_IN_ANGULAR_BRACKETS<sizeof(REN_##gfxApi##name)>; \
    EXTERN_C_BEGIN

#include <Renderer_Base/Renderer_Base.h>
#include <Renderer_Null/Renderer_Null.h>
#include <Renderer_Vk/Renderer_Vk.h>
#include <Renderer_Dx12/Renderer_Dx12.h>
#include <Renderer_Mtl/Renderer_Mtl.h>
