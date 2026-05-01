#ifndef MZNT_SHADER_COMPILER_H // ==================================================
#define MZNT_SHADER_COMPILER_H
#include "__Prelude.h"
#include "Renderer.h"
EXTERN_C_BEGIN

/**
 * The configuration to use when constructing the shader compiler.
 */
typedef struct MZNT_ShaderCompilerConfiguration
{
    PNSLR_Path      libSearchDir;
    PNSLR_Allocator allocator;
} MZNT_ShaderCompilerConfiguration;

/**
 * Handle to the shader compiler. This compiler can convert shader source files into corresponding
 * bytecode, based on the renderer.
 * - Vulkan: HLSL -> SPIR-V
 * - D3D12:  HLSL -> DXIL
 * - Metal:  HLSL -> SPIR-V -> MSL
 *
 * The ideal multithreading pattern for using this is creating one compiler per thread.
 */
typedef struct MZNT_ShaderCompiler
{
    rawptr handle;
} MZNT_ShaderCompiler;

/**
 * Create an instance of the shader compiler.
 * Returns zero-value, if any part of the process fails.
 */
MZNT_ShaderCompiler MZNT_CreateShaderCompiler(MZNT_ShaderCompilerConfiguration cfg, PNSLR_Allocator tempAllocator);

/**
 * Destroy the instance of the shader compiler.
 */
void MZNT_DestroyShaderCompiler(MZNT_ShaderCompiler compiler, PNSLR_Allocator tempAllocator);

/**
 * The options to be used when compiling a shader into its corresponding bytecode.
 * - `allocator`     - the allocator to use when allocating the bytecode output.
 * - `tempAllocator` - the allocator to use for intermediate collections when compiling the bytecode.
 * - `rendererType`  - the rendering api that this bytecode will be used with.
 * - `shaderType`    - the type of shader this is (vert, frag, etc.)
 * - `entryPoint`    - (optional) "main" by default, but can specify a custom entry point.
 */
typedef struct MZNT_ShaderCompilationOptions
{
    PNSLR_Allocator   allocator;
    PNSLR_Allocator   tempAllocator;
    MZNT_RendererType rendererType;
    MZNT_ShaderType   shaderType;
    utf8str           entryPoint;
} MZNT_ShaderCompilationOptions;

/**
 * Compile an input HLSL shader, into its corresponding bytecode.
 */
b8 CompileShader(MZNT_ShaderCompiler compiler, PNSLR_Path file, PNSLR_ArraySlice(u8)* output, MZNT_ShaderCompilationOptions options);

EXTERN_C_END
#endif // MZNT_SHADER_COMPILER_H ===================================================
