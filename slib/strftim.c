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

/*# name=strftime() function, since BoreLand (heh) doesn't include
    name=one with TC 2.0!
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "unistr.h"
#include "prog.h"

#ifndef NO_STRFTIME

/* Note:  TZ environment variable MUST be defined to use the %Z function! */

size_t cdecl strftime(char *string, size_t maxsize, const char *format, const struct tm *current_time)
{
  const char *in;
  
  char *out,
       *scrptr;

  char temp[250];

  maxsize=min(maxsize,230);

  for (in=format,out=temp;*in;in++)
  {
    if ((int)(out-(int)temp) >= maxsize)
      break;

    if (*in=='%')
    {
      switch(*++in)
      {
        case 'a':
          strcpy(out,weekday_ab[current_time->tm_wday]);
          break;

        case 'A':
          strcpy(out,weekday[current_time->tm_wday]);
          break;

        case 'b':
          strcpy(out,months_ab[current_time->tm_mon]);
          break;

        case 'B':
          strcpy(out,months[current_time->tm_mon]);
          break;

        case 'c':
          sprintf(out,"%02d-%02d-%02d %02d:%02d:%02d",
                  current_time->tm_mon+1,
                  current_time->tm_mday,
                  current_time->tm_year,
                  current_time->tm_hour,
                  current_time->tm_min,
                  current_time->tm_sec);
          break;

        case 'd':
          sprintf(out,"%02d",current_time->tm_mday);
          break;

        case 'H':
          sprintf(out,"%02d",current_time->tm_hour);
          break;

        case 'I':
          sprintf(out,"%02d",
                  (current_time->tm_hour >= 0 && current_time->tm_hour <= 12 ?
                   current_time->tm_hour :
                   current_time->tm_hour-12));
          break;

        case 'j':
          sprintf(out,"%03d",current_time->tm_yday+1);
          break;

        case 'm':
          sprintf(out,"%02d",(current_time->tm_mon)+1);
          break;

        case 'M':
          sprintf(out,"%02d",current_time->tm_min);
          break;

        case 'p':
          strcpy(out,(current_time->tm_hour < 12 ? "am" : "pm"));
          break;

        case 'S':
          sprintf(out,"%02d",current_time->tm_sec);
          break;

        case 'U': /* ??? */
          sprintf(out,"%02d",(current_time->tm_yday)/7);
          break;

        case 'w':
          sprintf(out,"%d",current_time->tm_wday);
          break;

        case 'W': /* ??? */
          sprintf(out,"%02d",(current_time->tm_yday)/7);
          break;

        case 'x':
          sprintf(out,"%02d-%02d-%02d",
                  (current_time->tm_mon)+1,
                  current_time->tm_mday,
                  current_time->tm_year);
          break;

        case 'X':
          sprintf(out,"%02d:%02d:%02d",
                  current_time->tm_hour,
                  current_time->tm_min,
                  current_time->tm_sec);
          break;

        case 'y':
          sprintf(out,"%02d",current_time->tm_year % 100);
          break;

        case 'Y':
          sprintf(out,"%04d",current_time->tm_year+1900);
          break;

        case 'Z': /* ??? */

          if ((scrptr=getenv("TZ")) != 0)
          {
            scrptr[3]=0;
            strcpy(out,strupr(scrptr));
          }
          else strcpy(string,"??T");
          break;

        case '%':
          strcpy(out,"%");
          break;
      }

      out += strlen(out);
    }
    else *out++=*in;
  }

  *out='\0';

  strcpy(string,temp);

  return(strlen(string));
}

#endif /* !NO_STRFTIME */

