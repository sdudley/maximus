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

/* $Id: sq_area.c,v 1.4 2004/01/22 08:04:28 wmcbrine Exp $ */

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"

/* Linked list of open Squish areas */

static HAREA haOpen=NULL;


#ifdef OS_2

  #define INCL_SUB
  #include "pos2.h"
#endif

/* Exitlist routine to make sure that all areas are closed */

unsigned _SquishCloseOpenAreas(void)
{
  #ifdef OS_2
    static char szDoneCloseMsg[]="MsgAPI is done closing areas.\r\n";
  #endif

  HAREA ha, haNext;

  /* If nothing to close, just get out. */

  if (!haOpen)
    return TRUE;

  for (ha=haOpen; ha; ha=haNext)
  {
    #ifdef OS_2
      static char szCloseMsg[]="MsgAPI is closing an open area...\r\n";
      static char szCloseErr[]="MsgAPI had problems closing this area!\r\n";
    #endif

    haNext=Sqd->haNext;

    #ifdef OS_2
      (void)VioWrtTTY(szCloseMsg, strlen(szCloseMsg), 0);
    #endif

    if (SquishCloseArea(ha) != 0)
      #ifdef OS_2
        (void)VioWrtTTY(szCloseErr, strlen(szCloseErr), 0);
      #else
        ;
      #endif
  }

  haOpen=NULL;

  #ifdef OS_2
    (void)VioWrtTTY(szDoneCloseMsg, strlen(szDoneCloseMsg), 0);
  #endif

  return TRUE;
}

static char dot_sqd[]=".sqd";
static char dot_sqi[]=".sqi";

/* List of function pointers to use in Squish areas */

static struct _apifuncs sq_funcs=
{
  SquishCloseArea,
  SquishOpenMsg,
  SquishCloseMsg,
  SquishReadMsg,
  SquishWriteMsg,
  SquishKillMsg,
  SquishLock,
  SquishUnlock,
  SquishSetCurPos,
  SquishGetCurPos,
  SquishMsgnToUid,
  SquishUidToMsgn,
  SquishGetHighWater,
  SquishSetHighWater,
  SquishGetTextLen,
  SquishGetCtrlLen,
  SquishGetNextUid
};


/* Open the .SQD and .SQI files for an existing base */

static unsigned near _SquishOpenBaseFiles(HAREA ha, byte OS2FAR *szName, int mode)
{
  char szFile[PATHLEN];

  (void)strcpy(szFile, szName);
  (void)strcat(szFile, dot_sqd);

  if ((Sqd->sfd=sopen(szFile, mode | O_RDWR | O_BINARY | O_NOINHERIT, SH_DENYNO,
                      S_IREAD | S_IWRITE))==-1)
  {
    msgapierr=MERR_NOENT;
    return FALSE;
  }

  (void)strcpy(szFile, szName);
  (void)strcat(szFile, dot_sqi);

  if ((Sqd->ifd=sopen(szFile, mode | O_RDWR | O_BINARY | O_NOINHERIT, SH_DENYNO,
                      S_IREAD | S_IWRITE))==-1)
  {
    (void)close(Sqd->sfd);
    msgapierr=MERR_NOENT;
    return FALSE;
  }

  return TRUE;
}



/* Delete the .SQD and .SQI files for an area */

static unsigned near _SquishUnlinkBaseFiles(byte OS2FAR *szName)
{
  char szFile[PATHLEN];
  unsigned rc=TRUE;

  (void)strcpy(szFile, szName);
  (void)strcat(szFile, dot_sqd);

  if (unlink(szFile) != 0)
    rc=FALSE;

  (void)strcpy(szFile, szName);
  (void)strcat(szFile, dot_sqi);

  if (unlink(szFile) != 0)
    rc=FALSE;

  return rc;
}




/* Close the data files for this message base */

static void near _SquishCloseBaseFiles(HAREA ha)
{
  (void)close(Sqd->sfd);
  (void)close(Sqd->ifd);

  Sqd->sfd=-1;
  Sqd->ifd=-1;
}


/* Ensure that the SQBASE header is valid */

static unsigned near _SquishValidateBaseHeader(SQBASE *psqb)
{
  if (psqb->num_msg > psqb->high_msg ||
      psqb->num_msg > psqb->uid+1 ||
      psqb->high_msg > psqb->uid+1 ||
      psqb->num_msg > 1000000L ||
      psqb->num_msg != psqb->high_msg ||
      psqb->len < sizeof(SQBASE) ||
      psqb->len >= 1024 ||
      psqb->begin_frame > psqb->end_frame ||
      psqb->last_frame > psqb->end_frame ||
      psqb->free_frame > psqb->end_frame ||
      psqb->last_free_frame > psqb->end_frame ||
      psqb->end_frame==0)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  return TRUE;
}


/* Copy information from the psqb disk header to our in-memory struct */

