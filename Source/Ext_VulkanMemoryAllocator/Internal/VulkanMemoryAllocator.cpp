#include <VulkanLoader.h>

#if GPU_VK && MSR_BUILD_DEPS
    MSR_SUPPRESS_WARN
    #define VMA_IMPLEMENTATION 1
    #include <VulkanMemoryAllocator.h>
    #undef VMA_IMPLEMENTATION
    MSR_UNSUPPRESS_WARN
#endif
