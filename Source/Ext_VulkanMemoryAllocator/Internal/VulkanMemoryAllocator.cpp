#include <VulkanLoader.h>

#if REN_VK
    #define VMA_IMPLEMENTATION 1
    #include <VulkanMemoryAllocator.h>
    #undef VMA_IMPLEMENTATION
#endif
