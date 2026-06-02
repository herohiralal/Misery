#pragma once
#include <__init.h>

/**
 * Defines a structure to represent a timestamp or duration in nanoseconds. This is used for both
 * absolute timestamps and relative durations, depending on the context. The value is typically
 * interpreted as nanoseconds since the Unix epoch for absolute timestamps, or as a duration in
 * nanoseconds for relative time intervals.
 */
typedef struct
{
    i64 ns; // for absolute system time, this is ns since unix epoch
} TIM_Value;

/**
 * Returns the current system time.
 * This is typically used for getting the current date and time.
 * The value returned by this function is not guaranteed to be monotonic, and may be affected
 * by changes to the system clock (e.g. due to NTP adjustments or manual changes).
 */
TIM_Value TIM_GetCurrentSystemTime();

/**
 * Returns the current monotonic time.
 * This is typically used for measuring elapsed time or time intervals, as it is guaranteed
 * to be monotonic and not affected by changes to the system clock.
 * The value returned by this function is not related to any specific epoch, and should only
 * be used for measuring time intervals or elapsed time.
 */
TIM_Value TIM_GetCurrentMonotonicTime();

/**
 * Converts a duration in nanoseconds into hours, minutes, and seconds. The output parameters
 * are optional and can be null if the caller is not interested in a specific component.
 */
b8 TIM_ToDuration(TIM_Value value, u8* outHour, u8* outMinute, u8* outSecond);

/**
 * Converts a timestamp into a date (year, month, day).
 * The output parameters are optional and can be null if the caller is not interested in a specific
 * component. This function returns false if the input timestamp is negative (before the epoch).
 */
b8 TIM_ToDate(TIM_Value value, u16* outYear, u8* outMonth, u8* outDay);
