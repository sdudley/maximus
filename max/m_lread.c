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
static char rcs_id[]="$Id: m_lread.c,v 1.1.1.1 2002/10/01 17:52:46 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Lastread and message-scanning routines.
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "alc.h"
#include "max_msg.h"


/* Update the user's lastread pointer in the LASTREAD.BBS file for each
   area.                                                                  */

void FixLastread(HAREA lsq, word type, dword lastmsg, char *path)
{
  void *where;
  char temp[PATHLEN];

  UMSGID uid;
  
  int lrfile, size;
  word tempword;

  dword tdword;
  long offset;

  if (!lsq || chkmail_reply)
    return;

  uid=MsgMsgnToUid(lsq, lastmsg);

  /* If it hasn't changed, or if we're doing a mailcheck, don't update it! */

  if (last_lastread==uid)
    return;

  if (type & MSGTYPE_SDM)
  {
    sprintf(temp, usr.lastread_ptr ? ps_lastread : ps_lastread_single,
            path);
          
    size=sizeof(word);
    tempword=(word)uid;
    where=&tempword;
  }
  else
  {
    sprintf(temp, sq_lastread, path);

    size=sizeof(UMSGID);
    where=&uid;
  }


  /* Open and/or create the file as necessary */

  if ((lrfile=sopen(temp, O_CREAT | O_RDWR | O_BINARY | O_NOINHERIT,
                    SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
  {
    cant_open(temp);
    return;
  }

  lseek(lrfile, 0L, SEEK_END);

  /* Have to write some zeros. */

  if (tell(lrfile) < (offset=(long)usr.lastread_ptr * (long)size))
  {
    tdword=0;

    while (tell(lrfile) < offset)         /* Write zeros... */
      if (write(lrfile, (char *)&tdword, size) != size)
      {
        logit(cantwrite, temp);
        close(lrfile);
        return;
      }
  }

  lseek(lrfile, offset, SEEK_SET);


  /* If no disk space... */

  if (write(lrfile, (char *)where, size) != size)
    logit(cantwrite, temp);

  close(lrfile);
  last_lastread=uid;
}



/* Read the user's lastread pointer for this area, from LASTREAD.BBS */

void ScanLastreadPointer(dword *lastmsg)
{
  void *where;
  char temp[PATHLEN];

  UMSGID uid;

  word tempword, size;
  int lrfile;


  if (mah.ma.type & MSGTYPE_SDM)
  {
    sprintf(temp, usr.lastread_ptr ? ps_lastread : ps_lastread_single,
            MAS(mah, path));

    size=sizeof(word);
    where=&tempword;
  }
  else
  {
    sprintf(temp, sq_lastread, MAS(mah, path));
    size=sizeof(UMSGID);
    where=&uid;
  }

  if (! fexist(temp))     /* Create new lastread file! */
  {
    uid=0;

    if ((lrfile=sopen(temp, O_WRONLY | O_CREAT | O_BINARY | O_NOINHERIT,
                            SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
    {
      cant_open(temp);
    }
    else close(lrfile);
  }
  else
  {
    if ((lrfile=shopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    {
      uid=0;
    }
    else
    {
      long ofs=(long)usr.lastread_ptr*(long)size;

      if (lseek(lrfile, ofs, SEEK_SET) != ofs ||
          read(lrfile, (char *)where, size) < size)
      {
        uid=0L;
      }
      else
      {
        if (size==sizeof(word))
          uid=(long)tempword;
      }

      close(lrfile);
    }
  }

  last_lastread=uid;

  *lastmsg=MsgUidToMsgn(sq, uid, UID_PREV);

  if (*lastmsg==(dword)-1L)
  {

    *lastmsg=0L;
    last_lastread=0L;
  }

  if (*lastmsg > MsgHighMsg(sq))
  {
    last_lastread=0L;
    *lastmsg=MsgHighMsg(sq);
  }
}


