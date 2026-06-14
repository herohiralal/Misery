#include <Core/Core.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>

int whatever(void);

i32 RealMain(APP_Handle app, Slice_(utf8str) args)
{
    whatever();

    Slice_(u8) a = SLICE(u8);
    (void) a;

    FMT_Args b = FMTARGS();
    (void) b;

    Slice_(u8) c = SLICE(u8, 1, 2, 3);
    (void) c;

    FMT_Args d = FMTARGS(FMT("a"), FMT(1), FMT(5.0f));
    (void) d;

    LOG_Inf(MAIN, "Hello, World!");

    TIM_Value prevTime = TIM_GetCurrentMonotonicTime();

    List_(i32) ints = COL_NewList(i32, 10, MEM_temp);
    COL_ResizeList(&ints, 40);
    COL_AppendToList(&ints, 420);

    Slice_(i32) kkk = SLICE(i32, 46, 50, 60);
    COL_AppendAllToList(&ints, kkk);

    Slice_(i32) k2 = COL_CloneSlice(kkk, MEM_temp);
    for (i32 i = 0; i < k2.count; i++) k2.data[i]++;
    COL_AppendAllToList(&ints, k2);

    utf8str abcdef = UTF8STR("abcdef");
    Slice_(FMT_Arg) fmts = FMTARGS(FMT(nil), FMT("coolz"), FMT(abcdef));
    (void) fmts;

    utf8str abc = STR_SubString(abcdef, 0, 3);
    (void) abc;

    Slice_(i32) intsSl = COL_SubSlice(ints, 0, ints.count);
    intsSl = COL_CloneSlice(intsSl, MEM_temp);

    COL_ResizeSlice(&intsSl, ints.count + 40, true, MEM_temp);
    for (i32 i = 0; i < 40; i++)
        intsSl.data[ints.count + i] = i;

    Slice_(i32) ko = COL_SubSlice(ints, 0, 2);
    if (ko.count >= 2) ko.data[1]--;

    LOG_Inf(TRASH, "ints");
    for (i32 i = 0; i < ints.count; i++)
        LOG_Inf(TRASH, "\t%: %", FMT(i), FMT(ints.data[i]));

    LOG_Inf(TRASH, "kkk");
    for (i32 i = 0; i < kkk.count; i++)
        LOG_Inf(TRASH, "\t%: %", FMT(i), FMT(kkk.data[i]));

    LOG_Inf(TRASH, "k2");
    for (i32 i = 0; i < k2.count; i++)
        LOG_Inf(TRASH, "\t%: %", FMT(i), FMT(k2.data[i]));

    LOG_Inf(TRASH, "ko");
    for (i32 i = 0; i < ko.count; i++)
        LOG_Inf(TRASH, "\t%: %", FMT(i), FMT(ko.data[i]));

    List_(PRC_EnvVarKVP) envVars = PRC_GetEnvVars(MEM_main);

    // save env vars as a markdown table

    FIL_Path p = FIL_Normalise(UTF8STR("C:\\Users\\HeroHiralal\\Desktop\\misery.md"), MEM_temp);
    IO_Stream f = IO_OpenFileToWrite(p, false, false);

    SrcLoc srcLoc = SRC_LOC();
    FMT_FPrintf(f, "> # Generated from: [`%()`] at `%:%`. \n\n", FMT(srcLoc.function), FMT(srcLoc.file), FMT(srcLoc.line));


    {
        TIM_Value t = TIM_GetCurrentSystemTime();
        t.ns += TIM_GetTimeZoneOffset().ns;

        u16 yr; u8 mnth, day, hr, min, sec;
        b8 gotDate = TIM_ToDate(t, &yr, &mnth, &day);
        b8 gotTime = TIM_ToDuration(t, &hr, &min, &sec);
        MSR_ASSERT(gotDate && gotTime && "failed to break time");
        (void) gotDate;
        (void) gotTime;

        utf8str months[] =
        {
            UTF8STR("Jan"), UTF8STR("Feb"), UTF8STR("Mar"), UTF8STR("Apr"), UTF8STR("May"), UTF8STR("Jun"),
            UTF8STR("Jul"), UTF8STR("Aug"), UTF8STR("Sep"), UTF8STR("Oct"), UTF8STR("Nov"), UTF8STR("Dec"),
        };

        FMT_FPrintf(f, "> Generated at `%:%:%` on `% %, %`. \n\n",
            FMT(hr), FMT(min), FMT(sec), FMT(months[mnth - 1]), FMT(day), FMT(yr));
    }

    IO_Write(f, UTF8STR("| Key | Value |\n"));
    IO_Write(f, UTF8STR("|-----|-------|\n"));

    for (isize i = 0; i < envVars.count; i++)
    {
        FMT_FPrintf(f, "| % | % |\n", FMT(envVars.data[i].key), FMT(envVars.data[i].value));
    }

    {
        IO_Write(f, UTF8STR("\n\n## Git Status\n\n```txt\n"));
        IO_Stream read, write;
        IO_CreatePipe(&read, &write);

        PRC_Handle process = PRC_Run(
            SLICE(utf8str, UTF8STR("git"), UTF8STR("status")),
            (Slice_(utf8str)) {0},
            (DIR_Path) {0},
            &write,
            nil);

        IO_Close(write);

        PRC_Wait(&process, nil);

        Slice_(u8) readBuff = COL_NewSlice(u8, 4096, true, MEM_temp);
        isize bytesRead = 0;
        while ((bytesRead = IO_Read(read, readBuff)) > 0)
        {
            Slice_(u8) realSlice = COL_SubSlice(readBuff, 0, bytesRead);
            IO_Write(f, realSlice);
        }

        IO_Close(read);
        IO_Write(f, UTF8STR("```\n"));
    }

    IO_Flush(f);
    IO_Close(f);

    PRC_FreeEnvVars(&envVars);

    WND_Data wnd = WND_Create((WND_Cfg)
    {
        .app = app,
        .posX = 100, .posY = 100,
        .sizeX = 800, .sizeY = 600,
        .title = UTF8STR("Misery"),
        .parent = (WND_Handle) {0},
        .bgCol = { 255, 127, 255, 255 },
        .acceptDropFiles = true,
    });

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

        WND_Rename(&wnd, FMT_TPrintf("Misery | cpu %ms", FMT_F32(dt * 1000, 2, 2)));

        THR_Sleep(10);
    }

    WND_Destroy(&wnd);

    return 0;
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
