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

#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mexall.h"

#ifdef MEX

  word EXPENTRY intrin_call_open(void)
  {
    regs_2[0]=FALSE;
    if (pmisThis->fhCallers == (word)-1)
    {
      int fd;
      char temp[PATHLEN];

      ci_filename(temp);
      if (*temp && (fd=shopen(temp, O_RDONLY|O_BINARY))!=-1)
      {
        regs_2[0]=TRUE;
        pmisThis->fhCallers=(word)fd;
      }
    }
    return 0;
  }

  word EXPENTRY intrin_call_close(void)
  {
    if (pmisThis->fhCallers != (word)-1)
      close(pmisThis->fhCallers);
    regs_2[0]=TRUE;
    return 0;
  }

  word EXPENTRY intrin_call_numrecs(void)
  {
    if (pmisThis->fhCallers==(word)-1)
      regs_4[0]=(dword)0;
    else regs_4[0]=(dword)lseek(pmisThis->fhCallers,0L,SEEK_END)/sizeof(struct callinfo);
    return 0;
  }

  word EXPENTRY intrin_call_read(void)
  {
    MA ma;
    dword rec;
    struct mex_callinfo * pmci;
    struct callinfo ci;

    MexArgBegin(&ma);
    rec=MexArgGetDword(&ma);
    pmci=MexArgGetRef(&ma);
    regs_2[0]=FALSE;
    if (pmisThis->fhCallers!=(word)-1 &&
        lseek(pmisThis->fhCallers,rec*sizeof(ci),SEEK_SET)!=-1L &&
        read(pmisThis->fhCallers,&ci,sizeof ci) == sizeof ci)
    {
      MexKillStructString(mex_callinfo, pmci, name);
      MexKillStructString(mex_callinfo, pmci, city);
      MexKillStructString(mex_callinfo, pmci, logon_xkeys);
      MexKillStructString(mex_callinfo, pmci, logoff_xkeys);
      StoreString(MexPtrToVM(pmci), struct mex_callinfo, name, ci.name);
      StoreString(MexPtrToVM(pmci), struct mex_callinfo, city, ci.city);
      StoreString(MexPtrToVM(pmci), struct mex_callinfo, logon_xkeys, Keys(ci.logon_xkeys));
      StoreString(MexPtrToVM(pmci), struct mex_callinfo, logoff_xkeys, Keys(ci.logoff_xkeys));
      StampToMexStamp(&ci.login, &pmci->login);
      StampToMexStamp(&ci.logoff, &pmci->logoff);
      pmci->task=ci.task;
      pmci->flags=ci.flags;
      pmci->logon_priv=ci.logon_priv;
      pmci->logoff_priv=ci.logoff_priv;
      pmci->filesup=ci.filesup;
      pmci->filesdn=ci.filesdn;
      pmci->kbup=ci.kbup;
      pmci->kbdn=ci.kbdn;
      pmci->calls=ci.calls;
      pmci->read=ci.read;
      pmci->posted=ci.posted;
      pmci->paged=ci.paged;
      pmci->added=ci.added;
      regs_2[0]=TRUE;
    }
    return MexArgEnd(&ma);
  }

#endif /* MEX */

