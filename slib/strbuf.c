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

/*
 * Strings buffer
 */

#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "strbuf.h"

#define sbofs2ptr(sb,ofs)   (char*)(((char *)(sb)) + (ofs))

strbuf * sb_new(word sz)
{
  strbuf *sb;

  if (sz <= sizeof(strbuf))
    sz=STRBUFSZ;
  sb=(strbuf *)malloc(sz);
  if (sb!=NULL)
  {
    char *p;

    sb->usSize=(word)sz;
    sb->usOffset=sizeof(strbuf);
    p=sbofs2ptr(sb,sb->usOffset);
    *p='\0';
    sb->usOffset++;
  }
  return sb;
}

char *sb_alloc(strbuf *sb, char *s)
{
  if (s)
  {
    word len=(word)strlen(s);
    if (!len)                                            /* Empty string */
      s=sbofs2ptr(sb,sizeof(strbuf));
    else
    {
      ++len;                                              /* Include NUL */
      if (((long)sb->usOffset+(long)len) >= (long)sb->usSize)
        s=NULL;                            /* Overflow, requires realloc */
      else
      {
        char *p=sbofs2ptr(sb,sb->usOffset);
        sb->usOffset += len;
        memcpy(p, s, len);
        s=p;
      }
    }
  }
  return s;
}

void sb_free(strbuf *sb, char *s)
{
  char *base=(char*)sb;
  if (s >= (base+sizeof(strbuf)+1) && s < (base+sb->usSize) && s[-1]=='\0')
  {
    char *used=sbofs2ptr(sb,sizeof(strbuf));
    word at, len;

    /* Delete this string */

    *s=STRDELMK;

    /* Now, walk up the buffer and find the last string,
       adjusting the next offset downwards if we can */

    for ( at=sizeof(strbuf)+1; at < sb->usOffset ; at+=len )
    {
      char *p=sbofs2ptr(sb,at);
      len=(word)strlen(p)+1;
      if (*p!=STRDELMK)
        used=p;
    }

    /* Set the last offset to one character past end of last string */

    sb->usOffset=(word)(used-(char*)sb+strlen(used)+1);

  }
}

void sb_reset(strbuf *sb)
{
  sb->usOffset=sizeof(strbuf)+1;
}

strbuf * sb_realloc(strbuf *sb, int sz, int (*reloc)(char*old,char*new))
{
  strbuf *sn=sb_new((word)(sb->usSize+sz));
  if (sn)
  {
    word at, len;

    for ( at=sizeof(strbuf)+1; at < sb->usOffset ; at+=len )
    {
      char *p=sbofs2ptr(sb,at);
      len=(word)strlen(p)+1;
      if (*p!=STRDELMK)
      {
        char *n=sbofs2ptr(sn,sn->usOffset);
        if (!reloc || reloc(p,n))
        { /* Drop the string if caller has lost it */
          /* Or just transfer if no relocation fn is given */
          memcpy(n,p,len);
          sn->usOffset+=len;
        }
      }
    }
    free(sb);
  }
  return sn;
}

word sb_inbuf(strbuf *sb, char *s, int icase)
{
  word  at, len;

  for ( at=sizeof(strbuf); at < sb->usOffset; at+=len )
  {
    char *p=sbofs2ptr(sb,at);
    len=(word)strlen(p)+1;
    if ((icase ? stricmp : strcmp)(s, p)==0)
      return at;
  }
  return (word)-1;
}

