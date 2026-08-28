#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(MiseryEntry)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Platform");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "GPU");

    // pre-built dependencies
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "MiseryDeps");
}
