#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_MetalCPP)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");

    if (package->platform == BRAHMA_PLATFORM_OSX || package->platform == BRAHMA_PLATFORM_IOS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:Foundation");
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:Metal");
    }

    if (package->platform == BRAHMA_PLATFORM_OSX)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:AppKit");
    }

    if (package->platform == BRAHMA_PLATFORM_IOS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:UIKit");
    }
}
