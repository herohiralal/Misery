#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(ExtDeps_Platform)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Core");

    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "shell32.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "user32.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "gdi32.lib");
    }
    else if (package->platform == BRAHMA_PLATFORM_LINUX)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "xcb");
        brahma_append_string_to_paged_list(&library->externalDependencies, "xcb-keysyms");
        brahma_append_string_to_paged_list(&library->externalDependencies, "xcb-xinput");
        brahma_append_string_to_paged_list(&library->externalDependencies, "xcb-icccm");
    }
}
