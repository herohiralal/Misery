#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Muzent)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Panshilar");

    Brahma_Definition def = {"MZNT_IMPLEMENTATION", ""};
    brahma_append_definition_to_paged_list(&library->internalDefinitions, def);
}
