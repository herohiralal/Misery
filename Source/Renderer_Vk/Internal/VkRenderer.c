#include "VkPrivate.h"

#if REN_VK

// returns number of unique queues
static Slice_(VkDeviceQueueCreateInfo) REN_SelectVkQueueFamilies(VkPhysicalDevice physDev, VkSurfaceKHR surfaceToPresent, u32* gfxQueue, u32* presQueue)
{
    MSR_ASSERT(gfxQueue && presQueue && "gfxQueue and presQueue must not be null");

    *gfxQueue = U32_MAX; *presQueue = U32_MAX;

    // get all queue families
    Slice_(VkQueueFamilyProperties) queueFamilies;
    {
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, nil);
        queueFamilies = COL_NewSlice(VkQueueFamilyProperties, queueFamilyCount, true, MEM_temp);
        vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, queueFamilies.data);
        queueFamilies.count = (isize) queueFamilyCount;
    }

    b8 foundComputeSupport = false;
    for (isize i = 0; i < queueFamilies.count; i++)
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

    MSR_ASSERT(*gfxQueue != U32_MAX && *presQueue != U32_MAX && "Failed to find required queue families on physical device");

    // create queue create infos
    u32 queueCount = (*gfxQueue == *presQueue) ? 1 : 2;
    Slice_(VkDeviceQueueCreateInfo) queueCreateInfos = COL_NewSlice(VkDeviceQueueCreateInfo, queueCount, true, MEM_temp);

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

