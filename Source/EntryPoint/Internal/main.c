#include <Core/Core.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>

static struct
{
    b8 die;
    REN_SwapChain swapChain;
} G_RenderData;

static struct
{
    SYN_Event renThrWake;
    SYN_Event renThrDone;
} G_ThreadSync;

void RenderThread(rawptr data);

i32 RealMain(APP_Handle app, Slice_(utf8str) args)
{
    THR_SetCurrentName(UTF8STR("main"));
    G_ThreadSync.renThrWake = SYN_CreateEvent(false);
    G_ThreadSync.renThrDone = SYN_CreateEvent(false);
    THR_Handle renderThread = THR_Start(RenderThread, nil, UTF8STR("render"));

    TIM_Value prevTime = TIM_GetCurrentMonotonicTime();

    REN_Instance ren = {0};
    REN_Create(&ren, (REN_InstanceCfg)
    {
        .type = REN_GfxAPIType_Vk,
        .appHandle = app,
        .appName = UTF8STR("Misery"),
    });

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

    REN_CreateSwapChainFromWindow(&G_RenderData.swapChain, &ren, wnd.handle, (REN_SwapChainCfg)
    {
        .width = startWidth, .height = startHeight,
        .vSync = true,
        .objectName = UTF8STR("mainwindow"),
    });

    MEM_DeallocateAll(MEM_temp);

    b8 running = true, fullscreen = false;
    while (running)
    {
        TIM_Value newTime = TIM_GetCurrentMonotonicTime();
        f32 dt = (f32) ((f64) (newTime.ns - prevTime.ns) / 1000000000.0);
        prevTime = newTime;

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
                isize resizeIt = 0; INP_WindowResizeData resizeData;
                while (INP_IterateResizeEvts(&resizeIt, &resizeData))
                {
                    REN_ReconfigureSwapChain(&G_RenderData.swapChain, (REN_SwapChainCfg)
                    {
                        .width = resizeData.sizeX,
                        .height = resizeData.sizeY,
                        .vSync = true,
                        .objectName = UTF8STR("mainwindow"),
                    });
                }

                REN_IterateSwapChain(&G_RenderData.swapChain);
            }
            SYN_SignalEvent(&G_ThreadSync.renThrWake);
        }

        WND_Rename(&wnd, FMT_TPrintf("Misery | cpu %ms", FMT_F32(dt * 1000, 2, 2)));
        MEM_DeallocateAll(MEM_temp);
    }

    SYN_WaitEvent(&G_ThreadSync.renThrDone);
    THR_Join(renderThread);

    REN_DestroySwapChain(&G_RenderData.swapChain);
    WND_Destroy(&wnd);
    REN_Destroy(&ren);

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

        REN_CmdBuffer* cmdBuf = REN_GetSwapChainCommandBuffer(&G_RenderData.swapChain, nil);
        if (cmdBuf)
        {
            REN_PresentSwapChain(&G_RenderData.swapChain);
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
