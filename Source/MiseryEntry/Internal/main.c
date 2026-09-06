#include <Core/Core.h>
#include <Platform/Platform.h>
#include <GPU/GPU.h>

static struct
{
    b8 die;
    GPU_SwapChain swapChain;
    struct
    {
        GPU_ProgramArgsLayout argsLayout;
        GPU_Program triangleVs;
        GPU_Program triangleMs;

        GPU_Buffer triangleVsPropsBuffer;
        GPU_Buffer triangleMsPropsBuffer;
    } programs;
} G_RenderData;

static struct
{
    SYN_Event renThrWake;
    SYN_Event renThrDone;
} G_ThreadSync;

typedef struct alignas(256) TriangleMatProps
{
    f32 prevVal[3][4];
    f32 newVal[3][4];

    f32 tint[4];
} TriangleMatProps;

void RenderThread(rawptr data);

i32 RealMain(APP_Handle app, Slice_(utf8str) args)
{
    THR_SetCurrentName(UTF8STR("main"));
    G_ThreadSync.renThrWake = SYN_CreateEvent(false);
    G_ThreadSync.renThrDone = SYN_CreateEvent(false);
    THR_Handle renderThread = THR_Start(RenderThread, nil, UTF8STR("render"));

    TIM_Value prevTime = TIM_GetCurrentMonotonicTime();

    DIR_Path rootDir = {0};
    {
        FIL_Path currExec = PRC_GetCurrentExecutablePath(MEM_temp);
        rootDir = DIR_Parent(DIR_Parent(DIR_Parent(currExec) /* platform-specific */) /* binaries dir */) /* root dir */;
    }

    GPU_ProgramStageByteCode triangleVs = {0}, triangleMs = {0}, triangleFs = {0}, fsBlitVs = {0}, fsBlitFs = {0};
    {
        // compile shaders
        {
            DIR_Path shadersDir = DIR_DirectoryInside(rootDir, UTF8STR("Shaders"), MEM_temp);
            DIR_Path triangleDir = DIR_DirectoryInside(shadersDir, UTF8STR("HelloTriangle"), MEM_temp);
            DIR_Path fsBlitDir = DIR_DirectoryInside(shadersDir, UTF8STR("FullScreenBlit"), MEM_temp);

            GPU_NewProgramStageByteCode(&triangleVs, (GPU_ProgramStageByteCodeCfg)
            {
                .gfxAPI = GPU_GfxAPIType_Vk,
                .stage = GPU_PgmStgTy_Vertex,
                .file = DIR_FileInside(triangleDir, UTF8STR("HelloTriangle.vert.hlsl"), MEM_temp),
                .entryPoint = UTF8STR("main"),
                .allocator = MEM_main,
            });

            GPU_NewProgramStageByteCode(&triangleMs, (GPU_ProgramStageByteCodeCfg)
            {
                .gfxAPI = GPU_GfxAPIType_Vk,
                .stage = GPU_PgmStgTy_Mesh,
                .file = DIR_FileInside(triangleDir, UTF8STR("HelloTriangle.mesh.hlsl"), MEM_temp),
                .entryPoint = UTF8STR("main"),
                .allocator = MEM_main,
            });

            GPU_NewProgramStageByteCode(&triangleFs, (GPU_ProgramStageByteCodeCfg)
            {
                .gfxAPI = GPU_GfxAPIType_Vk,
                .stage = GPU_PgmStgTy_Fragment,
                .file = DIR_FileInside(triangleDir, UTF8STR("HelloTriangle.frag.hlsl"), MEM_temp),
                .entryPoint = UTF8STR("main"),
                .allocator = MEM_main,
            });

            GPU_NewProgramStageByteCode(&fsBlitVs, (GPU_ProgramStageByteCodeCfg)
            {
                .gfxAPI = GPU_GfxAPIType_Vk,
                .stage = GPU_PgmStgTy_Vertex,
                .file = DIR_FileInside(fsBlitDir, UTF8STR("FullScreenBlit.vert.hlsl"), MEM_temp),
                .entryPoint = UTF8STR("main"),
                .allocator = MEM_main,
            });

            GPU_NewProgramStageByteCode(&fsBlitFs, (GPU_ProgramStageByteCodeCfg)
            {
                .gfxAPI = GPU_GfxAPIType_Vk,
                .stage = GPU_PgmStgTy_Fragment,
                .file = DIR_FileInside(fsBlitDir, UTF8STR("FullScreenBlit.frag.hlsl"), MEM_temp),
                .entryPoint = UTF8STR("main"),
                .allocator = MEM_main,
            });
        }
    }

    GPU_Instance ren = {0};
    GPU_Create(&ren, (GPU_InstanceCfg)
    {
        .type = GPU_GfxAPIType_Vk,
        .appHandle = app,
        .appName = UTF8STR("Misery"),
    });

    {
        GPU_NewProgramArgsLayout(&G_RenderData.programs.argsLayout, &ren, (GPU_ProgramArgsLayoutCfg)
        {
            .argsGroups = SLICE(GPU_ProgramArgsGroupCfg,
                ((GPU_ProgramArgsGroupCfg)
                {
                    .objectName = UTF8STR("defaultgroup"),
                    .type = GPU_PgmArgsGrpTy_Direct,
                    .args = SLICE(GPU_ProgramArgCfg,
                        ((GPU_ProgramArgCfg)
                        {
                            .name       = UTF8STR("G_MatProps"),
                            .visibility = GPU_PgmStgTy_AllGraphics,
                            .type       = GPU_PgmArg_ReadROBuffer,
                        }),
                    ),
                }),
            ),
            .inlineConstants =
            {
                .size       = 0,
                .visibility = GPU_PgmStgTy_AllGraphics,
            },
            .objectName = UTF8STR("mainlayout"),
        });

        GPU_NewBuffer(&G_RenderData.programs.triangleVsPropsBuffer, &ren, (GPU_BufferCfg)
        {
            .memType = GPU_MemType_Default,
            .usages = GPU_BufUsg_ReadOnly,
            .size = sizeof(TriangleMatProps),
            .align = alignof(TriangleMatProps),
            .objectName = UTF8STR("triangle props buffer (vs)"),
        });

        GPU_NewBuffer(&G_RenderData.programs.triangleMsPropsBuffer, &ren, (GPU_BufferCfg)
        {
            .memType = GPU_MemType_Default,
            .usages = GPU_BufUsg_ReadOnly,
            .size = sizeof(TriangleMatProps),
            .align = alignof(TriangleMatProps),
            .objectName = UTF8STR("triangle props buffer (ms)"),
        });

        GPU_ProgramStage triangleVsObj, triangleMsObj, triangleFsObj;
        GPU_NewProgramStage(&triangleVsObj, &ren, triangleVs);
        GPU_NewProgramStage(&triangleMsObj, &ren, triangleMs);
        GPU_NewProgramStage(&triangleFsObj, &ren, triangleFs);

        GPU_NewProgram(&G_RenderData.programs.triangleVs, &ren, (GPU_ProgramCfg)
        {
            .stages = SLICE(GPU_ProgramStageCfg,
                ((GPU_ProgramStageCfg) {.stage = &triangleVsObj}),
                ((GPU_ProgramStageCfg) {.stage = &triangleFsObj}),
            ),
            .targetFormats =
            {
                .draw = SLICE(GPU_TextureFormat,
                    GPU_TexFmt_B8G8R8A8_UNorm
                ),
                .depthStencil = GPU_TexFmt_Unknown,
            },
            .argsLayout = &G_RenderData.programs.argsLayout,
            .objectName = UTF8STR("triangle pipeline"),
        });

        GPU_NewProgram(&G_RenderData.programs.triangleMs, &ren, (GPU_ProgramCfg)
        {
            .stages = SLICE(GPU_ProgramStageCfg,
                ((GPU_ProgramStageCfg) {.stage = &triangleMsObj}),
                ((GPU_ProgramStageCfg) {.stage = &triangleFsObj}),
            ),
            .targetFormats =
            {
                .draw = SLICE(GPU_TextureFormat,
                    GPU_TexFmt_B8G8R8A8_UNorm
                ),
                .depthStencil = GPU_TexFmt_Unknown,
            },
            .argsLayout = &G_RenderData.programs.argsLayout,
            .objectName = UTF8STR("triangle pipeline"),
        });

        GPU_DeleteProgramStage(&triangleVsObj);
        GPU_DeleteProgramStage(&triangleMsObj);
        GPU_DeleteProgramStage(&triangleFsObj);
    }

    {
        GPU_DeleteProgramStageByteCode(&triangleVs);
        GPU_DeleteProgramStageByteCode(&triangleMs);
        GPU_DeleteProgramStageByteCode(&triangleFs);
        GPU_DeleteProgramStageByteCode(&fsBlitVs);
        GPU_DeleteProgramStageByteCode(&fsBlitFs);
    }

    u16 startWidth = 1280, startHeight = 720;

    WND_Data wnd = WND_Create((WND_Cfg)
    {
        .app = app,
        .posX = 100, .posY = 100,
        .sizeX = startWidth, .sizeY = startHeight,
        .title = UTF8STR("Misery"),
        .parent = (WND_Handle) {0},
        .bgCol = { 255, 127, 255, 255 },
        .acceptDropFiles = true,
    });

    GPU_CreateSwapChainFromWindow(&G_RenderData.swapChain, &ren, wnd.handle, (GPU_SwapChainCfg)
    {
        .width = startWidth, .height = startHeight,
        .vSync = true,
        .objectName = UTF8STR("mainwindow"),
    });

    MEM_DeallocateAll(MEM_temp);

    TriangleMatProps vsVals = {0}, msVals = {0};
    f32 rgbArr[3][4] = {{1.0, 1.0, 0.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 0.0, 1.0, 1.0}};
    i32 idx = 0, tintIdx = 0; f32 timer = 0.0f;
    b8 running = true, fullscreen = false;
    while (running)
    {
        TIM_Value newTime = TIM_GetCurrentMonotonicTime();
        f32 dt = (f32) ((f64) (newTime.ns - prevTime.ns) / 1000000000.0);
        prevTime = newTime;

        b8 nextTint = false, prevTint = false;
        INP_GatherEvts();
        isize evtIt = 0; INP_Evt evt;
        while (INP_IterateEvts(&evtIt, &evt))
        {
            b8 altBttn = (evt.ty == INP_Evt_Keyboard && evt.keyStatus == INP_KS_Pressed && !!(evt.keyModifiers & INP_KM_Alt));
            b8 altF4   = (altBttn && evt.keyCode == INP_KC_F4);
            b8 altRet  = (altBttn && evt.keyCode == INP_KC_Enter);

            // alt+f4 or quit event
            if (evt.ty == INP_Evt_Quit || altF4)
                running = false;

            // alt+enter
            if (altRet)
            {
                fullscreen = !fullscreen;
                WND_SetFullScreen(&wnd, fullscreen, nil, nil, nil, nil);
            }

            if (evt.ty == INP_Evt_DropFile)
            {
                utf8str file = INP_GetDroppedFile(evt.droppedFileId);
                LOG_Inf(INPUT, "Dropped file: %.", FMT(file));
            }

            switch (evt.ty)
            {
                case INP_Evt_Keyboard:   LOG_Inf(INPUT, "EVT: Keyboard");   break;
                case INP_Evt_MouseWheel: LOG_Inf(INPUT, "EVT: MouseWheel"); break;
                case INP_Evt_Touch:      LOG_Inf(INPUT, "EVT: Touch");      break;
                case INP_Evt_TextInput:  LOG_Inf(INPUT, "EVT: TextInput");  break;
                case INP_Evt_DropFile:   LOG_Inf(INPUT, "EVT: DropFile");   break;
                case INP_Evt_Quit:       LOG_Inf(INPUT, "EVT: Quit");       break;
                default:                 LOG_Inf(INPUT, "EVT: ???");        break;
            }

            if (evt.ty == INP_Evt_Keyboard && evt.keyStatus == INP_KS_Pressed && evt.keyCode == INP_KC_ArrowUp)
                nextTint = true;
            if (evt.ty == INP_Evt_Keyboard && evt.keyStatus == INP_KS_Pressed && evt.keyCode == INP_KC_ArrowDown)
                prevTint = true;
        }

        // do the pattern
        {
            static const f32 duration = 3.0f;
            f32 factor = timer / duration;
            timer -= dt;
            if (timer < 0.0f)
            {
                timer = duration; // every 3 seconds
                i32 aIdx0 = idx, aIdx1 = (idx + 1) % 3, aIdx2 = (idx + 2) % 3;
                idx++; idx %= 3;
                i32 bIdx0 = idx, bIdx1 = (idx + 1) % 3, bIdx2 = (idx + 2) % 3;
                i32 cIdx0 = (idx + 1) % 3, cIdx1 = (idx + 2) % 3, cIdx2 = idx;
                vsVals = (TriangleMatProps)
                {
                    .prevVal = {{rgbArr[bIdx0][0], rgbArr[bIdx0][1], rgbArr[bIdx0][2], 1.0},
                                {rgbArr[bIdx1][0], rgbArr[bIdx1][1], rgbArr[bIdx1][2], 1.0},
                                {rgbArr[bIdx2][0], rgbArr[bIdx2][1], rgbArr[bIdx2][2], 1.0}},
                    .newVal  = {{rgbArr[aIdx0][0], rgbArr[aIdx0][1], rgbArr[aIdx0][2], 1.0},
                                {rgbArr[aIdx1][0], rgbArr[aIdx1][1], rgbArr[aIdx1][2], 1.0},
                                {rgbArr[aIdx2][0], rgbArr[aIdx2][1], rgbArr[aIdx2][2], 1.0}},
                    .tint = {1.0, 1.0, 1.0, 1.0},
                };
                msVals = (TriangleMatProps)
                {
                    .prevVal = {{rgbArr[cIdx0][0], rgbArr[cIdx0][1], rgbArr[cIdx0][2], 1.0},
                                {rgbArr[cIdx1][0], rgbArr[cIdx1][1], rgbArr[cIdx1][2], 1.0},
                                {rgbArr[cIdx2][0], rgbArr[cIdx2][1], rgbArr[cIdx2][2], 1.0}},
                    .newVal  = {{rgbArr[bIdx0][0], rgbArr[bIdx0][1], rgbArr[bIdx0][2], 1.0},
                                {rgbArr[bIdx1][0], rgbArr[bIdx1][1], rgbArr[bIdx1][2], 1.0},
                                {rgbArr[bIdx2][0], rgbArr[bIdx2][1], rgbArr[bIdx2][2], 1.0}},
                    .tint = {1.0, 1.0, 1.0, 1.0},
                };
            }
            else
            {
                vsVals.prevVal[0][3] = vsVals.prevVal[1][3] = vsVals.prevVal[2][3] = factor;
                msVals.prevVal[0][3] = msVals.prevVal[1][3] = msVals.prevVal[2][3] = factor;
            }

            static const struct { f32 val[4]; } tints[][4] =
            {
                {1.00, 1.00, 1.00, 1.00}, // none
                {0.75, 0.75, 0.75, 1.00}, // light gray
                {0.25, 0.25, 0.25, 1.00}, // dark gray
                {1.00, 0.50, 0.50, 1.00}, // red
                {0.50, 1.00, 0.50, 1.00}, // green
                {0.50, 0.50, 1.00, 1.00}, // blue
                {1.00, 1.00, 0.50, 1.00}, // yellow
                {1.00, 0.50, 1.00, 1.00}, // magenta
                {0.50, 1.00, 1.00, 1.00}, // cyan
            };

            if (nextTint) tintIdx++; else if (prevTint) tintIdx--; else (void) tintIdx;
            tintIdx %= (sizeof(tints) / sizeof(tints[0]));

            MEM_Copy(vsVals.tint, tints[tintIdx], sizeof(f32) * 4);
            MEM_Copy(msVals.tint, tints[tintIdx], sizeof(f32) * 4);
        }


        // render thread kick-off
        {
            SYN_WaitEvent(&G_ThreadSync.renThrDone);
            if (!running)
            {
                G_RenderData.die = true;
            }
            else
            {
                MEM_Copy(
                    GPU_GetMappedBufferData(&G_RenderData.programs.triangleVsPropsBuffer).data,
                    &vsVals,
                    sizeof(TriangleMatProps)
                );

                MEM_Copy(
                    GPU_GetMappedBufferData(&G_RenderData.programs.triangleMsPropsBuffer).data,
                    &msVals,
                    sizeof(TriangleMatProps)
                );

                isize resizeIt = 0; INP_WindowResizeData resizeData;
                while (INP_IterateResizeEvts(&resizeIt, &resizeData))
                {
                    GPU_ReconfigureSwapChain(&G_RenderData.swapChain, (GPU_SwapChainCfg)
                    {
                        .width = resizeData.sizeX,
                        .height = resizeData.sizeY,
                        .vSync = true,
                        .objectName = UTF8STR("mainwindow"),
                    });
                }

                GPU_IterateSwapChain(&G_RenderData.swapChain);
            }
            SYN_SignalEvent(&G_ThreadSync.renThrWake);
        }

        utf8str rhi;
        switch (ren.base.type)
        {
            case GPU_GfxAPIType_Vk:   rhi = UTF8STR("VK");   break;
            case GPU_GfxAPIType_Dx12: rhi = UTF8STR("DX12"); break;
            case GPU_GfxAPIType_Mtl:  rhi = UTF8STR("MTL");  break;
            case GPU_GfxAPIType_Null: rhi = UTF8STR("NULL"); break;
            default:                  rhi = UTF8STR("UKWN"); break;
        }
        WND_Rename(&wnd, FMT_TPrintf("Misery | % | cpu %ms", FMT(rhi), FMT_F32(dt * 1000, 2, 2)));
        MEM_DeallocateAll(MEM_temp);
    }

    SYN_WaitEvent(&G_ThreadSync.renThrDone);
    THR_Join(renderThread);

    GPU_DestroySwapChain(&G_RenderData.swapChain);
    WND_Destroy(&wnd);
    GPU_DeleteProgram(&G_RenderData.programs.triangleMs);
    GPU_DeleteProgram(&G_RenderData.programs.triangleVs);
    GPU_DeleteBuffer(&G_RenderData.programs.triangleMsPropsBuffer);
    GPU_DeleteBuffer(&G_RenderData.programs.triangleVsPropsBuffer);
    GPU_DeleteProgramArgsLayout(&G_RenderData.programs.argsLayout);
    GPU_Destroy(&ren);

    SYN_DestroyEvent(&G_ThreadSync.renThrDone);
    SYN_DestroyEvent(&G_ThreadSync.renThrWake);

    return 0;
}

