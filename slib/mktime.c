/*
 * Maximus Version 3.02
 * Copyright 1989, 2002 by Lanius Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*******************************************************************
 *                   Date Manipulation Function                    *
 *                           Library                               *
 *                                                                 *
 * Written By:    Ray Gardner                                      *
 * Date Written:  Unknown                                          *
 *                                                                 *
 * Purpose: Provides various functions to convert and manipulate   *
 *          dates. These will work over the range 1/01/01 thru     *
 *          14699/12/31.                                           *
 *                                                                 *
 *******************************************************************/

/*******************************************************************
 *                          Change History                         *
 *                                                                 *
 *  Rev #   Date       By      Description of change               *
 *  1.00  | ??/??/?? | RG  | Original Version                      *
 *  1.10  | 07/17/90 | LRL | Wrote cvt_tm_time() routine.          *
 *                           Documented functions.                 *
 *  1.20  | 01/25/90 | SJD | Removed K&R trash, trimmmed some f()s *
 *-----------------------------------------------------------------*
 * Directory of initials:                                          *
 * Initials          Name                                          *
 * LRL        Lynn R. Lively                                       *
 * RG         Ray Gardner                                          *
 *******************************************************************/


#include "prog.h"


#ifndef NO_MKTIME /* Only used if compiler doesn't already have mktime() */

#include <time.h>

/* This function returns an approximation of the number of days that have   *
 * passed from the beginning of the year to the beginning of the specified  *
 * month. This function is static since the results need to be tuned        *
 * depending on the month and whether or not the year is a leap year.       */

static unsigned months_to_days (unsigned month)
{
  return ((month * 3057 - 3007) / 100);
}


/* This function returns the year expressed in days since 01/01/01 */

static long years_to_days (unsigned yr)
{
  return (yr * 365L + yr / 4 - yr / 100 + yr / 400);
}



/* This function returns the number of days (since 01/01/01) represented    *
 * by yr / mo / day.                                                        */

static long ymd_to_days (unsigned yr, unsigned mo, unsigned day)
{
  long days;

  days = day + months_to_days (mo);

  if (mo > 2)
    days -= (isleap (yr)) ? 1 : 2;

  yr--;
  days += years_to_days (yr);

  return (days);
}


time_t _stdc mktime(struct tm * tm_ptr)
{
  time_t scalar_time;

  scalar_time = ymd_to_days ((tm_ptr->tm_year + 1900),
                             (tm_ptr->tm_mon + 1),
                             tm_ptr->tm_mday);
  scalar_time -= ymd_to_days (1970, 1, 1);

  scalar_time *= (24L * 3600L);
  scalar_time += (long) (tm_ptr->tm_sec+timezone +
                         (tm_ptr->tm_min * 60L) +
                         (tm_ptr->tm_hour * 3600L) - 3600);

  return (scalar_time);
}

#endif /* NO_MKTIME */

