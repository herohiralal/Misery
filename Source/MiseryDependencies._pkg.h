#pragma once

#include "__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_PACKAGE(MiseryDependencies)
{
    package->primaryLibrary = "MiseryDeps";
    package->outputType = BRAHMA_PACKAGE_OUTPUT_TYPE_STATIC_LIBRARY;

    Brahma_String_KVP buildDepsDefine = {"MSR_BUILD_DEPS", "1"};
    brahma_append_definition_to_paged_list(&package->definitions, buildDepsDefine);
}
