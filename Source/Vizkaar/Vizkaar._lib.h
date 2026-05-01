#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Vizkaar)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Panshilar");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Muzent");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Dvaarpaal");

    Brahma_Definition def = {"VZKR_IMPLEMENTATION", ""};
    brahma_append_definition_to_paged_list(&library->internalDefinitions, def);
}
