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
static char rcs_id[]="$Id: rbsb.c,v 1.1.1.1 2002/10/01 17:54:40 sdudley Exp $";
#pragma on(unreferenced)

#include "zsjd.h"
#include "pdata.h"

/*
 * mode(n)
 *  3: save old tty stat, set raw mode with flow control
 *  2: set XON/XOFF for sb/sz with ZMODEM or YMODEM-g
 *  1: save old tty stat, set raw mode 
 *  0: restore original tty mode
 */

int mode(int m)
{
  dlogit(("@Zmodem mode %d", m));

  switch (m)
  {
    case 3:
      Mdm_Flow(FLOW_PARTIAL_OFF);
      break;

    case 2:
      Mdm_Flow(FLOW_NO_CCK);
      break;

    case 1:
      Mdm_Flow(FLOW_OFF);
      break;

    case 0:
      Mdm_Flow(FLOW_ON);
      break;

    default:
      return ERROR;
  }

  return OK;
}

void sendbrk(void)
{
  Delay(100);
  mdm_break(100);
}

