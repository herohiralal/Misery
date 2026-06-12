#pragma once
#include <ExtDeps_Platform.h>

#ifndef REN_VK
    // TODO: add MoltenVK to support OSX/iOS
    #define REN_VK (MSR_WINDOWS || MSR_LINUX || MSR_ANDROID)
#endif

#ifndef REN_DX12
    #define REN_DX12 (MSR_WINDOWS || MSR_XSERIES)
#endif

#ifndef REN_MTL
    // TODO: implement
    #define REN_MTL 0 /*(MSR_APPLE)*/
#endif

MSR_SUPPRESS_WARN



MSR_UNSUPPRESS_WARN
