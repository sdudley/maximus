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

#pragma off(unreferenced)
static char rcs_id[]="$Id: max_mcmd.c,v 1.3 2003/06/06 01:10:10 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Modem command dispatcher
*/

#define MAX_INCL_COMMS

#include <time.h>
#include <string.h>
#include "prog.h"
#include "mm.h"

#if defined(UNIX) || defined(NT)
# include "ntcomm.h"
#else
# define COMMAPI_VER 0
#endif

#ifndef SHORT_MDM_CMD
static char *mdm_rsp[]={"NO CARRIER",
                        "CONNECT",
                        "BUSY",
                        "VOICE",
                        "RING",
                        "RRING",
                        "RINGING",
                        "NO DIALTONE",
                        "NO DIAL TONE",
                        "DIAL TONE",
                        "ERROR",
                        "OK",
                        NULL};
#endif

int _mdm_cmd(char *command)
{
  #ifndef SHORT_MDM_CMD
  timer_t expire;

  unsigned int baud,
               y,z;

  int ch,
      buflen;

  char buffer[MAX_cmdlen],
       temp[50];

  MDM_RESPONSE retval;
  #endif


  unsigned int x;
  long timedone;
  
  /* Don't spend more than 20 secs total doing this */
  
  timedone=time(NULL)+30;


  for (x=0; x < strlen(command) && time(NULL) < timedone; x++)
  {
    switch(command[x])
    {
      case '^':
        Mdm_flush_ck_tic(2000, FALSE, FALSE);
        mdm_dtr(DTR_UP);
        break;

      case 'v':
        Mdm_flush_ck_tic(2000, FALSE, FALSE);
        mdm_dtr(DTR_DOWN);
        break;

      case '~':
        Mdm_flush_ck_tic(2000, FALSE, FALSE);
        Delay(100);
        break;

      case '`':
        Mdm_flush_ck_tic(2000, FALSE, FALSE);
        Delay(5);
        break;

      case '|':
        mdm_pputcw('\x0d');
        break;

      default:
        mdm_pputcw(command[x]);
        break;
    }

    Delay(1);
  }

  #ifdef SHORT_MDM_CMD

  return 0;

  #else

  retval=MDM_none;

  expire=timerset(500);

  while (retval==MDM_none)
  {
    for (buflen=ch=0;(retval==MDM_none) && (ch != '\r');)
    {
      if (timeup(expire))
        retval=MDM_timeout;
      else if (mdm_avail())
      {
        if (buflen >= MAX_cmdlen)
          retval=-1;
        else
        {
          buffer[buflen++]=ch=mdm_getc();

          if ((ch=='\n') || ((buflen==1) && (ch=='\r')))
          {
            buflen--;
            ch=0;
          }
        }
      }
      else Giveaway_Slice();
    }

    if (retval==MDM_none)
    {
      buffer[--buflen]='\0';

      if (! stricmp(buffer,command))
        continue;

      x=stricmpm(buffer,mdm_rsp,0xffff);

      for (z=0,y=1;z < 16;y = y << 1,z++)
        if (x & y)
          break;

      switch (z)
      {
        case 0: /* no carrier */
          retval=MDM_nocarrier;
          break;

        case 1: /* connect */
          getword(buffer,temp," \t\n",2);
          baud=atoi(temp);

          switch(baud)
          {
            case 0:
              if (eqstri(temp,"FAST"))    // Special case for PEP
              {
                retval=MDM_connect96
                break;
              }
            case 300:
              retval=MDM_connect3;
              break;

            case 600:
              retval=MDM_connect6;
              break;

            case 1200:
              retval=MDM_connect12;
              break;

            case 2400:
              retval=MDM_connect24;
              break;

            case 4800:
              retval=MDM_connect48;
              break;

            case 9600:
              retval=MDM_connect96;
              break;

            case 19200:
              retval=MDM_connect19;
              break;

            case 38400u:
              retval=MDM_connect38;
              break;

            default:
              retval=MDM_error;
              break;
          }
          break;

        case 2: /* busy */
          retval=MDM_busy;
          break;

        case 3: /* voice */
          retval=MDM_voice;
          break;

        case 4: /* ring */
          retval=MDM_ring;
          break;

        case 7: /* no dialtone */
        case 8: /* no dial tone */
          retval=MDM_nodialtone;
          break;

        case 9: /* dial tone */
          retval=MDM_dialtone;
          break;

        case 10: /* error */
          retval=MDM_error;
          break;

        case 11: /* ok */
          retval=MDM_ok;
          break;

        case 65535u:  /* anything else */
        case 5:       /* rring */
        case 6:       /* ringing */
          break;
      }
    }
  }

  mdm_dump(DUMP_INPUT);
  return retval;

  #endif
}

int mdm_cmd(char *cmd)
{
#if (COMMAPI_VER > 1)
  extern HCOMM hcModem;

  if (!ComIsAModem(hcModem))
    return 0;	/* Not much point sending modem commands to non-modems.. */
#endif

  return _mdm_cmd(cmd);
}

