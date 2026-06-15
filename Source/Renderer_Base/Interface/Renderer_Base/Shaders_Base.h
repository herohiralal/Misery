#pragma once
#include <Core/Core.h>
#include <Platform/Platform.h>

EXTERN_C_BEGIN

/**
 * Defines the types of shaders that are supported by this library.
 */
typedef u8 SHD_Type;
enum SHD_Types
{
    SHD_Ty_Unknown,
    SHD_Ty_Compute,
    SHD_Ty_Task,
    SHD_Ty_Mesh,
    SHD_Ty_Vertex,
    SHD_Ty_Fragment,
};

/**
 * Represents a shader bytecode blob.
 * - On Vulkan, this is SPIR-V bytecode.
 * - On D3D12, this is DXIL bytecode.
 * - On Metal, this is MSL source code.
 */
typedef struct
{
    SHD_Type type;
    MEM_Allocator allocator;
    Slice_(u8) byteCode;
    utf8str entryPoint;
} SHD_ByteCode;

EXTERN_C_END
