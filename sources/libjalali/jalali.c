/*
 * jalali.c - Tools for manipulating Jalali representation of Iranian calendar
 * and necessary conversations to Gregorian calendar.
 * Copyright (C) 2006, 2007, 2009, 2010, 2011 Ashkan Ghassemi.
 *
 * This file is part of libjalali.
 *
 * libjalali is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libjalali is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libjalali.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include "jalali.h"
#include "jconfig.h"

/*
 * Assuming *factor* numbers of *lo* make one *hi*, cluster *lo*s and change
 * *hi* appropriately. In the end:
 * - new lo will be in [0, factor)
 * - new hi will be hi + lo / factor
 */
#define RECLUSTER(hi, lo, factor) \
    if (lo < 0 || lo >= (factor)) {\
        hi += lo / (factor);\
        lo = lo % (factor);\
        if (lo < 0) { lo += (factor); hi--; }\
    }

const int cycle_patterns[] = { J_PT0, J_PT1, J_PT2, J_PT3, INT_MAX };
const int leaps[] = { J_L0, J_L1, J_L2, J_L3, INT_MAX };

const int jalali_month_len[] = { 31, 31, 31, 31, 31, 31, 30, 30, 30, 30,
                                 30, 29 };
const int accumulated_jalali_month_len[] = { 0, 31, 62, 93, 124, 155, 186,
                                             216, 246, 276, 306, 336 };

extern char* tzname[2];

/*
 * Jalali leap year indication function. The algorithm used here
 * is loosely based on the famous recurring 2820 years length period. This
 * period is then divided into 88 cycles, each following a 29, 33, 33, 33
 * years length pattern with the exception for the last being 37 years long.
 * In every of these 29, 33 or 37 years long periods starting with year 0,
 * leap years are multiples of four except for year 0 in each period.
 * The current 2820 year period started in the year AP 475 (AD 1096).
 */

int jalali_is_jleap(int year)
{
    int r = year % 33;
    return (r==1 || r==5 || r==9 || r==13 || r==17 || r==22 || r==26 || r==30);
}

/*
 * Creates absolute values for day, hour, minute and seconds from time_t.
 * Values are signed integers.
 */
void jalali_create_time_from_secs(time_t t, struct ab_jtm* d)
{
    d->ab_days = (t >= 0) ? (t / (time_t) J_DAY_LENGTH_IN_SECONDS) :
        ((t - (time_t) J_DAY_LENGTH_IN_SECONDS + (time_t) 1) /
         (time_t) J_DAY_LENGTH_IN_SECONDS);

    if (t >= 0) {
        t %= (time_t) J_DAY_LENGTH_IN_SECONDS;
    }
    else {
#if defined _WIN32 || defined __CYGWIN__
        t = (J_DAY_LENGTH_IN_SECONDS -
             (llabs(t - J_DAY_LENGTH_IN_SECONDS) %
              J_DAY_LENGTH_IN_SECONDS)) %
            J_DAY_LENGTH_IN_SECONDS;
#else
        t = (J_DAY_LENGTH_IN_SECONDS -
             (labs(t - J_DAY_LENGTH_IN_SECONDS) %
              J_DAY_LENGTH_IN_SECONDS)) %
            J_DAY_LENGTH_IN_SECONDS;
#endif
    }

    d->ab_hour = t / J_HOUR_LENGTH_IN_SECONDS;
    t %= J_HOUR_LENGTH_IN_SECONDS;
    d->ab_min = t / J_MINUTE_LENGTH_IN_SECONDS;
    d->ab_sec = t % J_MINUTE_LENGTH_IN_SECONDS;
}

/*
 * Creates a timestamp from day, hour, minute and seconds.
 * Values are signed integers.
 */
time_t jalali_create_secs_from_time(const struct ab_jtm* d)
{
    return (time_t)
        ((time_t) d->ab_days * (time_t) J_DAY_LENGTH_IN_SECONDS +
         (time_t) d->ab_hour * (time_t) J_HOUR_LENGTH_IN_SECONDS +
         (time_t) d->ab_min * (time_t) J_MINUTE_LENGTH_IN_SECONDS +
         (time_t) d->ab_sec);
}


