#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Renderer_Vk)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Platform");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Renderer_Base");

    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Renderer");
}
