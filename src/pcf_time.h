/*
   +----------------------------------------------------------------------+
   | php-baremetal: rtc-pcf85063                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@php-baremetal.com>         |
   +----------------------------------------------------------------------+
*/

/* Date checks and the weekday for set(). Pure, so they are tested on the PC. */
#pragma once

#include <stdbool.h>
#include <stdint.h>

static inline bool pcf_leap(int64_t y)
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

static inline int pcf_days_in_month(int64_t y, int64_t m)
{
    static const int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return m == 2 && pcf_leap(y) ? 29 : days[m - 1];
}

/* 0 when the date and time fit the chip (years 2000 to 2099), else the 1-based position of the first field
 * out of range: 1 year, 2 month, 3 day, 4 hour, 5 minute, 6 second. */
static inline int pcf_bad_field(int64_t y, int64_t mo, int64_t d, int64_t h, int64_t mi, int64_t s)
{
    if (y < 2000 || y > 2099) {
        return 1;
    }
    if (mo < 1 || mo > 12) {
        return 2;
    }
    if (d < 1 || d > pcf_days_in_month(y, mo)) {
        return 3;
    }
    if (h < 0 || h > 23) {
        return 4;
    }
    if (mi < 0 || mi > 59) {
        return 5;
    }
    if (s < 0 || s > 59) {
        return 6;
    }
    return 0;
}

/* Day of the week, 0 = Sunday to 6 = Saturday (the chip's Weekdays register). */
static inline int pcf_weekday(int64_t y, int64_t mo, int64_t d)
{
    static const int t[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };   /* Sakamoto */
    if (mo < 3) {
        y -= 1;
    }
    return (int) ((y + y / 4 - y / 100 + y / 400 + t[mo - 1] + d) % 7);
}
