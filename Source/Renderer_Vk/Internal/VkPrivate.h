#pragma once
#include <Renderer_Vk/Renderer_Vk.h>

#if REN_VK

#ifdef __cplusplus

template<typename T>
struct REN_VkObjectType;

template<typename T>
struct REN_VkObjectType {
    static constexpr VkObjectType value = VK_OBJECT_TYPE_UNKNOWN;
    static_assert(sizeof(T) == 0, "Unsupported Vulkan object type");
};

template<> struct REN_VkObjectType<VkInstance>                   { static constexpr VkObjectType value = VK_OBJECT_TYPE_INSTANCE; };
template<> struct REN_VkObjectType<VkPhysicalDevice>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_PHYSICAL_DEVICE; };
template<> struct REN_VkObjectType<VkDevice>                     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEVICE; };
template<> struct REN_VkObjectType<VkQueue>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_QUEUE; };
template<> struct REN_VkObjectType<VkSemaphore>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_SEMAPHORE; };
template<> struct REN_VkObjectType<VkCommandBuffer>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_COMMAND_BUFFER; };
template<> struct REN_VkObjectType<VkFence>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_FENCE; };
template<> struct REN_VkObjectType<VkDeviceMemory>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEVICE_MEMORY; };
template<> struct REN_VkObjectType<VkBuffer>                     { static constexpr VkObjectType value = VK_OBJECT_TYPE_BUFFER; };
template<> struct REN_VkObjectType<VkImage>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_IMAGE; };
template<> struct REN_VkObjectType<VkEvent>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_EVENT; };
template<> struct REN_VkObjectType<VkQueryPool>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_QUERY_POOL; };
template<> struct REN_VkObjectType<VkBufferView>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_BUFFER_VIEW; };
template<> struct REN_VkObjectType<VkImageView>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_IMAGE_VIEW; };
template<> struct REN_VkObjectType<VkShaderModule>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_SHADER_MODULE; };
template<> struct REN_VkObjectType<VkPipelineCache>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_CACHE; };
template<> struct REN_VkObjectType<VkPipelineLayout>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_LAYOUT; };
template<> struct REN_VkObjectType<VkRenderPass>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_RENDER_PASS; };
template<> struct REN_VkObjectType<VkPipeline>                   { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE; };
template<> struct REN_VkObjectType<VkDescriptorSetLayout>        { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
template<> struct REN_VkObjectType<VkSampler>                    { static constexpr VkObjectType value = VK_OBJECT_TYPE_SAMPLER; };
template<> struct REN_VkObjectType<VkDescriptorPool>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };
template<> struct REN_VkObjectType<VkDescriptorSet>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
template<> struct REN_VkObjectType<VkFramebuffer>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_FRAMEBUFFER; };
template<> struct REN_VkObjectType<VkCommandPool>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_COMMAND_POOL; };

template<> struct REN_VkObjectType<VkSamplerYcbcrConversion>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; };
template<> struct REN_VkObjectType<VkDescriptorUpdateTemplate>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; };
template<> struct REN_VkObjectType<VkPrivateDataSlot>            { static constexpr VkObjectType value = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; };

template<> struct REN_VkObjectType<VkSurfaceKHR>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_SURFACE_KHR; };
template<> struct REN_VkObjectType<VkSwapchainKHR>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_SWAPCHAIN_KHR; };
template<> struct REN_VkObjectType<VkDisplayKHR>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_DISPLAY_KHR; };
template<> struct REN_VkObjectType<VkDisplayModeKHR>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_DISPLAY_MODE_KHR; };

template<> struct REN_VkObjectType<VkDebugReportCallbackEXT>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; };
template<> struct REN_VkObjectType<VkDebugUtilsMessengerEXT>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; };

