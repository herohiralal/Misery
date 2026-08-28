#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(GPU)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Platform");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "GPU_Base");

    brahma_append_string_to_paged_list(&library->internalDependencies, "ExtDeps_GPU");

    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_SizeChecks");
    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Vk");
    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Dx12");
    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Mtl");
}
