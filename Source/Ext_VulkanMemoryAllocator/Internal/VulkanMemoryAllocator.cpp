#include <VulkanLoader.h>

#if REN_VK
    MSR_SUPPRESS_WARN
    #define VMA_IMPLEMENTATION 1
    #include <VulkanMemoryAllocator.h>
    #undef VMA_IMPLEMENTATION
    MSR_UNSUPPRESS_WARN
#endif
