#include "VkPrivate.h"

#if REN_VK

static VKAPI_ATTR VkBool32 VKAPI_CALL REN_VkDbgCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    u8 tagsBufferBuffer[50] = {0};
    List_(u8) tagsBuffer = (List_(u8))
    {
        .data = tagsBufferBuffer,
        .count = 0,
        .capacity = sizeof(tagsBufferBuffer),
        .allocator = (MEM_Allocator) {0},
    };

    if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        COL_AppendAllToList(&tagsBuffer, UTF8STR("[GENERAL] "));
    if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        COL_AppendAllToList(&tagsBuffer, UTF8STR("[VALIDATION] "));
    if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        COL_AppendAllToList(&tagsBuffer, UTF8STR("[PERFORMANCE] "));

    if (tagsBuffer.count)
        tagsBuffer.count -= 1; // remove trailing space

    cstring msg = (cstring) pCallbackData->pMessage;
    if (false) { }
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        LOG_Err(VULKAN, "% %", FMT(tagsBuffer.slice), FMT(msg));
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOG_Wrn(VULKAN, "% %", FMT(tagsBuffer.slice), FMT(msg));
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        LOG_Inf(VULKAN, "% %", FMT(tagsBuffer.slice), FMT(msg));
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        LOG_Dbg(VULKAN, "% %", FMT(tagsBuffer.slice), FMT(msg));
    else
        LOG_Dbg(VULKAN, "% %", FMT(tagsBuffer.slice), FMT(msg));

    return VK_FALSE;
}

PFN_vkDebugUtilsMessengerCallbackEXT REN_GetVkDebugCallback(void) { return REN_VkDbgCallback; }

