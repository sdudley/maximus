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

#ifdef MEX

#include "ffind.h"

  static void findinfo(struct mex_ffind *ff)
  {
    MexKillStructString(mex_ffind, ff, filename);
    StoreString(MexPtrToVM(ff), struct mex_ffind, filename, (ff->finddata)->szName);
    ff->filesize=(ff->finddata)->ulSize;
    ff->filedate.date.day   = (ff->finddata)->scWdate.msg_st.date.da;
    ff->filedate.date.month = (ff->finddata)->scWdate.msg_st.date.mo;
    ff->filedate.date.year  = (ff->finddata)->scWdate.msg_st.date.yr;
    ff->filedate.time.hh    = (ff->finddata)->scWdate.msg_st.time.hh;
    ff->filedate.time.mm    = (ff->finddata)->scWdate.msg_st.time.mm;
    ff->filedate.time.ss    = (ff->finddata)->scWdate.msg_st.time.ss;
    ff->fileattr=(ff->finddata)->usAttr;
  }

  word EXPENTRY intrin_filefindfirst(void)
  {
    MA ma;
    char *psz;
    word attribs;
    struct mex_ffind *ff;

    MexArgBegin(&ma);
    ff=MexArgGetRef(&ma);
    psz=MexArgGetString(&ma,FALSE);
    attribs=MexArgGetWord(&ma);
    regs_2[0]=0;
    if (!psz)
      ff->finddata=NULL;
    else
    {
      ff->finddata=FindOpen(psz, attribs);
      if (ff->finddata)
      {
        findinfo(ff);
        regs_2[0]=1;
      }
      free(psz);
    }

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_filefindnext(void)
  {
    MA ma;
    struct mex_ffind *ff;

    MexArgBegin(&ma);
    ff=MexArgGetRef(&ma);
    regs_2[0]=0;
    if (ff->finddata && FindNext(ff->finddata)==0)
    {
      findinfo(ff);
      regs_2[0]=1;
    }

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_filefindclose(void)
  {
    MA ma;
    struct mex_ffind *ff;

    MexArgBegin(&ma);
    ff=MexArgGetRef(&ma);
    if (ff->finddata)
    {
      FindClose(ff->finddata);
      ff->finddata=0;
    }
    regs_2[0]=0;

    return MexArgEnd(&ma);
  }

#endif /* MEX */

