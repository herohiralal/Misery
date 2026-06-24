#include <VulkanHeaders.h>

#if REN_VK && !MSR_IOS
    #define VOLK_IMPLEMENTATION 1
    #include <VulkanLoader.h>
    #undef VOLK_IMPLEMENTATION
#endif
