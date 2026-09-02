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
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_TinyGLTF");

    const char* msrBuildDeps = NULL;
    if (!brahma_find_in_definitions(&package->definitions, "MSR_BUILD_DEPS", &msrBuildDeps))
    {
        library->error = brahma_sprintf("Definition 'MSR_BUILD_DEPS' not found in package '%s'. Expected '1' or '0'.",
            package->name);
        return;
    }

    bool buildDeps = false;
    if (!brahma_bool_from_string(msrBuildDeps, &buildDeps))
    {
        library->error = brahma_sprintf("Invalid value for definition 'MSR_BUILD_DEPS' in package '%s'. Expected '1' or '0', got '%s'.",
            package->name, msrBuildDeps ? msrBuildDeps : "null");
        return;
    }

    if (buildDeps)
    {
        // do not link with static libs, build them from source instead
    }
    else
    {
        // add prebuilt static library dependency

        const char* libPrefix = "lib";
        if (package->platform == BRAHMA_PLATFORM_WINDOWS) libPrefix = "";

        const char* extension = ".a";
        if (package->platform == BRAHMA_PLATFORM_WINDOWS) extension = ".lib";

        char* prebuiltDep = brahma_sprintf("relpath:Dependencies/%s-%s-%s/%sMiseryDependencies%s",
            BRAHMA_PLATFORM_NAMES[package->platform],
            BRAHMA_ARCHITECTURE_NAMES[package->architecture],
            (package->includeDebugInfo ? "dbg" : "rel"),
            libPrefix,
            extension);

        brahma_append_string_to_paged_list(&library->externalDependencies, prebuiltDep);
    }
}
