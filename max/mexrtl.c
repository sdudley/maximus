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
#include "max_msg.h"

#ifdef MEX

/* Before-call hook function.  This is called before any user-defined
 * intrinsic function (in the table at the top of the file) is called.
 */

void EXPENTRY intrin_hook_before(void)
{
  /* If the MEX program changed the current message, import it now. */

  if (pmisThis->set_current != pmisThis->pmm->current)
  {
    last_msg=pmisThis->pmm->current;

    if (sq && (prm.flags2 & FLAG2_UMSGID))
      last_msg=MsgUidToMsgn(sq, last_msg, UID_NEXT);
  }

  /* Save the current last_msg so we can see if the intrinsic changes
   * it to something else.
   */

  pmisThis->set_last_msg = last_msg;
  strcpy(pmisThis->szSetArea, usr.msg);
  MexImportString(linebuf, pmisThis->vmaLinebuf, BUFLEN);
}

/* After-call hook function.  This is called after any user-defined
 * intrinsic function (in the table at the top of the file) is called.
 */

void EXPENTRY intrin_hook_after(void)
{
  /* Update the screen position */

  pmisThis->pms->current_row = current_line;
  pmisThis->pms->current_col = current_col;
  pmisThis->pms->more_lines = display_line;

  if (last_msg != pmisThis->set_last_msg ||
      !eqstri(usr.msg, pmisThis->szSetArea))
  {
    pmisThis->set_current = pmisThis->pmm->current = UIDnum(last_msg);
  }

  MexExportString(pmisThis->vmaLinebuf, linebuf);
}



/* MexImportString:
 *
 * Routine used to import a string from the MEX heap into
 * a C-style string.
 *
 * szDest is the destination string.
 *
 * vma is the VM address, pointing to the string's IADDR descriptor.
 * (This is typically returned by MexStoreString.)
 *
 * iMax is the maximum length of the string to copy.
 */

void MexImportString(char *szDest, VMADDR vma, int iMax)
{
  IADDR ia;
  IADDR *pia;

  /* Fetch the IADDR that points to the beginning of the string */

  ia.segment = SEG_GLOBAL;
  ia.indirect = FALSE;
  ia.offset = vma;

  pia = MexFetch(FormAddr, &ia);

  /* Now use the IADDR to find the beginning of the string text */

  MexStringCopy(szDest, pia, iMax);
}


/* MexExportString:
 *
 * Routine used to export a C-style string into the MEX heap.  This
 * function will automatically kill any string at the specified location.
 *
 * vma is the VM address of the string's IADDR descriptor, such as
 * returned by MexStoreString().
 *
 * szSrc is a pointer to the string to export.
 */

void MexExportString(VMADDR vma, char *szSrc)
{
  IADDR ia;

  /* Kill the old string */

  ia.segment = SEG_GLOBAL;
  ia.indirect = FALSE;
  ia.offset = vma;

  MexKillString(&ia);

  /* Now store the new one */

  MexStoreStringAt(vma, szSrc);
}


/* Copy a string from the MEX data space into something that we can
 * recognize as a C string in 'dest', up to a max length of max_len
 * characters.
 */

void MexStringCopy(char *dest, IADDR *piSrc, int max_len)
{
  char *szSrc;
  int len;

  szSrc=MexFetch(FormString, piSrc);
  len=(int)*(word *)szSrc;
  szSrc += sizeof(word);

  len=min(len, max_len-1);

  strncpy(dest, szSrc, len);
  dest[len]=0;
}


#endif /* MEX */

