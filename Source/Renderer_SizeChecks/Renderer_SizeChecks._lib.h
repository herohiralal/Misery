#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Renderer_SizeChecks)
{
    brahma_append_string_to_paged_list(&library->internalDependencies, "Core");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Platform");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Base");

    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Vk");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Dx12");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Mtl");
}
