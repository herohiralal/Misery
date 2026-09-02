#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_TinyGLTF)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Ext_STB_Image");

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

    // including this to define file-system & allocator callbacks
    if (!buildDeps)
        brahma_append_string_to_paged_list(&library->interfaceDependencies, "Core");
}