template<> struct REN_VkObjectType<VkVideoSessionKHR>            { static constexpr VkObjectType value = VK_OBJECT_TYPE_VIDEO_SESSION_KHR; };
template<> struct REN_VkObjectType<VkVideoSessionParametersKHR>  { static constexpr VkObjectType value = VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; };

template<> struct REN_VkObjectType<VkCuModuleNVX>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_CU_MODULE_NVX; };
template<> struct REN_VkObjectType<VkCuFunctionNVX>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_CU_FUNCTION_NVX; };

template<> struct REN_VkObjectType<VkAccelerationStructureKHR>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; };
template<> struct REN_VkObjectType<VkValidationCacheEXT>         { static constexpr VkObjectType value = VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; };
template<> struct REN_VkObjectType<VkAccelerationStructureNV>    { static constexpr VkObjectType value = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; };

template<> struct REN_VkObjectType<VkPerformanceConfigurationINTEL> { static constexpr VkObjectType value = VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; };
template<> struct REN_VkObjectType<VkDeferredOperationKHR>       { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; };

template<> struct REN_VkObjectType<VkIndirectCommandsLayoutNV>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; };

template<> struct REN_VkObjectType<VkMicromapEXT>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_MICROMAP_EXT; };

template<> struct REN_VkObjectType<VkTensorARM>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_TENSOR_ARM; };
template<> struct REN_VkObjectType<VkTensorViewARM>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_TENSOR_VIEW_ARM; };

template<> struct REN_VkObjectType<VkOpticalFlowSessionNV>       { static constexpr VkObjectType value = VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; };

template<> struct REN_VkObjectType<VkShaderEXT>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_SHADER_EXT; };
template<> struct REN_VkObjectType<VkPipelineBinaryKHR>          { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_BINARY_KHR; };

template<> struct REN_VkObjectType<VkDataGraphPipelineSessionARM>{ static constexpr VkObjectType value = VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM; };

template<> struct REN_VkObjectType<VkExternalComputeQueueNV>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV; };

template<> struct REN_VkObjectType<VkIndirectCommandsLayoutEXT>  { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT; };
template<> struct REN_VkObjectType<VkIndirectExecutionSetEXT>    { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT; };

#define REN_GET_VK_OBJECT_TYPE(x) \
     (REN_VkObjectType<decltype(x)>::value)

#else

