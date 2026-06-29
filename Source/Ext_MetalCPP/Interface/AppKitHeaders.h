#pragma once

#include <__init.h>

MSR_SUPPRESS_WARN
#if MSR_APPLE
    #include <objc/runtime.h>
    #include <Foundation/Foundation.h>

    #ifdef __cplusplus
        #include <Foundation/Foundation.hpp>
    #endif

    #if MSR_OSX
        #include <AppKit/AppKit.h>

        #ifdef __cplusplus
            #include <AppKit/AppKit.hpp>
        #endif

        #include <Cocoa/Cocoa.h>
    #elif MSR_IOS
        #include <UIKit/UIKit.h>
    #endif
#endif
MSR_UNSUPPRESS_WARN