void RenderThread(rawptr data)
{
    (void) MEM_temp; // initialise temp allocator for this thread
    LOG_Inf(RENDER, "Render thread initialised!");
    SYN_SignalEvent(&G_ThreadSync.renThrDone);

    while (true)
    {
        MEM_DeallocateAll(MEM_temp);
        SYN_WaitEvent(&G_ThreadSync.renThrWake);
        if (G_RenderData.die)
            break;

        GPU_SwapChainFrameContext frameCtx = GPU_BeginSwapChainFrame(&G_RenderData.swapChain);
        if (frameCtx.valid)
        {
            GPU_CmdsBegin(frameCtx.cmdBuffer);

            GPU_CmdBarrier(frameCtx.cmdBuffer, (GPU_BarrierCfg)
            {
                .textureBarriers = SLICE(GPU_TextureBarrierCfg,
                    ((GPU_TextureBarrierCfg)
                    {
                        .texture = frameCtx.swapChainImg,
                        .src = {.stage = GPU_BarStg_Present,    .access = GPU_BarAcc_None      },
                        .dst = {.stage = GPU_BarStg_DrawTarget, .access = GPU_BarAcs_DrawTarget},
                        .srcLayout = GPU_TexLyt_Unknown,
                        .dstLayout = GPU_TexLyt_DrawTarget,
                    }),
                ),
            });

            GPU_CmdBeginPass(frameCtx.cmdBuffer, (GPU_PassCfg)
            {
                .drawTargets = SLICE(GPU_PassDrawTargetCfg,
                    ((GPU_PassDrawTargetCfg)
                    {
                        .target      = frameCtx.swapChainImg,
                        .idealLayout = true,
                        .loadOp      = GPU_LoadOp_Clear,
                        .storeOp     = GPU_StoreOp_Store,
                        .clearColor  = {0.15, 0.15, 0.2, 1.0},
                    }),
                ),
                .viewport =
                {
                    .x = 0.0f, .y = 0.0f,
                    .width = (f32) frameCtx.imageWidth,
                    .height = (f32) frameCtx.imageHeight,
                    .minDepth = 0.0f, .maxDepth = 1.0f,
                },
                .scissor =
                {
                    .offsetX = 0, .offsetY = 0,
                    .width = frameCtx.imageWidth,
                    .height = frameCtx.imageHeight,
                },
            });

            GPU_CmdBindProgram(frameCtx.cmdBuffer, (GPU_BindProgramCfg)
            {
                .program = &G_RenderData.programs.triangleVs,
                .cullMode = GPU_Cull_CounterClockwise,
                .topology = GPU_Topo_TriangleList,
            });

            GPU_CmdBindProgramArgsGroup(frameCtx.cmdBuffer, (GPU_ProgramArgsGroupBindingCfg)
            {
                .groupType = GPU_PgmArgsGrpTy_Direct,
                .groupIdx = 0,
                .programType = GPU_ProgramType_VertexFragment,
                .layout = &G_RenderData.programs.argsLayout,
                .value.direct = SLICE(GPU_ProgramArgBindingCfg,
                    ((GPU_ProgramArgBindingCfg)
                    {
                        .type = GPU_PgmArg_ReadROBuffer,
                        .value.buffer =
                        {
                            .buffer = &G_RenderData.programs.triangleVsPropsBuffer,
                            .offset = 0,
                            .size = sizeof(TriangleMatProps),
                        },
                    })
                ),
            });

            GPU_CmdDrawBasic(frameCtx.cmdBuffer, (GPU_DrawBasicCfg) {.vertCount = 3, .primitivesCount = 1});

            GPU_CmdBindProgram(frameCtx.cmdBuffer, (GPU_BindProgramCfg)
            {
                .program = &G_RenderData.programs.triangleMs,
                .cullMode = GPU_Cull_CounterClockwise,
                .topology = GPU_Topo_TriangleList,
            });

            GPU_CmdBindProgramArgsGroup(frameCtx.cmdBuffer, (GPU_ProgramArgsGroupBindingCfg)
            {
                .groupType = GPU_PgmArgsGrpTy_Direct,
                .groupIdx = 0,
                .programType = GPU_ProgramType_VertexFragment,
                .layout = &G_RenderData.programs.argsLayout,
                .value.direct = SLICE(GPU_ProgramArgBindingCfg,
                    ((GPU_ProgramArgBindingCfg)
                    {
                        .type = GPU_PgmArg_ReadROBuffer,
                        .value.buffer =
                        {
                            .buffer = &G_RenderData.programs.triangleMsPropsBuffer,
                            .offset = 0,
                            .size = sizeof(TriangleMatProps),
                        },
                    })
                ),
            });

            GPU_CmdDrawMeshlets(frameCtx.cmdBuffer, (GPU_DrawMeshletsCfg) {1, 1, 1});

            GPU_CmdEndPass(frameCtx.cmdBuffer);

            GPU_CmdBarrier(frameCtx.cmdBuffer, (GPU_BarrierCfg)
            {
                .textureBarriers = SLICE(GPU_TextureBarrierCfg,
                    ((GPU_TextureBarrierCfg)
                    {
                        .texture = frameCtx.swapChainImg,
                        .src = {.stage = GPU_BarStg_DrawTarget, .access = GPU_BarAcs_DrawTarget},
                        .dst = {.stage = GPU_BarStg_Present,    .access = GPU_BarAcc_None      },
                        .srcLayout = GPU_TexLyt_DrawTarget,
                        .dstLayout = GPU_TexLyt_Present,
                    }),
                ),
            });

            GPU_CmdsEnd(frameCtx.cmdBuffer);
            GPU_EndSwapChainFrame(&G_RenderData.swapChain);
        }

        SYN_SignalEvent(&G_ThreadSync.renThrDone);
    }

    SYN_SignalEvent(&G_ThreadSync.renThrDone);
    LOG_Inf(RENDER, "Render thread exiting!");
}

#if MSR_DBG && MSR_WINDOWS
    // cli uses `int main` and gui uses `WinMain`
    // using the former in debug mode on windows
    // will make it easier to see debug output in
    // the console without needing to attach a debugger

    APP_AS_CLI_EXEC(RealMain);
#else
    APP_AS_GUI_EXEC(RealMain);
#endif
