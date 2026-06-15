#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Renderer)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Platform");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Renderer_Base");

    brahma_append_string_to_paged_list(&library->internalDependencies, "ExtDeps_Renderer");

    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_SizeChecks");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Null");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Vk");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Dx12");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Renderer_Mtl");
}
