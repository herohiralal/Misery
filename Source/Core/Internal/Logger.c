#include <Core/Core.h>
#include "StreamPrivate.h"

#if MSR_DESKTOP
    static SYN_Mutex G_LOG_Internal_StdOutMutex = {0};
#endif

static void LOG_Internal_Initialise(void)
{
    #if MSR_DESKTOP
        G_LOG_Internal_StdOutMutex = SYN_CreateMutex();
        setvbuf(stdout, nil, _IONBF, 0); // disable stdout buffering
    #endif
}

void LOG_Internal_AddEntry(const LOG_Internal_Entry* entry)
{
    static SYN_DoOnce doOnce = {0};
    SYN_ExecuteDoOnce(&doOnce, LOG_Internal_Initialise);

    utf8str fileStr = {.data = (u8*) entry->loc.file,     .count = entry->loc.fileLen    };
    utf8str fnStr   = {.data = (u8*) entry->loc.function, .count = entry->loc.functionLen};

    if (MSR_DESKTOP)
    {
        #if 0
        utf8str lvlStr;
        switch ((enum LOG_Lvls) entry->lvl)
        {
            case LOG_Lvl_Debug:   lvlStr = UTF8STR("DBG"); break;
            case LOG_Lvl_Info:    lvlStr = UTF8STR("INF"); break;
            case LOG_Lvl_Warning: lvlStr = UTF8STR("WRN"); break;
            case LOG_Lvl_Error:   lvlStr = UTF8STR("ERR"); break;
            case LOG_Lvl_Fatal:   lvlStr = UTF8STR("XXX"); break;
        }
        #endif

        utf8str catStr = {.data = (u8*) (entry->cat), .count = 7};
        utf8str timestampStr = {0};
        char timestampBuf[64] = {0};
        {
            TIM_Value t = TIM_GetCurrentSystemTime();
            t.ns += TIM_GetTimeZoneOffset().ns;

            u16 yr; u8 mnth, day, hr, min, sec;
            b8 gotDate = TIM_ToDate(t, &yr, &mnth, &day);
            b8 gotTime = TIM_ToDuration(t, &hr, &min, &sec);
            MSR_ASSERT(gotDate && gotTime && "failed to break time");

            int n = snprintf(
                timestampBuf, sizeof(timestampBuf),
                " [%04d-%02d-%02d] [%02d:%02d:%02d] ",
                (i32) yr, (i32) mnth, (i32) day,
                (i32) hr, (i32) min, (i32) sec
            );

            timestampStr = (utf8str) {.data = (u8*) timestampBuf, .count = n};
        }

        SYN_LockMutex(&G_LOG_Internal_StdOutMutex);

        IO_Stream out = IO_GetStdOut();

        switch ((enum LOG_Lvls) entry->lvl)
        {
            case LOG_Lvl_Debug:   IO_Write(out, UTF8STR("\033[0m\033[1m"   )); break; // Reset, Bold
            case LOG_Lvl_Info:    IO_Write(out, UTF8STR("\033[0m\033[1;36m")); break; // Reset, Bold Cyan
            case LOG_Lvl_Warning: IO_Write(out, UTF8STR("\033[0m\033[1;33m")); break; // Reset, Bold Yellow
            case LOG_Lvl_Error:   IO_Write(out, UTF8STR("\033[0m\033[1;31m")); break; // Reset, Bold Red
            case LOG_Lvl_Fatal:   IO_Write(out, UTF8STR("\033[0m\033[1;41m")); break; // Reset, Bold Red background
        }

        FMT_FPrintf(out, "[%] ", FMT(catStr));

        switch ((enum LOG_Lvls) entry->lvl)
        {
            case LOG_Lvl_Debug: IO_Write(out, UTF8STR("\033[0m\033[90m"  )); break; // Reset, Dark Grey
            case LOG_Lvl_Fatal: IO_Write(out, UTF8STR("\033[0m\033[1;31m")); break; // Reset, Red
            default:            IO_Write(out, UTF8STR("\033[0m"          )); break; // Reset
        }

        IO_Write(out, timestampStr);
        FMT_FPrintf_(out, entry->msg, entry->fmtArgs);
        IO_Write(out, UTF8STR("\033[0m\033[90m")); // Reset, Dark Grey
        FMT_FPrintf(out, " (from `%()` at %:%:%)", FMT(fnStr), FMT(fileStr), FMT(entry->loc.line), FMT(entry->loc.column));
        IO_Write(out, UTF8STR("\033[0m\n")); // Reset and newline

        SYN_UnlockMutex(&G_LOG_Internal_StdOutMutex);
    }
    else if (MSR_MOBILE)
    {
        char catBuf[8] = {0};
        memcpy(catBuf, entry->cat, 7);
        catBuf[7] = '\0';

        utf8str msg = FMT_APrintf_(MEM_temp, entry->msg, entry->fmtArgs);

        #if MSR_DESKTOP
        {
            (void) catBuf;
            (void) msg;
            MSR_ASSERT(false && "unexpected entry");
        }
        #elif MSR_ANDROID
        {
            cstring msgFr = FMT_CTPrintf(
                "% (from `%()` at %:%:%)\n",
                FMT(msg),
                FMT(fnStr), FMT(fileStr), FMT(entry->loc.line), FMT(entry->loc.column)
            );

            int androidPrio = ANDROID_LOG_DEFAULT;
            switch ((enum LOG_Lvls) entry->lvl)
            {
                case LOG_Lvl_Debug:   androidPrio = ANDROID_LOG_DEBUG;    break;
                case LOG_Lvl_Info:    androidPrio = ANDROID_LOG_INFO;     break;
                case LOG_Lvl_Warning: androidPrio = ANDROID_LOG_WARN;     break;
                case LOG_Lvl_Error:   androidPrio = ANDROID_LOG_ERROR;    break;
                case LOG_Lvl_Fatal:   androidPrio = ANDROID_LOG_FATAL;    break;
                default:              androidPrio = ANDROID_LOG_DEFAULT;  break;
            }

            __android_log_write(androidPrio, catBuf, msgFr);
        }
        #elif MSR_IOS
        {
            os_log_type_t osPrio = OS_LOG_TYPE_DEFAULT;
            switch ((enum LOG_Lvls) entry->lvl)
            {
                case LOG_Lvl_Debug:   osPrio = OS_LOG_TYPE_DEBUG;    break;
                case LOG_Lvl_Info:    osPrio = OS_LOG_TYPE_INFO;     break;
                case LOG_Lvl_Warning: osPrio = OS_LOG_TYPE_ERROR;    break; // no warn level, so using error
                case LOG_Lvl_Error:   osPrio = OS_LOG_TYPE_ERROR;    break;
                case LOG_Lvl_Fatal:   osPrio = OS_LOG_TYPE_FAULT;    break;
                default:              osPrio = OS_LOG_TYPE_DEFAULT;  break;
            }

            os_log_with_type(OS_LOG_DEFAULT, osPrio,
                "[%s] %.*s (from `%.*s()` at %.*s:%d:%d)",
                catBuf, (int) msg.count, msg.data,
                (int) fnStr.count, fnStr.data,
                (int) fileStr.count, fileStr.data, entry->loc.line, entry->loc.column
            );
        }
        #else
            #error "Unknown mobile platform"
        #endif
    }
}
