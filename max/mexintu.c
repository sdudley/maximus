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

/* Utilities for MEX intrinstics */

#pragma off(unreferenced)
static char rcs_id[]="$Id: mexintu.c,v 1.1.1.1 2002/10/01 17:52:26 sdudley Exp $";
#pragma on(unreferenced)

#include "mm.h"
#include "mexall.h"

#ifdef MEX

  /* SKILL a string that is contained in a structure */

  void _MexKillStructString(void *pstr, int increment)
  {
    IADDR where;

    where.segment=SEG_GLOBAL;
    where.offset=MexPtrToVM(pstr) + increment;
    where.indirect=FALSE;
    MexKillString(&where);
  }

  /* Helper function for returning strings */

  void MexReturnStringBytes(char *s, int len)
  {
    IADDR where;

    /* Place the padded string on the heap */

    where=MexStoreHeapByteString(s, len);

    /* Add it as our return value */

    *(IADDR *)&REGS_ADDR[0]=where;
  }

  void MexReturnString(char *s)
  {
    MexReturnStringBytes(s, strlen(s));
  }

  /* Begin fetching the MEX arguments for this function */

  void MexArgBegin(PMA pma)
  {
    pma->last.segment=SEG_AR;
    pma->last.offset=(VMADDR)AR_CONTROL_DATA;
    pma->arg_size=0;
  }


  /* Get the next word argument */

  word MexArgGetWord(PMA pma)
  {
    word *pw;
    pma->last.indirect=FALSE;
    pma->arg_size += sizeof(word);

    pw=MexFetch(FormWord, &pma->last);
    pma->last.offset += sizeof(word);

    return pw ? *pw : 0;
  }

  /* Get the next dword argument */

  dword MexArgGetDword(PMA pma)
  {
    dword *pdw;

    pma->last.indirect=FALSE;
    pma->arg_size += sizeof(dword);

    pdw=MexFetch(FormDword, &pma->last);
    pma->last.offset += sizeof(dword);

    return pdw ? *pdw : 0;
  }


  /* Get an argument that was passed by reference */

  void * MexArgGetRef(PMA pma)
  {
    void *rc;

    pma->last.indirect=TRUE;
    pma->arg_size += sizeof(IADDR);

    rc=MexFetch(FormAddr, &pma->last);
    pma->last.offset += sizeof(IADDR);

    return rc;
  }



  /* Get next string argument.  This function automatically
   * converts the MEX string into a C nul-terminated string, and
   * if necessary, SKILLs the string.
   */

  char * MexArgGetString(PMA pma, int fPassByRef)
  {
    IADDR ia;
    char *pcData;
    char *rc;
    word wLen;

    if (fPassByRef)
      pcData=MexArgGetRefString(pma, &ia, &wLen);
    else
      pcData=MexArgGetNonRefString(pma, &ia, &wLen);

    if ((rc=malloc(wLen+1))==NULL)
      return NULL;

    memcpy(rc, pcData, wLen);
    rc[wLen]=0;

    /* Release memory for a pass-by-value string */

    if (!fPassByRef)
      MexKillString(&ia);

    return rc;
  }


  /* Retrieve a pass-by-reference argument.
   *
   * This function places the string descriptor in *pia so that the
   * calling function can modify it.  The length of the string is
   * stored in *pwLen.
   *
   * This function returns a pointer to the beginning of the string
   * in the data segment.  THIS STRING IS NOT NUL-TERMINATED.
   *
   * If you do not need the explicit descriptor of the string
   * (and do not need to modify the string itself), use
   * MexArgGetString instead.
   */

  char * MexArgGetRefString(PMA pma, IADDR *pia, word *pwLen)
  {
    IADDR *piaFetch;
    char *str;

    pma->last.indirect=FALSE;
    pma->arg_size += sizeof(IADDR);

    piaFetch=MexFetch(FormAddr, &pma->last);
    pma->last.offset += sizeof(IADDR);

    if (piaFetch != NULL)
    {
      /* Get the string itself */

      piaFetch->indirect=TRUE;  /* omit this if string is pass-by-value */

      if ((str=MexFetch(FormString, piaFetch)) != NULL)
      {
        /* Store the address of the string */

        if (pia)
        {
          *pia=*piaFetch;
          pia->indirect=FALSE;    /* omit this if string is pass-by-value */
        }

        if (pwLen)
          *pwLen=*(word *)str;

        str += sizeof(word);
        return str;
      }
    }

    return NULL;
  }

  /* Get the address of a string descriptor, the pointer to the string
   * data, and the length of the data.
   *
   * This function returns a pointer to the beginning of the string
   * in the data segment.  THIS STRING IS NOT NUL-TERMINATED.
   *
   * This function operates the same as MexArgGetRefString, except that
   * the string passed in is not be a reference string.
   *
   * If you do not need the explicit descriptor of the string, use
   * MexArgGetString instead.
   */

  char * MexArgGetNonRefString(PMA pma, IADDR *pia, word *pwLen)
  {
    IADDR *piaFetch;
    char *str;

    pma->last.indirect=FALSE;
    pma->arg_size += sizeof(IADDR);

    piaFetch=MexFetch(FormAddr, &pma->last);

    /* Store the address of the string */

    if (pia)
      *pia=pma->last;

    pma->last.offset += sizeof(IADDR);

    if (piaFetch != NULL)
    {
      /* Get the string itself */

      if ((str=MexFetch(FormString, piaFetch)) != NULL)
      {
        if (pwLen)
          *pwLen=*(word *)str;

        str += sizeof(word);
        return str;
      }
    }

    return NULL;
  }


  /* Get the next byte argument */

  byte MexArgGetByte(PMA pma)
  {
    byte *pb;

    pma->last.indirect=FALSE;
    pma->arg_size += sizeof(byte);

    pb=MexFetch(FormByte, &pma->last);
    pma->last.offset += sizeof(byte);

    return pb ? *pb : 0;
  }


  /* Return the size of the arguments used by this function */

  word MexArgEnd(PMA pma)
  {
    return pma->arg_size;
  }

#endif

