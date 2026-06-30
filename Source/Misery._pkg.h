#pragma once

#include "__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_PACKAGE(Misery)
{
    package->primaryLibrary = "EntryPoint";

    // TODO: implement
    Brahma_String_KVP disableMtl = {"REN_MTL", "0"};
    brahma_append_definition_to_paged_list(&package->definitions, disableMtl);
}
