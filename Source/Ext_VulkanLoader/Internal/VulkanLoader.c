#include <VulkanHeaders.h>

#if GPU_VK && !MSR_IOS && MSR_BUILD_DEPS
    #define VOLK_IMPLEMENTATION 1
    #include <VulkanLoader.h>
    #undef VOLK_IMPLEMENTATION
#endif
