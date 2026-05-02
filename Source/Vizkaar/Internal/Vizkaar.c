#define DVRPL_EXECUTABLE_IMPL_ANY VzkrMain
#include "Vizkaar/Vizkaar.h"

#if MZNT_DX12
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
#endif

const MZNT_RendererType k_RendererTypes[] =
{
#if !MZNT_VULKAN && !MZNT_DX12 && !MZNT_METAL
    MZNT_RendererType_Null,
#endif

#if MZNT_DX12
    MZNT_RendererType_DirectX12,
#endif
#if MZNT_VULKAN
    MZNT_RendererType_Vulkan,
#endif
};

#define RENDERER_TYPE_COUNT (sizeof(k_RendererTypes) / sizeof(MZNT_RendererType))

typedef struct VzkrRenderData
{
    b8 die;
    b8 valid[RENDERER_TYPE_COUNT];
    MZNT_SwapChain* swapChains[RENDERER_TYPE_COUNT];
} VzkrRenderData;

volatile VzkrRenderData G_RenderData;
PNSLR_Event G_RenderThreadWake;
PNSLR_Event G_RenderThreadDone;

void VzkrRender(rawptr data);

i32 VzkrMain(DVRPL_App app, PNSLR_ArraySlice(utf8str) args)
{
    PNSLR_SetThreadName(PNSLR_GetCurrentThreadHandle(), PNSLR_StringLiteral("Main Thread"));
    G_RenderThreadWake = PNSLR_CreateEvent(false);
    G_RenderThreadDone = PNSLR_CreateEvent(false);
    PNSLR_ThreadHandle renderThreadHandle = PNSLR_StartThread(VzkrRender, nil, PNSLR_StringLiteral("Render Thread"));

    PNSLR_SetDefaultLogger(
        PNSLR_GetDefaultLoggerWithOptions(
            PNSLR_LoggerLevel_Info,
            PNSLR_LogOption_None
                | PNSLR_LogOption_IncludeLevel
                | PNSLR_LogOption_IncludeColours
            #if PNSLR_MOBILE
                | PNSLR_LogOption_IncludeFile
            #else
                | PNSLR_LogOption_IncludeFn
            #endif
        )
    );

    PNSLR_AllocatorError err = PNSLR_AllocatorError_None;
    PNSLR_Allocator tempAllocator = PNSLR_NewAllocator_Arena(PNSLR_GetAllocator_DefaultHeap(), 16 * 1024 * 1024 /* 16 MiB */, PNSLR_GET_LOC(), &err);
    if (err != PNSLR_AllocatorError_None)
    {
        // failed to create temp allocator
        return -1;
    }

    PNSLR_Path rootDir = {0};
    {
        PNSLR_Path rootDirTemp = {0};
        PNSLR_Path executableFile = PNSLR_NormalisePath(args.data[0], PNSLR_PathNormalisationType_File, tempAllocator);
        PNSLR_Path binariesDir = {0};
        PNSLR_SplitPath(executableFile, &binariesDir, nil, nil, nil);
        PNSLR_SplitPath(binariesDir, &rootDirTemp, nil, nil, nil);

        rootDir.path = PNSLR_MakeString(rootDirTemp.path.count, false, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
        PNSLR_MemCopy(rootDir.path.data, rootDirTemp.path.data, (i32) rootDirTemp.path.count);
    }

    i64 prevTime = PNSLR_NanosecondsSinceUnixEpoch();

    struct {
        MZNT_Renderer*   renderer;
        DVRPL_WindowData window;
        MZNT_SwapChain*  swapChain;
    } openWindows[RENDERER_TYPE_COUNT] = {0};

    MZNT_ShaderCompiler shaderCompiler = {0};
    {
        PNSLR_Path executableFile = PNSLR_NormalisePath(args.data[0], PNSLR_PathNormalisationType_File, tempAllocator);

        PNSLR_Path binariesDir = {0};
        PNSLR_SplitPath(executableFile, &binariesDir, nil, nil, nil);
        PNSLR_Path dxcDir = PNSLR_GetPathForSubdirectory(binariesDir, PNSLR_StringLiteral("DXC"), tempAllocator);

        shaderCompiler = MZNT_CreateShaderCompiler((MZNT_ShaderCompilerConfiguration)
        {
            .libSearchDir = dxcDir,
            .allocator = PNSLR_GetAllocator_DefaultHeap(),
        }, tempAllocator);
    }

    for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
    {
        openWindows[i].renderer = MZNT_CreateRenderer((MZNT_RendererConfiguration)
        {
            .type = k_RendererTypes[i],
            .allocator = PNSLR_GetAllocator_DefaultHeap(),
            .appName = PNSLR_StringLiteral("Vizkaar"),
            .appHandle = {.handle = app.handle},
        }, tempAllocator);

        openWindows[i].window = DVRPL_CreateWindow((DVRPL_WindowCreationOptions)
        {
            .app = app,
            .posX = 200 + ((800 + 10) * i), .posY = 150,
            .title = PNSLR_StringLiteral("Test Window"),
            .sizeX = 800, .sizeY = 600,
            .parent = {0},
            .msaa = false,
            .acceptDropFiles = true,
            .bgColR = 38, .bgColG = 38, .bgColB = 51, .bgColA = 255,
        });

        openWindows[i].swapChain = MZNT_CreateSwapChainFromWindow(openWindows[i].renderer,
            (MZNT_WindowHandle) {.handle = openWindows[i].window.window.handle},
            (MZNT_SwapChainConfiguration) {.vSync = true, .framesInFlight = 2, .objectName = PNSLR_StringLiteral("MainWindow")},
            tempAllocator);

        G_RenderData.swapChains[i] = openWindows[i].swapChain;
    }

    PNSLR_FreeAll(tempAllocator, PNSLR_GET_LOC(), nil);

    b8 running = true, fullscreen = false;
    while (running)
    {
        i64 newTime = PNSLR_NanosecondsSinceUnixEpoch();
        f32 dt = (f32) ((f64) (newTime - prevTime) / 1000000000.0);
        prevTime = newTime;

        DVRPL_GatherEvents(tempAllocator);
        i64 iterator = 0; DVRPL_Event evt;
        while (DVRPL_IterateEvents(&iterator, &evt))
        {
            b8 altBttn = (evt.ty == DVRPL_EvtTy_Keyboard && evt.keyStatus == DVRPL_KeyStatus_Pressed && !!(evt.keyModifiers & DVRPL_KeyModifier_Alt));
            b8 altF4   = (altBttn && evt.keyCode == DVRPL_KeyCode_F4);
            b8 altRet  = (altBttn && evt.keyCode == DVRPL_KeyCode_Enter);

            // alt+f4 or quit event
            if (evt.ty == DVRPL_EvtTy_Quit || altF4)
                running = false;

            // alt+enter
            if (altRet)
            {
                fullscreen = !fullscreen;
                for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
                {
                    if (openWindows[i].window.window.handle != evt.windowId.handle)
                        continue;

                    DVRPL_SetFullScreen(&(openWindows[i].window), fullscreen, nil, nil, nil, nil);
                }
            }

            if (evt.ty == DVRPL_EvtTy_DropFile)
            {
                utf8str file = DVRPL_GetDroppedFile(evt.droppedFileId);
                PNSLR_LogIf(PNSLR_StringLiteral("Dropped file: $"), PNSLR_FmtArgs(PNSLR_FmtString(file)), PNSLR_GET_LOC());
            }

            switch (evt.ty)
            {
                case DVRPL_EvtTy_Unknown:    PNSLR_LogI(PNSLR_StringLiteral("EVT: Unknown"),    PNSLR_GET_LOC()); break;
                case DVRPL_EvtTy_Keyboard:   PNSLR_LogI(PNSLR_StringLiteral("EVT: Keyboard"),   PNSLR_GET_LOC()); break;
                case DVRPL_EvtTy_MouseWheel: PNSLR_LogI(PNSLR_StringLiteral("EVT: MouseWheel"), PNSLR_GET_LOC()); break;
                case DVRPL_EvtTy_Touch:      PNSLR_LogI(PNSLR_StringLiteral("EVT: Touch"),      PNSLR_GET_LOC()); break;
                case DVRPL_EvtTy_TextInput:  PNSLR_LogI(PNSLR_StringLiteral("EVT: TextInput"),  PNSLR_GET_LOC()); break;
                case DVRPL_EvtTy_DropFile:   PNSLR_LogI(PNSLR_StringLiteral("EVT: DropFile"),   PNSLR_GET_LOC()); break;
                case DVRPL_EvtTy_Quit:       PNSLR_LogI(PNSLR_StringLiteral("EVT: Quit"),       PNSLR_GET_LOC()); break;
                default:                     PNSLR_LogI(PNSLR_StringLiteral("EVT: ???"),        PNSLR_GET_LOC()); break;
            }
        }

        PNSLR_WaitEvent(&G_RenderThreadDone);
        if (!running)
        {
            G_RenderData.die = true;
        }
        else
        {
            i32 resizeIterator = 0; DVRPL_WindowResizeData resizeData;
            while (DVRPL_IterateResizeEvent(&resizeIterator, &resizeData))
            {
                for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
                {
                    if (openWindows[i].window.window.handle != resizeData.id.handle)
                        continue;

                    MZNT_ReconfigureSwapChain(openWindows[i].swapChain, (MZNT_SwapChainConfiguration)
                    {
                        .vSync          = true,
                        .framesInFlight = 2,
                        .objectName     = PNSLR_StringLiteral("MainWindow"),
                    },
                     tempAllocator);
                }
            }

            for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
            {
                G_RenderData.valid[i] = MZNT_IterateSwapChain(openWindows[i].swapChain, tempAllocator);
            }
        }
        PNSLR_SignalEvent(&G_RenderThreadWake);

        for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
        {
            utf8str rendererStr = {0};
            switch (k_RendererTypes[i])
            {
                case MZNT_RendererType_Null:
                    rendererStr = PNSLR_StringLiteral("NULL");
                    break;
                case MZNT_RendererType_Vulkan:
                    rendererStr = PNSLR_StringLiteral("VLKN");
                    break;
                case MZNT_RendererType_DirectX12:
                    rendererStr = PNSLR_StringLiteral("DX12");
                    break;
                case MZNT_RendererType_Metal:
                    rendererStr = PNSLR_StringLiteral("MTL_");
                    break;
                default:
                    rendererStr = PNSLR_StringLiteral("UNKW");
                    break;
            }

            utf8str tempStr = PNSLR_FormatString(
                PNSLR_StringLiteral("Vizkaar [REN_$] | cpu: $ms"),
                PNSLR_FmtArgs(
                    PNSLR_FmtString(rendererStr),
                    PNSLR_FmtF32(dt * 1000, 2)
                ),
                tempAllocator
            );

            DVRPL_RenameWindow(openWindows[i].window.window, tempStr);
        }

        PNSLR_FreeAll(tempAllocator, PNSLR_GET_LOC(), nil);
    }

    PNSLR_WaitEvent(&G_RenderThreadDone);
    PNSLR_JoinThread(renderThreadHandle);

    for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
    {
        MZNT_DestroySwapChain(openWindows[i].swapChain, tempAllocator);
        DVRPL_DestroyWindow(&(openWindows[i].window));
        MZNT_DestroyRenderer(openWindows[i].renderer, tempAllocator);
    }

    MZNT_DestroyShaderCompiler(shaderCompiler, tempAllocator);

    PNSLR_FreeString(rootDir.path, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);
    PNSLR_DestroyAllocator_Arena(tempAllocator, PNSLR_GET_LOC(), nil);

    PNSLR_DestroyEvent(&G_RenderThreadDone);
    PNSLR_DestroyEvent(&G_RenderThreadWake);

    return 0;
}

void VzkrRender(rawptr data)
{
    PNSLR_Allocator tempAllocator = PNSLR_NewAllocator_Arena(PNSLR_GetAllocator_DefaultHeap(), 16 * 1024 * 1024 /* 16 MiB */, PNSLR_GET_LOC(), nil);
    PNSLR_SignalEvent(&G_RenderThreadDone);

    while (true)
    {
        PNSLR_WaitEvent(&G_RenderThreadWake);
        if (G_RenderData.die)
        {
            PNSLR_DestroyAllocator_Arena(tempAllocator, PNSLR_GET_LOC(), nil);
            PNSLR_SignalEvent(&G_RenderThreadDone);
            return;
        }

        for (i16 i = 0; i < (i16) RENDERER_TYPE_COUNT; i++)
        {
            if (!G_RenderData.valid[i]) continue;

            MZNT_SwapChain* sc = G_RenderData.swapChains[i];
            if (!sc) continue;

            MZNT_RendererCommandBuffer* cmdBuf = MZNT_GetSwapChainCommandBuffer(sc, nil, tempAllocator);
            if (!cmdBuf) continue;

            MZNT_PresentSwapChain(sc, tempAllocator);
        }

        PNSLR_SignalEvent(&G_RenderThreadDone);
        PNSLR_FreeAll(tempAllocator, PNSLR_GET_LOC(), nil);
    }
}

#undef RENDERER_TYPE_COUNT
