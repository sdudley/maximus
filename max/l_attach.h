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

#ifndef __LFATT_H_DEFINED
#define __LFATT_H_DEFINED

#include "typedefs.h"
#include "stamp.h"
#include "dbasec.h"

#ifndef __MSGAPI_H_DEFINED
  typedef dword UMSGID;
#endif

#ifndef __MAX_U_H_DEFINED
  #define MAX_ALEN 64
#endif

  /* Max length of an owner's name (ASCII format) */

#define LFA_ASCII_NAME_SIZE 36

  /* Max length of a filename */

#define LFA_ASCII_FILE_SIZE 120


  /* File attributes & status flags */

#define LFA_FILE_PATH       0x00001000L     /* File is fully qualified name */
#define LFA_AREA_OVERRIDE   0x00002000L      /* Area path override was used */
#define LFA_RECEIVED        0x00000001L         /* File has been downloaded */
#define LFA_NODELETEATTACH  0x00000002L  /* Don't delete attach after dload */
#define LFA_NODELETE        0x00000004L /* Don't delete file after download */

  /* Number of fields per record */

#define LFA_NUM_FIELDS      9

typedef struct
{
  dword   ulAttachID;                            /* Unique Id of the attach */
  char    szTo[LFA_ASCII_NAME_SIZE];       /* To whom the file is addressed */
  char    szArea[MAX_ALEN];                                 /* Maximus area */
  UMSGID  uid;                                /* Umsgid of attached message */
  char    szFrom[LFA_ASCII_NAME_SIZE];       /* From whom the file was sent */
  char    szFile[LFA_ASCII_FILE_SIZE];         /* File name (pathed or not) */
  dword   ulAttributes;                     /* Attach atributes (see above) */
  SCOMBO  scDateAttached;                     /* Date the file was attached */
  SCOMBO  scDateReceived;                     /* Date the file was received */
} LFA_REC;

  /* File attach API */

DBASE OS2FAR * LFAdbCreate(char OS2FAR *szName);
DBASE OS2FAR * LFAdbOpen(char OS2FAR *szName);
void LFARecInit(LFA_REC *plfa, char *szTo, char *szArea, UMSGID uid, char *szFrom, char *szFile);
void LFAdbClose(DBASE OS2FAR * pdb);

#define LFAdbInsert(pdb,plfa)             DbInsert(pdb,(void OS2FAR*)plfa)
#define LFAdbAdd(pdb,plfa,to,ar,id,fm,fi) (LFARecInit(plfa,to,ar,id,fm,fi), LFAdbInsert(pdb,plfa))
#define LFAdbLookup(pdb,ppvf,ppl,pvf)     DbLookup(pdb,ppvf,ppl,(void OS2FAR*)pvf)
#define LFAdbUpdate(pdb,plfO,plfN)        DbUpdate(pdb,plfO,plfN)
#define LFAdbRemove(pdb,ppvf)             DbRemove(pdb,ppvf)

typedef struct
{
  DBASE OS2FAR * pdb;
  PALIST * pplLook;
  void *pvLookId[3];
  LFA_REC lfa;
} LFAFIND;

DBASE OS2FAR * Read_Attach(LFA_REC *plfa, struct _xmsg *pxmsg, char * szCtrl, int isnetmsg);
int Attach_File(LFA_REC *plfa, char * szFile, char * szMsg);

#define FTSATTACHDBH  ((DBASE OS2FAR *)-1)

#endif  /* __LFATT_H_DEFINED */

