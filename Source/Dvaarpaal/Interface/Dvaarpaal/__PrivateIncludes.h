#ifndef DVRPL_PRIVATE_INLCUDES_H
#define DVRPL_PRIVATE_INLCUDES_H

#include "Panshilar/__PrivateIncludes.h"

PNSLR_SUPPRESS_WARN

#if PNSLR_WINDOWS
    #pragma comment(lib, "User32.lib")
    #pragma comment(lib, "Shell32.lib")
    #pragma comment(lib, "Gdi32.lib")
#endif

#if PNSLR_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <shellapi.h>
    #include <hidusage.h>
    #undef WIN32_LEAN_AND_MEAN
#endif

#if PNSLR_ANDROID
    #include <android_native_app_glue.h>
    #include <android/window.h>
#endif

#if PNSLR_APPLE && defined(__OBJC__)
    #include <Foundation/Foundation.h>

    #if PNSLR_OSX
        #include <Cocoa/Cocoa.h>
    #elif PNSLR_IOS
        #include <UIKit/UIKit.h>
    #endif

    #include <Metal/Metal.h>

#endif

PNSLR_UNSUPPRESS_WARN

#endif//DVRPL_PRIVATE_INCLUDES_H
