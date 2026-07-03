#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(MiseryDeps)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_DirectX12MemoryAllocator");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_MeshOpt");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_RadDbgMarkup");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_SpirvCross");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_SpirvReflect");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_STB_Image");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_VulkanLoader");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_VulkanMemoryAllocator");

    if (strcmp(package->name, "MiseryDependencies") == 0)
    {
        // do not depend on static libs, they're being built
    }
    else if (strcmp(package->name, "Misery") == 0)
    {
        const char* libPrefix = "lib";
        if (package->platform == BRAHMA_PLATFORM_WINDOWS) libPrefix = "";

        const char* extension = ".a";
        if (package->platform == BRAHMA_PLATFORM_WINDOWS) extension = ".lib";

        char* prebuiltDep = brahma_sprintf("relpath:Dependencies/%s-%s-rel/%sMiseryDependencies%s",
            BRAHMA_PLATFORM_NAMES[package->platform],
            BRAHMA_ARCHITECTURE_NAMES[package->architecture],
            libPrefix,
            extension);

        brahma_append_string_to_paged_list(&library->externalDependencies, prebuiltDep);
        // add dependency on static libs
    }
    else
    {
        library->error = brahma_sprintf("Unknown package '%s' for library '%s'.", package->name, library->name);
        return;
    }
}
