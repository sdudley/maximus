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
static char rcs_id[]="$Id: m_editor.c,v 1.1.1.1 2002/10/01 17:52:39 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Editor shell
*/

#define MAX_LANG_max_chat
#define MAX_LANG_max_bor

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "max_msg.h"
#include "max_edit.h"
#include "m_reply.h"

int Editor(XMSG *msg, HMSG msgh, long msgnum, char *ctrl_buf, struct _replyp *pr)
{
  struct _css css;

  int ret, useext=FALSE;

  last_maxed=FALSE;

  ChatSaveStatus(&css);
  ChatSetStatus(FALSE, cs_enter_msg);

  logit(usingeditor);
  
  Puts(GRAY "\n");

  /* This might be used to display help, or set up the screen
   * for the FSR in RIP mode etc.
   */

  if (prm.hlp_msgentry)
    Display_File(0, NULL, PRM(hlp_msgentry));

  if (*PRM(local_editor))
  {
    if (local || *PRM(local_editor)=='@')
    {
      if (local && mailflag(CFLAGM_LEDITOR))
        useext = TRUE;

      if (!local && mailflag(CFLAGM_EDITOR))
        useext = TRUE;
    }
  }

  if (useext)
    ret=Local_Editor(msg, msgh, msgnum, ctrl_buf, pr);
  else if ((usr.bits2 & BITS2_BORED) || (prm.flags & FLAG_no_magnet))
  {
    Puts(CLEOS);
    ret=Bored(msg, msgh, pr);
  }
  else if ((usr.bits2 & BITS2_BORED)==0 &&
           (usr.video==GRAPH_TTY || TermWidth() < 79 || TermLength() < 24))
  {
    Puts(CLEOS);
    Puts(req_graph);
    ret=Bored(msg, msgh, pr);
  }
  else
  {
    Puts(CLS);

    inmagnet=last_maxed=TRUE;
    ret=MagnEt(msg,msgh,pr);
    inmagnet=FALSE;
  }

  ChatRestoreStatus(&css);
  logit(outofeditor);

  return ret;
}

