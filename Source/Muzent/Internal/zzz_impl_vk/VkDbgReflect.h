#ifndef MZNT_VK_DBG_REFLECT_H
#define MZNT_VK_DBG_REFLECT_H
#include "Muzent/__Prelude.h"
#include "Muzent/RendererFns.h"
#if defined(MZNT_IMPLEMENTATION) && MZNT_VULKAN

#ifdef __cplusplus

template<typename T>
struct MZNT_Internal_VkObjectType;

template<typename T>
struct MZNT_Internal_VkObjectType {
    static constexpr VkObjectType value = VK_OBJECT_TYPE_UNKNOWN;
    static_assert(sizeof(T) == 0, "Unsupported Vulkan object type");
};

template<> struct MZNT_Internal_VkObjectType<VkInstance>                   { static constexpr VkObjectType value = VK_OBJECT_TYPE_INSTANCE; };
template<> struct MZNT_Internal_VkObjectType<VkPhysicalDevice>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_PHYSICAL_DEVICE; };
template<> struct MZNT_Internal_VkObjectType<VkDevice>                     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEVICE; };
template<> struct MZNT_Internal_VkObjectType<VkQueue>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_QUEUE; };
template<> struct MZNT_Internal_VkObjectType<VkSemaphore>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_SEMAPHORE; };
template<> struct MZNT_Internal_VkObjectType<VkCommandBuffer>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_COMMAND_BUFFER; };
template<> struct MZNT_Internal_VkObjectType<VkFence>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_FENCE; };
template<> struct MZNT_Internal_VkObjectType<VkDeviceMemory>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEVICE_MEMORY; };
template<> struct MZNT_Internal_VkObjectType<VkBuffer>                     { static constexpr VkObjectType value = VK_OBJECT_TYPE_BUFFER; };
template<> struct MZNT_Internal_VkObjectType<VkImage>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_IMAGE; };
template<> struct MZNT_Internal_VkObjectType<VkEvent>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_EVENT; };
template<> struct MZNT_Internal_VkObjectType<VkQueryPool>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_QUERY_POOL; };
template<> struct MZNT_Internal_VkObjectType<VkBufferView>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_BUFFER_VIEW; };
template<> struct MZNT_Internal_VkObjectType<VkImageView>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_IMAGE_VIEW; };
template<> struct MZNT_Internal_VkObjectType<VkShaderModule>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_SHADER_MODULE; };
template<> struct MZNT_Internal_VkObjectType<VkPipelineCache>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_CACHE; };
template<> struct MZNT_Internal_VkObjectType<VkPipelineLayout>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_LAYOUT; };
template<> struct MZNT_Internal_VkObjectType<VkRenderPass>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_RENDER_PASS; };
template<> struct MZNT_Internal_VkObjectType<VkPipeline>                   { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE; };
template<> struct MZNT_Internal_VkObjectType<VkDescriptorSetLayout>        { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
template<> struct MZNT_Internal_VkObjectType<VkSampler>                    { static constexpr VkObjectType value = VK_OBJECT_TYPE_SAMPLER; };
template<> struct MZNT_Internal_VkObjectType<VkDescriptorPool>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };
template<> struct MZNT_Internal_VkObjectType<VkDescriptorSet>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
template<> struct MZNT_Internal_VkObjectType<VkFramebuffer>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_FRAMEBUFFER; };
template<> struct MZNT_Internal_VkObjectType<VkCommandPool>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_COMMAND_POOL; };

template<> struct MZNT_Internal_VkObjectType<VkSamplerYcbcrConversion>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; };
template<> struct MZNT_Internal_VkObjectType<VkDescriptorUpdateTemplate>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; };
template<> struct MZNT_Internal_VkObjectType<VkPrivateDataSlot>            { static constexpr VkObjectType value = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; };

template<> struct MZNT_Internal_VkObjectType<VkSurfaceKHR>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_SURFACE_KHR; };
template<> struct MZNT_Internal_VkObjectType<VkSwapchainKHR>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_SWAPCHAIN_KHR; };
template<> struct MZNT_Internal_VkObjectType<VkDisplayKHR>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_DISPLAY_KHR; };
template<> struct MZNT_Internal_VkObjectType<VkDisplayModeKHR>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_DISPLAY_MODE_KHR; };

template<> struct MZNT_Internal_VkObjectType<VkDebugReportCallbackEXT>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; };
template<> struct MZNT_Internal_VkObjectType<VkDebugUtilsMessengerEXT>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; };

template<> struct MZNT_Internal_VkObjectType<VkVideoSessionKHR>            { static constexpr VkObjectType value = VK_OBJECT_TYPE_VIDEO_SESSION_KHR; };
template<> struct MZNT_Internal_VkObjectType<VkVideoSessionParametersKHR>  { static constexpr VkObjectType value = VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; };

template<> struct MZNT_Internal_VkObjectType<VkCuModuleNVX>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_CU_MODULE_NVX; };
template<> struct MZNT_Internal_VkObjectType<VkCuFunctionNVX>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_CU_FUNCTION_NVX; };

template<> struct MZNT_Internal_VkObjectType<VkAccelerationStructureKHR>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; };
template<> struct MZNT_Internal_VkObjectType<VkValidationCacheEXT>         { static constexpr VkObjectType value = VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; };
template<> struct MZNT_Internal_VkObjectType<VkAccelerationStructureNV>    { static constexpr VkObjectType value = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; };

template<> struct MZNT_Internal_VkObjectType<VkPerformanceConfigurationINTEL> { static constexpr VkObjectType value = VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; };
template<> struct MZNT_Internal_VkObjectType<VkDeferredOperationKHR>       { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; };

template<> struct MZNT_Internal_VkObjectType<VkIndirectCommandsLayoutNV>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; };

template<> struct MZNT_Internal_VkObjectType<VkMicromapEXT>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_MICROMAP_EXT; };

template<> struct MZNT_Internal_VkObjectType<VkTensorARM>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_TENSOR_ARM; };
template<> struct MZNT_Internal_VkObjectType<VkTensorViewARM>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_TENSOR_VIEW_ARM; };

template<> struct MZNT_Internal_VkObjectType<VkOpticalFlowSessionNV>       { static constexpr VkObjectType value = VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; };

template<> struct MZNT_Internal_VkObjectType<VkShaderEXT>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_SHADER_EXT; };
template<> struct MZNT_Internal_VkObjectType<VkPipelineBinaryKHR>          { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_BINARY_KHR; };

template<> struct MZNT_Internal_VkObjectType<VkDataGraphPipelineSessionARM>{ static constexpr VkObjectType value = VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM; };

template<> struct MZNT_Internal_VkObjectType<VkExternalComputeQueueNV>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV; };

template<> struct MZNT_Internal_VkObjectType<VkIndirectCommandsLayoutEXT>  { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT; };
template<> struct MZNT_Internal_VkObjectType<VkIndirectExecutionSetEXT>    { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT; };

#define MZNT_INTERNAL_GET_VK_OBJECT_TYPE(x) \
     (MZNT_Internal_VkObjectType<decltype(x)>::value)

#else

#define MZNT_INTERNAL_GET_VK_OBJECT_TYPE(x) \
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

#endif
#endif
