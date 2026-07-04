#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(ExtDeps_Core)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_RadDbgMarkup");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_MetalCPP");

    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:shell32.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:user32.lib");
    }
}
