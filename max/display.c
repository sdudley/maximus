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

/* $Id: display.c,v 1.3 2004/01/22 08:04:26 wmcbrine Exp $ */

/*# name=.BBS-file display routines
*/

#define MAX_INCL_COMMS
#define MAX_LANG_f_area

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "ffind.h"
#include "mm.h"
#include "max_msg.h"
#include "max_file.h"
#include "display.h"
#include "displayp.h"

#ifndef ORACLE
static sword near DisplayFilesBbs(DSTK *d);
#endif

static word near DispParsePriv(DSTK *d);
static void near PF(DSTK *d, word n);


int _stdc Display_File(word type, char *nonstop, char *fname,...)
{
  DSTK *d, *dtsave;
  va_list var_args;

  /* Save pointer to top of display stack */

  dtsave=dtop;

  /* Make sure that we don't nest display files too far... */

  if ((d=DispNewDstk(fname))==NULL)
    return -1;

  d->type=type;
  DisplayInitDstk(d, nonstop);

  /* Prepare the filename's arguments */
  
  va_start(var_args, fname);
  vsprintf(d->filename, fname, var_args);
  va_end(var_args);

  while (*d->filename && DisplayOneFile(d)==0)
    ;
  
  DisplayCleanup(d);

  /* Restore pointer to top of display stack */

  dtop=dtsave;

  return d->ret;
}


static DSTK * near DispNewDstk(char *fname)
{
  DSTK *d;
  
  if (++nested > MAX_NEST)
  {
    logit(log_max_nest, fname);
    nested--;
    return NULL;
  }
  
  if ((d=malloc(sizeof(DSTK)))==NULL)
  {
    logit(mem_none);
    nested--;
    return NULL;
  }
  
  return d;
}


static void DisplayCleanup(DSTK *d)
{
  free(d);

  /* If we're on our way OUT of the .BBS file, then restore to the         *
   * directory we started displaying from.  We use the ALTERNATE save-     *
   * directory routines, so that we end up in the same directory the       *
   * function was called in, regardless of whether or not the calling      *
   * function used the primary Save_Directory() routine.                   */

  if (--nested <= 0)
  {
    Restore_Directories2();
    nested=0;
  }
}

static void near DisplayInitDstk(DSTK *d, byte *nonstop)
{
  /* If the caller didn't provide a variable to use for non-stop checking,  *
   * provide our own.                                                       */

  d->nonstop=(nonstop ? nonstop : &d->intern_nonstop);


  /* If we're searching for files, set the current column to one */

  if (d->type & DISPLAY_SEARCH)
    current_col=1;


  /* Options should carry through across linked files! */
  
  if (nested <= 1 || (d->type & DISPLAY_FILESBBS))
  {
    /* If we're not doing a L)ocate command */

    if ((d->type & DISPLAY_AREANAME)==0)
      display_line=display_col=1;

    /*lastmenu=0;*/

    d->ck_abort=d->automore=TRUE;

    /* So that the nonstop-more command will carry across file areas */

    if ((d->type & (DISPLAY_SEARCH|DISPLAY_NEWFILES))==0 || first_search)
      *d->nonstop=FALSE;

    if (nested <= 1)
      nested=1;
  }
  else
  {
    if (dtop)
    {
      d->ck_abort=dtop->ck_abort;
      d->automore=dtop->automore;
      
      if (d->nonstop && dtop->nonstop)
        *d->nonstop=*dtop->nonstop;
    }
  }

  /* Set the top-of-stack pointer */
  
  dtop=d;
   

  *d->onexit='\0';

  d->allanswers=d->recd_chars=FALSE;
  d->chcount=d->chcount_n=0;
  d->questfile=NULL;
}





