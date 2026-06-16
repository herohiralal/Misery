#pragma once
#include <__init.h>
#include <ExtDeps_Renderer.h>
#include <Renderer_Base/Renderer_Base.h>

EXTERN_C_BEGIN

REN_EXTEND_OBJECT(Vk, Instance,
    VkInstance       instance;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;
    u32              gfxQueueFamilyIndex;
    u32              presQueueFamilyIndex;
    VkQueue          gfxQueue;
    VkQueue          presQueue;

    VkDebugUtilsMessengerEXT debugMessenger;
    utf8str                  appName;

    VmaAllocator     vmaAllocator;
);

EXTERN_C_END
