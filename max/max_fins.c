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
static char rcs_id[]="$Id: max_fins.c,v 1.1.1.1 2002/10/01 17:51:38 sdudley Exp $";
#pragma on(unreferenced)

#define MAX_LANG_max_init
#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "mm.h"

void Fossil_Install(int fInitVideo)
{
  if (mdm_init(local ? 0xff : port)==INIT_nofossil)
  {
    if (local)
      return;
    else
    {
      logit(log_no_fossil);
      quit(ERROR_NOFOSSIL);
    }
  }

  fossil_initd=TRUE;

  Mdm_Flow_On();
  
  if (fInitVideo)
  {
    if (!no_video)
    #ifdef TTYVIDEO
      if (displaymode==VIDEO_IBM)
    #endif
        WinSyncAll();
  }
}

