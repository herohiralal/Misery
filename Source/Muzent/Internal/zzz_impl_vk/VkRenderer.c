#include "VkFns.h"
#if MZNT_VULKAN

// returns number of unique queues
static PNSLR_ArraySlice(VkDeviceQueueCreateInfo) MZNT_Internal_SelectVkQueueFamilies(VkPhysicalDevice physDev, VkSurfaceKHR surfaceToPresent, u32* gfxQueue, u32* presQueue, PNSLR_Allocator tempAllocator)
{
    if (!gfxQueue || !presQueue) FORCE_DBG_TRAP;

    *gfxQueue = U32_MAX; *presQueue = U32_MAX;

    // get all queue families
    PNSLR_ArraySlice(VkQueueFamilyProperties) queueFamilies;
    {
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, nil);
        queueFamilies = PNSLR_MakeSlice(VkQueueFamilyProperties, queueFamilyCount, false, tempAllocator, PNSLR_GET_LOC(), nil);
        vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, queueFamilies.data);
        queueFamilies.count = (i64) queueFamilyCount;
    }

    b8 foundComputeSupport = false;
    for (i64 i = 0; i < queueFamilies.count; i++)
    {
        VkQueueFlags flags = queueFamilies.data[i].queueFlags;

        if (flags & VK_QUEUE_GRAPHICS_BIT)
        {
            b8 currentQueueSupportsCompute = !!(flags & VK_QUEUE_COMPUTE_BIT);
            if (*gfxQueue == U32_MAX || (!foundComputeSupport && currentQueueSupportsCompute))
            {
                *gfxQueue = (u32) i;
                foundComputeSupport = currentQueueSupportsCompute;
            }
        }

        VkBool32 supportsPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physDev, (u32) i, surfaceToPresent, &supportsPresent);
        if (supportsPresent && *presQueue == U32_MAX)
            *presQueue = (u32) i;

        if (*gfxQueue != U32_MAX && *presQueue != U32_MAX)
            break;
    }

    if (*gfxQueue == U32_MAX || *presQueue == U32_MAX)
    {
        PNSLR_LogE(PNSLR_StringLiteral("Failed to find required queue families on physical device"), PNSLR_GET_LOC());
        FORCE_DBG_TRAP;
    }

    // create queue create infos
    u32 queueCount = (*gfxQueue == *presQueue) ? 1 : 2;
    PNSLR_ArraySlice(VkDeviceQueueCreateInfo) queueCreateInfos = PNSLR_MakeSlice(VkDeviceQueueCreateInfo, queueCount, false, tempAllocator, PNSLR_GET_LOC(), nil);

    float queuePriority = 1.0f;
    queueCreateInfos.data[0] = (VkDeviceQueueCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = *gfxQueue,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    if (queueCount == 2)
    {
        queueCreateInfos.data[1] = (VkDeviceQueueCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = *presQueue,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
    }

    return queueCreateInfos;
}

static const VkFormat k_MZNT_Internal_PreferredVkColourAttchFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
static const VkFormat k_MZNT_Internal_PreferredVkDepthAttchFormat  = VK_FORMAT_D32_SFLOAT_S8_UINT;

MZNT_VulkanRenderer* MZNT_CreateRenderer_Vulkan(MZNT_RendererConfiguration config, PNSLR_Allocator tempAllocator)
{
    MZNT_INTERNAL_VK_CHECKED_CALL(volkInitialize());

    MZNT_VulkanRenderer* output = PNSLR_New(MZNT_VulkanRenderer, config.allocator, PNSLR_GET_LOC(), nil);
    if (!output) FORCE_DBG_TRAP;

    output->parent.type      = MZNT_RendererType_Vulkan;
    output->parent.allocator = config.allocator;
    output->parent.appHandle = config.appHandle;

    static const i32 k_MaxSupportedInstanceLayers = 16;
    cstring* enabledLayers = (cstring*) PNSLR_Allocate(tempAllocator, false, k_MaxSupportedInstanceLayers * sizeof(cstring), alignof(cstring), PNSLR_GET_LOC(), nil);
    u32 enabledLayersCount = 0;

    u32 availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nil);
    PNSLR_ArraySlice(VkLayerProperties) availableLayers = PNSLR_MakeSlice(VkLayerProperties, availableLayerCount, false, tempAllocator, PNSLR_GET_LOC(), nil);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data);
    availableLayers.count = (i64) availableLayerCount;

    b8 validationLayersEnabled = false;
    for (i64 i = 0; i < availableLayers.count; i++)
    {
        VkLayerProperties* layer = &availableLayers.data[i];

        if (PNSLR_DBG && PNSLR_AreCStringsEqual(layer->layerName, "VK_LAYER_KHRONOS_validation", 0))
        {
            validationLayersEnabled = true;
            enabledLayers[enabledLayersCount++] = &(layer->layerName[0]);
            PNSLR_LogI(PNSLR_StringLiteral("Found validation layers in vulkan. Enabling."), PNSLR_GET_LOC());
            continue;
        }

        PNSLR_LogIf(
            PNSLR_StringLiteral("Skipped, but available layer: $ ($)."),
            PNSLR_FmtArgs(
                PNSLR_FmtCString((cstring) &(layer->layerName[0])),
                PNSLR_FmtCString((cstring) &(layer->description[0]))
            ),
            PNSLR_GET_LOC()
        );
    }

    static const i32 k_MaxSupportedInstanceExtensions = 16;
    cstring* enabledExtensions = (cstring*) PNSLR_Allocate(tempAllocator, false, k_MaxSupportedInstanceExtensions * sizeof(cstring), alignof(cstring), PNSLR_GET_LOC(), nil);
    u32 enabledExtensionsCount = 0;

    u32 availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nil, &availableExtensionCount, nil);
    PNSLR_ArraySlice(VkExtensionProperties) availableExtensions = PNSLR_MakeSlice(VkExtensionProperties, availableExtensionCount, false, tempAllocator, PNSLR_GET_LOC(), nil);
    vkEnumerateInstanceExtensionProperties(nil, &availableExtensionCount, availableExtensions.data);
    availableExtensions.count = (i64) availableExtensionCount;

    for (i64 i = 0; i < availableExtensions.count; i++)
    {
        VkExtensionProperties* ext = &availableExtensions.data[i];

        if (PNSLR_AreCStringsEqual(ext->extensionName, VK_KHR_SURFACE_EXTENSION_NAME, 0))
        {
            enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
            continue;
        }

        #if PNSLR_WINDOWS
            if (PNSLR_AreCStringsEqual(ext->extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, 0))
            {
                enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
                continue;
            }
        #endif

        #if PNSLR_LINUX
            if (PNSLR_AreCStringsEqual(ext->extensionName, VK_KHR_XCB_SURFACE_EXTENSION_NAME, 0))
            {
                enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
                continue;
            }
        #endif

        #if PNSLR_ANDROID
            if (PNSLR_AreCStringsEqual(ext->extensionName, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, 0))
            {
                enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
                continue;
            }
        #endif

        #if PNSLR_APPLE
            if (PNSLR_AreCStringsEqual(ext->extensionName, VK_EXT_METAL_SURFACE_EXTENSION_NAME, 0))
            {
                enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
                continue;
            }
        #endif

        if (validationLayersEnabled && PNSLR_AreCStringsEqual(ext->extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, 0))
        {
            enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
            continue;
        }

        if (PNSLR_AreCStringsEqual(ext->extensionName, VK_EXT_LAYER_SETTINGS_EXTENSION_NAME, 0))
        {
            enabledExtensions[enabledExtensionsCount++] = &(ext->extensionName[0]);
            continue;
        }
    }

    VkLayerSettingEXT settings[] =
    {
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "validate_core",
            .type         = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount   = 1,
            .pValues      = (VkBool32[]) {(VkBool32) PNSLR_DBG},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "validate_sync",
            .type         = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount   = 1,
            .pValues      = (VkBool32[]) {(VkBool32) PNSLR_DBG},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "thread_safety",
            .type         = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount   = 1,
            .pValues      = (VkBool32[]) {(VkBool32) PNSLR_DBG},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "debug_action",
            .type         = VK_LAYER_SETTING_TYPE_STRING_EXT,
            .valueCount   = 1,
            .pValues      = (char*[]) {"VK_DBG_LAYER_ACTION_LOG_MSG"},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "report_flags",
            .type         = VK_LAYER_SETTING_TYPE_STRING_EXT,
            .valueCount   = 4,
            .pValues      = (char*[]) {"info", "warn", "perf", "error"},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "enable_message_limit",
            .type         = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount   = 1,
            .pValues      = (VkBool32[]) {VK_TRUE},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "duplicate_message_limit",
            .type         = VK_LAYER_SETTING_TYPE_UINT32_EXT,
            .valueCount   = 1,
            .pValues      = (u32[]) {5},
        },
    };

    i32 settingsCount = sizeof(settings) / sizeof(settings[0]);

    MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateInstance(&(VkInstanceCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = 0,
        .pApplicationInfo = &(VkApplicationInfo)
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext              = nil,
            .pApplicationName   = PNSLR_CStringFromString(config.appName, tempAllocator),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "M_U_Z_E_N_T",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = VK_API_VERSION_1_3, // fuck it, we dyna-rendering fr this time
        },
        .enabledLayerCount = enabledLayersCount,
        .ppEnabledLayerNames = (const char* const*) enabledLayers,
        .enabledExtensionCount = enabledExtensionsCount,
        .ppEnabledExtensionNames = (const char* const*) enabledExtensions,
        .pNext = &(VkLayerSettingsCreateInfoEXT)
        {
            .sType        = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
            .pNext        = nil,
            .settingCount = settingsCount,
            .pSettings    = settings,
        },
    }, nil, &output->instance));

    volkLoadInstanceOnly(output->instance);

    if (validationLayersEnabled)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nil,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = MZNT_Internal_GetVkDebugCallback(),
            .pUserData       = output,
        };

        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateDebugUtilsMessengerEXT(output->instance, &debugCreateInfo, nil, &output->debugMessenger));
    }

    output->appName = PNSLR_CloneString(config.appName, output->parent.allocator);

    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(output->instance, &deviceCount, nil);
    PNSLR_ArraySlice(VkPhysicalDevice) devices = PNSLR_MakeSlice(VkPhysicalDevice, deviceCount, false, tempAllocator, PNSLR_GET_LOC(), nil);
    vkEnumeratePhysicalDevices(output->instance, &deviceCount, devices.data);
    devices.count = (i64) deviceCount;

    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    for (i64 i = 0; i < devices.count; i++)
    {
        VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT};
        VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT, .pNext = &descriptorBufferFeatures};

        VkPhysicalDeviceVulkan13Features deviceFeatures13 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &meshFeatures};
        VkPhysicalDeviceVulkan12Features deviceFeatures12 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &deviceFeatures13};
        VkPhysicalDeviceVulkan11Features deviceFeatures11 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, .pNext = &deviceFeatures12};
        VkPhysicalDeviceFeatures2 deviceFeatures = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &deviceFeatures11};
        vkGetPhysicalDeviceFeatures2(devices.data[i], &deviceFeatures);

        VkPhysicalDeviceProperties2 deviceProperties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(devices.data[i], &deviceProperties);

        utf8str deviceTyStr = {0};
        switch (deviceProperties.properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                deviceTyStr = PNSLR_StringLiteral("OTHER");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                deviceTyStr = PNSLR_StringLiteral("INTEGRATED_GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                deviceTyStr = PNSLR_StringLiteral("DISCRETE_GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                deviceTyStr = PNSLR_StringLiteral("VIRTUAL_GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                deviceTyStr = PNSLR_StringLiteral("CPU");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            default:
                deviceTyStr = PNSLR_StringLiteral("Unknown");
                break;
        }

        PNSLR_LogIf(
            PNSLR_StringLiteral(
                "Device: $. ty: $.\n"
                "\tsampler anisotropy:                              $.\n" // 1.0
                "\tshader draw parameters:                          $.\n" // 1.1
                "\tdescriptor indexing:                             $.\n" // 1.2
                "\tshader sampled image array non uniform indexing: $.\n"
                "\tdescriptor binding variable descriptor count:    $.\n"
                "\truntime descriptor array:                        $.\n"
                "\tbuffer device address:                           $.\n"
                "\tsynchronization 2:                               $.\n" // 1.3
                "\tdynamic rendering:                               $.\n"
                "\tmesh shaders:                                    $.\n" // VK_EXT_mesh_shader
                "\ttask shaders:                                    $.\n"
                "\tdescriptor buffer:                               $.\n" // VK_EXT_descriptor_buffer
            ),
            PNSLR_FmtArgs(
                PNSLR_FmtCString(deviceProperties.properties.deviceName),
                PNSLR_FmtString(deviceTyStr),

                PNSLR_FmtB8(!!deviceFeatures.features.samplerAnisotropy),
                PNSLR_FmtB8(!!deviceFeatures11.shaderDrawParameters),
                PNSLR_FmtB8(!!deviceFeatures12.descriptorIndexing),
                PNSLR_FmtB8(!!deviceFeatures12.shaderSampledImageArrayNonUniformIndexing),
                PNSLR_FmtB8(!!deviceFeatures12.descriptorBindingVariableDescriptorCount),
                PNSLR_FmtB8(!!deviceFeatures12.runtimeDescriptorArray),
                PNSLR_FmtB8(!!deviceFeatures12.bufferDeviceAddress),
                PNSLR_FmtB8(!!deviceFeatures13.synchronization2),
                PNSLR_FmtB8(!!deviceFeatures13.dynamicRendering),
                PNSLR_FmtB8(!!meshFeatures.meshShader),
                PNSLR_FmtB8(!!meshFeatures.taskShader),
                PNSLR_FmtB8(!!descriptorBufferFeatures.descriptorBuffer)
            ),
            PNSLR_GET_LOC()
        );

        // prefer discrete gpu on desktop
        if ((!PNSLR_DESKTOP || deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            && !!deviceFeatures.features.samplerAnisotropy
            && !!deviceFeatures11.shaderDrawParameters
            && !!deviceFeatures12.descriptorIndexing
            && !!deviceFeatures12.shaderSampledImageArrayNonUniformIndexing
            && !!deviceFeatures12.descriptorBindingVariableDescriptorCount
            && !!deviceFeatures12.runtimeDescriptorArray
            && !!deviceFeatures12.bufferDeviceAddress
            && !!deviceFeatures13.synchronization2
            && !!deviceFeatures13.dynamicRendering
            && !!meshFeatures.meshShader
            && !!meshFeatures.taskShader
            && !!descriptorBufferFeatures.descriptorBuffer
            && selectedDevice == VK_NULL_HANDLE)
        {
            selectedDevice = devices.data[i];
        }
    }

    if (selectedDevice == VK_NULL_HANDLE)
    {
        PNSLR_LogE(PNSLR_StringLiteral("Failed to find suitable physical device!"), PNSLR_GET_LOC());
        FORCE_DBG_TRAP;
    }

    output->physicalDevice = selectedDevice;

    {
        VkFormatProperties formatProps = {0};
        vkGetPhysicalDeviceFormatProperties(selectedDevice, k_MZNT_Internal_PreferredVkColourAttchFormat, &formatProps);
        if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
        {
            PNSLR_LogE(PNSLR_StringLiteral("Preferred colour format not supported as color attachment!"), PNSLR_GET_LOC());
            FORCE_DBG_TRAP;
        }

        formatProps = (VkFormatProperties) {0};
        vkGetPhysicalDeviceFormatProperties(selectedDevice, k_MZNT_Internal_PreferredVkDepthAttchFormat, &formatProps);
        if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            PNSLR_LogE(PNSLR_StringLiteral("Preferred depth format not supported as depth attachment!"), PNSLR_GET_LOC());
            FORCE_DBG_TRAP;
        }
    }

    VkSurfaceKHR tempSurfaceForQueueSelect = VK_NULL_HANDLE;
    #if PNSLR_WINDOWS
        HWND tempWindow = CreateWindowA("STATIC", "temp", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1, 1, nil, nil, (HINSTANCE) (uintptr_t) config.appHandle.handle, nil);
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
        {
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = (HINSTANCE) (uintptr_t) config.appHandle.handle,
            .hwnd      = tempWindow,
        };

        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateWin32SurfaceKHR(output->instance, &surfaceCreateInfo, nil, &tempSurfaceForQueueSelect));
    #elif PNSLR_ANDROID
        ANativeWindow* window = ((struct android_app*) config.appHandle.handle)->window;
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .window = window,
        };

        MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateAndroidSurfaceKHR(output->instance, &surfaceCreateInfo, nil, &tempSurfaceForQueueSelect));
    #else
        #error "unimplemented"
    #endif

    PNSLR_ArraySlice(VkDeviceQueueCreateInfo) qcis = MZNT_Internal_SelectVkQueueFamilies(selectedDevice, tempSurfaceForQueueSelect, &(output->gfxQueueFamilyIndex), &(output->presQueueFamilyIndex), tempAllocator);

    vkDestroySurfaceKHR(output->instance, tempSurfaceForQueueSelect, nil);
    #if PNSLR_WINDOWS
        DestroyWindow(tempWindow);
    #elif PNSLR_ANDROID
        // nothing to do
    #else
        #error "unimplemented"
    #endif

    const char* enabledDeviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
    };

    u32 enabledDeviceExtensionCount = sizeof(enabledDeviceExtensions) / sizeof(char*);

    MZNT_INTERNAL_VK_CHECKED_CALL(vkCreateDevice(selectedDevice, &(VkDeviceCreateInfo)
    {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount    = (u32) qcis.count,
        .pQueueCreateInfos       = qcis.data,
        .enabledExtensionCount   = enabledDeviceExtensionCount,
        .ppEnabledExtensionNames = enabledDeviceExtensions,
        .pEnabledFeatures        = &(VkPhysicalDeviceFeatures)
        {
            .samplerAnisotropy   = VK_TRUE,
        },
        .pNext                   = &(VkPhysicalDeviceVulkan13Features)
        {
            .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext               = &(VkPhysicalDeviceVulkan12Features)
            {
                .sType           = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext           = &(VkPhysicalDeviceVulkan11Features)
                {
                    .sType       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                    .pNext       = &(VkPhysicalDeviceMeshShaderFeaturesEXT)
                    {
                        .sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
                        .pNext   = nil,

                        .meshShader                        = VK_TRUE,
                        .taskShader                        = VK_TRUE,
                    },

                    .shaderDrawParameters                  = VK_TRUE,
                },
                .descriptorIndexing                        = VK_TRUE,
                .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
                .descriptorBindingVariableDescriptorCount  = VK_TRUE,
                .runtimeDescriptorArray                    = VK_TRUE,
                .bufferDeviceAddress                       = VK_TRUE,
            },
            .synchronization2                              = VK_TRUE,
            .dynamicRendering                              = VK_TRUE,
        },
    }, nil, &output->device));

    volkLoadDevice(output->device);

    MZNT_Internal_SetVkObjDebugName(output, output->instance, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(output->instance),
        PNSLR_StringLiteral("$"), PNSLR_FmtArgs(PNSLR_FmtString(output->appName)), tempAllocator);

    MZNT_Internal_SetVkObjDebugName(output, output->device, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(output->device),
        PNSLR_StringLiteral("$.device"), PNSLR_FmtArgs(PNSLR_FmtString(output->appName)),
        tempAllocator);

    vkGetDeviceQueue2(output->device, &(VkDeviceQueueInfo2)
    {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
        .queueFamilyIndex = output->gfxQueueFamilyIndex,
        .queueIndex       = 0,
    },  &output->gfxQueue);

    MZNT_Internal_SetVkObjDebugName(output, output->gfxQueue, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(output->gfxQueue),
        PNSLR_StringLiteral("$.gfxQueue"), PNSLR_FmtArgs(PNSLR_FmtString(output->appName)),
        tempAllocator);

    vkGetDeviceQueue2(output->device, &(VkDeviceQueueInfo2)
    {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
        .queueFamilyIndex = output->presQueueFamilyIndex,
        .queueIndex       = 0,
    }, &output->presQueue);

    MZNT_Internal_SetVkObjDebugName(output, output->presQueue, MZNT_INTERNAL_GET_VK_OBJECT_TYPE(output->presQueue),
        PNSLR_StringLiteral("$.presQueue"), PNSLR_FmtArgs(PNSLR_FmtString(output->appName)),
        tempAllocator);

    MZNT_INTERNAL_VK_CHECKED_CALL(vmaCreateAllocator(&(VmaAllocatorCreateInfo)
    {
        .physicalDevice            = output->physicalDevice,
        .device                    = output->device,
        .instance                  = output->instance,
        .flags                     = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .pVulkanFunctions          = &(VmaVulkanFunctions)
        {
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr   = vkGetDeviceProcAddr,
        },
    }, &(output->vmaAllocator)));

    return output;
}

b8 MZNT_WaitTillRendererIdle_Vulkan(const MZNT_VulkanRenderer* renderer)
{
    if (!renderer || !(renderer->device)) return false;

    MZNT_INTERNAL_VK_CHECKED_CALL(vkDeviceWaitIdle(renderer->device));
    return true;
}

b8 MZNT_DestroyRenderer_Vulkan(MZNT_VulkanRenderer* renderer, PNSLR_Allocator tempAllocator)
{
    if (!renderer) return false;

    MZNT_WaitTillRendererIdle_Vulkan(renderer);

    vmaDestroyAllocator(renderer->vmaAllocator);

    vkDestroyDevice(renderer->device, nil);

    if (renderer->debugMessenger != VK_NULL_HANDLE)
        vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->debugMessenger, nil);

    vkDestroyInstance(renderer->instance, nil);

    PNSLR_FreeString(renderer->appName, renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    PNSLR_Delete(renderer, renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    volkFinalize();

    return true;
}

#endif