static sword near DisplayOneFile(DSTK *d)
{
  sword ret;

  if (nullptr())
    Got_A_Null_Pointer("file ", d->filename);

  d->skipcr=FALSE;
  d->beginline=TRUE;
  d->ret=0;
  
  /* Parse '%' translation chars */
  
  DisplayFixFilename(d);

  /* Handle a MEX script */

  if (*d->filename==':')
  {
#ifdef MEX
    Mex(d->filename+1);
    *d->filename='\0';
    *last_onexit='\0';
#endif
    return 0;
  }

  if (DisplayOpenFile(d) != 0)
    return -1;

  /* This isn't displayed locally, so make a pseudo log entry */

  if ((d->type & DISPLAY_NOLOCAL) && snoop)
    logit(log_display_file, d->filename);

  strcpy(d->lastfile, d->filename);
  *d->filename='\0';
  *last_onexit='\0';

  if (d->type & DISPLAY_FILESBBS)
  {
    #ifndef ORACLE
      ret=DisplayFilesBbs(d);
    #endif
  }
  else ret=DisplayNormal(d);

  /* Now handle the onexit filename, if any */

  return (DispCloseFiles(d, ret));
}


static sword near DisplayOpenFile(DSTK *d)
{
  d->bbsfile=-1;

  /* DLN 28 Feb 95 Try to open a .RBS file first (if RIP support is enabled) */

  if (hasRIP())
  {
    sprintf(d->scratch, ss, d->filename, dotrbs);

    d->bbsfile=shopen(d->scratch, O_RDONLY | O_BINARY | O_NOINHERIT);

    /* DLN 03 Mar 95 Suppress local output for .RBS files by default */

    if (d->bbsfile != -1)
      d->type |= DISPLAY_NOLOCAL;
  }

  /* Try to open a .GBS file */

  if (d->bbsfile==-1 && usr.video==GRAPH_ANSI)
  {
    sprintf(d->scratch, ss, d->filename, dotgbs);

    d->bbsfile=shopen(d->scratch, O_RDONLY | O_BINARY | O_NOINHERIT);
  }

  /* If that didn't work, try a .BBS file */
  
  if (d->bbsfile==-1)
  {
    sprintf(d->scratch, ss, d->filename, dotbbs);

    d->bbsfile=shopen(d->scratch, O_RDONLY | O_BINARY | O_NOINHERIT);
  }

  /* Finally, try with no extension */
  
  if (d->bbsfile==-1)
    d->bbsfile=shopen(d->filename, O_RDONLY | O_BINARY | O_NOINHERIT);

  if (d->bbsfile==-1)
    return (d->ret=DRET_NOTFOUND);


  /* Allocate memory for this file's buffer */

  if ((d->filebufr=malloc(FILEBUFSIZ))==NULL)
    return (d->ret=DRET_NOMEM);
  
  d->bufp=d->highp=d->filebufr;

  /* Seek to the restore offset, if any */
  
  if (rst_offset != -1L)
  {
    lseek(d->bbsfile, rst_offset, SEEK_SET);
    strcpy(d->onexit, last_onexit);
    rst_offset=-1L;
  }
  
  return 0;
}


static void near DisplayFixFilename(DSTK *d)
{
  char *p;
  
  Convert_Star_To_Task(d->filename);

  while ((p=strchr(d->filename,'+')) != NULL)
    *p='%';

  if (strchr(d->filename, '%'))
  {
    Parse_Outside_Cmd(d->filename, d->scratch);
    strcpy(d->filename, d->scratch);
  }


#if 0 /*SJD Wed  06-02-1993  16:06:03 */
  /* Add full path to 'filename', and put result in 'string'. */

  Add_Full_Path(d->filename, d->scratch);

  /* Now copy it back to 'filename', so we can use it later. */

  strcpy(d->filename, d->scratch);
#endif
}



static sword near DispCloseFiles(DSTK *d, sword ret)
{
  close(d->bbsfile);

  if (d->filebufr)
  {
    free(d->filebufr);
    d->filebufr=NULL;
  }

  if (d->questfile)
  {
    fputc('\n', d->questfile);
    fclose(d->questfile);
  }

  /* If we have an onexit filename, and there's nothing else to display,    *
   * do the onexit next!                                                    */

  if (*d->filename=='\0' && *d->onexit)
  {
    strcpy(d->filename, d->onexit);
    *d->onexit='\0';
    ret=0;
  }
  
  return ret;
}




