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

#include "mm.h"
#include "max_msg.h"
#include "string.h"

word Msg_Read_Lines(HMSG msgh, word msglines, word width, word soft_width, byte **outline, byte linetype[], byte *last_line_attr, word flag)
{
  static char last_initials[20]="";
  int howmuch;
  word temp, outl, hard, cnt;

  byte *msgbuffer, *msgbuf, *p;

  byte *msgend;
  byte *start, *s, *out, *end, *outend;
  byte *hardcr;

  if ((msgbuffer=malloc(BMSGBUFLEN))==NULL)
  {
    logit(mem_nrmb);
    return 0;
  }


  /* If no soft wordwrap width specified, use default of 'width' */

  if ((sword)soft_width <= 0)
    soft_width=width;

  if ((sword)width <= 0)
    width=20;

  outl=0;
  msgeof=FALSE;


  /* Make sure that the specified width isn't greater than our maximum */

  if (width >= MAX_MSGWIDTH)
    width=MAX_MSGWIDTH;

  /* Initialize the line types to NORMAL, as opposed to QUOTE, SEEN-BY,    *
   * etc...                                                                */

  for (temp=0; temp < msglines; temp++)
  {
    *outline[temp]='\0';
    linetype[temp]=MSGLINE_NORMAL;
  }

  /* Handle wrapped quote/seen-by lines properly. (Don't carry over        *
   * wrapped lines.)                                                       */

  linetype[0]=(byte)(*last_line_attr & ~MSGLINE_SOFT);
  *last_line_attr='\0';

  /* Get the first batch of characters from the input file */

  temp=(int)Read_Chars(msgh, msgbuf=msgbuffer, MSGBUFLEN);
  msgend=min(msgbuf+MSGBUFLEN, msgbuf+temp);
  msgend[1]='\0';

  /* Skip over any possible 'AREA:' tag */
  
  if ((mah.ma.attribs & MA_ECHO) &&
      strncmp(msgbuf, "AREA:", 5)==0 &&
      !GEPriv(usr.priv, prm.ctla_priv))
  {
    /* Skip 'till we find the \r */

    while (*msgbuf && *msgbuf != '\x0d')
      msgbuf++;

    /* Now skip over any trailing junk */

    while (*msgbuf == (unsigned char)'\x8d' || *msgbuf=='\x0a')
      msgbuf++;
  }

  /* Keep reading lines from the message, until we either hit the end       *
   * of the message, or until we've got as many lines as we need.           */

  while ((! msgeof || msgbuf < msgend) && *msgbuf && outl < msglines)
  {
    /* If we're past the end of the buffer, read another line */

    if (msgbuf+width+2 >= msgend && !msgeof)
    {
      memmove(msgbuffer, msgbuf, (msgend-msgbuf));
      msgend=msgbuffer+(msgend-msgbuf);
      msgbuf=msgbuffer;

      howmuch=min(MSGBUFLEN, (msgbuffer+BMSGBUFLEN)-msgend);
      temp=(int)Read_Chars(msgh, msgend, howmuch);
      msgend += temp;
      msgend[1]='\0';
    }


    /* Skip any '\n's or soft CRs at the beginning of a line */

    while ((*msgbuf==(unsigned char)'\x8d' || *msgbuf=='\x0a') &&
           msgbuf < msgend)
    {
      msgbuf++;
    }

    if (msgbuf >= msgend)
      break;


    /* Flag kludge lines appropriately */

    if (*msgbuf=='\x01')
    {
      linetype[outl] |= MSGLINE_KLUDGE;

      if (++msgbuf >= msgend)
        break;
    }

    start=msgbuf;

    /* Blank out the first 5 chars of the line, so that our quote-checking  *
     * mechanism doesn't fail for short lines.                              */

    memset(out=outline[outl], '\0', 10);

    /* Handle quotes which may have been wrapped from previous lines */

    if ((flag & MRL_QEXP) &&
        (linetype[outl] & (MSGLINE_LASTWRAP|MSGLINE_QUOTE))==
          (MSGLINE_LASTWRAP|MSGLINE_QUOTE) &&
        msgbuf[0] != '>' && msgbuf[1] != '>' &&
        msgbuf[2] != '>' && msgbuf[3] != '>' &&
        msgbuf[4] != '>')
    {
      strcpy(out, last_initials);
      out += strlen(out);
    }
    

    /* Now copy this line to the output buffer */

    end=msgbuf+width-strlen(out);
    end=min(end,msgend);

    hardcr=strchr(msgbuf+1, '\r');

    if (hardcr)
      end=min(end, hardcr);

    
    /* For lines not ended by a hard return, use the "quoting" right        *
     * margin, which is generally much smaller.                             */

    if ((hardcr != NULL &&
         (sword)(hardcr-msgbuf) > (sword)width-strlen(out) && hardcr < msgend) ||
        ((outl==0 ? *last_line_attr : linetype[outl-1]) & MSGLINE_SOFT))
    {
      end=min(end, msgbuf+soft_width-strlen(out));
    }
    
    outend=outline[outl]+width;

    for (hard=FALSE; ; msgbuf++)
    {
      if (*msgbuf=='\r' || *msgbuf=='\0')
      {
        hard=TRUE;

        if (*msgbuf=='\r')
          msgbuf++;

        break;
      }

      if (msgbuf >= end || out >= outend)
        break;

      if (*msgbuf=='\t')
      {
        for (cnt=8-((out-outline[outl]) % 8) && out < outend; cnt--; )
          *out++=' ';
      }
      else if (*msgbuf >= ' ' && *msgbuf != (unsigned char)'\x8d')
        *out++=*msgbuf;
    }

    
    *out='\0';

    p=outline[outl];

    /* If it's a seen-by, mark it as such */
      
    if (*p=='S' && eqstrn(p, "SEEN-BY:",8))
      linetype[outl] |= MSGLINE_SEENBY;

    /* Now check for a quote, too */

    if ((p[0]=='>' || p[1]=='>' || p[2]=='>' ||
         p[3]=='>' || p[4]=='>') &&
        (linetype[outl] & MSGLINE_LASTWRAP)==0)
    {
      char *s;
      
      linetype[outl] |= MSGLINE_QUOTE;
      strncpy(last_initials, p, 10);
      last_initials[10]='\0';
      
      if ((s=strrchr(last_initials, '>')) != NULL)
      {
        if (s[1]==' ')
          s[2]='\0';
        else s[1]='\0';
      }
    }


    /* If we went over end of buffer, it's time to wordwrap */

    if ((msgbuf >= end || out >= outend) && !hard)
    {
      /* Find the space or dash */

      for (s=msgbuf /* end */-1; s >= start; s--)
        if (*s==' ' || *s=='-')
          break;

      /* If we couldn't find one, just trunc at end of line */

      if (s < start)
        s=end;

      msgbuf=s+1;


      /* Now trim the wrapped word from the output buffer */

      for (s=out-1; s >= outline[outl]; s--)
        if (*s==' ' || *s=='-')
        {
          s[1]='\0';
          break;
        }

      linetype[outl] |= MSGLINE_SOFT;

      if (linetype[outl] & (MSGLINE_SEENBY | MSGLINE_KLUDGE | 
                            MSGLINE_QUOTE))
      {
        if (outl+1==msglines)
          *last_line_attr=(byte)(linetype[outl] & ~MSGLINE_SOFT);
        else linetype[outl+1]=(byte)(linetype[outl] & ~MSGLINE_SOFT);
      }

      if (outl+1==msglines)
        *last_line_attr |= MSGLINE_LASTWRAP;
      else linetype[outl+1] |= MSGLINE_LASTWRAP;
    }
    
    outl++;
  }

  /* Set the current position back by the amount of text which we never     *
   * got around to using.                                                   */

  MsgSetCurPos(msgh,MsgGetCurPos(msgh)-(long)(msgend-msgbuf));
  
  /* If we're at the end of the message, then set appropriate flags */

  for (temp=outl; temp < msglines; temp++)
    linetype[temp]=MSGLINE_END;

  free(msgbuffer);

  /* Return number of lines we got */
  return outl;
}



