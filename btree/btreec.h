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

/* $Id: btreec.h,v 1.1 2002/10/01 17:49:22 sdudley Exp $ */

#include "btype.h"

/* We can't define the real type for DBASE and/or PALIST in here, so settle  *
 * for one-byte placeholders.                                               */

#ifndef __cplusplus
typedef char BTREE;
typedef char PALIST;
#endif


PALIST OS2FAR * BAPIENTRY PalistNew(void);
void BAPIENTRY PalistDelete(PALIST OS2FAR *ppl);

BTREE OS2FAR *BAPIENTRY BtreeNew(void);
void BAPIENTRY BtreeDelete(BTREE OS2FAR *pbt);

int BAPIENTRY BtOpen(BTREE OS2FAR *pbt, char OS2FAR *pszFile, keycomp_t kf_ins, keycomp_t kf_srch, unsigned uiKeySz, unsigned new_file, unsigned uiOrd);
int BAPIENTRY BtClose(BTREE OS2FAR *pbt);
void * BAPIENTRY BtLookup(BTREE OS2FAR *pbt, void OS2FAR *key, PALIST OS2FAR *pl);
int BAPIENTRY BtInsert(BTREE OS2FAR *pbt, void OS2FAR *key, unsigned flags);
int BAPIENTRY BtValidate(BTREE OS2FAR *pbt);
int BAPIENTRY BtRemove(BTREE OS2FAR *pbt, void OS2FAR *key);
unsigned long BAPIENTRY BtSize(BTREE OS2FAR *pbt);


