#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Core)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");

    Brahma_String_KVP natvis = {"Natvis/Core.natvis", "Natvis/Core.natvis"};
    brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, natvis);
}
