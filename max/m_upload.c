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
static char rcs_id[]="$Id: m_upload.c,v 1.1.1.1 2002/10/01 17:52:51 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: U)pload command
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "max_file.h"
#include "arcmatch.h"


/* Determines if an uploaded message is a compressed .zip file */

static int near MsgIsCompressed(FILE *fp)
{
  int fd=fileno(fp);
  struct _arcinfo *ai;

  Load_Archivers();

  /* Scan for the various archive types */

  for (ai=ari; ai; ai=ai->next)
    if (MatchArcType(fd, ai))
      return TRUE;

  return FALSE;
}



void Msg_Upload(void)
{
  FILE *inmsg;
  XMSG msg;

  char temp[PATHLEN];

  int aborted;
  sword protocol;

  FENTRY fent;


  if (local)
  {
    Puts(err999_1);
    InputGets(temp,err999_2);

    return;
  }

  aborted=isareply=isachange=FALSE;

  Blank_Msg(&msg);
  *netnode='\0';
  *orig_msgid='\0';

  
  /* Get the header for the message to receive. */
  
  if (GetMsgAttr(&msg, &mah, usr.msg, 0L, -1L)==-1)
  {
    Puts(xferaborted);
    return;
  }

  if (File_Get_Protocol(&protocol, FALSE, TRUE)==-1)
    return;

  Free_Filenames_Buffer(0);

  /* Name of the file we want to get. */
  
  sprintf(temp, msgtemp_name, blank_str, task_num);

  
  /* Perform the actual upload */

  if (AddFileEntry(temp, 0, 0L)!=-1)
  {
    word sf2;

    /* Turn off dupe checking for msg uploads */

    sf2=prm.flags2;
    prm.flags2 &= ~(FLAG2_CHECKDUPE|FLAG2_CHECKEXT);
  
    File_Get_Files(protocol, NULL, PRM(temppath));

    prm.flags2=sf2;
  }


  /* If we didn't get anything, abort */
  
  if (FileEntries()==0)
  {
    Puts(xferaborted);
    return;
  }

  GetFileEntry(0, &fent);
 

  sprintf(temp, ss, PRM(temppath), fent.szName);
  
  /* Try to open the file we just received */
  
  if ((inmsg=shfopen(temp, fopen_readb, O_RDONLY | O_BINARY | O_NOINHERIT))==NULL)
  {
    cant_open(temp);
    Puts(xferaborted);
    unlink(temp);
  }
  else
  {

       /* Make sure that the user didn't try to upload a compressed file */

    if (MsgIsCompressed(inmsg))
    {
      Puts(xferaborted);
      Puts(dont_upload_compressed);
      Press_ENTER();
      fclose(inmsg);
      unlink(temp);
    }
    else
    {
      word fn;
  
      aborted=SaveMsg(&msg, inmsg, FALSE, 0L, FALSE, &mah, usr.msg, sq, NULL, NULL, FALSE);
      fclose(inmsg);

                       /* Delete all of the files that the user uploaded */

      for (fn=0; GetFileEntry(fn, &fent); ++fn)
      {
        sprintf(temp, ss, PRM(temppath), No_Path(fent.szName));
        unlink(temp);
    /*  logit("@Deleting %s", temp);*/
      }

      if (aborted)
        Puts(msg_aborted);

    }
  }

  Free_Filenames_Buffer(0);
}

