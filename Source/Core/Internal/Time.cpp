#include <Core/Time.h>

int64_t GetCurrentSystemTime()
{
    #if MSR_WINDOWS
    {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        uint64_t dt = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        return (int64_t) (100 * (dt - 116444736000000000ULL));
    }
    #elif MSR_UNIX
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            return INT64_MIN;

        return (int64_t) ts.tv_sec * 1000000000 + ts.tv_nsec;
    }
    #endif
}

int64_t GetCurrentMonotonicTime()
{
    #if MSR_WINDOWS
    {
        static LARGE_INTEGER freq = { };
        if (freq.QuadPart == 0)
            QueryPerformanceFrequency(&freq);

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return (int64_t) (counter.QuadPart * 1000000000LL / freq.QuadPart);
    }
    #elif MSR_UNIX
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
            return INT64_MIN;

        return (int64_t) ts.tv_sec * 1000000000 + ts.tv_nsec;
    }
    #endif
}

bool DurationFromNs(int64_t nanoseconds, uint8_t* outHour, uint8_t* outMinute, uint8_t* outSecond)
{
    // Convert nanoseconds to seconds
    int64_t seconds = nanoseconds / 1000000000;

    // Handle negative durations by taking the absolute value
    if (seconds < 0) { seconds = -seconds; }

    if (outHour)   *outHour   = (uint8_t)((seconds / 3600) % 24);
    if (outMinute) *outMinute = (uint8_t)((seconds / 60) % 60);
    if (outSecond) *outSecond = (uint8_t)(seconds % 60);

    return true;
}

bool DateFromNsSinceUnixEpoch(int64_t nsSinceUnixEpoch, int16_t* outYear, uint8_t* outMonth, uint8_t* outDay)
{
    // Convert nanoseconds to seconds
    int64_t seconds = nsSinceUnixEpoch / 1000000000;

    // Handle negative timestamps (before epoch)
    if (seconds < 0)
    {
        return false;
    }

    // Calculate days since epoch
    int64_t days = seconds / 86400; // 86400 seconds in a day

    // Start from Unix epoch: January 1, 1970
    int64_t year = 1970;
    int64_t month = 1;
    int64_t day = 1;

    // Add the days to get the actual date
    day += days;

    // Days in each month (non-leap year)
    static const uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Normalize the date
    while (true)
    {
        // Check if current year is a leap year
        bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        uint8_t daysThisMonth = daysInMonth[month - 1];
        if (month == 2 && isLeapYear)
            daysThisMonth = 29;

        if (day <= daysThisMonth)
            break; // Date is valid

        // Move to next month
        day -= daysThisMonth;
        month++;

        if (month > 12)
        {
            month = 1;
            year++;
        }

        // Sanity check to prevent infinite loops
        if (year > 9999)
            return false;
    }

    // Set output values
    if (outYear)  *outYear  = (int16_t) year;
    if (outMonth) *outMonth = (uint8_t) month;
    if (outDay)   *outDay   = (uint8_t) day;

    return true;
}
