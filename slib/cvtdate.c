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

#include <time.h>
#include "prog.h"

static int is_dst=-1;


/* Find out the current status of daylight savings time */

static void near InitCvt(void)
{
  time_t tnow;

  tnow=time(NULL);

  is_dst=!! (localtime(&tnow)->tm_isdst);
}


/* Convert a DOS-style bitmapped date into a 'struct tm'-type date. */

struct tm * _fast DosDate_to_TmDate(union stamp_combo *dosdate,
                                     struct tm *tmdate)
{
  if (is_dst==-1)
    InitCvt();

  tmdate->tm_mday=dosdate->msg_st.date.da;
  tmdate->tm_mon =dosdate->msg_st.date.mo-1;
  tmdate->tm_year=dosdate->msg_st.date.yr+80;

  tmdate->tm_hour=dosdate->msg_st.time.hh;
  tmdate->tm_min =dosdate->msg_st.time.mm;
  tmdate->tm_sec =dosdate->msg_st.time.ss << 1;

  tmdate->tm_isdst=is_dst;

  return tmdate;
}

/* Convert a 'struct tm'-type date into an Opus/DOS bitmapped date */

union stamp_combo * _fast TmDate_to_DosDate(struct tm *tmdate,
                                             union stamp_combo *dosdate)
{
  dosdate->msg_st.date.da=tmdate->tm_mday;
  dosdate->msg_st.date.mo=tmdate->tm_mon+1;
  dosdate->msg_st.date.yr=(tmdate->tm_year < 80) ? 0 : tmdate->tm_year-80;

  dosdate->msg_st.time.hh=tmdate->tm_hour;
  dosdate->msg_st.time.mm=tmdate->tm_min;
  dosdate->msg_st.time.ss=tmdate->tm_sec >> 1;

  return dosdate;
}



char * _fast sc_time(union stamp_combo *sc,char *string)
{
  if (sc->msg_st.date.yr==0)
    *string='\0';
  else
  {
    sprintf(string,
           "%2d %s %02d %02d:%02d:%02d",
           sc->msg_st.date.da,
           months_ab[sc->msg_st.date.mo-1],
           (80+sc->msg_st.date.yr) % 100,
           sc->msg_st.time.hh,
           sc->msg_st.time.mm,
           sc->msg_st.time.ss << 1);
  }

  return string;
}