static sword near DisplayNormal(DSTK *d)
{
#if (COMMAPI_VER > 1)
  extern HCOMM hcModem;
  BOOL lastState = ComBurstMode(hcModem, TRUE);
#endif
  word has_hot, doing_hotmenu;
  word ret, ch;
  byte was_no_local=no_local_output;

  /* DLN 03 Mar 95 Suppress local for .RBS files, save/restore local state */

  no_local_output=(d->type & DISPLAY_NOLOCAL) != 0;
  doing_hotmenu=(d->type & DISPLAY_HOTMENU);
  has_hot=(usr.bits & BITS_HOTKEYS);
  
  for (;;)
  {
    /* Hotkeys, when displaying a menu */

    if (doing_hotmenu && has_hot && ++d->chcount >= 4)
      if (DisplayHandleHotkey(d))
      {
        d->ret=DRET_HOT;
        break;
      }

    /* Get a character */
      
    if ((ch=DispGetChar())==DISP_EOF)
      break;
      
    /* Call the handler routine to display this character */

    if (ch <= 26)
      ret=(*dispfn[ch])(d);
    else ret=DCNormal(d, ch);
      
    if (ret)
      break;

    if (display_col==1 && d->automore &&
        DispMoreYnBreak(d->nonstop, NULL, d->type))
    {
      d->ret=DRET_BREAK;
      break;
    }
  }

#if (COMMAPI_VER > 1)
  ComBurstMode(hcModem, lastState);
#endif
  no_local_output=was_no_local;
  return d->ret;
}


static sword near DisplayHandleHotkey(DSTK *d)
{
  sword mk;

  d->chcount=0;
  
  if (!d->ck_abort || (mk=Mdm_keyp())==0)
    return FALSE;
  
  if (mk=='\x08' || mk=='\x09')
    Mdm_getcw();
  else if (mk=='\x00') /* Throw out scan code too */
  {
    Mdm_getcw();
    Mdm_getcw();
  }
  else
  {
    /* Clear pending output */

    mdm_dump(DUMP_OUTPUT);
    return TRUE;
  }
  
  return FALSE;
}


#ifndef ORACLE

static sword near DisplayFilesBbs(DSTK *d)
{
#if (COMMAPI_VER > 1)
  extern HCOMM hcModem;
  BOOL lastState = ComBurstMode(hcModem, TRUE);
#endif
  byte last_col;
  word ret, ch;
  byte was_no_local=no_local_output;

  /* DLN 03 Mar 95 Suppress output for .RBS files, save/restore local state */

  no_local_output=(d->type & DISPLAY_NOLOCAL) != 0;
  last_col=1;
  wrap=FALSE;
  
  for (;;)
  {
    /* Get a character */
      
    if ((ch=DispGetChar())==DISP_EOF)
      break;
      
    /* If we're at the beginning of a line, process it. */
      
    if (last_col==1 && !wrap /*&& ch > ' ' && ch <= '\x7f' && ch != '-'*/)
    {
      if (ProcessFBBSLine(d, ch))
      {
        d->ret=DRET_BREAK;
        break;
      }
    }
    else if ((d->type & (DISPLAY_NEWFILES|DISPLAY_SEARCH))==0)
    {
      if (ch <= 26)
        ret=(*dispfn[ch])(d);
      else ret=DCNormal(d, ch);
      
      if (ret)
        break;
    }
    else  /* (newfiles || searchfiles) */
    {
      current_col=1;
      wrap=FALSE;
    }
      
    if (d->ret)
      break;

    if (d->automore && display_col==1 &&
        DispMoreYnBreak(d->nonstop, NULL, d->type))
    {
      d->ret=DRET_BREAK;
      break;
    }

    /* Save the current column number */

    last_col=current_col;
  }

  no_local_output=was_no_local;
#if (COMMAPI_VER > 1)
  ComBurstMode(hcModem, lastState);
#endif
  return d->ret;
}