unsigned _SquishCopyBaseToData(HAREA ha, SQBASE *psqb)
{
  Sqd->cbSqbase=psqb->len;
  Sqd->cbSqhdr=psqb->sz_sqhdr;
  Sqd->wSkipMsg=(word)psqb->skip_msg;
  Sqd->dwMaxMsg=psqb->max_msg;
  Sqd->wMaxDays=psqb->keep_days;
  Sqd->dwHighWater=psqb->high_water;
  Sqd->uidNext=psqb->uid;
  Sqd->foFirst=psqb->begin_frame;
  Sqd->foLast=psqb->last_frame;
  Sqd->foFree=psqb->free_frame;
  Sqd->foLastFree=psqb->last_free_frame;
  Sqd->foEnd=psqb->end_frame;
  Sqd->sqbDelta=*psqb;

  ha->num_msg=psqb->num_msg;
  ha->high_msg=psqb->num_msg;
  ha->high_water=psqb->high_water;

  return TRUE;
}


/* Copy data from the in-memory struct into the disk header */

unsigned _SquishCopyDataToBase(HAREA ha, SQBASE *psqb)
{
  (void)memset(psqb, 0, sizeof(SQBASE));

  psqb->len=Sqd->cbSqbase;
  psqb->sz_sqhdr=Sqd->cbSqhdr;
  psqb->skip_msg=Sqd->wSkipMsg;
  psqb->max_msg=Sqd->dwMaxMsg;
  psqb->keep_days=Sqd->wMaxDays;
  psqb->high_water=Sqd->dwHighWater;
  psqb->uid=Sqd->uidNext;
  psqb->begin_frame=Sqd->foFirst;
  psqb->last_frame=Sqd->foLast;
  psqb->free_frame=Sqd->foFree;
  psqb->last_free_frame=Sqd->foLastFree;
  psqb->end_frame=Sqd->foEnd;

  psqb->num_msg=ha->num_msg;
  psqb->high_msg=ha->high_msg;
  psqb->high_water=ha->high_water;

  return TRUE;
}


/* Set the starting values for this message base */

static unsigned near _SquishSetBaseDefaults(HAREA ha)
{
  /* Set up our current position in the linked list */

  Sqd->foNext=Sqd->foFirst;
  Sqd->foCur=NULL_FRAME;
  Sqd->foPrev=NULL_FRAME;
  ha->cur_msg=0;
  ha->sz_xmsg=XMSG_SIZE;
  return TRUE;
}

/* Fill out the initial values in a Squish base header */

static unsigned near _SquishFillBaseHeader(SQBASE *psqb, byte OS2FAR *szName)
{
  psqb->len=sizeof *psqb;
  psqb->rsvd1=0;
  psqb->num_msg=0L;
  psqb->high_msg=0L;
  psqb->skip_msg=0L;
  psqb->high_water=0L;
  psqb->uid=1L;
  (void)strcpy(psqb->base, szName);
  psqb->begin_frame=NULL_FRAME;
  psqb->last_frame=NULL_FRAME;
  psqb->free_frame=NULL_FRAME;
  psqb->last_free_frame=NULL_FRAME;
  psqb->end_frame=sizeof(SQBASE);
  psqb->max_msg=0L;
  psqb->keep_days=0;
  psqb->sz_sqhdr=SQHDR_SIZE;
  (void)memset(psqb->rsvd2, 0, sizeof psqb->rsvd2);

  return TRUE;
}


/* Create a new Squish message area */

static unsigned near _SquishCreateNewBase(HAREA ha, byte OS2FAR *szName)
{
  SQBASE sqb;           /* Header from Squish base */

  /* Try to open the files */

  if (!_SquishOpenBaseFiles(ha, szName, O_CREAT | O_TRUNC))
    return FALSE;

  if (!_SquishFillBaseHeader(&sqb, szName) ||
      !_SquishWriteBaseHeader(ha, &sqb) ||
      !_SquishCopyBaseToData(ha, &sqb) ||
      !_SquishSetBaseDefaults(ha))
  {
    /* The open failed, so delete the partially-created Squishbase */

    _SquishCloseBaseFiles(ha);
    (void)_SquishUnlinkBaseFiles(szName);

    return FALSE;
  }

  return TRUE;
}


/* Open an existing Squish base and fill out 'ha' appropriately */

static unsigned near _SquishOpenExistingBase(HAREA ha, byte OS2FAR *szName)
{
  SQBASE sqb;           /* Header from Squish base */

  /* Try to open the files */

  if (!_SquishOpenBaseFiles(ha, szName, 0))
    return FALSE;

  if (!_SquishReadBaseHeader(ha, &sqb) ||
      !_SquishValidateBaseHeader(&sqb) ||
      !_SquishCopyBaseToData(ha, &sqb) ||
      !_SquishSetBaseDefaults(ha))
  {
    _SquishCloseBaseFiles(ha);
    return FALSE;
  }

  return TRUE;
}



/* Allocate a new area handle */

static HAREA near NewHarea(word wType)
{
  HAREA ha;

  /* Try to allocate memory for the area handle */

  if ((ha=(HAREA)palloc(sizeof(*ha)))==NULL)
    return NULL;

  (void)memset(ha, 0, sizeof *ha);

  ha->id=MSGAPI_ID;
  ha->len=sizeof(struct _msgapi);
  ha->type=wType & ~MSGTYPE_ECHO;
  ha->isecho=(byte)!!(wType & MSGTYPE_ECHO);

  return ha;
}




