#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_DirectX12)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Platform");

    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "dxguid.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "d3d12.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "dxgi.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "d3dcompiler.lib");
    }
}
