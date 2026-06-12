#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(RendererBase)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Platform");
}