/* Show the body of a message, invoking a user-specified handler to         *
 * display or otherwise process each line.                                  */

word ShowBody(HMSG msgh, word (*handle)(byte *txt,void *info,word lt), void *info, word norm_margin, word quote_margin, word flag)
{
  byte *ol[MAX_MSGDISPLAY];
  byte lt[MAX_MSGDISPLAY];
  byte last_msg_attr;
  word line, got, n_ol;
  
  last_msg_attr=0;
  
  if ((n_ol=Alloc_Outline(ol)) < 1)
    return FALSE;

  while ((got=Msg_Read_Lines(msgh, n_ol,
                            norm_margin, quote_margin,
                            ol, lt, &last_msg_attr, flag)) > 0)
  {
    for (line=0; line < got; line++)
      if (! (*handle)(ol[line], info, lt[line]))
      {
        Dealloc_Outline(ol);
        return FALSE;
      }
  }
  
  Dealloc_Outline(ol);
  return TRUE;
}



struct _handinfo
{
  FILE *out;
  word quote;
  word change;
  byte *initials;
  word num_n;
};


/* This must not be static for the DOS overlay manager to work properly */

word BodyToFile(byte *txt, void *info, word lt)
{
  struct _handinfo *hi=info;
  
  if (lt & MSGLINE_SEENBY)
  {
    if (! GEPriv(usr.priv, prm.seenby_priv))
      return TRUE;
  }
  else if (lt & MSGLINE_KLUDGE)
  {
    if (! GEPriv(usr.priv, prm.ctla_priv) || hi->change)
      return TRUE;
  }

  if (hi->change && strncmp(txt, max_tear, strlen(max_tear))==0)
    return FALSE;

  /* Buffer blank lines till the end */

  if (! *txt)
  {
    hi->num_n++;
    return TRUE;
  }

  while (hi->num_n)
  {
    fputc('\n', hi->out);
    hi->num_n--;
  }

  if (hi->quote && QuoteThisLine(txt))
    fprintf(hi->out, " %s> ", hi->initials);

  fprintf(hi->out, "%s\n", txt);
  
  return TRUE;
}




int MsgBodyToFile(HMSG msgh, FILE *out, int quote, int chng, byte *initials, word flag)
{
  struct _handinfo hi;
  word mquote, mnormal;
  word len;
  
  hi.out=out;
  hi.quote=quote;
  hi.change=chng;
  hi.initials=initials;
  hi.num_n=0;
  
  mnormal=/*usr.width*/ 80-HARD_SAFE;
  mquote=/*usr.width*/ 80-SOFT_SAFE;
  
  if (quote)
  {
    len=strlen(initials)+3;
    
    mquote -= len;
    mnormal -= len;
  }
  else mquote=mnormal;

  return (ShowBody(msgh, BodyToFile, (void *)&hi, mnormal, mquote, flag));
}

long Read_Chars(HMSG msgh,char *s,int num)
{
  long n;

  if ((n=MsgReadMsg(msgh, NULL, MsgGetCurPos(msgh), num, s, 0L, NULL)) <= 0 ||
      *s=='\0')
    msgeof=TRUE;

  return n;
}