/*
 * Calculate day of year (0-365) based on month and day.
 */
int jalali_create_days_from_date(struct jtm* j)
{
    int p;
    if (j->tm_mon < 0 || j->tm_mon > 11)
        return -1;

    if (j->tm_mday < 1 || j->tm_mday > 31)
        return -1;

    p = accumulated_jalali_month_len[j->tm_mon];
    p += j->tm_mday;
    j->tm_yday = p - 1 /* zero based offset */;

    return 0;
}

/*
 * Get useful information on a desired jalali year, including:
 * 1. Leap status. -lf
 * 2. Year position in grand leap cycle, passed and remaining years. -p, -r
 * 3. Passed and remaining leap years in grand leap cycle. -pl, -rl
 * 4. Absolute passed leap years since grand leap cycle epoch (AP 475). -apl
 */
void jalali_get_jyear_info(struct jyinfo* year)
{
    int y = year->y;
    year->lf = jalali_is_jleap(year->y);
    int i;
    int d = (year->y >= JALALI_LEAP_BASE) ? 1 : -1;
    int c = 0;

    for (i=JALALI_LEAP_BASE; ; i+=d) {
        if (jalali_is_jleap(i)) {
            c++;
        }

        if (i == year->y)
            break;
    }

    year->apl = c * d;
    year->pl = (d > 0) ? c % JALALI_TOTAL_LEAPS_IN_PERIOD :
        JALALI_TOTAL_LEAPS_IN_PERIOD - (c % JALALI_TOTAL_LEAPS_IN_PERIOD);
    year->rl = JALALI_TOTAL_LEAPS_IN_PERIOD - year->pl;

    y-= JALALI_LEAP_BASE;
    y%= JALALI_LEAP_PERIOD;
    if (y < 0)
        y+= JALALI_LEAP_PERIOD;

    year->p = y;
    year->r = JALALI_LEAP_PERIOD - y - 1;

    return ;
}


/*
 * Calculates UTC epoch difference of a desired date by measure of days.
 */
int jalali_get_diff(const struct jtm* j)
{
    int p = 0;
    int i;
    int s, sd;
    int e, ed;
    int f = 1;

    if (j->tm_yday > 365 || j->tm_yday < 0)
        return -1;

    if (j->tm_year == J_UTC_EPOCH_YEAR) {
        p = j->tm_yday - J_UTC_EPOCH_DIFF;
        return p;
    }
    else if (j->tm_year > J_UTC_EPOCH_YEAR) {
        s = J_UTC_EPOCH_YEAR + 1;
        sd = J_UTC_EPOCH_DIFF;
        e = j->tm_year - 1;
        ed = j->tm_yday + 1;
    } else {
        f = -1;
        s = j->tm_year + 1;
        sd = j->tm_yday;
        e = J_UTC_EPOCH_YEAR - 1;
        ed = J_UTC_EPOCH_DIFF + 1;
    }

    for (i=s; i<=e; i++) {
        p+= jalali_is_jleap(i) ? JALALI_LEAP_YEAR_LENGTH_IN_DAYS :
            JALALI_NORMAL_YEAR_LENGTH_IN_DAYS;
    }

    int r = jalali_is_jleap(s) ? JALALI_LEAP_YEAR_LENGTH_IN_DAYS - sd - 1 :
        JALALI_NORMAL_YEAR_LENGTH_IN_DAYS - sd - 1;

    p += r + ed;
    p*= f;

    return p;
}

/*
 * Number of days in provided year and month
 */
int jalali_year_month_days(int year, int month) {
    int dim = jalali_month_len[month];
    if (month == 11 && jalali_is_jleap(year))
        dim += 1;
    return dim;
}

/*
 * Calculates the jalali date based on number of days since UTC epoch.
 */
