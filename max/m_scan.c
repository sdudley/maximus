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
static char rcs_id[]="$Id: m_scan.c,v 1.1.1.1 2002/10/01 17:52:50 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Msg_Checkmail, S)can and L)ist
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"


void Msg_Scan(char *menuname)
{
  SEARCH first;

  RipClear();

  Init_Search(&first);
  first.txt=strdup(usr.name);
  first.attr=MSGREAD;
  first.flag=SF_NOT_ATTR | SF_OR;
  first.where=WHERE_TO;

  Msg_Browse(BROWSE_AALL | BROWSE_NEW | BROWSE_EXACT | BROWSE_LIST, &first,
             menuname);
}

int Msg_List(char *menuname)
{
  RipClear();

  Msg_Browse(BROWSE_ACUR | BROWSE_FROM | BROWSE_LIST, NULL, menuname);
  return 0;
}

