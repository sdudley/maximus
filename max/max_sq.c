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
static char rcs_id[]="$Id: max_sq.c,v 1.1.1.1 2002/10/01 17:52:03 sdudley Exp $";
#pragma on(unreferenced)

#include <string.h>
#include "alc.h"
#include "prog.h"
#include "max_msg.h"


LLPUSH lam=NULL, laf=NULL;

void EnterFileAreaBarricade(void)
{
  if (!eqstri(CurrentMenuName(), FAS(fah, barricademenu)))
    return;

  if (mah.heap && !eqstri(CurrentMenuName(), MAS(mah, barricademenu)))
    ExitMsgAreaBarricade();

  if (fah.bi.use_barpriv)
  {
    /* Save current priv level on stack */

    laf->biOldPriv.use_barpriv=TRUE;
    laf->biOldPriv.priv=usr.priv;
    laf->biOldPriv.keys=usr.xkeys;

    /* Set the user's priv level to that of the barricade priv */

    usr.priv = fah.bi.priv;
    usr.xkeys |= fah.bi.keys;
  }
  else
  {
    laf->biOldPriv.use_barpriv=FALSE;
  }
}

void ExitFileAreaBarricade(void)
{
  /* Restore the user's priv level after the barricade */

  if (laf && laf->biOldPriv.use_barpriv)
  {
    /* If the sysop didn't already change the user's priv level */

    if (!locked)
    {
      usr.priv=laf->biOldPriv.priv;
      usr.xkeys=laf->biOldPriv.keys;
    }

    laf->biOldPriv.use_barpriv=FALSE;
  }
}




void EnterMsgAreaBarricade(void)
{
  if (!eqstri(CurrentMenuName(), MAS(mah, barricademenu)))
    return;

  if (fah.heap && !eqstri(CurrentMenuName(), FAS(fah, barricademenu)))
    ExitFileAreaBarricade();

  if (mah.bi.use_barpriv)
  {
    /* Save current priv level on stack */

    lam->biOldPriv.use_barpriv=TRUE;
    lam->biOldPriv.priv=usr.priv;
    lam->biOldPriv.keys=usr.xkeys;

    /* Set the user's priv level to that of the barricade priv */

    usr.priv = mah.bi.priv;
    usr.xkeys |= mah.bi.keys;
  }
  else
  {
    lam->biOldPriv.use_barpriv=FALSE;
  }
}


void ExitMsgAreaBarricade(void)
{
  if (lam && lam->biOldPriv.use_barpriv)
  {
    /* If the sysop didn't already change the user's priv level */

    if (!locked)
    {
      usr.priv=lam->biOldPriv.priv;
      usr.xkeys=lam->biOldPriv.keys;
    }

    lam->biOldPriv.use_barpriv=FALSE;
  }
}


static int near EnterMsgArea(void)
{
  EnterMsgAreaBarricade();

  if ((sq=MaxOpenArea(&mah))==NULL)
  {
    sq=NULL;
    return FALSE;
  }
  
  ScanLastreadPointer(&last_msg);
  lam->last_msg=last_msg;

  return TRUE;
}


static void near ExitMsgArea(void)
{
  /* Restore the user's priv level after the barricade */

  ExitMsgAreaBarricade();

  if (sq)
  {
    /* If the user's lastread pointer has changed, update it */

    if (lam->last_msg != last_msg)
    {
      FixLastread(sq, mah.ma.type, last_msg, MAS(mah, path));
      lam->last_msg=last_msg;
    }

    MsgCloseArea(sq);
  }

  sq=NULL;
}


/* Set the current message area to be *pmah */

int PushMsgAreaSt(PMAH pmah, BARINFO *pbi)
{
  LLPUSH llp;


  /* If an old area exists, save its status information on the stack */

  if (lam)
  {
    CopyMsgArea(&lam->ah.mah, &mah);

    /* Close out the current message area, if necessary */

    ExitMsgArea();
  }

  /* Allocate memory for the new stack entry */

  if ((llp=malloc(sizeof(*llp)))==NULL)
    return FALSE;

  memset(llp, 0, sizeof *llp);
  llp->next=lam;
  lam=llp;

  CopyMsgArea(&mah, pmah);

  if (pbi)
    mah.bi=*pbi;
  else memset(&mah.bi, 0, sizeof mah.bi);


  return EnterMsgArea();
}


/* Set the current message area to be *pmah */

int PushMsgArea(char *name, BARINFO *pbi)
{
  MAH ma;
  int rc;

  memset(&ma, 0, sizeof ma);

  if (!ReadMsgArea(ham, name, &ma))
    return FALSE;

  rc=PushMsgAreaSt(&ma, pbi);
  DisposeMah(&ma);
  return rc;
}




