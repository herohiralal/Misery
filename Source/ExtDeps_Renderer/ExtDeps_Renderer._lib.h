#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(ExtDeps_Renderer)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Platform");
}
