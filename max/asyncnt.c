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

#ifndef __GNUC__
#pragma off(unreferenced)
#endif
static char __attribute__((unused)) rcs_id[]="$Id: asyncnt.c,v 1.3 2003/06/29 20:49:00 wesgarland Exp $";
#ifndef __GNUC__
#pragma on(unreferenced)
#endif

#if defined(NT) || defined(UNIX)

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "mm.h"
#include "modem.h"
#include "ntcomm.h"

extern void cdecl logit (char *fmt,...);

extern char waitforcaller;  /* Wait and grab caller ourselves  */

HCOMM hcModem=NULL;            /* comm.dll handle */

int GetConnectionType(void)
{
  return CTYPE_ASYNC;
}

void com_DTR_on(void)
{
  DCB dcb;

  GetCommState(ComGetHandle(hcModem), &dcb);
  dcb.fDtrControl=1;
  SetCommState(ComGetHandle(hcModem), &dcb);
}

void com_DTR_off(void)
{
  DCB dcb;

  GetCommState(ComGetHandle(hcModem), &dcb);
  dcb.fDtrControl=0;
  SetCommState(ComGetHandle(hcModem), &dcb);
}

void com_HHS_enable(int mask)
{
  DCB dcb;

  GetCommState(ComGetHandle(hcModem), &dcb);

  if (mask & FLOW_CTS)
  {
    dcb.fOutxCtsFlow = TRUE;
    /*dcb.fRtsControl = RTS_CONTROL_ENABLE;*/
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
  }

  SetCommState(ComGetHandle(hcModem), &dcb);
}

void com_HHS_disable(int mask)
{
  DCB dcb;

  GetCommState(ComGetHandle(hcModem), &dcb);
  dcb.fOutxCtsFlow = FALSE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
  SetCommState(ComGetHandle(hcModem), &dcb);
}

void com_XON_disable(void)
{
  DCB dcb;

  GetCommState(ComGetHandle(hcModem), &dcb);
  dcb.fOutX=dcb.fInX=0;
  SetCommState(ComGetHandle(hcModem), &dcb);
}

void com_XON_enable(void)
{
  DCB dcb;

  GetCommState(ComGetHandle(hcModem), &dcb);
  dcb.fOutX=dcb.fInX=1;
  SetCommState(ComGetHandle(hcModem), &dcb);
}


void com_break(int on)
{
  COMMHANDLE h=ComGetHandle(hcModem);

  if (on)
    SetCommBreak(h);
  else ClearCommBreak(h);
}

int Cominit(int port)
{
    char tmp[20];
    USHORT rc;
    HFILE hf;

    if(hcModem == NULL)
    {
        if (port_is_device)
        {
            sprintf(tmp, "com%1u", port+1);

            /* Bink/Opus/Max use 8K zmodem blocks.  Make the Rx and Tx queues at
             * least that size.  This won't affect Tx much,  but we don't want
             * Rx to ever overflow.
             */
            rc = !ComOpen(tmp, &hcModem, 8200, 8200);
        }
        else
        {
#ifndef UNIX
          sprintf(tmp, "handle %d", port+1);

          hf = (HFILE)port+1;  /* maximus subtracts 1 from the value on the command line. Add it back here. */
          rc = !ComOpenHandle((COMMHANDLE)hf, &hcModem, 8200, 8200);
#else
	  logit("!Not yet implemented; %s, %s:%i", __FUNCTION__, __FILE__, __LINE__);
	  _exit(1);
#endif
        }

        if(rc)
        {
            logit("!SYS%04u:  ComOpen(%s)", rc, tmp);
            return(0);
        }
    }
    else
        ComResume(hcModem);

    return (0x1954);
}

#endif /* NT */

