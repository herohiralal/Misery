#include <__init.h>

template <b8 condition>
struct GPU_TNoValueOnFailure { };

template <>
struct GPU_TNoValueOnFailure<true> { static constexpr const b8 VALUE = true; };

#define GPU_OBJ_SIZE_CHECK_NAME(gfxApi, name) \
    ______________________________________________________##gfxApi##IF_YOU_SEE_THIS_LINE_IN_YOUR_COMPILER_ERROR_THEN_SET_THE_PADDING_OF_TYPE_____GPU_##name##_____TO_

#define GPU_OBJ_SIZE_CHECK(gfxApi, name) \
    static_assert( \
        alignof(GPU_##name) % alignof(GPU_##gfxApi##name) == 0, \
        "The alignment of GPU_" #name " must be divisible by the alignment of GPU_" #gfxApi #name "." \
        "\n\tPlease ensure that GPU_" #gfxApi #name " does not contain any over-aligned types." \
    ); \
    EXTERN_C_END \
    template <std::size_t N> \
    struct GPU_OBJ_SIZE_CHECK_NAME(gfxApi, name) \
        : public GPU_TNoValueOnFailure<(sizeof(GPU_##name) >= sizeof(GPU_##gfxApi##name))> \
    { \
    }; \
    static_assert(GPU_OBJ_SIZE_CHECK_NAME(gfxApi, name)<sizeof(GPU_##gfxApi##name)>::VALUE, \
        "The size of GPU_" #name " must be greater than or equal to the size of GPU_" #gfxApi #name "." \
    ); \
    EXTERN_C_BEGIN

#include <GPU_Base/GPU_Base.h>
#include <GPU_Vk/GPU_Vk.h>
#include <GPU_Dx12/GPU_Dx12.h>
#include <GPU_Mtl/GPU_Mtl.h>

#undef GPU_OBJ_SIZE_CHECK
#undef GPU_OBJ_SIZE_CHECK_NAME
