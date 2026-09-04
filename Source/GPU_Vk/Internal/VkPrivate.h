#pragma once
#include <GPU_Vk/GPU_Vk.h>

#if GPU_VK

#ifdef __cplusplus

template<typename T>
struct GPU_VkObjectType;

template<typename T>
struct GPU_VkObjectType {
    static constexpr VkObjectType value = VK_OBJECT_TYPE_UNKNOWN;
    static_assert(sizeof(T) == 0, "Unsupported Vulkan object type");
};

template<> struct GPU_VkObjectType<VkInstance>                   { static constexpr VkObjectType value = VK_OBJECT_TYPE_INSTANCE; };
template<> struct GPU_VkObjectType<VkPhysicalDevice>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_PHYSICAL_DEVICE; };
template<> struct GPU_VkObjectType<VkDevice>                     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEVICE; };
template<> struct GPU_VkObjectType<VkQueue>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_QUEUE; };
template<> struct GPU_VkObjectType<VkSemaphore>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_SEMAPHORE; };
template<> struct GPU_VkObjectType<VkCommandBuffer>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_COMMAND_BUFFER; };
template<> struct GPU_VkObjectType<VkFence>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_FENCE; };
template<> struct GPU_VkObjectType<VkDeviceMemory>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEVICE_MEMORY; };
template<> struct GPU_VkObjectType<VkBuffer>                     { static constexpr VkObjectType value = VK_OBJECT_TYPE_BUFFER; };
template<> struct GPU_VkObjectType<VkImage>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_IMAGE; };
template<> struct GPU_VkObjectType<VkEvent>                      { static constexpr VkObjectType value = VK_OBJECT_TYPE_EVENT; };
template<> struct GPU_VkObjectType<VkQueryPool>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_QUERY_POOL; };
template<> struct GPU_VkObjectType<VkBufferView>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_BUFFER_VIEW; };
template<> struct GPU_VkObjectType<VkImageView>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_IMAGE_VIEW; };
template<> struct GPU_VkObjectType<VkShaderModule>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_SHADER_MODULE; };
template<> struct GPU_VkObjectType<VkPipelineCache>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_CACHE; };
template<> struct GPU_VkObjectType<VkPipelineLayout>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_LAYOUT; };
template<> struct GPU_VkObjectType<VkRenderPass>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_RENDER_PASS; };
template<> struct GPU_VkObjectType<VkPipeline>                   { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE; };
template<> struct GPU_VkObjectType<VkDescriptorSetLayout>        { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
template<> struct GPU_VkObjectType<VkSampler>                    { static constexpr VkObjectType value = VK_OBJECT_TYPE_SAMPLER; };
template<> struct GPU_VkObjectType<VkDescriptorPool>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };
template<> struct GPU_VkObjectType<VkDescriptorSet>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
template<> struct GPU_VkObjectType<VkFramebuffer>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_FRAMEBUFFER; };
template<> struct GPU_VkObjectType<VkCommandPool>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_COMMAND_POOL; };

template<> struct GPU_VkObjectType<VkSamplerYcbcrConversion>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; };
template<> struct GPU_VkObjectType<VkDescriptorUpdateTemplate>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; };
template<> struct GPU_VkObjectType<VkPrivateDataSlot>            { static constexpr VkObjectType value = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; };

template<> struct GPU_VkObjectType<VkSurfaceKHR>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_SURFACE_KHR; };
template<> struct GPU_VkObjectType<VkSwapchainKHR>               { static constexpr VkObjectType value = VK_OBJECT_TYPE_SWAPCHAIN_KHR; };
template<> struct GPU_VkObjectType<VkDisplayKHR>                 { static constexpr VkObjectType value = VK_OBJECT_TYPE_DISPLAY_KHR; };
template<> struct GPU_VkObjectType<VkDisplayModeKHR>             { static constexpr VkObjectType value = VK_OBJECT_TYPE_DISPLAY_MODE_KHR; };

template<> struct GPU_VkObjectType<VkDebugReportCallbackEXT>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; };
template<> struct GPU_VkObjectType<VkDebugUtilsMessengerEXT>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; };

template<> struct GPU_VkObjectType<VkVideoSessionKHR>            { static constexpr VkObjectType value = VK_OBJECT_TYPE_VIDEO_SESSION_KHR; };
template<> struct GPU_VkObjectType<VkVideoSessionParametersKHR>  { static constexpr VkObjectType value = VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; };

