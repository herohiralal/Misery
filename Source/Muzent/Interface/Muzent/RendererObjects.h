#ifndef MZNT_TYPES_H // ============================================================
#define MZNT_TYPES_H
#include "__Prelude.h"
EXTERN_C_BEGIN

/**
 * Represents an opaque handle to the application instance.
 * Matches Dvaarpaal's app handle.
 * - On Windows, this is an HINSTANCE.
 * - On OSX, this is an NSApplication*.
 * - On Android, this is a struct android_app*.
 */
typedef struct MZNT_AppHandle
{
    u64 handle;
} MZNT_AppHandle;

/**
 * Represents an opaque handle to a window.
 * Matches Dvaarpaal's window handle.
 * - On Windows, this is an HWND.
 * - On OSX, this is an NSWindow*.
 * - On Android, this is an ANativeWindow*.
 */
typedef struct MZNT_WindowHandle
{
    u64 handle;
} MZNT_WindowHandle;

/**
 * Defines the available Renderer types.
 */
ENUM_START(MZNT_RendererType, u8)
    #define MZNT_RendererType_Null      ((MZNT_RendererType) 0)
    #define MZNT_RendererType_Vulkan    ((MZNT_RendererType) 1)
    #define MZNT_RendererType_DirectX12 ((MZNT_RendererType) 2)
    #define MZNT_RendererType_Metal     ((MZNT_RendererType) 3)
ENUM_END

/**
 * The main renderer interface.
 * This is the main object that represents the renderer instance. While a process may have
 * multiple renderers, it's more common to have just one. The renderer is used as the primary
 * entry point for creating other renderer objects.
 */
typedef struct MZNT_Renderer
{
    MZNT_RendererType type;
    PNSLR_Allocator   allocator;
    MZNT_AppHandle    appHandle;
} MZNT_Renderer;

/**
 * A command buffer for recording rendering commands.
 * Modern graphics APIs often require command buffers that are allocated from the renderer,
 * and get submitted to the GPU for execution.
 */
typedef struct MZNT_RendererCommandBuffer
{
    MZNT_RendererType type;
} MZNT_RendererCommandBuffer;

/**
 * Defines the available texture formats.
 */
ENUM_START(MZNT_TextureFormat, u8)
    #define MZNT_TextureFormat_Unknown            ((MZNT_TextureFormat) 0)
    #define MZNT_TextureFormat_D32_Float          ((MZNT_TextureFormat) 1)
    #define MZNT_TextureFormat_B8G8R8A8_UNorm     ((MZNT_TextureFormat) 2)
    #define MZNT_TextureFormat_R8G8B8A8_UNorm     ((MZNT_TextureFormat) 3)
    #define MZNT_TextureFormat_R16G16B16A16_UNorm ((MZNT_TextureFormat) 4)
ENUM_END

/**
 * Represents a texture resource that can be used for rendering.
 */
typedef struct MZNT_Texture
{
    MZNT_RendererType type;
} MZNT_Texture;

/**
 * A swap-chain corresponding to a window that can be rendered to.
 * A swap-chain manages the images that are presented to the screen, and handles
 * synchronization between rendering and presentation.
 */
typedef struct MZNT_SwapChain
{
    MZNT_RendererType type;
} MZNT_SwapChain;

/**
 * Defines the types of shaders that are supported by this library.
 */
ENUM_START(MZNT_ShaderType, u8)
    #define MZNT_ShaderType_Unknown  ((MZNT_ShaderType) 0)
    #define MZNT_ShaderType_Compute  ((MZNT_ShaderType) 1)
    #define MZNT_ShaderType_Task     ((MZNT_ShaderType) 2)
    #define MZNT_ShaderType_Mesh     ((MZNT_ShaderType) 3)
    #define MZNT_ShaderType_Vertex   ((MZNT_ShaderType) 4)
    #define MZNT_ShaderType_Fragment ((MZNT_ShaderType) 5)
ENUM_END

/**
 * Represents a shader bytecode blob.
 * - On Vulkan, this is SPIR-V bytecode.
 * - On D3D12, this is DXIL bytecode.
 * - On Metal, this is MSL source code.
 */
typedef struct MZNT_ShaderByteCode
{
    PNSLR_ArraySlice(u8) byteCode;
} MZNT_ShaderByteCode;

PNSLR_DECLARE_ARRAY_SLICE(MZNT_ShaderByteCode);

/**
 * Represents a shader program.
 * It represents a "pipeline" of shaders that can be used together, and the resources they
 * require.
 */
typedef struct MZNT_Program
{
    MZNT_RendererType type;
} MZNT_Program;

EXTERN_C_END
#endif // MZNT_TYPES_H =============================================================