static word near ProcessFBBSLine(DSTK *d, word ch)
{
  sword ret;
  
  if (ch=='@' && d->beginline && !acsflag(CFLAGA_FHIDDEN))
    return TRUE;
  else if (ch=='\r')
    d->beginline=TRUE;
  else if (ch=='-')
  {
    if ((d->type & (DISPLAY_NEWFILES|DISPLAY_SEARCH))==0)
      Puts(WHITE "-");
    
    d->beginline=FALSE;
  }
  else if (ch <= 32 || ch >= 127)
  {
    d->beginline=(ch==10);

    if ((d->type & (DISPLAY_NEWFILES|DISPLAY_SEARCH))==0 || ch==16)
    {
      if (ch==22)
        wrap=TRUE;

      /* Now display the character itself */
      
      if (ch <= 26)
        (*dispfn[ch])(d);
      else DCNormal(d, ch);
    }
  }
  else
  {
    static int cnt=0;
    
    if (d->ck_abort && halt())
    {
      d->ret=DRET_BREAK;
      return TRUE;
    }

    if (++cnt >= 40)
    {
      Mdm_check();
      cnt=0;
    }


    /* Add the character we just got to the beginning of the string */

    *d->scratch=(byte)ch;

    /* Read from the display file 'till EOF */
    
    DispReadline(d, d->scratch+1, sizeof(d->scratch)-1);
    

    if (d->beginline)
    {
      ret=Process_Files_Entry(d, d->scratch);

      if (ret==DRET_BREAK)
      {
        d->ret=DRET_BREAK;
        return TRUE;
      }
    }
    
    d->beginline=TRUE;
  }
  
  return FALSE;
}

#endif





word near DCEnter(DSTK *d)
{
  *d->nonstop=FALSE;
  Press_ENTER();
  d->skipcr=TRUE;

  return FALSE;
}


word near DCMore(DSTK *d)
{
  word ch;
  word ret=FALSE;

  if ((byte)(ch=MoreYnns())==M_NONSTOP)
    *d->nonstop=TRUE;
  else if (ch==NO)
  {
    d->ret=DRET_BREAK;
    ret=TRUE;
  }
  
  display_line=1;
  return ret;
}


word near DCBell(DSTK *d)
{
  d->recd_chars=TRUE; 
  Putc('\x07'); 
  return FALSE;
}



word near DCQuest(DSTK *d)
{
  return (DisplayQuestionnaire(d)==SKIP_FILE);
}



word near DCPriv(DSTK *d)
{
  switch (DispParsePriv(d))
  {
    default:
    case SKIP_NONE:
      break;

    case SKIP_FILE:
      d->ret=DRET_OK;
      return TRUE;

    case SKIP_LINE:
      DispSkipLine(d);
  }
  
  return FALSE;
}

word near DCAvatar(DSTK *d)
{
#if (COMMAPI_VER > 1)
  extern HCOMM hcModem;
  BOOL lastState = ComBurstMode(hcModem, TRUE);
#endif
  word ch;
  
  /* AVATAR beginning sequence */

  Putc('\x16');
  Putc(ch=DispSlowGetChar(d));

  switch (ch)
  {
    case '\x01':                  /* ^A */
      if ((ch=DispSlowGetChar(d))==DLE)
      {
        Putc(DLE);           /* ^P */
        ch=DispSlowGetChar(d);
      }

      Putc(ch);
      break;

    case '\x08':
      PF(d, 2);
      break;

    case '\x0a':            /* Five trailing chars total */
    case '\x0b':
      PF(d, 5);
      break;

    case '\x0d':            /* Four trailing chars total */
      PF(d, 4);
      break;

    case '\x0c':            /* Three trailing chars total */
      PF(d, 3);
      break;

    case '\x19':            /* Repeat pattern */
      Putc(ch=DispSlowGetChar(d));
      PF(d, ch+1);
      break;
  }
  
#if (COMMAPI_VER > 1)
  ComBurstMode(hcModem, lastState);
#endif
  return FALSE;
}



word near DCMaximus(DSTK *d)
{
  word ret;
  
  /* Make sure that stack points to most current record */
  
  dtop=d;

  if ((ret=DisplayMaxCode(d))==SKIP_FILE)
    return TRUE;
  else if (ret==SKIP_LINE)
  {
    DispSkipLine(d);
    d->beginline=TRUE;
  }

  return FALSE;
}



