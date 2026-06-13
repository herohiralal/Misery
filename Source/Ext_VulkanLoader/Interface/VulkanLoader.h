#pragma once
#include <VulkanHeaders.h>

MSR_SUPPRESS_WARN
#if REN_VK && !MSR_APPLE
    #include "ExtDeps/volk/volk.h"
#endif
MSR_UNSUPPRESS_WARN
