#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(ExtDeps_Core)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_RadDbgMarkup");

    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "shell32.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "user32.lib");
    }

    if (package->platform == BRAHMA_PLATFORM_OSX)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:Foundation");
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:AppKit");
    }

    if (package->platform == BRAHMA_PLATFORM_IOS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:Foundation");
        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:UIKit");
    }
}
