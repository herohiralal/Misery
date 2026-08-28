#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(GPU_SizeChecks)
{
    brahma_append_string_to_paged_list(&library->internalDependencies, "Core");
    brahma_append_string_to_paged_list(&library->internalDependencies, "Platform");
    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Base");

    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Vk");
    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Dx12");
    brahma_append_string_to_paged_list(&library->internalDependencies, "GPU_Mtl");
}
