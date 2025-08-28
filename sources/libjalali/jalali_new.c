#include <stdio.h>
#include "jalali_new.h"

void gregorian_to_jalali(int gy, int gm, int gd, int* jy, int* jm, int* jd)
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