/* Open a Squish base */

HAREA MSGAPI SquishOpenArea(byte OS2FAR *szName, word wMode, word wType)
{
  HAREA ha;                       /* Area handle for this area */
  unsigned fOpened;               /* Has this area been opened? */


  /* Make sure that we have a valid base name */

  if (!szName)
  {
    msgapierr=MERR_BADA;
    return NULL;
  }


  /* Allocate memory for the Squish handle */

  if ((ha=NewHarea(wType))==NULL)
    return NULL;


  /* Allocate memory for the Squish-specific part of the handle */

  if ((ha->apidata=(void *)palloc(sizeof(struct _sqdata)))==NULL)
  {
    pfree(ha);
    return NULL;
  }

  memset(ha->apidata, 0, sizeof(struct _sqdata));


  /* Allocate memory to hold the function pointers */

  if ((ha->api=(struct _apifuncs *)palloc(sizeof(struct _apifuncs)))==NULL)
  {
    pfree(ha->apidata);
    pfree(ha);
    return NULL;
  }

  /* Fill out the function pointers for this area */

  *ha->api=sq_funcs;


  /* Open the index interface for this area */

  if ((Sqd->hix=_SquishOpenIndex(ha))==NULL)
    return NULL;

  fOpened=FALSE;

  /* If we want to open an existing area, try it here */

  msgapierr=0;

  if (wMode==MSGAREA_NORMAL || wMode==MSGAREA_CRIFNEC)
    fOpened=_SquishOpenExistingBase(ha, szName);
  else
    msgapierr=MERR_NOENT;

  /* If we want to create a new area, try that now */

  if (msgapierr==MERR_NOENT &&
      (wMode==MSGAREA_CREATE ||
       (wMode==MSGAREA_CRIFNEC && !fOpened)))
  {
    fOpened=_SquishCreateNewBase(ha, szName);
  }


  /* If the open succeeded */

  if (fOpened)
  {
    /* Add us to the linked list of open areas */

    Sqd->haNext=haOpen;
    haOpen=ha;
  }
  else
  {
    pfree(ha->apidata);
    pfree(ha);
    ha=NULL;
  }

  /* Return the handle to this area */

  return ha;
}


/* Close any messages in this area which may be open */

static unsigned near _SquishCloseAreaMsgs(HAREA ha)
{
  HMSG hm, hmNext;

  /* Close any open messages, if necessary */

  for (hm=Sqd->hmsgOpen; hm; hm=hmNext)
  {
    hmNext=hm->hmsgNext;

    if (SquishCloseMsg(hm)==-1)
    {
      msgapierr=MERR_EOPEN;
      return FALSE;
    }
  }

  return TRUE;
}


static unsigned near _SquishRemoveAreaList(HAREA haThis)
{
  HAREA ha, haNext;

  if (!haOpen)
  {
    msgapierr=MERR_BADA;
    return FALSE;
  }

  /* If we were at the head of the list, adjust the main pointer only */

  if (haOpen==haThis)
  {
    ha=haThis;
    haOpen=Sqd->haNext;
    return TRUE;
  }

  /* Try to find us in the middle of the list */

  for (ha=haOpen; ha; ha=haNext)
  {
    haNext=Sqd->haNext;

    if (haNext==haThis)
    {
      Sqd->haNext=((SQDATA *)(haThis->apidata))->haNext;
      return TRUE;
    }
  }

  msgapierr=MERR_BADA;
  return FALSE;
}


/* Close an open message area */

sword MAPIENTRY SquishCloseArea(HAREA ha)
{
  if (MsgInvalidHarea(ha))
    return -1;

  /* Close any open messages */

  if (!_SquishCloseAreaMsgs(ha))
    return -1;

  /* Unlock the area, if necessary */

  if (Sqd->fHaveExclusive)
  {
    Sqd->fHaveExclusive=1;
    (void)_SquishExclusiveEnd(ha);
  }

  /* Unlock the area as well */

  if (Sqd->fLockFunc)
  {
    if (Sqd->fLocked)
      Sqd->fLocked=1;

    Sqd->fLockFunc=1;

    SquishUnlock(ha);
  }

  (void)_SquishCloseIndex(Sqd->hix);

  /* Close off the Squish data files */

  _SquishCloseBaseFiles(ha);

  /* Remove ourselves from the list of open areas */

  (void)_SquishRemoveAreaList(ha);

  /* Blank out the ID, then free all of the memory associated with this     *
   * area handle.                                                           */

  ha->id=0;

  pfree(ha->api);
  pfree(ha->apidata);
  pfree(ha);

  return 0;
}


/* This function ensures that the specified Squish base name exists */

sword MSGAPI SquishValidate(byte OS2FAR *szName)
{
  char szFile[PATHLEN];

  (void)strcpy(szFile, szName);
  (void)strcat(szFile, dot_sqd);

  if (!fexist(szFile))
    return FALSE;

  (void)strcpy(szFile, szName);
  (void)strcat(szFile, dot_sqi);

  return fexist(szFile);
}

