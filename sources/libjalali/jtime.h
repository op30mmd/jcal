/*
 * jtime.h - Tools for manipulating Jalali representation of Iranian calendar
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

#ifndef JTIME_H
#define JTIME_H

#include <time.h>
#include "jalali.h"

#ifdef __cplusplus
extern "C" {
#if 0 /* /me mutters something about emacs. */
}
#endif
#endif

#define MAX_BUF_SIZE 2048

JALALI_API extern char* jasctime(const struct jtm* jtm);

JALALI_API extern char* jctime(const time_t* timep);

JALALI_API extern struct jtm* jgmtime(const time_t* timep);

JALALI_API extern struct jtm* jlocaltime(const time_t* timep);

JALALI_API extern time_t jmktime(struct jtm* jtm);

JALALI_API extern size_t jstrftime(char* s, size_t max, const char* format,
            const struct jtm* jtm);

JALALI_API extern char* jstrptime(const char* s, const char* format, struct jtm* jtm);

JALALI_API extern char* jasctime_r(const struct jtm* jtm, char* buf);

JALALI_API extern char* jctime_r(const time_t* timep, char* buf);

JALALI_API extern struct jtm* jgmtime_r(const time_t* timep, struct jtm* result);

JALALI_API extern struct jtm* jlocaltime_r(const time_t* timep, struct jtm* result);

JALALI_API extern int jalali_to_farsi(char* buf, size_t n, int padding, char* pad, int d);

#ifdef __cplusplus
}
#endif

#endif /* JTIME_H */
