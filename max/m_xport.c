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
static char rcs_id[]="$Id: m_xport.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message Section: X)port routine
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#ifdef UNIX
# include <errno.h>
#endif

void Msg_Xport(void)
{
  FILE *out;
  HMSG hmsg;
  XMSG msg;
  word n;

  dword msgnum;

  char temp[PATHLEN];

  RipClear();

  if (!GetMsgNum(xport_which, &msgnum))
    return;

  if ((hmsg=MsgOpenMsg(sq, MOPEN_READ, msgnum))==NULL ||
      MsgReadMsg(hmsg, &msg, 0L, 0L, NULL, 0L, NULL)==-1 ||
      !CanSeeMsg(&msg))
  {
    if (hmsg)
      MsgCloseMsg(hmsg);
    
    return;
  }
    
  InputGets(temp, xport_where);
        
  if (! *temp ||
      (out=shfopen(temp, fopen_append, O_WRONLY | O_APPEND | O_NOINHERIT))==NULL)
  {
    if (*temp)
      Printf(cantopen+1, temp, errno);

    MsgCloseMsg(hmsg);
    return;
  }
  
  Puts(xporting);

  /* Message number, area, date */
  
  {
    char *lf, *ri;
    word doit;
    
    if (mah.ma.attribs & MA_NET)
    {
      lf="(";
      ri=")";
      doit=TRUE;
    }
    else
    {
      lf=ri=blank_str;
      doit=FALSE;
    }

    fprintf(out, xp_hdr1, MAS(mah, name),  msgnum, MsgDate(&msg, temp));

    fprintf(out, xp_hdr2, msg.from, lf, 
            doit ? Address(&msg.orig) : blank_str, ri);

    fprintf(out, xp_hdr3, msg.to,   lf, 
            doit ? Address(&msg.dest) : blank_str, ri);

    fprintf(out, xp_hdr4, msg.subj);
  }

  /* Now copy the message body to the file */

  MsgBodyToFile(hmsg, out, FALSE, FALSE, blank_str, MRL_QEXP);
    
  /* Dividing line */
  
  for (n=39; n--; )
    fputs("--", out);

  fputc('\n', out);
  fputc('\n', out);

  fclose(out);
  MsgCloseMsg(hmsg);
  
  Puts(done_ex);
}


