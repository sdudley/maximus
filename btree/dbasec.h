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

/* $Id: dbasec.h,v 1.1.1.1 2002/10/01 17:49:28 sdudley Exp $ */

#ifndef __DBASEC_H_DEFINED
#define __DBASEC_H_DEFINED

#include "btreec.h"

/* We can't define the real type for DBASE and/or PALIST in here, so settle  *
 * for one-byte placeholders.                                               */

typedef char DBASE;

/* An array of these structs is used to describe the entire database */

typedef struct
{
  char OS2FAR *szKey;                     /* Name of this key               */
  unsigned uiOfs;                         /* Offset of this field           */
  unsigned uiSize;                        /* Size of this field             */
  keycomp_t kf;                           /* Comparison function.           */
                                          /* if kf==NULL, this field is     */
                                          /* not an index.  All index fields*/
                                          /* must be at front of record!    */
  keycomp_t kf_base;
} FIELD;

typedef struct _seqfind
{
  int foo;
} OS2FAR *SEQFIND;

DBASE OS2FAR * BAPIENTRY DbOpen(char OS2FAR *szNam, FIELD OS2FAR *pf, unsigned uiNumFields, unsigned new_file, unsigned uiOrder);
int BAPIENTRY DbClose(DBASE OS2FAR *pdb);
int BAPIENTRY DbInsert(DBASE OS2FAR *pdb, void OS2FAR *pvRecord);
int BAPIENTRY DbLookup(DBASE OS2FAR *pdb, void OS2FAR * OS2FAR *ppvFields, PALIST OS2FAR *ppl, void OS2FAR *pvFoundRec);
int BAPIENTRY DbLookupI(DBASE OS2FAR *pdb, void OS2FAR * OS2FAR *ppvFields, PALIST OS2FAR *ppl, void OS2FAR *pvFoundRec, unsigned uiIdx);
//int BAPIENTRY DbLookupR(DBASE OS2FAR *pdb, void OS2FAR * OS2FAR *ppvFields, PALIST OS2FAR *ppl, void OS2FAR *pvFoundRec);
int BAPIENTRY DbUpdate(DBASE OS2FAR *pdb, void OS2FAR *pvRecOld, void OS2FAR *pvRecNew);
int BAPIENTRY DbRemove(DBASE OS2FAR *pdb, void OS2FAR * OS2FAR *ppvFields);
int BAPIENTRY DbObtainLock(DBASE OS2FAR *pdb);
int BAPIENTRY DbReleaseLock(DBASE OS2FAR *pdb);
unsigned long BAPIENTRY DbSize(DBASE OS2FAR *pdb);

SEQFIND       BAPIENTRY DbFindSeqOpen(DBASE OS2FAR *pdb, void OS2FAR *pvRec);
void OS2FAR * BAPIENTRY DbFindSeqNext(DBASE OS2FAR *pdb, SEQFIND sf);
int           BAPIENTRY DbFindSeqClose(DBASE OS2FAR *pdb, SEQFIND sf);

#endif /* __DBASEC_H_DEFINED */