word near DCRLE(DSTK *d)
{
  if (d->ck_abort && halt())
  {
    d->ret=DRET_BREAK;
    return TRUE;
  }

  Putc('\x19');

  PF(d, 2);
  return FALSE;
}


word near DCZ(DSTK *d)
{
  d->beginline=TRUE;
  return FALSE;
}

word near DCCR(DSTK *d)
{
  if (!d->skipcr)
    d->recd_chars=TRUE;

  return FALSE;
}


word near DCLF(DSTK *d)
{
  d->beginline=TRUE;

  if (d->skipcr)
    d->skipcr=FALSE;
  else DCNormal(d, '\x0a');

  return FALSE;
}



word near DCNormal(DSTK *d, word ch)
{
  if (++d->chcount_n >= 3)
  {
    if (d->ck_abort && halt())
    {
      d->ret=DRET_BREAK;
      return TRUE;
    }

    d->chcount_n=0;
  }
  
  d->recd_chars=TRUE;

  Putc(ch);

  return FALSE;
}


word near DCCls(DSTK *d)
{
  if (d->skipcr)
    d->skipcr=FALSE;

  Putc('\x0c'); /* cls */
  d->recd_chars=TRUE;
  return FALSE;
}


word near DCCKOff(DSTK *d) { d->ck_abort=FALSE; return FALSE; }
word near DCCKOn(DSTK *d)  { d->ck_abort=TRUE;  return FALSE; }
word near DCMoreOn(DSTK *d)    { d->automore=TRUE; return FALSE;  }
word near DCParseData(DSTK *d) { DisplayDatacode(d); return FALSE; }
word near DCMoreOff(DSTK *d)   { d->automore=FALSE; return FALSE;}
word near DCNul(DSTK *d)   { DCNormal(d, 0); return FALSE;}
word near DC8(DSTK *d)   { DCNormal(d, 0x08);  return FALSE; }
word near DC9(DSTK *d)   { DCNormal(d, 0x09);  return FALSE; }
word near DCe(DSTK *d)   { DCNormal(d, 0x0e);  return FALSE; }
word near DC11(DSTK *d)  { DCNormal(d, 0x11);  return FALSE; }
word near DC12(DSTK *d)  { DCNormal(d, 0x12);  return FALSE; }
word near DC13(DSTK *d)  { DCNormal(d, 0x13);  return FALSE; }
word near DC14(DSTK *d)  { DCNormal(d, 0x14);  return FALSE; }
word near DC15(DSTK *d)  { DCNormal(d, 0x15);  return FALSE; }
word near DC18(DSTK *d)  { DCNormal(d, 0x18);  return FALSE; }




#ifndef ORACLE
static void near DispReadline(DSTK *d, char *str, sword limit)
{
  word ch;
  char *p=str;
  
  while (--limit > 0 && (ch=DispSlowGetChar(d)) != '\n' && ch != DISP_EOF)
    *p++=(char)ch;
  
  *p='\0';
}
#endif




/* Function to refill the .BBS file buffer */

word _DispGetChar(DSTK *d, word inc)
{
  sword got=read(d->bbsfile, d->filebufr, FILEBUFSIZ);
  
  if (got <= 0)
    return DISP_EOF;

  d->highp=d->filebufr+got;
  d->bufp=d->filebufr;
  
  return (word)(inc ? *d->bufp++ : *d->bufp);
}




static void near PF(DSTK *d, word n)
{
#if (COMMAPI_VER > 1)
  extern HCOMM hcModem;
  BOOL lastState = ComBurstMode(hcModem, TRUE);
#endif

  while (n--)
    Putc(DispSlowGetChar(d));

#if (COMMAPI_VER > 1)
  ComBurstMode(hcModem, lastState);
#endif
}
      



char * DispGetString(DSTK *d, char *str, word maxlen)
{
  word ch;
  char *s, *end;
  
  for (s=str, end=str+maxlen-1;
       s < end && (ch=DispSlowGetChar(d)) != '\n' && ch != DISP_EOF; )
    *s++=(char)ch;
  
  *s++='\0';
  
  Fix_RLE(str);
  Trim_Line(str);
  
  return str;
}

