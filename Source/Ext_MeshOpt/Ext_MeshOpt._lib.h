#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_MeshOpt)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
}
