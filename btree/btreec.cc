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
static char rcs_id[]="$Id: btreec.cc,v 1.1.1.1 2002/10/01 17:49:22 sdudley Exp $";
#pragma on(unreferenced)

#include "btree.h"

// C wrapper for BTREE class

extern "C"
{
  BTREE *BEXPENTRY BtreeNew(void)
  {
    BTREE *pbt=new BTREE;
    return pbt;
  }

  void BEXPENTRY BtreeDelete(BTREE *pbt)
  {
    delete pbt;
  }

  int BEXPENTRY BtOpen(BTREE *pbt, char *pszFile, keycomp_t kf_ins, keycomp_t kf_srch, unsigned uiKeySz, unsigned new_file, unsigned uiOrd)
  {
    return pbt->open(pszFile, kf_ins, kf_srch, uiKeySz, new_file, uiOrd);
  }

  int BEXPENTRY BtClose(BTREE *pbt)
  {
    return pbt->close();
  }

  void * BEXPENTRY BtLookup(BTREE *pbt, void *key, PALIST *pl)
  {
    return pbt->lookup(key, pl);
  }

  int BEXPENTRY BtInsert(BTREE *pbt, void *key, unsigned flags)
  {
    return pbt->insert(key, flags);
  }

  int BEXPENTRY BtValidate(BTREE *pbt)
  {
    return pbt->validate();
  }

  int BEXPENTRY BtRemove(BTREE *pbt, void *key)
  {
    return pbt->remove(key);
  }

  unsigned long BEXPENTRY BtSize(BTREE *pbt)
  {
    return pbt->size();
  }

  BTERROR BEXPENTRY BtError(BTREE *pbt)
  {
    return pbt->error();
  }

  PALIST * BEXPENTRY PalistNew(void)
  {
    return new PALIST;
  }

  void BEXPENTRY PalistDelete(PALIST *ppl)
  {
    if (ppl)
      delete ppl;
  }
};

