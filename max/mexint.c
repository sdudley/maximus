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
static char rcs_id[]="$Id: mexint.c,v 1.1.1.1 2002/10/01 17:52:25 sdudley Exp $";
#pragma on(unreferenced)

/* This file contains intrinsic MEX functions.
 *
 * NOTE!!!!
 *
 * If you want to add an intrinsic function that can be called by
 * an application MEX program, you must add an entry to the
 * intrinfunc table in mex.c, and you must also add the prototype
 * for the function to mexint.h.
 */

#include "mexall.h"

#ifdef MEX

  /* Get the chat status of a particular user */

  word EXPENTRY intrin_ChatQueryStatus(void)
  {
    MA ma;
    struct _cstat cs;
    struct mex_cstat *pmcs;
    word rc;

    /* Get pointer to the cstat structure */

    MexArgBegin(&ma);
    pmcs=MexArgGetRef(&ma);
    rc=MexArgEnd(&ma);

    /* See if the specified node is available... */

    if (!ChatFindIndividual(pmcs->task_num, cs.username, cs.status, &cs.avail) ||
        *cs.username==0)
    {
      regs_2[0]=FALSE;
      return rc;
    }

    /* Copy information to structure */

    pmcs->avail=cs.avail;

    MexKillStructString(mex_cstat, pmcs, username);
    StoreString(MexPtrToVM(pmcs), struct mex_cstat, username, cs.username);

    MexKillStructString(mex_cstat, pmcs, status);
    StoreString(MexPtrToVM(pmcs), struct mex_cstat, status, cs.status);

    regs_2[0]=TRUE;
    return rc;
  }




  /* Add a specified message to the system log */

  word EXPENTRY intrin_log(void)
  {
    MA ma;
    char *s;

    MexArgBegin(&ma);
    s=MexArgGetString(&ma, FALSE);
    logit(s);
    free(s);

    return MexArgEnd(&ma);
  }


  /* Convert a protocol number to a name */

  word EXPENTRY intrin_ProtocolNumberToName(void)
  {
    MA ma;
    char temp[PATHLEN];
    sbyte pnum;

    MexArgBegin(&ma);
    pnum=MexArgGetByte(&ma);

    if (pnum==PROTOCOL_NONE)
      strcpy(temp, proto_none);
    else
      Protocol_Name(pnum, temp);

    MexReturnString(temp);

    return MexArgEnd(&ma);
  }


  /* Convert a protocol number to a name */

  word EXPENTRY intrin_CompressorNumberToName(void)
  {
    MA ma;
    struct _arcinfo *ar;
    char temp[PATHLEN];
    sbyte cnum;

    MexArgBegin(&ma);
    cnum=MexArgGetByte(&ma);

    /* Get the name of the user's compressor */

    ar=UserAri(cnum);
    strcpy(temp, (ar && cnum && cnum <= MAX_ARI) ? ar->arcname : proto_none);

    MexReturnString(temp);

    return MexArgEnd(&ma);
  }

  /* Convert a language number to a name */

  word EXPENTRY intrin_LanguageNumberToName(void)
  {
    MA ma;
    char temp[PATHLEN];
    sbyte lnum;

    MexArgBegin(&ma);
    lnum=MexArgGetByte(&ma);

    if (lnum < MAX_LANG)
      strcpy(temp, PRM(lang_file[lnum]));
    else
      strcpy(temp, PRM(lang_file[0]));

    MexReturnString(temp);

    return MexArgEnd(&ma);
  }


  /* Return a specific string from the .PRM file */

  word EXPENTRY intrin_prm_string(void)
  {
    MA ma;
    OFS *pofs;
    int stringnum;

    MexArgBegin(&ma);
    stringnum=MexArgGetWord(&ma);

    #define PRM_NUM_STRINGS (offsetof(struct m_pointers, PRM_HEAP_END)-offsetof(struct m_pointers, PRM_HEAP_START))

    pofs=&prm.PRM_HEAP_START;

    if (stringnum < PRM_NUM_STRINGS)
      MexReturnString(offsets + pofs[stringnum]);
    else
      MexReturnString("");

    return MexArgEnd(&ma);
  }



  /* Sleep for a number of centiseconds */

  word EXPENTRY intrin_sleep(void)
  {
    MA ma;
    word wSleep;

    MexArgBegin(&ma);
    wSleep=MexArgGetWord(&ma);
    Delay(wSleep);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_privok(void)
  {
    MA ma;
    char *s;

    MexArgBegin(&ma);
    s=MexArgGetString(&ma, FALSE);

    regs_2[0] = FALSE;

    if (s)
    {
      regs_2[0] = (word)PrivOK(s, FALSE);
      free(s);
    }

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_chatstart(void)
  {
    MA ma;

    MexArgBegin(&ma);
    inchat=TRUE;
    chatreq=FALSE;

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_xfertime(void)
  {
    MA ma;
    sword protocol;
    long bytes;

    MexArgBegin(&ma);
    protocol=MexArgGetWord(&ma);
    bytes=MexArgGetDword(&ma);
    regs_4[0]=XferTime(protocol, bytes);
    return MexArgEnd(&ma);
  }

#endif /* MEX */

