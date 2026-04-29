#pragma once

#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(HelloWorld)
{
    brahma_append_string_to_paged_list(&library->internalDependencies, "Core");

    Brahma_Definition coolDefine = { "I_AM_SO_COOL", "1" };
    brahma_append_definition_to_paged_list(&library->interfaceDefinitions, coolDefine);
}