void REN_LogVkResultOnFailure(VkResult result, utf8str fnCall, SrcLoc loc)
{
    if (result == VK_SUCCESS)
        return;

    utf8str message = {0};
    // chatgpt generated
    switch (result)
    {
        case VK_SUCCESS: message = UTF8STR("Command successfully completed"); break;
        case VK_NOT_READY: message = UTF8STR("A fence or query has not yet completed"); break;
        case VK_TIMEOUT: message = UTF8STR("A wait operation has not completed in the specified time"); break;
        case VK_EVENT_SET: message = UTF8STR("An event is signaled"); break;
        case VK_EVENT_RESET: message = UTF8STR("An event is unsignaled"); break;
        case VK_INCOMPLETE: message = UTF8STR("A return array was too small for the result"); break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: message = UTF8STR("Host memory allocation has failed"); break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: message = UTF8STR("Device memory allocation has failed"); break;
        case VK_ERROR_INITIALIZATION_FAILED: message = UTF8STR("Initialization of an object could not be completed"); break;
        case VK_ERROR_DEVICE_LOST: message = UTF8STR("The logical or physical device has been lost"); break;
        case VK_ERROR_MEMORY_MAP_FAILED: message = UTF8STR("Mapping of a memory object has failed"); break;
        case VK_ERROR_LAYER_NOT_PRESENT: message = UTF8STR("A requested layer is not present or could not be loaded"); break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: message = UTF8STR("A requested extension is not supported"); break;
        case VK_ERROR_FEATURE_NOT_PRESENT: message = UTF8STR("A requested feature is not supported"); break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: message = UTF8STR("The requested version of Vulkan is not supported by the driver"); break;
        case VK_ERROR_TOO_MANY_OBJECTS: message = UTF8STR("Too many objects of the type have already been created"); break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: message = UTF8STR("A requested format is not supported on this device"); break;
        case VK_ERROR_FRAGMENTED_POOL: message = UTF8STR("A pool allocation has failed due to fragmentation"); break;
        case VK_ERROR_UNKNOWN: message = UTF8STR("An unknown error has occurred"); break;
        case VK_ERROR_OUT_OF_POOL_MEMORY: message = UTF8STR("A pool memory allocation has failed"); break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: message = UTF8STR("An external handle is not valid"); break;
        case VK_ERROR_FRAGMENTATION: message = UTF8STR("A descriptor pool creation has failed due to fragmentation"); break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: message = UTF8STR("A buffer creation or memory allocation failed due to invalid opaque capture address"); break;
        case VK_PIPELINE_COMPILE_REQUIRED: message = UTF8STR("Pipeline compilation required but not performed"); break;
        case VK_ERROR_NOT_PERMITTED: message = UTF8STR("Operation not permitted"); break;
        case VK_ERROR_SURFACE_LOST_KHR: message = UTF8STR("A surface is no longer available"); break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: message = UTF8STR("The requested window is already connected to another instance"); break;
        case VK_SUBOPTIMAL_KHR: message = UTF8STR("Swapchain no longer matches surface properties exactly, but is still usable"); break;
        case VK_ERROR_OUT_OF_DATE_KHR: message = UTF8STR("Swapchain is no longer compatible with the surface"); break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: message = UTF8STR("Display is incompatible with the requested mode"); break;
        case VK_ERROR_VALIDATION_FAILED_EXT: message = UTF8STR("Validation layer found an error"); break;
        case VK_ERROR_INVALID_SHADER_NV: message = UTF8STR("Invalid shader was provided to NV extension"); break;
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: message = UTF8STR("The requested image usage is not supported"); break;
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: message = UTF8STR("Video picture layout not supported"); break;
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: message = UTF8STR("Video profile operation not supported"); break;
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: message = UTF8STR("Video profile format not supported"); break;
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: message = UTF8STR("Video profile codec not supported"); break;
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: message = UTF8STR("Video Std version not supported"); break;
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: message = UTF8STR("DRM format modifier plane layout is invalid"); break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: message = UTF8STR("Full screen exclusive mode lost"); break;
        case VK_THREAD_IDLE_KHR: message = UTF8STR("A deferred operation is not complete but there is currently no work for this thread"); break;
        case VK_THREAD_DONE_KHR: message = UTF8STR("A deferred operation is not complete but there is no work remaining for this thread"); break;
        case VK_OPERATION_DEFERRED_KHR: message = UTF8STR("A deferred operation was requested and will be completed later"); break;
        case VK_OPERATION_NOT_DEFERRED_KHR: message = UTF8STR("A deferred operation was not deferred and has been completed"); break;
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: message = UTF8STR("Parameters for a video Std are invalid"); break;
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: message = UTF8STR("Compression resources are exhausted"); break;
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT: message = UTF8STR("Shader binary is incompatible"); break;
        case VK_PIPELINE_BINARY_MISSING_KHR: message = UTF8STR("Pipeline binary is missing"); break;
        case VK_ERROR_NOT_ENOUGH_SPACE_KHR: message = UTF8STR("Not enough space for pipeline binary cache"); break;
        case VK_RESULT_MAX_ENUM: message = UTF8STR("Max enum value, do not use"); break;
        default: message = UTF8STR("Unknown VkResult value"); break;
    }

    LOG_Internal_AddEntry(&(LOG_Internal_Entry)
    {
        .lvl     = LOG_Lvl_Error,
        .cat[0]  = '-',
        .cat[1]  = 'V',
        .cat[2]  = 'U',
        .cat[3]  = 'L',
        .cat[4]  = 'K',
        .cat[5]  = 'A',
        .cat[6]  = 'N',
        .msg     = UTF8STR("ERROR: % from %"),
        .fmtArgs = FMTARGS(FMT(message), FMT(fnCall)),
        .loc     = loc,
    });

    MSR_ASSERT(false && "Vk error");
}

void REN_SetVkObjDebugName(const REN_VkInstance* renderer, void* obj, VkObjectType objTy, utf8str fmtStr, FMT_Args fmtArgs)
{
    if (!renderer->debugMessenger) return;

    vkSetDebugUtilsObjectNameEXT(renderer->device, &(VkDebugUtilsObjectNameInfoEXT)
    {
        .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext        = nil,
        .objectHandle = (u64) obj,
        .objectType   = objTy,
        .pObjectName  = FMT_CAPrintf_(MEM_temp, fmtStr, fmtArgs),
    });
}

#endif
