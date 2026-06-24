#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_Vulkan)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Platform");

    if (package->platform == BRAHMA_PLATFORM_OSX || package->platform == BRAHMA_PLATFORM_IOS)
    {
        Brahma_String_KVP vkDylib = {"Dependencies/MoltenVK/osx/libvulkan.dylib", "libvulkan.dylib"};
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, vkDylib);

        brahma_append_string_to_paged_list(&library->externalDependencies, "framework:QuartzCore");
    }
}
