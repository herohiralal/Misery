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

    bool found = false;
    bool buildDeps = false;
    for (size_t i = 0; i < package->definitions.count; i++)
    {
        Brahma_String_KVP* definition = brahma_index_definition_paged_list(&package->definitions, i);
        if (strcmp(definition->key, "MSR_BUILD_DEPS") == 0)
        {
            found = true;

            bool isTrue = definition->value && definition->value[0] == '1' && definition->value[1] == '\0';
            bool isFalse = definition->value && definition->value[0] == '0' && definition->value[1] == '\0';
            if (!isTrue && !isFalse)
            {
                library->error = brahma_sprintf("Invalid value for definition 'MSR_BUILD_DEPS' in package '%s'. Expected '1' or '0', got '%s'.",
                    package->name, definition->value ? definition->value : "null");
                return;
            }

            if (isTrue && isFalse)
            {
                library->error = brahma_sprintf("Conflicting values for definition 'MSR_BUILD_DEPS' in package '%s'. Expected '1' or '0', got both.",
                    package->name);
                return;
            }

            buildDeps = isTrue;
        }
    }

    if (!found)
    {
        library->error = brahma_sprintf("Definition 'MSR_BUILD_DEPS' not found in package '%s'. Expected '1' or '0'.",
            package->name);
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

        char* prebuiltDep = brahma_sprintf("relpath:Dependencies/%s-%s-rel/%sMiseryDependencies%s",
            BRAHMA_PLATFORM_NAMES[package->platform],
            BRAHMA_ARCHITECTURE_NAMES[package->architecture],
            libPrefix,
            extension);

        brahma_append_string_to_paged_list(&library->externalDependencies, prebuiltDep);
    }
}