void REN_VkCreate(REN_Instance* outBaseInstance, REN_InstanceCfg cfg)
{
    #if !MSR_IOS
    {
        REN_VK_CHECKED_CALL(volkInitialize());
    }
    #endif

    MSR_ASSERT(!!outBaseInstance && "outBaseInstance can't be null");
    outBaseInstance->base.type = REN_GfxAPIType_Vk;
    REN_VkInstance* output = REN_ToVkInstance(outBaseInstance);
    MSR_ASSERT(output && "output must not be null");

    output->appHandle = cfg.appHandle;

    List_(cstring) enabledLayers = COL_NewList(cstring, 16, MEM_temp);

    u32 availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nil);
    Slice_(VkLayerProperties) availableLayers = COL_NewSlice(VkLayerProperties, availableLayerCount, true, MEM_temp);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data);
    availableLayers.count = (isize) availableLayerCount;

    b8 validationLayersEnabled = false;
    for (isize i = 0; i < availableLayers.count; i++)
    {
        VkLayerProperties* layer = &availableLayers.data[i];

        if (MSR_DBG && STR_Eq(STR_AliasCStr(layer->layerName), UTF8STR("VK_LAYER_KHRONOS_validation")))
        {
            validationLayersEnabled = true;
            COL_AppendToList(&enabledLayers, layer->layerName);
            LOG_Inf(VULKAN, "Found validation layers! Enabling...");
            continue;
        }

        LOG_Inf(VULKAN, "Skipped available layer: % (%).", FMT(layer->layerName), FMT(layer->description));
    }

    List_(cstring) enabledExtensions = COL_NewList(cstring, 16, MEM_temp);

    u32 avlblInstExtsCount = 0;
    vkEnumerateInstanceExtensionProperties(nil, &avlblInstExtsCount, nil);
    Slice_(VkExtensionProperties) avlblInstExts = COL_NewSlice(VkExtensionProperties, avlblInstExtsCount, true, MEM_temp);
    vkEnumerateInstanceExtensionProperties(nil, &avlblInstExtsCount, avlblInstExts.data);
    avlblInstExts.count = (isize) avlblInstExtsCount;

    for (isize i = 0; i < avlblInstExts.count; i++)
    {
        VkExtensionProperties* ext = &avlblInstExts.data[i];

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_SURFACE_EXTENSION_NAME)))
        {
            COL_AppendToList(&enabledExtensions, ext->extensionName);
            continue;
        }

        #if MSR_WINDOWS
            if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)))
            {
                COL_AppendToList(&enabledExtensions, ext->extensionName);
                continue;
            }
        #endif

        #if MSR_LINUX
            if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_XCB_SURFACE_EXTENSION_NAME)))
            {
                COL_AppendToList(&enabledExtensions, ext->extensionName);
                continue;
            }
        #endif

        #if MSR_ANDROID
            if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)))
            {
                COL_AppendToList(&enabledExtensions, ext->extensionName);
                continue;
            }
        #endif

        #if MSR_APPLE
            if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_METAL_SURFACE_EXTENSION_NAME)))
            {
                COL_AppendToList(&enabledExtensions, ext->extensionName);
                continue;
            }

            if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)))
            {
                COL_AppendToList(&enabledExtensions, ext->extensionName);
                continue;
            }
        #endif

        if (validationLayersEnabled && STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)))
        {
            COL_AppendToList(&enabledExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME)))
        {
            COL_AppendToList(&enabledExtensions, ext->extensionName);
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
            .pValues      = (VkBool32[]) {(VkBool32) MSR_DBG},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "validate_sync",
            .type         = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount   = 1,
            .pValues      = (VkBool32[]) {(VkBool32) MSR_DBG},
        },
        {
            .pLayerName   = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "thread_safety",
            .type         = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount   = 1,
            .pValues      = (VkBool32[]) {(VkBool32) MSR_DBG},
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

    REN_VK_CHECKED_CALL(vkCreateInstance(&(VkInstanceCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = 0 | (MSR_APPLE ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0),
        .pApplicationInfo = &(VkApplicationInfo)
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext              = nil,
            .pApplicationName   = STR_CloneToCStr(cfg.appName, MEM_temp),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "M_U_Z_E_N_T",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = VK_API_VERSION_1_3, // fuck it, we dyna-rendering fr this time
        },
        .enabledLayerCount = (u32) enabledLayers.count,
        .ppEnabledLayerNames = enabledLayers.data,
        .enabledExtensionCount = (u32) enabledExtensions.count,
        .ppEnabledExtensionNames = enabledExtensions.data,
        .pNext = &(VkLayerSettingsCreateInfoEXT)
        {
            .sType        = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
            .pNext        = nil,
            .settingCount = settingsCount,
            .pSettings    = settings,
        },
    }, nil, &output->instance));

    #if !MSR_IOS
    {
        volkLoadInstanceOnly(output->instance);
    }
    #endif

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
            .pfnUserCallback = REN_GetVkDebugCallback(),
            .pUserData       = output,
        };

        REN_VK_CHECKED_CALL(vkCreateDebugUtilsMessengerEXT(output->instance, &debugCreateInfo, nil, &output->debugMessenger));
    }

    {
        utf8str nameStrToUse = cfg.appName;
        if (sizeof(output->buffers.appName) < (usize) cfg.appName.count)
            nameStrToUse = STR_SubString(cfg.appName, 0, sizeof(output->buffers.appName));

        MEM_Copy(output->buffers.appName, nameStrToUse.data, nameStrToUse.count);
        output->appName = (utf8str) {.data = output->buffers.appName, .count = nameStrToUse.count};
    }

    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(output->instance, &deviceCount, nil);
    Slice_(VkPhysicalDevice) devices = COL_NewSlice(VkPhysicalDevice, deviceCount, true, MEM_temp);
    vkEnumeratePhysicalDevices(output->instance, &deviceCount, devices.data);
    devices.count = (isize) deviceCount;

    VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
    u32 selectedDeviceScore = 0;
    for (isize i = 0; i < devices.count; i++)
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
                deviceTyStr = UTF8STR("OTHER");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                deviceTyStr = UTF8STR("INTEGRATED_GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                deviceTyStr = UTF8STR("DISCRETE_GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                deviceTyStr = UTF8STR("VIRTUAL_GPU");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                deviceTyStr = UTF8STR("CPU");
                break;

            case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            default:
                deviceTyStr = UTF8STR("Unknown");
                break;
        }

        u32 score = 0;
        score += (MSR_DESKTOP && deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ? 1 : 0;
        score += 0 +
                deviceFeatures.features.samplerAnisotropy +
                deviceFeatures11.shaderDrawParameters +
                deviceFeatures12.descriptorIndexing +
                deviceFeatures12.shaderSampledImageArrayNonUniformIndexing +
                deviceFeatures12.descriptorBindingVariableDescriptorCount +
                deviceFeatures12.runtimeDescriptorArray +
                deviceFeatures12.bufferDeviceAddress +
                deviceFeatures12.timelineSemaphore +
                deviceFeatures13.synchronization2 +
                deviceFeatures13.dynamicRendering +
                meshFeatures.meshShader +
                meshFeatures.taskShader +
                descriptorBufferFeatures.descriptorBuffer +
                0;

        LOG_Inf(
            VULKAN,
            "Device: %. ty: %.\n"
            "\tsampler anisotropy:                              %.\n" // 1.0
            "\tshader draw parameters:                          %.\n" // 1.1
            "\tdescriptor indexing:                             %.\n" // 1.2
            "\tshader sampled image array non uniform indexing: %.\n"
            "\tdescriptor binding variable descriptor count:    %.\n"
            "\truntime descriptor array:                        %.\n"
            "\tbuffer device address:                           %.\n"
            "\ttimeline semaphore:                              %.\n"
            "\tsynchronization 2:                               %.\n" // 1.3
            "\tdynamic rendering:                               %.\n"
            "\tmesh shaders:                                    %.\n" // VK_EXT_mesh_shader
            "\ttask shaders:                                    %.\n"
            "\tdescriptor buffer:                               %.\n" // VK_EXT_descriptor_buffer
            "\t                                          SCORE: %.\n"
            "",

            FMT(deviceProperties.properties.deviceName),
            FMT(deviceTyStr),

            FMT_B8(!!deviceFeatures.features.samplerAnisotropy),
            FMT_B8(!!deviceFeatures11.shaderDrawParameters),
            FMT_B8(!!deviceFeatures12.descriptorIndexing),
            FMT_B8(!!deviceFeatures12.shaderSampledImageArrayNonUniformIndexing),
            FMT_B8(!!deviceFeatures12.descriptorBindingVariableDescriptorCount),
            FMT_B8(!!deviceFeatures12.runtimeDescriptorArray),
            FMT_B8(!!deviceFeatures12.bufferDeviceAddress),
            FMT_B8(!!deviceFeatures12.timelineSemaphore),
            FMT_B8(!!deviceFeatures13.synchronization2),
            FMT_B8(!!deviceFeatures13.dynamicRendering),
            FMT_B8(!!meshFeatures.meshShader),
            FMT_B8(!!meshFeatures.taskShader),
            FMT_B8(!!descriptorBufferFeatures.descriptorBuffer),
            FMT_U32(score, FMT_IntBase_Dec)
        );

        if (selectedDevice == VK_NULL_HANDLE || score > selectedDeviceScore)
        {
            selectedDevice = devices.data[i];

            output->meshShadersSupported = !!meshFeatures.meshShader;
            output->taskShadersSupported = !!meshFeatures.taskShader;
            output->descriptorBufferSupported = !!descriptorBufferFeatures.descriptorBuffer;

            selectedDeviceScore = score;
        }
    }

    MSR_ASSERT(selectedDevice != VK_NULL_HANDLE && "Failed to find suitable physical device");

    output->physicalDevice = selectedDevice;

    {
        VkFormatProperties formatProps = {0};
        vkGetPhysicalDeviceFormatProperties(selectedDevice, k_MZNT_Internal_PreferredVkColourAttchFormat, &formatProps);
        MSR_ASSERT((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) &&
            "Preferred colour format not supported as color attachment!");

        formatProps = (VkFormatProperties) {0};
        vkGetPhysicalDeviceFormatProperties(selectedDevice, k_MZNT_Internal_PreferredVkDepthAttchFormat, &formatProps);
        MSR_ASSERT((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
            "Preferred depth format not supported as depth attachment!");
    }

    VkSurfaceKHR tempSurfaceForQueueSelect = VK_NULL_HANDLE;
    #if MSR_WINDOWS
        HWND tempWindow = CreateWindowA("STATIC", "temp", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1, 1, nil, nil, (HINSTANCE) (uintptr_t) cfg.appHandle.handle, nil);
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
        {
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = APP_FromHandle(cfg.appHandle),
            .hwnd      = tempWindow,
        };

        REN_VK_CHECKED_CALL(vkCreateWin32SurfaceKHR(output->instance, &surfaceCreateInfo, nil, &tempSurfaceForQueueSelect));
    #elif MSR_ANDROID
        ANativeWindow* window = APP_FromHandle(cfg.appHandle)->window;
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .window = window,
        };

        REN_VK_CHECKED_CALL(vkCreateAndroidSurfaceKHR(output->instance, &surfaceCreateInfo, nil, &tempSurfaceForQueueSelect));
    #elif MSR_APPLE
        CAMetalLayer* tempLayer = [[CAMetalLayer alloc] init];

        REN_VK_CHECKED_CALL(vkCreateMetalSurfaceEXT(output->instance, &(VkMetalSurfaceCreateInfoEXT)
        {
            .sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
            .pLayer = tempLayer,
        }, nil, &tempSurfaceForQueueSelect));
    #elif MSR_LINUX
    xcb_connection_t* connection = nil;
    xcb_window_t window = 0;
    {
        int screenIndex = 0;
        connection = xcb_connect(NULL, &screenIndex);

        xcb_screen_iterator_t screenIter = xcb_setup_roots_iterator(xcb_get_setup(connection));
        while (screenIndex--)
            xcb_screen_next(&screenIter);

        xcb_screen_t* screen = screenIter.data;

        window = xcb_generate_id(connection);

        uint32_t values[] =
        {
            screen->black_pixel,
            XCB_EVENT_MASK_NO_EVENT,
        };

        xcb_create_window(
            connection,
            XCB_COPY_FROM_PARENT,
            window,
            screen->root,
            0, 0,
            1, 1,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
            values);

        xcb_flush(connection);

        REN_VK_CHECKED_CALL(vkCreateXcbSurfaceKHR(output->instance, &(VkXcbSurfaceCreateInfoKHR)
        {
            .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
            .connection = connection,
            .window     = window,
        }, nil, &tempSurfaceForQueueSelect));
    }
    #else
        #error "unimplemented"
    #endif

    Slice_(VkDeviceQueueCreateInfo) qcis = REN_SelectVkQueueFamilies(selectedDevice, tempSurfaceForQueueSelect, &(output->gfxQueueFamilyIndex), &(output->presQueueFamilyIndex));

    vkDestroySurfaceKHR(output->instance, tempSurfaceForQueueSelect, nil);
    #if MSR_WINDOWS
        DestroyWindow(tempWindow);
    #elif MSR_ANDROID
        // nothing to do
    #elif MSR_OSX
        [tempLayer release];
    #elif MSR_LINUX
        xcb_destroy_window(connection, window);
        xcb_disconnect(connection);
    #else
        #error "unimplemented"
    #endif

    List_(cstring) enabledDeviceExtensions = COL_NewList(cstring, 8, MEM_temp);
    COL_AppendAllToList(&enabledDeviceExtensions, SLICE(cstring,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    ));

    u32 avlblDevExtsCount = 0;
    vkEnumerateDeviceExtensionProperties(selectedDevice, nil, &avlblDevExtsCount, nil);
    Slice_(VkExtensionProperties) avlblDevExts = COL_NewSlice(VkExtensionProperties, avlblDevExtsCount, true, MEM_temp);
    vkEnumerateDeviceExtensionProperties(selectedDevice, nil, &avlblDevExtsCount, avlblDevExts.data);
    avlblDevExts.count = (isize) avlblDevExtsCount;

    b8 dedicatedAllocExtFound = false,
       getMemReq2ExtFound = false,
       bindMem2ExtFound = false,
       memBudgetExtFound = false,
       memPrioExtFound = false,
       #if MSR_WINDOWS
           extMemWin32ExtFound = false,
       #endif
       maintenance4ExtFound = false,
       maintenance5ExtFound = false;

    for (isize i = 0; i < avlblDevExts.count; i++)
    {
        VkExtensionProperties* ext = &avlblDevExts.data[i];

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)))
        {
            dedicatedAllocExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME)))
        {
            getMemReq2ExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)))
        {
            bindMem2ExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)))
        {
            memBudgetExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)))
        {
            memPrioExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        #if MSR_WINDOWS
        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME)))
        {
            extMemWin32ExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }
        #endif

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_MAINTENANCE_4_EXTENSION_NAME)))
        {
            maintenance4ExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_MAINTENANCE_5_EXTENSION_NAME)))
        {
            maintenance5ExtFound = true;
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME)))
        {
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_EXT_MESH_SHADER_EXTENSION_NAME)))
        {
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }

        if (STR_Eq(STR_AliasCStr(ext->extensionName), UTF8STR(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)))
        {
            COL_AppendToList(&enabledDeviceExtensions, ext->extensionName);
            continue;
        }
    }

    REN_VK_CHECKED_CALL(vkCreateDevice(selectedDevice, &(VkDeviceCreateInfo)
    {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount    = (u32) qcis.count,
        .pQueueCreateInfos       = qcis.data,
        .enabledExtensionCount   = (u32) enabledDeviceExtensions.count,
        .ppEnabledExtensionNames = enabledDeviceExtensions.data,
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

                        .meshShader                        = output->meshShadersSupported ? VK_TRUE : VK_FALSE,
                        .taskShader                        = output->taskShadersSupported ? VK_TRUE : VK_FALSE,
                    },

                    .shaderDrawParameters                  = VK_TRUE,
                },
                .descriptorIndexing                        = output->descriptorBufferSupported ? VK_TRUE : VK_FALSE,
                .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
                .descriptorBindingVariableDescriptorCount  = VK_TRUE,
                .runtimeDescriptorArray                    = VK_TRUE,
                .bufferDeviceAddress                       = VK_TRUE,
                .timelineSemaphore                         = VK_TRUE,
            },
            .synchronization2                              = VK_TRUE,
            .dynamicRendering                              = VK_TRUE,
        },
    }, nil, &output->device));

    #if !MSR_IOS
    {
        volkLoadDevice(output->device);
    }
    #endif

    REN_VK_SET_OBJ_DEBUG_NAME(output, output->instance, "%", FMT(output->appName));
    REN_VK_SET_OBJ_DEBUG_NAME(output, output->device, "%.device", FMT(output->appName));

    vkGetDeviceQueue2(output->device, &(VkDeviceQueueInfo2)
    {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
        .queueFamilyIndex = output->gfxQueueFamilyIndex,
        .queueIndex       = 0,
    },  &output->gfxQueue);

    REN_VK_SET_OBJ_DEBUG_NAME(output, output->gfxQueue, "%.gfxQueue", FMT(output->appName));

    vkGetDeviceQueue2(output->device, &(VkDeviceQueueInfo2)
    {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
        .queueFamilyIndex = output->presQueueFamilyIndex,
        .queueIndex       = 0,
    }, &output->presQueue);

    REN_VK_SET_OBJ_DEBUG_NAME(output, output->presQueue, "%.presQueue", FMT(output->appName));

    VmaAllocatorCreateFlags vmaFlags = 0
                                | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
                                | (dedicatedAllocExtFound && getMemReq2ExtFound ? VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT : 0)
                                | (bindMem2ExtFound ? VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT : 0)
                                | (memBudgetExtFound ? VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT : 0)
                                | (memPrioExtFound ? VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT : 0)
                                #if MSR_WINDOWS
                                | (extMemWin32ExtFound ? VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT : 0)
                                #endif
                                | (maintenance4ExtFound ? VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT : 0)
                                | (maintenance5ExtFound ? VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT : 0)
                                | 0;

    REN_VK_CHECKED_CALL(vmaCreateAllocator(&(VmaAllocatorCreateInfo)
    {
        .physicalDevice            = output->physicalDevice,
        .device                    = output->device,
        .instance                  = output->instance,
        .flags                     = vmaFlags,
        .pVulkanFunctions          = &(VmaVulkanFunctions)
        {
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr   = vkGetDeviceProcAddr,
        },
    }, &(output->vmaAllocator)));
}

void REN_VkWaitTillRendererIdle(REN_Instance* baseRenderer)
{
    const REN_VkInstance* renderer = REN_ToVkInstance(baseRenderer);

    if (!renderer || !(renderer->device)) return;

    REN_VK_CHECKED_CALL(vkDeviceWaitIdle(renderer->device));
}

void REN_VkDestroy(REN_Instance* baseRenderer)
{
    REN_VkInstance* renderer = REN_ToVkInstance(baseRenderer);

    if (!renderer) return;

    REN_VkWaitTillRendererIdle(baseRenderer);

    vmaDestroyAllocator(renderer->vmaAllocator);

    vkDestroyDevice(renderer->device, nil);

    if (renderer->debugMessenger != VK_NULL_HANDLE)
        vkDestroyDebugUtilsMessengerEXT(renderer->instance, renderer->debugMessenger, nil);

    vkDestroyInstance(renderer->instance, nil);

    #if !MSR_IOS
    {
        volkFinalize();
    }
    #endif
}

#endif
