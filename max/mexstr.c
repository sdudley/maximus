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

  /* Internal function which returns the length of a string */

  word EXPENTRY intrin_strlen(void)
  {
    IADDR where;
    char *str;

    where.segment=SEG_AR;
    where.indirect=TRUE;
    where.offset=(VMADDR)(AR_CONTROL_DATA);

    regs_2[0]=*(word *)(str=MexFetch(FormString, &where));

    /* Since strlen() always uses pass-by-value, we must now free the string */

    where.indirect=FALSE;
    MexKillString(&where);

    return sizeof(IADDR);
  }


  /* Pad a string to a certain length using a specified character */

  static word near strpadsub(int fPadleft)
  {
    MA ma;
    IADDR arg, where;
    word wLen;
    char *s=NULL, *buf=NULL;
    int pad;
    char ch;

    fPadleft = 0;

    MexArgBegin(&ma);
    s=MexArgGetNonRefString(&ma, &arg, &wLen);
    pad=MexArgGetWord(&ma);
    ch=MexArgGetByte(&ma);

    /* Check for an unsigned overflow */

    if (pad > 32768u)
      pad=0;

    /* If we need to pad this string */

    if (pad && (wLen < pad) && (buf=malloc(pad)))
    {
      int padlen=pad-wLen;

      char *p=(fPadleft) ? buf+padlen : buf;

      memmove(p, s, wLen);

      p=(fPadleft) ? buf : buf+wLen;
      memset(p, ch, padlen);

      /* Place the padded string on the heap */

      where=MexStoreHeapByteString(buf, pad);

      /* Add it as our return value */

      *(IADDR *)&REGS_ADDR[0]=where;
      free(buf);
    }
    else
    {
      where=MexStoreHeapByteString(s, wLen);
      *(IADDR *)&REGS_ADDR[0]=where;
    }

    /* Kill our pass-by-value argument */

    MexKillString(&arg);

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_strpad(void)
  {
    return strpadsub(FALSE);
  }

  word EXPENTRY intrin_strpadlf(void)
  {
    return strpadsub(TRUE);
  }


  /* Find the position of a substring within a larger string */

  word EXPENTRY intrin_strfind(void)
  {
    MA ma;
    IADDR w1, w2;
    word wLen1, wLen2;
    char *s1, *s2;
    char *find;

    MexArgBegin(&ma);
    s1=MexArgGetNonRefString(&ma, &w1, &wLen1);
    s2=MexArgGetNonRefString(&ma, &w2, &wLen2);

    if ((find=memstr(s1, s2, wLen1, wLen2)) != NULL)
      regs_2[0]=find-s1+1;
    else
      regs_2[0]=0;

    MexKillString(&w1);
    MexKillString(&w2);

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_stridx(void)
  {
    MA ma;
    int pos, ch;
    word l1;
    IADDR w1;
    char *s1, *s2;

    MexArgBegin(&ma);
    s1=MexArgGetNonRefString(&ma, &w1, &l1);
    pos=MexArgGetWord(&ma);
    ch=MexArgGetWord(&ma);

    if (pos < 1)
      pos = 1;

    if ((pos <= l1) && (s2=memchr(s1+pos-1,ch,l1-pos+1))!=NULL)
      regs_2[0]=s2-s1+1;
    else
      regs_2[0]=0;

    MexKillString(&w1);

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_strridx(void)
  {
    MA ma;
    int pos, ch;
    word l1;
    IADDR w1;
    char *s1;

    MexArgBegin(&ma);
    s1=MexArgGetNonRefString(&ma, &w1, &l1);
    pos=MexArgGetWord(&ma);
    ch=MexArgGetWord(&ma);

    regs_2[0]=0;

    if (pos <= 0)
      pos = l1;

    if (pos > l1)
      pos = l1;

    while (pos-- >= 0)
    {
      if (s1[pos]==ch)
      {
        regs_2[0] = pos+1;
        break;
      }
    }

    MexKillString(&w1);

    return MexArgEnd(&ma);
  }

  /* Get a chunk out of the middle of a string */

  word EXPENTRY intrin_substr(void)
  {
    char szMsg[PATHLEN];
    MA ma;
    IADDR where;
    char *str;
    int pos, len;
    word wLen;

    MexArgBegin(&ma);

    str=MexArgGetNonRefString(&ma, &where, &wLen);
    pos=MexArgGetWord(&ma);
    len=MexArgGetWord(&ma);

    /* Return zero-length string if arguments are aberrant */

    if (!str || !pos)
    {
      sprintf(szMsg, called_with_idx_0, "substr");
      MexRTError(szMsg);
    }
    else
    {

      /* Make sure that the arguments are in range */

      if (len < 0)
        len = 0;

      if (pos > wLen)
        pos=wLen+1;
      else if (pos < 1)
        pos = 1;

      if (pos+len > wLen+1)
        len=wLen+1-pos;

      MexReturnStringBytes(str+(pos-1), len);
    }  

    MexKillString(&where);

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_strtok(void)
  {
    MA ma;
    char *psz, *tok;
    word *ppos, pos, len;

    static char blank[]="";

    MexArgBegin(&ma);

    /* string strtok(ref string: src, string: toks, ref int: pos); */

    psz=MexArgGetString(&ma, FALSE);
    tok=MexArgGetString(&ma, FALSE);
    ppos=MexArgGetRef(&ma);
    pos=len=0;
    if (!psz || !tok)
      psz=blank;
    else
    {
      word l;

      l=(word)strlen(psz);

      pos=*ppos;

      if (pos <= 0)
        pos = 1;
      else if (pos > l+1)
        pos = l + 1;

      if (pos != 0) /* Adjust to cstring (0=first call which is ok) */
        pos=pos-1;

      if (pos < l)                /* We must be positioned before the end */
      {
        pos += (word)strspn(psz+pos,tok); /* Skip past any leading tokens */
        len = (word)strcspn(psz+pos,tok);    /* Skip to next token or eos */
        *ppos = pos + len + 1;         /* Save current position in string */
      }
    }

    /* Dispense with strings no longer needed */

    if (tok)
      free(tok);

    /* Return the resulting string */

    MexReturnStringBytes(psz+pos, len);

    if (psz && psz != blank)
      free(psz);

    return MexArgEnd(&ma);
  }

  static word near struprlwr(int upper)
  {
    MA ma;
    char *psz;

    MexArgBegin(&ma);
    psz=MexArgGetString(&ma,FALSE);
    if (!psz)
      MexReturnStringBytes("",0);
    else
    {
      MexReturnString(upper ? strupr(psz) : strlwr(psz));
      free(psz);
    }

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_strupper(void)
  {
    return struprlwr(TRUE);
  }

  word EXPENTRY intrin_strlower(void)
  {
    return struprlwr(FALSE);
  }

  word EXPENTRY intrin_trim(void)
  {
    MA ma;
    char *psz, *tok;

    MexArgBegin(&ma);

    /* string trim(string: src, string: tok); */

    psz=MexArgGetString(&ma,FALSE);
    tok=MexArgGetString(&ma,FALSE);

    if (!psz || !tok)
      MexReturnStringBytes("",0);
    else
    {
      char *newstart;
      int l;

      newstart = psz + strspn(psz,tok);  /* Skip past any leading chrs */
      l=strlen(newstart); /* Now shorten string for any trailing ones */

      /* Now find the characters at the end of the string too */

      while (l && strchr(tok, newstart[l-1]))
        l--;

      MexReturnStringBytes(newstart, l);
    }

    if (psz)
      free(psz);

    if (tok)
      free(tok);

    return MexArgEnd(&ma);
  }

  /* Convert an integer to a string */

  word EXPENTRY intrin_itostr(void)
  {
    MA ma;
    char buf[10];
    sword i;

    MexArgBegin(&ma);

    i=MexArgGetWord(&ma);
    sprintf(buf, "%d", i);
    MexReturnString(buf);

    return MexArgEnd(&ma);
  }


  /* Convert an unsigned integer to a string */

  word EXPENTRY intrin_uitostr(void)
  {
    MA ma;
    char buf[10];
    word i;

    MexArgBegin(&ma);

    i=MexArgGetWord(&ma);
    sprintf(buf, "%u", i);
    MexReturnString(buf);

    return MexArgEnd(&ma);
  }


  /* Convert a long to a string */

  word EXPENTRY intrin_ltostr(void)
  {
    MA ma;
    char buf[30];
    long l;

    MexArgBegin(&ma);

    l=MexArgGetDword(&ma);

    sprintf(buf, "%ld", l);
    MexReturnString(buf);

    return MexArgEnd(&ma);
  }


  /* Convert an unsigned long to a string */

  word EXPENTRY intrin_ultostr(void)
  {
    MA ma;
    char buf[30];
    long l;

    MexArgBegin(&ma);

    l=MexArgGetDword(&ma);

    sprintf(buf, "%lu", l);
    MexReturnString(buf);
 
    return MexArgEnd(&ma);
  }

  /* Convert a string to an integer */

  word EXPENTRY intrin_strtoi(void)
  {
    MA ma;
    char *s;

    MexArgBegin(&ma);

    s=MexArgGetString(&ma, FALSE);

    if (!s)
      regs_2[0]=0;
    else
    {
      regs_2[0]=atoi(s);
      free(s);
    }

    return MexArgEnd(&ma);
  }


  /* Convert a string to a long */

  word EXPENTRY intrin_strtol(void)
  {
    MA ma;
    char *s;

    MexArgBegin(&ma);

    s=MexArgGetString(&ma,FALSE);
    if (!s)
      regs_4[0]= 0;
    else
    {
      regs_4[0]=atol(s);
      free(s);
    }

    return MexArgEnd(&ma);
  }

#endif /* MEX */
