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

  static void near ReadUser(struct mex_usr *pusr, struct _usr *user)
  {
    MexKillStructString(mex_usr, pusr, name);
    MexKillStructString(mex_usr, pusr, city);
    MexKillStructString(mex_usr, pusr, alias);
    MexKillStructString(mex_usr, pusr, phone);
    MexKillStructString(mex_usr, pusr, pwd);
    MexKillStructString(mex_usr, pusr, dataphone);
    MexKillStructString(mex_usr, pusr, xkeys);
    MexKillStructString(mex_usr, pusr, msg);
    MexKillStructString(mex_usr, pusr, files);
    MexKillStructString(mex_usr, pusr, dob);
    if (strcmp(usr.name,user->name)==0)
      user=&usr;
    MexExportUser(pusr, user);
  }

  static int near OpenUser(void)
  {
    if (pmisThis->huf != NULL ||
        (pmisThis->huf=UserFileOpen(PRM(user_file), 0))!=NULL)
      return TRUE;
    return FALSE;
  }

  word EXPENTRY intrin_userfindopen(void)
  {
    MA ma;
    char *szName, *szAlias;
    struct mex_usr *u;

    MexArgBegin(&ma);
    szName=MexArgGetString(&ma, FALSE);
    szAlias=MexArgGetString(&ma, FALSE);
    u=MexArgGetRef(&ma);
    regs_2[0]=FALSE;
    if (OpenUser())
    {
      if (pmisThis->huff)
        UserFileFindClose(pmisThis->huff);
      if ((pmisThis->huff=UserFileFindOpen(pmisThis->huf,
                                           (szName && *szName) ? szName : NULL,
                                           (szAlias && *szAlias) ? szAlias : NULL))
                          !=NULL)
      {
        ReadUser(u,&pmisThis->huff->usr);
        regs_2[0]=TRUE;
      }
    }

    if (szName)
      free(szName);
    if (szAlias)
      free(szAlias);

    return MexArgEnd(&ma);
  }

  static word UserFind(int dir)
  {
    MA ma;
    struct mex_usr *u;

    MexArgBegin(&ma);
    u=MexArgGetRef(&ma);
    regs_2[0]=FALSE;
    if (pmisThis->huf != NULL && pmisThis->huff != NULL)
    {
      int rc;

      rc = (dir ? UserFileFindNext : UserFileFindPrior)(pmisThis->huff, NULL, NULL);
      if (rc)
      {
        ReadUser(u,&pmisThis->huff->usr);
        regs_2[0]=TRUE;
      }
    }
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_userfindnext(void)
  {
    return UserFind(TRUE);
  }
  word EXPENTRY intrin_userfindprev(void)
  {
    return UserFind(FALSE);
  }
  word EXPENTRY intrin_userfindclose(void)
  {
    if (pmisThis->huf != NULL && pmisThis->huff != NULL)
    {
      UserFileFindClose(pmisThis->huff);
      pmisThis->huff=NULL;
    }
    return 0;
  }

  word EXPENTRY intrin_userfilesize(void)
  {
    regs_4[0] = (OpenUser()) ? UserFileSize(pmisThis->huf) : -1L;
    return 0;
  }

  word EXPENTRY intrin_userupdate(void)
  {
    MA ma;
    char *szName, *szAlias;
    struct mex_usr *u;

    MexArgBegin(&ma);
    u=MexArgGetRef(&ma);
    szName=MexArgGetString(&ma, FALSE);
    szAlias=MexArgGetString(&ma, FALSE);
    regs_2[0]=FALSE;
    if (OpenUser())
    {
      struct _usr user;

      MexImportUser(u, &user);
      if (strcmp(szName, usr.name)==0)
      {
        usr=user;
        regs_2[0]=TRUE;
      }
      else if (UserFileUpdate(pmisThis->huf,
                              (szName && *szName) ? szName : NULL,
                              (szAlias && *szAlias) ? szAlias : NULL,
                              &user))
        regs_2[0]=TRUE;
    }

    if (szName)
      free(szName);
    if (szAlias)
      free(szAlias);

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_usercreate(void)
  {
    MA ma;
    struct mex_usr *u;

    MexArgBegin(&ma);
    u=MexArgGetRef(&ma);
    regs_2[0]=FALSE;
    if (OpenUser())
    {
      struct _usr user;
      int iHighLastread = -1;
      HUFF huff;

      /* Find a new lastread pointer for this user */

      if ((huff=UserFileFindOpen(pmisThis->huf, NULL, NULL))==NULL)
        iHighLastread=-1;
      else
      {
        do
        {
          if (huff->usr.lastread_ptr > iHighLastread)
            iHighLastread=huff->usr.lastread_ptr;
        }
        while (UserFileFindNext(huff, NULL, NULL));

        UserFileFindClose(huff);
      }

      /* Now import the user record and set it appropriately */

      MexImportUser(u, &user);
      user.lastread_ptr = iHighLastread + 1;


      if (UserFileCreateRecord(pmisThis->huf, &user, TRUE))
        regs_2[0]=TRUE;
    }

    return MexArgEnd(&ma);
  }
  word EXPENTRY intrin_userremove(void)
  {
    MA ma;
    struct mex_usr *u;

    MexArgBegin(&ma);
    u=MexArgGetRef(&ma);
    regs_2[0]=FALSE;
    if (OpenUser())
    {
      struct _usr user;

      MexImportUser(u, &user);
      if (strcmp(user.name, usr.name)!=0 &&
        UserFileRemove(pmisThis->huf, &user))
        regs_2[0]=TRUE;
    }

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_userfindseek(void)
  {
    MA ma;
    dword recnum;
    struct mex_usr *u;
    struct _usr user;

    MexArgBegin(&ma);
    recnum=MexArgGetDword(&ma);
    u=MexArgGetRef(&ma);
    regs_2[0]=FALSE;
    if (OpenUser() && UserFileSeek(pmisThis->huf, recnum, &user, sizeof user))
    {
      ReadUser(u,&user);
      regs_2[0]=TRUE;
    }
    return MexArgEnd(&ma);
  }

#endif /* MEX */
