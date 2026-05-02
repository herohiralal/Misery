#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Panshilar)
{
    Brahma_Definition def = {"PNSLR_IMPLEMENTATION", ""};
    brahma_append_definition_to_paged_list(&library->internalDefinitions, def);
}
