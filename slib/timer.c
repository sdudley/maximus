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

/*# name=Timing routines
*/

#ifdef OS_2
#define INCL_NOPM
#include <os2.h>
#endif

#include <dos.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "prog.h"

#if defined(OS_2)

long _stdc timerset(unsigned int duration)
{
    DATETIME dt;
    DosGetDateTime(&dt);
    return ( ((dt.minutes % 60)*6000L) +
             ((dt.seconds % 60)*100L) +
             dt.hundredths +
             (long)duration
            );
}

#elif defined(NT)

#include "pwin.h"

long _stdc timerset(unsigned int duration)
{
  SYSTEMTIME st;

  GetLocalTime(&st);

  return ( (st.wMinute % 60)*6000L +
           (st.wSecond % 60)*100L +
           st.wMilliseconds / 10L +
           (long)duration );

}

#elif defined(__MSDOS__)

long _stdc timerset(unsigned int duration)
{
  #ifdef __MSC__
      struct dostime_t dt;
      _dos_gettime(&dt);
      return ( ((dt.minute % 60)*6000L) +
               ((dt.second % 60)*100L) +
               dt.hsecond +
               (long)duration
              );
  #else /* assume generic MSDOS compiler */
      union REGS regs;
      regs.h.ah = 0x2c;
      intdos(&regs, &regs);
      return ( ((regs.h.cl % 60)*6000L) +
               ((regs.h.dh % 60)*100L) +
               regs.h.dl +
               (long)duration
              );
  #endif
}

#elif defined(UNIX)

/*long _stdc timerset(unsigned int duration)
{
  #ifndef CLK_TCK
    #define CLK_TCK 100
  #endif

  return ((clock() + duration) * ((long)CLK_TCK / 100L));
}*/

long _stdc timerset(unsigned int duration)
{
    struct tm* dt;
    time_t t;
    struct timeval tv;
    
    t= time(NULL);
    
    dt = gmtime(&t);    
    gettimeofday(&tv, NULL);
//    tv.tv_usec = (tv.tv_usec + 500) / 1000;
    tv.tv_usec = tv.tv_usec / 1000;

    return ( ((dt->tm_min % 60)*6000L) +
             ((dt->tm_sec % 60)*100L) +
             tv.tv_usec / 10L +
             (long)duration
            );
}


#else
  #error Unknown OS
#endif

int _stdc timeup(long timer)
{
  long now;

  now=timerset (0);

  if (now < (timer-65536L))
    now += 360000L;

  return ((now-timer) >= 0L);
}


void _stdc timer(unsigned int duration)
{
  long tim;

  tim=timerset(duration);

  while (! timeup(tim))
#ifdef UNIX
    sleep(0)
#endif
    ;
}