void DispGetToBlank(DSTK *d, char *s)
{
  char *orig;

  word c;

  word s1, s2;

  orig=s;

  while ((c=DispSlowGetChar(d)) != DISP_EOF && (c > ' ' || c==25))
  {
    if (c==25)  /* Expand any RLE sequences */
    {
      s1=DispSlowGetChar(d);
      s2=DispSlowGetChar(d);

      while (s2--)  /* Expand the character 's1', a total of 's2' times */
        *s++=(char)s1;

      if (! (s1 > 32))  /* Break out of loop if RLE'd char is a space */
        break;
    }
    else *s++=(char)c;
  }

  /* Get rid of the LF after the CR */
  if (c==13)
    (void)DispSlowGetChar(d);

  *s='\0';

  /* Strip all trailing blanks */
  for (s--;s >= orig && *s <= 32;s--)
    *s='\0';

}


/* Skip until the next \r (plus possibly \n) in a display-type file */

void DispSkipLine(DSTK *d)
{
  word ch;
  int state=0;

  while ((ch=DispSlowGetChar(d)) != DISP_EOF)
  {
    if (ch=='\r')
      state=1;
    else if (state==1 && ch=='\n')
      break;
    else state=0;
  }

  d->beginline=TRUE;
}




static word near DispParsePriv(DSTK *d)
{
  word ch;

  switch (ch=DispSlowGetChar(d))
  {
    case 'a':     /* Acs string */
      DispGetToBlank(d, d->scratch);
      return PrivOK(d->scratch, FALSE) ? SKIP_NONE : SKIP_LINE;

    case 'f':     /* Acs string - file */
      DispGetToBlank(d, d->scratch);
      return PrivOK(d->scratch, FALSE) ? SKIP_NONE : SKIP_FILE;

    case 'L':     /* Show rest of line to <ch> and above */
    case 'B':
    case 'Q':
    case 'X':
      return (Priv_Code(DispSlowGetChar(d), ch));

    case 'F':
      ch=DispSlowGetChar(d);
      /* fall-through */

    default:
      if (Priv_Code(ch,'L')==SKIP_LINE)
        return SKIP_FILE;
      else return SKIP_NONE;
  }
}

word DispSlowGetChar(DSTK *d)
{
  return (DispGetChar());
}


int DispMoreYnBreak(char *nonstop,char *colour,int type)
{
  int cantag;

  if (! (type & DISPLAY_FILESBBS))
    return (MoreYnBreak(nonstop, colour));

  if (display_line < (byte)TermLength())
    return FALSE;

  if ((usr.bits2 & BITS2_MORE)==0 || *nonstop)
  {
    display_line=1;
    vbuf_flush();
    return FALSE;
  }

  for (;;)
  {
    byte c;

    if (colour != NULL)
      Puts(colour);

    /* DLN: Requires minimum baud, saves on user frustration... */

    cantag=(local || (baud >= ClassGetInfo(cls,CIT_MIN_XFER_BAUD))) &&
           CanAccessFileCommand(&fah, file_tag, 0, NULL);

    c=GetListAnswer(cantag ? MoreYnTag : Yne,
                    NULL,
                    cantag ? useyforyesnst : useyforyesns,
                    CINPUT_NOLF | CINPUT_DISPLAY,
                    cantag ? fbbs_more_promptt : fbbs_more_prompt);

    if (usr.video==GRAPH_TTY)
    {
      if (c==*Tag)
        Putc('\n');
      else Puts(moreynns_blank);
    }
    else Puts(fbbs_more_blank);

    if (c==*Tag && cantag)
    {
      byte was_display_line=display_line; /* RIP sequences can change this */
      File_Get_Download_Names(TAG_ONELINE, PROTOCOL_ZMODEM);
      display_line=was_display_line;
      Puts(WHITE);
    }
    else if (c==YES)
    {
      display_line=1;
      return FALSE;
    }
    else if (c==M_NONSTOP)
    {
      *nonstop=TRUE;
      display_line=1;
      return FALSE;
    }
    else if (c==NO)
    {
      display_line=1;
      return TRUE;
    }
  }
}


