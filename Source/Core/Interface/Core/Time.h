#pragma once
#include <__init.h>

/**
 * Returns the current system time in nanoseconds since the Unix epoch (January 1, 1970).
 * This is typically used for getting the current date and time.
 * The value returned by this function is not guaranteed to be monotonic, and may be affected
 * by changes to the system clock (e.g. due to NTP adjustments or manual changes).
 */
int64_t GetCurrentSystemTime();

/**
 * Returns the current monotonic time in nanoseconds. This is typically used for measuring
 * elapsed time or time intervals, as it is guaranteed to be monotonic and not affected by
 * changes to the system clock.
 * The value returned by this function is not related to any specific epoch, and should only
 * be used for measuring time intervals or elapsed time.
 */
int64_t GetCurrentMonotonicTime();

/**
 * Breaks down the given nanoseconds since the unix epoch into its human-readable date and time
 * components. The output parameters are optional and can be null if the caller is not interested
 * in a specific component.
 */
bool BreakUnixEpochTime(
    int64_t nsSinceUnixEpoch,
    int16_t* outYear,
    uint8_t* outMonth,
    uint8_t* outDay,
    uint8_t* outHour,
    uint8_t* outMinute,
    uint8_t* outSecond
);

/**
 * Converts a duration in nanoseconds into hours, minutes, and seconds. The output parameters
 * are optional and can be null if the caller is not interested in a specific component.
 */
bool DurationFromNs(int64_t nanoseconds, uint8_t* outHour, uint8_t* outMinute, uint8_t* outSecond);

/**
 * Converts a timestamp in nanoseconds since the Unix epoch into a date (year, month, day).
 * The output parameters are optional and can be null if the caller is not interested in a specific
 * component. This function returns false if the input timestamp is negative (before the epoch).
 */
bool DateFromNsSinceUnixEpoch(int64_t nsSinceUnixEpoch, int16_t* outYear, uint8_t* outMonth, uint8_t* outDay);
