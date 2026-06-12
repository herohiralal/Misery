#include <VulkanHeaders.h>

#if REN_VK
    #define VOLK_IMPLEMENTATION 1
    #include <VulkanLoader.h>
    #undef VOLK_IMPLEMENTATION
#endif
