#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Panshilar)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Citrin");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "raddbg");

    Brahma_Definition def = {"PNSLR_IMPLEMENTATION", ""};
    brahma_append_definition_to_paged_list(&library->internalDefinitions, def);
}
