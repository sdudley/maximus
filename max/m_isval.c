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
static char rcs_id[]="$Id: m_isval.c,v 1.1.1.1 2002/10/01 17:52:44 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Routine to validate message areas
*/

#define MAX_LANG_max_main
#include <string.h>
#include "prog.h"
#include "max_msg.h"

static int near _ValidMsgArea(PMAH pmah, unsigned flags, BARINFO *pbi)
{
  char *bar;

  if ((flags & VA_OVRPRIV)==0 && !PrivOK(PMAS(pmah, acs), TRUE))
    return FALSE;

    /* Make sure there's a message area here... */
      
  if (isblstr(PMAS(pmah, path)))
    return FALSE;

  if ((flags & VA_VAL) && !MsgValidate(pmah->ma.type, PMAS(pmah, path)))
    return FALSE;

  if ((flags & VA_OVRPRIV)==0 &&
      *(bar=PMAS(pmah, barricade)) && (flags & VA_PWD))
  {
    if (!GetBarPriv(bar, TRUE, pmah, NULL, pbi, !!(flags & VA_EXTONLY)))
      return FALSE;
  }

  return TRUE;
}


int ValidMsgArea(char *name, MAH *pmah, unsigned flags, BARINFO *pbi)
{
  MAH ma;
  int rc;

  memset(&ma, 0, sizeof ma);

  pbi->use_barpriv=FALSE;

  /* Use the current area, if supplied */

  if (pmah)
    ma=*pmah;
  else if (!ReadMsgArea(ham, name, &ma))
    return FALSE;

  rc=_ValidMsgArea(&ma, flags, pbi);

  if (!pmah)
    DisposeMah(&ma);

  return rc;
}

void ForceGetMsgArea(void)
{
  BARINFO bi;
  int bad=FALSE;    /* To ensure that the begin_msgarea is valid */
  int first;

  do
  {
    /* Enable the VA_PWD check only on the first time through this loop.
     * On second and subsequent times, the password checking will be done
     * within Msg_Area itself, but we need to prompt for the password
     * at least once.
     */

    first=TRUE;

    while (!ValidMsgArea(usr.msg, NULL, VA_VAL | (first ? VA_PWD : 0), &bi) ||
           bad)
    {
      first=FALSE;
      Puts(inval_cur_msg);
      Msg_Area();
      return;       /* Msg_Area will have pushed a valid area on the stack */
    }

    bad=TRUE;
  }
  while (!PushMsgArea(usr.msg, &bi));
}