template<> struct GPU_VkObjectType<VkCuModuleNVX>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_CU_MODULE_NVX; };
template<> struct GPU_VkObjectType<VkCuFunctionNVX>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_CU_FUNCTION_NVX; };

template<> struct GPU_VkObjectType<VkAccelerationStructureKHR>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; };
template<> struct GPU_VkObjectType<VkValidationCacheEXT>         { static constexpr VkObjectType value = VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; };
template<> struct GPU_VkObjectType<VkAccelerationStructureNV>    { static constexpr VkObjectType value = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; };

template<> struct GPU_VkObjectType<VkPerformanceConfigurationINTEL> { static constexpr VkObjectType value = VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; };
template<> struct GPU_VkObjectType<VkDeferredOperationKHR>       { static constexpr VkObjectType value = VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; };

template<> struct GPU_VkObjectType<VkIndirectCommandsLayoutNV>   { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; };

template<> struct GPU_VkObjectType<VkMicromapEXT>                { static constexpr VkObjectType value = VK_OBJECT_TYPE_MICROMAP_EXT; };

template<> struct GPU_VkObjectType<VkTensorARM>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_TENSOR_ARM; };
template<> struct GPU_VkObjectType<VkTensorViewARM>              { static constexpr VkObjectType value = VK_OBJECT_TYPE_TENSOR_VIEW_ARM; };

template<> struct GPU_VkObjectType<VkOpticalFlowSessionNV>       { static constexpr VkObjectType value = VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; };

template<> struct GPU_VkObjectType<VkShaderEXT>                  { static constexpr VkObjectType value = VK_OBJECT_TYPE_SHADER_EXT; };
template<> struct GPU_VkObjectType<VkPipelineBinaryKHR>          { static constexpr VkObjectType value = VK_OBJECT_TYPE_PIPELINE_BINARY_KHR; };

template<> struct GPU_VkObjectType<VkDataGraphPipelineSessionARM>{ static constexpr VkObjectType value = VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM; };

template<> struct GPU_VkObjectType<VkExternalComputeQueueNV>     { static constexpr VkObjectType value = VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV; };

template<> struct GPU_VkObjectType<VkIndirectCommandsLayoutEXT>  { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT; };
template<> struct GPU_VkObjectType<VkIndirectExecutionSetEXT>    { static constexpr VkObjectType value = VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT; };

#define GPU_GET_VK_OBJECT_TYPE(x) \
     (GPU_VkObjectType<decltype(x)>::value)

#else

#define GPU_GET_VK_OBJECT_TYPE(x) \
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

PFN_vkDebugUtilsMessengerCallbackEXT GPU_GetVkDebugCallback(void);
void GPU_LogVkResultOnFailure(VkResult result, utf8str fnCall, SrcLoc loc);
void GPU_SetVkObjDebugName(const GPU_VkInstance* renderer, void* obj, VkObjectType objTy, utf8str fmtStr, FMT_Args fmtArgs);

#define GPU_VK_CHECKED_CALL(call) \
    GPU_LogVkResultOnFailure((call), UTF8STR(#call), SRC_LOC())

#define GPU_VK_SET_OBJ_DEBUG_NAME(renderer, obj, fmt, ...) \
    GPU_SetVkObjDebugName((renderer), (obj), GPU_GET_VK_OBJECT_TYPE(obj), UTF8STR(fmt), FMTARGS(__VA_ARGS__))

VkFormat GPU_BreakVkTextureFormat(GPU_TextureFormat);
GPU_TextureFormat GPU_MakeVkTextureFormat(VkFormat);
VkImageLayout GPU_BreakVkTextureLayout(GPU_TextureLayout);
VkPipelineStageFlags2 GPU_BreakVkBarrierStage(GPU_BarrierStage);
VkAccessFlags2 GPU_BreakVkBarrierAccess(GPU_BarrierAccess);
VkAttachmentLoadOp GPU_BreakVkLoadOp(GPU_LoadOp);
VkAttachmentStoreOp GPU_BreakVkStoreOp(GPU_StoreOp);
VkDescriptorType GPU_BreakVkProgramArgType(GPU_ProgramArgType);
VkShaderStageFlags GPU_BreakVkProgramStage(GPU_ProgramStageType);

Slice_(VkWriteDescriptorSet) GPU_BreakVkProgramArgsBindings(VkDescriptorSet, Slice_(GPU_ProgramArgBindingCfg), MEM_Allocator);

EXTERN_C_END
#endif
