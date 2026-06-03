#include <Core/Core.h>

TIM_Value TIM_GetCurrentSystemTime(void)
{
    #if MSR_WINDOWS
    {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        u64 dt = ((u64) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        return (TIM_Value) {(i64) (100 * (dt - 116444736000000000ULL))};
    }
    #elif MSR_UNIX
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return (TIM_Value) {I64_MIN};

        return (TIM_Value) {(i64) ts.tv_sec * 1000000000 + ts.tv_nsec};
    }
    #endif
}

TIM_Value TIM_GetTimeZoneOffset(void)
{
    #if MSR_WINDOWS
    {
        TIME_ZONE_INFORMATION tzi;
        DWORD result = GetTimeZoneInformation(&tzi);

        LONG bias = tzi.Bias;

        if (result == TIME_ZONE_ID_DAYLIGHT)
            bias += tzi.DaylightBias;
        else if (result == TIME_ZONE_ID_STANDARD)
            bias += tzi.StandardBias;

        // Windows bias is minutes west of UTC.
        i64 offsetNs = -(i64)bias * 60LL * 1000000000LL;

        return (TIM_Value){offsetNs};
    }
    #elif MSR_UNIX
    {
        time_t now = time(NULL);

        struct tm localTm;
        if (localtime_r(&now, &localTm) == NULL)
            return (TIM_Value){I64_MIN};

        return (TIM_Value){(i64)localTm.tm_gmtoff * 1000000000LL};
    }
    #endif
}

TIM_Value TIM_GetCurrentMonotonicTime(void)
{
    #if MSR_WINDOWS
    {
        static LARGE_INTEGER freq = {0};
        if (freq.QuadPart == 0)
            QueryPerformanceFrequency(&freq);

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);

        i64 ns = (counter.QuadPart / freq.QuadPart) * 1000000000LL +
                 ((counter.QuadPart % freq.QuadPart) * 1000000000LL) / freq.QuadPart;

        return (TIM_Value){ns};
    }
    #elif MSR_UNIX
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
            return (TIM_Value) {I64_MIN};

        return (TIM_Value) {(i64) ts.tv_sec * 1000000000 + ts.tv_nsec};
    }
    #endif
}

b8 TIM_ToDuration(TIM_Value value, u8* outHour, u8* outMinute, u8* outSecond)
{
    // Convert nanoseconds to seconds
    i64 seconds = value.ns / 1000000000;

    // Handle negative durations by taking the absolute value
    if (seconds < 0) { seconds = (seconds == I64_MIN ? I64_MAX : -seconds); }

    if (outHour)   *outHour   = (u8)((seconds / 3600) % 24);
    if (outMinute) *outMinute = (u8)((seconds / 60) % 60);
    if (outSecond) *outSecond = (u8)(seconds % 60);

    return true;
}

b8 TIM_ToDate(TIM_Value value, u16* outYear, u8* outMonth, u8* outDay)
{
    i64 days = value.ns / 1000000000LL / 86400LL;

    // Howard Hinnant's civil_from_days algorithm.
    days += 719468;

    i64 era = (days >= 0 ? days : days - 146096) / 146097;
    u32 doe = (u32) (days - era * 146097);                      // [0, 146096]
    u32 yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    i64 year = (i64) yoe + era * 400;

    u32 doy = doe - (365 * yoe + yoe/4 - yoe/100);
    u32 mp = (5 * doy + 2) / 153;

    u32 day = doy - (153 * mp + 2) / 5 + 1;
    u32 month = mp + (mp < 10 ? 3 : -9);

    year += (month <= 2);

    if (year < 0 || year > U16_MAX)
        return false;

    if (outYear)  *outYear  = (u16)year;
    if (outMonth) *outMonth = (u8)month;
    if (outDay)   *outDay   = (u8)day;

    return true;
}