void jalali_get_date(int p, struct jtm* jtm)
{
    time_t t = (time_t) p * (time_t) J_DAY_LENGTH_IN_SECONDS;
    struct tm gtm;
#ifdef _WIN32
    gmtime_s(&gtm, &t);
#else
    gmtime_r(&t, &gtm);
#endif
    jalali_from_gregorian(gtm.tm_year + 1900, gtm.tm_mon + 1, gtm.tm_mday, &jtm->tm_year, &jtm->tm_mon, &jtm->tm_mday);

    /* Adjust weekday. Gregorian Sunday is 0. Jalali Saturday is 0. */
    jtm->tm_wday = (gtm.tm_wday + 1) % 7;

    jalali_create_days_from_date(jtm);
}

/*
 * Updates a jalali date struct fields based on tm_year, tm_mon and tm_mday
 */
void jalali_update(struct jtm* jtm)
{
    int dim; // number of days in current month
    RECLUSTER(jtm->tm_min, jtm->tm_sec, J_MINUTE_LENGTH_IN_SECONDS);
    RECLUSTER(jtm->tm_hour, jtm->tm_min, J_HOUR_LENGTH_IN_MINUTES);
    RECLUSTER(jtm->tm_mday, jtm->tm_hour, J_DAY_LENGTH_IN_HOURS);

    /* start by calculating a year based on month and change month and year till mday fit */
    RECLUSTER(jtm->tm_year, jtm->tm_mon, J_YEAR_LENGTH_IN_MONTHS);

    if (jtm->tm_mday < 1) {
        /* breaking months to days */
        while (jtm->tm_mday < 1) {
            if (jtm->tm_mon == 0) {
                jtm->tm_mon = 11;
                jtm->tm_year -= 1;
            } else {
                jtm->tm_mon -= 1;
            }
            jtm->tm_mday += jalali_year_month_days(jtm->tm_year, jtm->tm_mon);
        }
    } else {
        /* clustering days as months */
        while (jtm->tm_mday > (dim=jalali_year_month_days(jtm->tm_year, jtm->tm_mon))) {
            jtm->tm_mday -= dim;
            if (jtm->tm_mon == 11) {
                jtm->tm_mon = 0;
                jtm->tm_year += 1;
            } else {
                jtm->tm_mon += 1;
            }
        }
    }

    /* date is normalized, compute tm_wday and tm_yday */
    jalali_create_days_from_date(jtm);
    jalali_get_date(jalali_get_diff(jtm), jtm);
}

void jalali_from_gregorian(int gy, int gm, int gd, int* jy, int* jm, int* jd)
{
    long long g_days_in_month[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    long long gy2 = (gm > 2) ? (gy + 1) : gy;
    long long days = 355666 + (365 * gy) + (long long)((gy2 + 3) / 4) - (long long)((gy2 + 99) / 100) + (long long)((gy2 + 399) / 400) + gd + g_days_in_month[gm - 1];

    *jy = -1595 + (33 * (long long)(days / 12053));
    days %= 12053;
    *jy += 4 * (long long)(days / 1461);
    days %= 1461;

    if (days > 365) {
        *jy += (long long)((days - 1) / 365);
        days = (days - 1) % 365;
    }

    if (days < 186) {
        *jm = 1 + (long long)(days / 31);
        *jd = 1 + (days % 31);
    } else {
        *jm = 7 + (long long)((days - 186) / 30);
        *jd = 1 + ((days - 186) % 30);
    }
}

/*
 * Displays a jalali date struct fields.
 * should be used for debugging purposes only.
 */
void jalali_show_time(const struct jtm* j)
{
    printf("%d/%02d/%02d (%02d:%02d:%02d) [%d]",
           j->tm_year, j->tm_mon + 1, j->tm_mday, j->tm_hour, j->tm_min,
           j->tm_sec, j->tm_wday);
    printf(" yday: %d, dst: %d, off: %ld, zone: %s\n",
           j->tm_yday, j->tm_isdst, j->tm_gmtoff,
           j->tm_zone);
}
