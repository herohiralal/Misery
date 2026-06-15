#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(ExtDeps_Renderer)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Platform");

    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_DirectXShaderCompiler");

    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_DirectX12");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_DirectX12MemoryAllocator");

    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_Vulkan");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_VulkanLoader");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_VulkanMemoryAllocator");

    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_Metal");
}
