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
static char rcs_id[]="$Id: m_inq.c,v 1.1.1.1 2002/10/01 17:52:43 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: I)nquire command
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"

void Msg_Inquire(char *menuname)
{
  RipClear();

  Msg_Browse(BROWSE_ACUR | BROWSE_SEARCH | BROWSE_LIST, NULL, menuname);

#ifdef NEVER
  HMSG msgh;
  XMSG msg;
  struct _rep rexp;

  char temp[PATHLEN],
       nonstop,             /* If we're doing non-stop listing   */
       from[45],            /* Holding area for FROM             */
       to[45],              /* Holding area for TO               */
       subject[81],         /* Holding area for SUBJ             */
       expr[PATHLEN],
       *p;

  int found,
      x,y,z;
    
  long msgn;

  while (1)
  {
    WhiteN();

    if (usr.bits & BITS_HOTKEYS)
      InputGetsLL(temp,PATHLEN,inq_prompt);
    else
    {
      if (! *linebuf)
        Puts(inq_p1);

      InputGetsLL(temp,PATHLEN,inq_p2);
    }

    strcpy(expr,temp);

    if (eqstr(temp,qmark))
      Display_File(0,NULL,PRM(msg_inquire));
    else break;
  }

  strlwr(temp);

  if (! *temp || Compile_REP(temp,&rexp) != 0)
  {
    Puts(inq_badexpr);
    return;
  }

  display_line=display_col=1;
  nonstop=FALSE;

  Printf(inq_search,temp);

  for (msgn=1L,found=FALSE;
       msgn < MsgHighMsg(sq) && (msgh=MsgOpenMsg(sq,MOPEN_READ,msgn)) != NULL;
       msgn++)
  {
    MsgReadMsg(msgh,&msg,0L,0L,NULL,0L,NULL);
    MsgCloseMsg(msgh);

    if (MoreYnBreak(&nonstop,CYAN))
      break;

    if (brk_trapped || mdm_halt())
    {
      brk_trapped=0;
      mdm_dump(DUMP_ALL);
      break;
    }
    
    Mdm_check();

    if (! CanSeeMsg(&msg))
      continue;

    strcpy(to,strlwr(msg.to));
    strcpy(from,strlwr(msg.from));
    strcpy(subject,strlwr(msg.subj));

    Strip_Ansi(to,usr.msg,msgn);
    Strip_Ansi(from,usr.msg,msgn);
    Strip_Ansi(subject,usr.msg,msgn);

    if ((p=stristrx(to,&rexp)) != NULL)
      z=1;
    else
    {
      if ((p=stristrx(from,&rexp)) != NULL)
        z=2;
      else
      {
        p=stristrx(subject,&rexp);
        z=3;
      }
    }

    if (!p)     /* If we didn't find anything, then go on to next msg */
      continue;
      
    found=TRUE;

    /* Move everything AFTER the string we found, to the right
       three places, so we can insert the gray colour code here */

    memmove(p+rexp.max_ch+3,p+rexp.max_ch,strlen(p+rexp.max_ch)+1);
    p[rexp.max_ch  ]='\x16';
    p[rexp.max_ch+1]='\x01';
    p[rexp.max_ch+2]='\x07';

    for (y=0;y < rexp.max_ch;y++)
      p[y]=toupper(p[y]);

    /* Now move everything after the FRONT of the string we found,
       to the right three places, so we can insert the light-yellow
       colour code.                                              */

    memmove(p+3,p,strlen(p)+1);
    p[0]='\x16';
    p[1]='\x01';
    p[2]='\x0e';

    /* Now fancy_str anything in the fields that we didn't touch */

    switch (z)
    {
      case 1:             /* String found in TO: field */
        fancier_str(from);
        fancy_str(subject);
        break;

      case 2:             /* String found in the FROM: field */
        fancy_str(subject);
        fancier_str(to);
        break;

      case 3:             /* String found in the SUBJ: field */
        fancier_str(from);
        fancier_str(to);
        break;
    }

    /* Now display the message header */

    Printf(ifmt_1,msgn,to);
    Printf(ifmt_2,from);
    Printf(ifmt_3,subject);

    vbuf_flush();

    if (MoreYnBreak(&nonstop,CYAN))
      break;
  }

  /* Free set-search tables */

  for (x=0;x < rexp.max_ch;x++)
    if (rexp.table[x])
      free(rexp.table[x]);

  if (!found)
    Printf(inq_nf,expr);

  Clear_KBuffer();
#endif
}