#define REN_GET_VK_OBJECT_TYPE(x) \
    _Generic((x), \
        VkInstance: VK_OBJECT_TYPE_INSTANCE, \
        VkPhysicalDevice: VK_OBJECT_TYPE_PHYSICAL_DEVICE, \
        VkDevice: VK_OBJECT_TYPE_DEVICE, \
        VkQueue: VK_OBJECT_TYPE_QUEUE, \
        VkSemaphore: VK_OBJECT_TYPE_SEMAPHORE, \
        VkCommandBuffer: VK_OBJECT_TYPE_COMMAND_BUFFER, \
        VkFence: VK_OBJECT_TYPE_FENCE, \
        VkDeviceMemory: VK_OBJECT_TYPE_DEVICE_MEMORY, \
        VkBuffer: VK_OBJECT_TYPE_BUFFER, \
        VkImage: VK_OBJECT_TYPE_IMAGE, \
        VkEvent: VK_OBJECT_TYPE_EVENT, \
        VkQueryPool: VK_OBJECT_TYPE_QUERY_POOL, \
        VkBufferView: VK_OBJECT_TYPE_BUFFER_VIEW, \
        VkImageView: VK_OBJECT_TYPE_IMAGE_VIEW, \
        VkShaderModule: VK_OBJECT_TYPE_SHADER_MODULE, \
        VkPipelineCache: VK_OBJECT_TYPE_PIPELINE_CACHE, \
        VkPipelineLayout: VK_OBJECT_TYPE_PIPELINE_LAYOUT, \
        VkRenderPass: VK_OBJECT_TYPE_RENDER_PASS, \
        VkPipeline: VK_OBJECT_TYPE_PIPELINE, \
        VkDescriptorSetLayout: VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, \
        VkSampler: VK_OBJECT_TYPE_SAMPLER, \
        VkDescriptorPool: VK_OBJECT_TYPE_DESCRIPTOR_POOL, \
        VkDescriptorSet: VK_OBJECT_TYPE_DESCRIPTOR_SET, \
        VkFramebuffer: VK_OBJECT_TYPE_FRAMEBUFFER, \
        VkCommandPool: VK_OBJECT_TYPE_COMMAND_POOL, \
        VkSamplerYcbcrConversion: VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION, \
        VkDescriptorUpdateTemplate: VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE, \
        VkPrivateDataSlot: VK_OBJECT_TYPE_PRIVATE_DATA_SLOT, \
        VkSurfaceKHR: VK_OBJECT_TYPE_SURFACE_KHR, \
        VkSwapchainKHR: VK_OBJECT_TYPE_SWAPCHAIN_KHR, \
        VkDisplayKHR: VK_OBJECT_TYPE_DISPLAY_KHR, \
        VkDisplayModeKHR: VK_OBJECT_TYPE_DISPLAY_MODE_KHR, \
        VkDebugReportCallbackEXT: VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT, \
        VkDebugUtilsMessengerEXT: VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT, \
        VkVideoSessionKHR: VK_OBJECT_TYPE_VIDEO_SESSION_KHR, \
        VkVideoSessionParametersKHR: VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR, \
        VkCuModuleNVX: VK_OBJECT_TYPE_CU_MODULE_NVX, \
        VkCuFunctionNVX: VK_OBJECT_TYPE_CU_FUNCTION_NVX, \
        VkAccelerationStructureKHR: VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, \
        VkValidationCacheEXT: VK_OBJECT_TYPE_VALIDATION_CACHE_EXT, \
        VkAccelerationStructureNV: VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV, \
        VkPerformanceConfigurationINTEL: VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL, \
        VkDeferredOperationKHR: VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR, \
        VkIndirectCommandsLayoutNV: VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV, \
        VkMicromapEXT: VK_OBJECT_TYPE_MICROMAP_EXT, \
        VkTensorARM: VK_OBJECT_TYPE_TENSOR_ARM, \
        VkTensorViewARM: VK_OBJECT_TYPE_TENSOR_VIEW_ARM, \
        VkOpticalFlowSessionNV: VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV, \
        VkShaderEXT: VK_OBJECT_TYPE_SHADER_EXT, \
        VkPipelineBinaryKHR: VK_OBJECT_TYPE_PIPELINE_BINARY_KHR, \
        VkDataGraphPipelineSessionARM: VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM, \
        VkExternalComputeQueueNV: VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV, \
        VkIndirectCommandsLayoutEXT: VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT, \
        VkIndirectExecutionSetEXT: VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT \
    )

#endif

EXTERN_C_BEGIN

PFN_vkDebugUtilsMessengerCallbackEXT REN_GetVkDebugCallback(void);
void REN_LogVkResultOnFailure(VkResult result, utf8str fnCall, SrcLoc loc);
void REN_SetVkObjDebugName(const REN_VkInstance* renderer, void* obj, VkObjectType objTy, utf8str fmtStr, FMT_Args fmtArgs);

#define REN_VK_CHECKED_CALL(call) \
    REN_LogVkResultOnFailure((call), UTF8STR(#call), SRC_LOC())

#define REN_VK_SET_OBJ_DEBUG_NAME(renderer, obj, fmt, ...) \
    REN_SetVkObjDebugName((renderer), (obj), REN_GET_VK_OBJECT_TYPE(obj), UTF8STR(fmt), FMTARGS(__VA_ARGS__))

VkFormat REN_BreakVkTextureFormat(REN_TextureFormat);
REN_TextureFormat REN_MakeVkTextureFormat(VkFormat);

EXTERN_C_END
#endif
