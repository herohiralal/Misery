#include "VkFns.h"
#if MZNT_VULKAN

static VKAPI_ATTR VkBool32 VKAPI_CALL MZNT_Internal_VkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    PNSLR_LoggerLevel lvl = (PNSLR_LoggerLevel) {0};
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)        lvl = PNSLR_LoggerLevel_Error;
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) lvl = PNSLR_LoggerLevel_Warn;
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    lvl = PNSLR_LoggerLevel_Info;
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) lvl = PNSLR_LoggerLevel_Debug;
    else                                                                 lvl = PNSLR_LoggerLevel_Debug;

    utf8str general = (types & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) ? PNSLR_StringLiteral("[GENERAL]") : PNSLR_StringLiteral("");
    utf8str validation = (types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? PNSLR_StringLiteral("[VALIDATION]") : PNSLR_StringLiteral("");
    utf8str performance = (types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ? PNSLR_StringLiteral("[PERFORMANCE]") : PNSLR_StringLiteral("");

    PNSLR_Logf(lvl, PNSLR_StringLiteral("VK DEBUG UTILS: $. $ $ $"),
        PNSLR_FmtArgs(
            PNSLR_FmtCString((cstring) pCallbackData->pMessage),
            PNSLR_FmtString(general),
            PNSLR_FmtString(validation),
            PNSLR_FmtString(performance),
        ),
        PNSLR_GET_LOC()
    );

    return VK_FALSE;
}

PFN_vkDebugUtilsMessengerCallbackEXT MZNT_Internal_GetVkDebugCallback(void) { return MZNT_Internal_VkDebugCallback; }

void MZNT_Internal_LogVkResultOnFailure(VkResult result, utf8str fnCall, PNSLR_SourceCodeLocation loc)
{
    utf8str message = {0};
    // chatgpt generated
    switch (result)
    {
        case VK_SUCCESS: message = PNSLR_StringLiteral("Command successfully completed"); break;
        case VK_NOT_READY: message = PNSLR_StringLiteral("A fence or query has not yet completed"); break;
        case VK_TIMEOUT: message = PNSLR_StringLiteral("A wait operation has not completed in the specified time"); break;
        case VK_EVENT_SET: message = PNSLR_StringLiteral("An event is signaled"); break;
        case VK_EVENT_RESET: message = PNSLR_StringLiteral("An event is unsignaled"); break;
        case VK_INCOMPLETE: message = PNSLR_StringLiteral("A return array was too small for the result"); break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: message = PNSLR_StringLiteral("Host memory allocation has failed"); break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: message = PNSLR_StringLiteral("Device memory allocation has failed"); break;
        case VK_ERROR_INITIALIZATION_FAILED: message = PNSLR_StringLiteral("Initialization of an object could not be completed"); break;
        case VK_ERROR_DEVICE_LOST: message = PNSLR_StringLiteral("The logical or physical device has been lost"); break;
        case VK_ERROR_MEMORY_MAP_FAILED: message = PNSLR_StringLiteral("Mapping of a memory object has failed"); break;
        case VK_ERROR_LAYER_NOT_PRESENT: message = PNSLR_StringLiteral("A requested layer is not present or could not be loaded"); break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: message = PNSLR_StringLiteral("A requested extension is not supported"); break;
        case VK_ERROR_FEATURE_NOT_PRESENT: message = PNSLR_StringLiteral("A requested feature is not supported"); break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: message = PNSLR_StringLiteral("The requested version of Vulkan is not supported by the driver"); break;
        case VK_ERROR_TOO_MANY_OBJECTS: message = PNSLR_StringLiteral("Too many objects of the type have already been created"); break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: message = PNSLR_StringLiteral("A requested format is not supported on this device"); break;
        case VK_ERROR_FRAGMENTED_POOL: message = PNSLR_StringLiteral("A pool allocation has failed due to fragmentation"); break;
        case VK_ERROR_UNKNOWN: message = PNSLR_StringLiteral("An unknown error has occurred"); break;
        case VK_ERROR_OUT_OF_POOL_MEMORY: message = PNSLR_StringLiteral("A pool memory allocation has failed"); break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: message = PNSLR_StringLiteral("An external handle is not valid"); break;
        case VK_ERROR_FRAGMENTATION: message = PNSLR_StringLiteral("A descriptor pool creation has failed due to fragmentation"); break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: message = PNSLR_StringLiteral("A buffer creation or memory allocation failed due to invalid opaque capture address"); break;
        case VK_PIPELINE_COMPILE_REQUIRED: message = PNSLR_StringLiteral("Pipeline compilation required but not performed"); break;
        case VK_ERROR_NOT_PERMITTED: message = PNSLR_StringLiteral("Operation not permitted"); break;
        case VK_ERROR_SURFACE_LOST_KHR: message = PNSLR_StringLiteral("A surface is no longer available"); break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: message = PNSLR_StringLiteral("The requested window is already connected to another instance"); break;
        case VK_SUBOPTIMAL_KHR: message = PNSLR_StringLiteral("Swapchain no longer matches surface properties exactly, but is still usable"); break;
        case VK_ERROR_OUT_OF_DATE_KHR: message = PNSLR_StringLiteral("Swapchain is no longer compatible with the surface"); break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: message = PNSLR_StringLiteral("Display is incompatible with the requested mode"); break;
        case VK_ERROR_VALIDATION_FAILED_EXT: message = PNSLR_StringLiteral("Validation layer found an error"); break;
        case VK_ERROR_INVALID_SHADER_NV: message = PNSLR_StringLiteral("Invalid shader was provided to NV extension"); break;
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: message = PNSLR_StringLiteral("The requested image usage is not supported"); break;
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: message = PNSLR_StringLiteral("Video picture layout not supported"); break;
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: message = PNSLR_StringLiteral("Video profile operation not supported"); break;
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: message = PNSLR_StringLiteral("Video profile format not supported"); break;
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: message = PNSLR_StringLiteral("Video profile codec not supported"); break;
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: message = PNSLR_StringLiteral("Video Std version not supported"); break;
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: message = PNSLR_StringLiteral("DRM format modifier plane layout is invalid"); break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: message = PNSLR_StringLiteral("Full screen exclusive mode lost"); break;
        case VK_THREAD_IDLE_KHR: message = PNSLR_StringLiteral("A deferred operation is not complete but there is currently no work for this thread"); break;
        case VK_THREAD_DONE_KHR: message = PNSLR_StringLiteral("A deferred operation is not complete but there is no work remaining for this thread"); break;
        case VK_OPERATION_DEFERRED_KHR: message = PNSLR_StringLiteral("A deferred operation was requested and will be completed later"); break;
        case VK_OPERATION_NOT_DEFERRED_KHR: message = PNSLR_StringLiteral("A deferred operation was not deferred and has been completed"); break;
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: message = PNSLR_StringLiteral("Parameters for a video Std are invalid"); break;
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: message = PNSLR_StringLiteral("Compression resources are exhausted"); break;
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT: message = PNSLR_StringLiteral("Shader binary is incompatible"); break;
        case VK_PIPELINE_BINARY_MISSING_KHR: message = PNSLR_StringLiteral("Pipeline binary is missing"); break;
        case VK_ERROR_NOT_ENOUGH_SPACE_KHR: message = PNSLR_StringLiteral("Not enough space for pipeline binary cache"); break;
        case VK_RESULT_MAX_ENUM: message = PNSLR_StringLiteral("Max enum value, do not use"); break;
        default: message = PNSLR_StringLiteral("Unknown VkResult value"); break;
    }

    if (result != VK_SUCCESS)
    {
        PNSLR_LogEf(PNSLR_StringLiteral("Vk error: $ from $"),
                    PNSLR_FmtArgs(
                        PNSLR_FmtString(message),
                        PNSLR_FmtString(fnCall)
                    ),
                    loc);

        FORCE_DBG_TRAP;
    }
}

void MZNT_Internal_SetVkObjDebugName(const MZNT_VulkanRenderer* renderer, void* obj, VkObjectType objTy, utf8str fmtStr, PNSLR_ArraySlice(PNSLR_PrimitiveFmtOptions) fmtArgs, PNSLR_Allocator tempAllocator)
{
    if (!renderer->debugMessenger) return;

    vkSetDebugUtilsObjectNameEXT(renderer->device, &(VkDebugUtilsObjectNameInfoEXT)
    {
        .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext        = nil,
        .objectHandle = (u64) obj,
        .objectType   = objTy,
        .pObjectName  = PNSLR_FormatCString(fmtStr, fmtArgs, tempAllocator),
    });
}

#endif
