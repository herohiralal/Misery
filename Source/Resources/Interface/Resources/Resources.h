#pragma once
#include <__init.h>
#include <Core/Core.h>

/**
 * This is the identifier of a resource.
 * It is used to refer to a resource that may or may not be loaded in memory.
 *
 * These are supposed to be serialized to disk, and so they are expected
 * to be stable across different runs of the program, and even across different machines.
 */
typedef struct
{
    u64 val[2];
} RES_Id;

/**
 * This is the handle of a resource.
 * It is used to refer to a resource that is loaded in memory.
 *
 * These are not expected to be stable across different runs of the program.
 *
 * A resource handle will be considered invalid if the corresponding
 * resource is unloaded from memory, or due to an orphaned reference.
 */
typedef struct
{
    u64 val;
} RES_Handle;

typedef struct
{
    utf8str name;
} RES_Importer;
