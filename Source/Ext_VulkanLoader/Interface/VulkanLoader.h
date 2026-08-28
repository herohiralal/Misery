#pragma once
#include <VulkanHeaders.h>

MSR_SUPPRESS_WARN
#if GPU_VK && !MSR_IOS
    #include "ExtDeps/volk/volk.h"
#endif
MSR_UNSUPPRESS_WARN