/* Pop off the old message area and push on a new one */

int PopPushMsgAreaSt(PMAH pmah, BARINFO *pbi)
{
  if (!lam)
    return PushMsgAreaSt(pmah, pbi);

  ExitMsgArea();

  CopyMsgArea(&mah, pmah);

  if (pbi)
    mah.bi=*pbi;
  else memset(&mah.bi, 0, sizeof mah.bi);

  return EnterMsgArea();
}

/* Set us to a new (named) message area */

int PopPushMsgArea(char *name, BARINFO *pbi)
{
  MAH ma;
  int rc;

  memset(&ma, 0, sizeof ma);

  if (!ReadMsgArea(ham, name, &ma))
    return FALSE;

  rc=PopPushMsgAreaSt(&ma, pbi);
  DisposeMah(&ma);

  return rc;
}


/* Restore msg area state to that as it was before the last push */

int PopMsgArea(void)
{
  LLPUSH lp;

  if (!lam)
    return FALSE;

  /* Get out of this message area */

  ExitMsgArea();

  /* Remove this entry from the stack */

  lp=lam;
  lam=lam->next;

  DisposeMah(&lp->ah.mah);
  free(lp);

  /* Popped off last msg area? */

  if (!lam)
    return FALSE;

  /* Now prepare for entering the new message area */

  CopyMsgArea(&mah, &lam->ah.mah);
  return EnterMsgArea();
}



static int near EnterFileArea(void)
{
  EnterFileAreaBarricade();
  return TRUE;
}


static void near ExitFileArea(void)
{
  ExitFileAreaBarricade();
}


/* Set the current file area to be *pfah */

int PushFileAreaSt(PFAH pfah, BARINFO *pbi)
{
  LLPUSH llp;

  /* If an old area exists, save its status information on the stack */

  if (laf)
  {
    CopyFileArea(&laf->ah.fah, &fah);

    /* Close out the current message area, if necessary */

    ExitFileArea();
  }

  /* Allocate memory for the new stack entry */

  if ((llp=malloc(sizeof(*llp)))==NULL)
    return FALSE;

  memset(llp, 0, sizeof *llp);
  llp->next=laf;
  laf=llp;

  CopyFileArea(&fah, pfah);

  if (pbi)
    fah.bi=*pbi;
  else memset(&fah.bi, 0, sizeof fah.bi);

  return EnterFileArea();
}

/* Set the current file area to be *pfah */

int PushFileArea(char *name, BARINFO *pbi)
{
  FAH fa;
  int rc;

  memset(&fa, 0, sizeof fa);

  if (!ReadFileArea(haf, name, &fa))
    return FALSE;

  rc=PushFileAreaSt(&fa, pbi);
  DisposeFah(&fa);
  return rc;
}


/* Pop off the old message area and push on a new one */

int PopPushFileAreaSt(PFAH pfah, BARINFO *pbi)
{
  if (!laf)
    return PushFileAreaSt(pfah, pbi);

  ExitFileArea();

  CopyFileArea(&fah, pfah);

  if (pbi)
    fah.bi=*pbi;
  else memset(&fah.bi, 0, sizeof fah.bi);

  return EnterFileArea();
}

int PopPushFileArea(char *name, BARINFO *pbi)
{
  FAH fa;
  int rc;

  memset(&fa, 0, sizeof fa);

  if (!ReadFileArea(haf, name, &fa))
    return FALSE;

  rc=PopPushFileAreaSt(&fa, pbi);
  DisposeFah(&fa);
  return rc;
}


/* Restore msg area state to that as it was before the last push */

int PopFileArea(void)
{
  LLPUSH lp;

  if (!laf)
    return FALSE;

  /* Get out of this message area */

  ExitFileArea();

  /* Remove this entry from the stack */

  lp=laf;
  laf=laf->next;

  DisposeFah(&lp->ah.fah);
  free(lp);

  if (!laf)
    return FALSE;

  /* Now prepare for entering the new message area */

  CopyFileArea(&fah, &laf->ah.fah);
  return EnterFileArea();
}




/* Open a Max-style message area, taking area flags into account */

HAREA MaxOpenArea(MAH *pmah)
{
  return (MsgOpenArea(MAS(*pmah, path),
                      MSGAREA_CRIFNEC,
                      pmah->ma.type |
                        ((pmah->ma.type & MA_ECHO) ? MSGTYPE_ECHO : 0)));
}


