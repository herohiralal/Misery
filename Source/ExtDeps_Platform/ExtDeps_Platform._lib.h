#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(ExtDeps_Platform)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Core");

    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:shell32.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:user32.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:gdi32.lib");
    }
    else if (package->platform == BRAHMA_PLATFORM_LINUX)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:xcb");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:xcb-keysyms");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:xcb-xinput");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:xcb-icccm");
    }
}
