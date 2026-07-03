#pragma once

#include "__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_PACKAGE(Misery)
{
    package->primaryLibrary = "EntryPoint";

    Brahma_String_KVP buildDepsDefine = {"MSR_BUILD_DEPS", "0"};
    brahma_append_definition_to_paged_list(&package->definitions, buildDepsDefine);
}
