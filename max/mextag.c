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

#include "mexall.h"
#include "max_file.h"

#ifdef MEX

  /* Add a file to the download queue */

  word EXPENTRY intrin_tag_queue_file(void)
  {
    MA ma;
    word wFlags;
    char *szFile;

    MexArgBegin(&ma);

    szFile = MexArgGetString(&ma, FALSE);
    wFlags = MexArgGetWord(&ma);

    /* Assume return value of FALSE unless all checks succeed */

    regs_2[0]=FALSE;

    if (szFile)
    {
      long lSize=fsize(szFile);
      upper_fn(szFile);

      if (lSize != -1L &&
          AddFileEntry(szFile,wFlags|FFLAG_OK|FFLAG_TAG, lSize)!=-1)
      {
        regs_2[0]=TRUE;
      }

      free(szFile);
    }

    return MexArgEnd(&ma);
  }


  /* Remove one file from the download queue */

  word EXPENTRY intrin_tag_dequeue_file(void)
  {
    MA ma;
    word wPosn;

    MexArgBegin(&ma);

    wPosn=MexArgGetWord(&ma);
    regs_2[0]=(RemoveFileEntry(wPosn)) ? TRUE : FALSE;
    return MexArgEnd(&ma);
  }



  /* Return size of file tag download queue */

  word EXPENTRY intrin_tag_queue_size(void)
  {
    MA ma;

    MexArgBegin(&ma);

    regs_2[0]=FileEntries();

    return MexArgEnd(&ma);
  }


  /* Return the file in a particular position in the queue */

  word EXPENTRY intrin_tag_get_name(void)
  {
    int fn;
    IADDR ia;
    word wLen;
    word *pw;
    char *str;
    MA ma;

    FENTRY fent;

    MexArgBegin(&ma);

    /* Get the queue position, the flags reference, and the name reference */

    fn=MexArgGetWord(&ma);
    pw=(word *)MexArgGetRef(&ma);
    MexArgGetRefString(&ma, &ia, &wLen);

    if (GetFileEntry(fn, &fent))
    {
      str=fent.szName;
      *pw=fent.fFlags;
      regs_2[0]=TRUE;
    }
    else
    {
      str=blank_str;
      *pw=0;
      regs_2[0]=FALSE;
    }

    MexKillString(&ia);
    MexStoreByteStringAt(MexIaddrToVM(&ia), str, strlen(str));

    return MexArgEnd(&ma);
  }



#endif /* MEX */

