#include <__init.h>

#if MSR_APPLE
#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

MSR_SUPPRESS_WARN

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#if MSR_OSX
    #include <AppKit/AppKit.hpp>
#endif
#include <MetalKit/MetalKit.hpp>

MSR_UNSUPPRESS_WARN

#endif
