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
static char rcs_id[]="$Id: max_in.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Character/word/line general modem and keyboard input functions
*/

#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <mem.h>
#include <time.h>
#include <stdarg.h>
#include "prog.h"
#include "keys.h"
#include "mm.h"

/* Max recursion level support for GetListAnswer() */

#define MAX_RECUR 8

static char *szInputString=NULL;
static char *szCharString=NULL;
static char *aszListString[MAX_RECUR];

#ifndef __GNUC__
static int near Inputv(char *dest,int type,int ch,int max,char *prompt, char *va_args[1]);
static int near Input_Charv(int type, char *extra, char *va_args[1]);
#else
static int near Inputv(char *dest,int type,int ch,int max,char *prompt, va_list va_args);
static int near Input_Charv(int type, char *extra, va_list va_args);
#endif

void InputAllocStr(void)
{
  if (szInputString==NULL && (szInputString=malloc(MAX_PRINTF))==NULL)
  {
    printf(printfstringtoolong, "Inputv:vs");
    quit(ERROR_CRITICAL);
  }

  if (szCharString==NULL && (szCharString=malloc(MAX_PRINTF))==NULL)
  {
    printf(printfstringtoolong, "Inputv:vs");
    quit(ERROR_CRITICAL);
  }

  if ((aszListString[0]=malloc(MAX_PRINTF))==NULL)
  {
    printf(printfstringtoolong, "GetListAnswer:vs");
    quit(ERROR_CRITICAL);
  }
}

void InputFreeStr(void)
{
  free(aszListString[0]);
  free(szCharString);
  free(szInputString);

  aszListString[0] = NULL;
  szCharString = NULL;
  szInputString = NULL;
}


#ifdef MCP_VIDEO

extern int usStrokes;
extern byte cbStrokeBuf[MAX_BUF_STROKE];

/* Functions to peek and get characters from the MCP keyboard buffer */

static int near McpKeyGetch(void)
{
  int rc;

  if (!usStrokes)
    return -1;

  rc=*cbStrokeBuf;
  memmove(cbStrokeBuf, cbStrokeBuf+1, --usStrokes);

  return rc;
}

static int near McpKeyPeek(void)
{
  return usStrokes ? *cbStrokeBuf : -1;
}

#endif



int loc_getch(void)
{
#ifdef MCP_VIDEO
  int ch;

  while (kpeek()==-1 && McpKeyPeek()==-1)
    DosSleep(1);

  if ((ch=kpeek()) != -1)
    return kgetch();

  return McpKeyGetch();
#else
  return kgetch();
#endif
}




int loc_peek(void)   /* Returns -1 on no character, else returns ch */
{
  int ch=kpeek();

  if (ch != -1)
    return ch;

#ifdef MCP_VIDEO
  ch=McpKeyPeek();
#endif

  return ch;
}

int cdecl InputGets(char *dest, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_WORD,0,0,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetsM(char *dest, int max, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_WORD,0,max,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetse(char *dest, char ch, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_WORD | INPUT_ECHO,ch,0,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetsC(char *dest, char ch, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_WORD | INPUT_ALREADYCH,ch,0,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetsL(char *dest, int max, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_NLB_LINE,0,max,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetsLe(char *dest, int max, char ch, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_NLB_LINE | INPUT_ECHO,ch,max,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetsLLe(char *dest, int max, char ch, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_LB_LINE | INPUT_ECHO,ch,max,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl InputGetsLL(char *dest, int max, char *prompt, ...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,INPUT_LB_LINE,0,max,prompt,arg);
  va_end(arg);
  return rc;
}

int cdecl Inputf(char *dest,int type,int ch,int max,char *prompt,...)
{
  int rc;
  va_list arg;

  va_start(arg,prompt);
  rc=Inputv(dest,type,ch,max,prompt,arg);
  va_end(arg);
  return rc;
}


/* General-purpose word/string input function.  `dest' is where we want
 * the output to go.  `type' defines the behaviour of Input();  See
 * MAX.H for details.  `ch' is only used if a particular value is used
 * for `type';  Again, see MAXIMUS.H.  `prompt' is the prompt to display
 * to the user.  (Use NULL if none required)
 */

#ifndef __GNUC__
static int near Inputv(char *dest,int type,int ch,int max,char *prompt, char *arg[1])
#else
static int near Inputv(char *dest,int type,int ch,int max,char *prompt, va_list arg)
#endif
{
  int ret=0;
  char *s;

  if (prompt && strchr(prompt,'%'))
  {
    vsprintf(szInputString, prompt, arg);
    prompt=szInputString;
  }

  if (max==0)
    max=BUFLEN;

  /* Throw away any ^c's */
  
  halt();

  if (! *linebuf || (type & (INPUT_NLB_LINE | INPUT_ALREADYCH)))
  {
    /* Display the prompt */

    if (prompt)
    {
      /* Strip off any leading '\n's when in hotflash mode, so we stay on   *
       * one line!                                                          */

      Puts(prompt);
    }

    if ((type & INPUT_MSGENTER) || (type & INPUT_WORDWRAP))
      strcpy(linebuf, dest);

    ret=mdm_gets(linebuf, type, ch, min(max, BUFLEN-1), prompt);
  }

  /* Allow "|;<text>" or "|<cr>" to be used instead of <enter> */

  if (*linebuf=='|' &&
      (strchr(cmd_delim, linebuf[1]) || linebuf[1]=='\0') &&
      (type & INPUT_WORDWRAP)==0)
  {
    *dest='\0';

    if (linebuf[1])
      strocpy(linebuf, linebuf+2);
    else *linebuf='\0';
  }
  else /* normal text */
  {
    /* If we're just getting a WORD... */

    if (type & INPUT_WORD)
    {
      getword(linebuf, dest, cmd_delim, 1);

      if ((s=firstchar(linebuf, cmd_delim, 2))==NULL)
        s=blank_str;

      strocpy(linebuf, s);
    }
    else  /* else get the entire line! */
    {
      strcpy(dest, linebuf);

      if (ret==1 && (type & INPUT_MSGENTER)==0 && (type & INPUT_WORDWRAP))
        strcpy(dest+strlen(dest)+1, linebuf+strlen(linebuf)+1);

      *linebuf='\0';
    }
  }

  return ret;
}


int cdecl Input_Charf(int type,char *extra,...)
{
  int rc;
  va_list arg;

  va_start(arg,extra);
  rc=Input_Charv(type,extra,arg);
  va_end(arg);
  return rc;
}

#ifndef __GNUC__
static int near Input_Charv(int type, char *extra, char *arg[1])
#else
static int near Input_Charv(int type, char *extra, va_list arg)
#endif
{
  int mdm_keyp;
  int isnllb;
  int timer2;

  char *of;
  unsigned ret=0;

  if (extra)
  {
    vsprintf(szCharString, extra, arg);
    extra=szCharString;
  }

  /* Don't allow CINPUT_DUMP ifthe user doesn't have hotkeys */

  if ((usr.bits & BITS_HOTKEYS)==0 && (type & CINPUT_DUMP))
    type &= ~CINPUT_DUMP;


  halt();

  mdm_keyp=(usr.bits & BITS_HOTKEYS) && Mdm_keyp();

  if ((isnllb=! *linebuf) != FALSE)
  {
    if ((type & CINPUT_PROMPT) && !(type & CINPUT_P_CTRLC) && !mdm_keyp)
      Puts(extra);

    vbuf_flush();
  }

  for (;;)
  {
    if (! *linebuf)
    {
      if ((usr.bits & BITS_HOTKEYS)==0)
      {
        ret=mdm_gets(linebuf,
                     ((type & CINPUT_NOCTRLC) ? INPUT_NOCTRLC : 0) |
                     ((type & CINPUT_MSGREAD) ? INPUT_MSGREAD : 0) |
                     ((type & CINPUT_NOLF) ? INPUT_NOLF : 0) |
                     ((type & CINPUT_SCAN) ? INPUT_SCAN : 0),
                     '\0', BUFLEN-1, (type & CINPUT_PROMPT) ? extra : NULL);
      }
      else
      {
        timer2=FALSE;
        input_timeout=timerset(timeout_tics);

        if (!Mdm_keyp() && (type & CINPUT_AUTOP) && *linebuf==0)
          Puts(extra);

        vbuf_flush();

        while (!Mdm_keyp())
        {
          if (halt())
          {
            if ((type & CINPUT_PROMPT) && !(type & CINPUT_NOCTRLC))
            {
              mdm_dump(DUMP_ALL);

              ResetAttr();

              if (!mdm_keyp)
                Putc('\n');

              Puts(extra);

              mdm_keyp=FALSE;
              vbuf_flush();
            }
          }

          Check_Time_Limit(&input_timeout,&timer2);

          Check_For_Message(NULL, NULL);
          Giveaway_Slice();
        }

        *linebuf=(char)Mdm_getcw();

        if (*linebuf == 0x0d)
          EatNulAfterCr();

        if (type & CINPUT_DUMP)
        {
          mdm_dump(DUMP_OUTPUT);
          ResetAttr();
        }

        timer2=FALSE;

        if (*linebuf=='\x00')
        {
          if (loc_peek()==K_ALTJ)
          {
            loc_getch();

            Shell_To_Dos();

            if (type & CINPUT_PROMPT)
            {
              Putc('\r');
              Puts(extra);
            }

            vbuf_flush();
            continue;
          }
          else
          {
            if ((ret=DoEditKey(((type & CINPUT_MSGREAD) ? INPUT_MSGREAD : 0) |
                               ((type & CINPUT_SCAN)    ? INPUT_SCAN : 0),
                               NULL,
                               Mdm_getcw(),
                               '\0')) != 0)
            {
              break;
            }
          }
        }

        if (*linebuf != '\r' && 
            ((byte)*linebuf < (byte)32 || *linebuf=='\x7f'))
        {
          /* If we didn't go to the top line because of type-ahead... */

          if (*linebuf==K_CTRLX)
            Puts(bs_sn);

          if (*linebuf==K_CTRLX && (type & CINPUT_PROMPT))
            Puts(extra);

          vbuf_flush();

          mdm_keyp=FALSE;

          *linebuf='\0';
          continue;
        }

        if ((type & CINPUT_DISPLAY) || !(usr.bits & BITS_HOTKEYS))
        {
          Putc((*linebuf=='\n' || *linebuf=='\r') ? ' ' : *linebuf);
          Putc((type & CINPUT_NOLF) ? '\r' : '\n');
        }

        vbuf_flush();

        linebuf[1]='\0';
      }
    }

    if ((type & CINPUT_NOXLT)==0) /* If we should translate characters */
    {
      of=firstchar(linebuf, cmd_delim, 1);

      if (of != NULL && of != linebuf)
        strocpy(linebuf, of);
    }

    if ((type & CINPUT_ACCEPTABLE)==0 ||
        strpbrk(strupr(linebuf), strupr(extra)) == linebuf ||
        ((*linebuf=='\r' || *linebuf=='\0' || *linebuf==' ') &&
         (strchr(extra,'|') || (type & CINPUT_ALLANSWERS)==0)) )
    {
      break;
    }
    else
    {
      if ((byte)linebuf[0] >= (byte)32)
        Printf(no_undrstnd, linebuf[0]);

      Puts(tryagain);
      vbuf_flush();

      Clear_KBuffer();
    }
  }

  if (ret < 255)    /* pos-process only non-cursor & non-function keys */
  {

    ret=*linebuf;

    if ((type & CINPUT_NOUPPER)==0)
      ret=(char)toupper(ret);

    if (*linebuf && *linebuf != '\r')
    {
      if (type & CINPUT_LASTMENU)
        lastmenu=(byte)toupper(linebuf[0]);

      if ((of=firstchar(linebuf+1, cmd_delim, 1))==NULL)
        of=blank_str;

      strocpy(linebuf, of);
    }
    else
    {
      if (! (type & CINPUT_NOXLT)) /* If we should translate CR to a '|' */
        lastmenu=ret='|';

      *linebuf='\0';
    }
  }

  /* If this has been recursed, we can junk the buffer to conserve memory */

  return (int)ret;
}



/* Gets an answer out of a list.  The string LIST contains all of the
   acceptable answers;  All must be lower case, except for the DEFAULT
   option (if any), which can be selected by pressing Enter.  If help_file
   is non-NULL, then if the LAST CHARACTER in the list is selected, the
   .BBS file `help_file' will be displayed, and the user will be
   re-prompted.  `invalid_response' is what the function displays if
   the user hits a wrong key, but before s/he is re-prompted.  `o_prompt'
   is the actual prompt, such as "More" or "Use OPed full-screen editor".  */

int cdecl GetListAnswer(char *list, char *help_file, char *invalid_response,
                        int type, char *o_prompt, ...)
{
  int len, ch;
  int isnllb;
  int retval;
  int rtnhelp=!!(type & CINPUT_RTNHELP);

  char *p;
  char *scratch;

  static char lvl=-1;

  /* We can recurse into this via the 'help' option */
  /* So we may need more than one buffer            */

  if (++lvl==MAX_RECUR || (aszListString[lvl]==NULL && (aszListString[lvl]=malloc(MAX_PRINTF))==NULL))
  {
    --lvl;
    printf(printfstringtoolong, "GetListAnswer:vs");
    return -1;
  }

  scratch=aszListString[lvl];

  if (o_prompt==NULL)
    *scratch='\0';
  else
  {
    if (strchr(o_prompt,'%')==NULL)
      strcpy(scratch,o_prompt);     /* The easy way */
    else
    {
      va_list arg;                  /* Wants to format */

      va_start(arg,o_prompt);
      vsprintf(scratch, o_prompt, arg);
      va_end(arg);
    }
  }

  /* These are (almost) always used */

  type |= (CINPUT_NOXLT | CINPUT_DISPLAY | CINPUT_DUMP);

  /* Force fullprompt if we're prompting with a RIP   */
  /* sequence to prevent unexpected side-effects      */
  /* Also suppress display (assumes hotkeys with RIP) */

  if (hasRIP() && strstr(scratch,"!|"))
  {
    type |= CINPUT_FULLPROMPT;
    type &= ~CINPUT_DISPLAY;
  }

  if (! (type & CINPUT_FULLPROMPT))
  {
    int first=TRUE;

    /* Tack on the options onto the end of the prompt string. */

    len=strlen(scratch);

    for (p=list; *p; p++)
    {
      if ((*p > ' ' || *p < 0 ) && *p != '|')
      {
        if (!first)
          scratch[len++]=',';
        else
        {
          first=FALSE;
          strcat(scratch, listanswer_left);
          len=strlen(scratch);
        }
        scratch[len++]=*p;
      }
    }

    /* Close the prompt, adding "=help" if there's a help file */

    if (!first)
    {
      scratch[len]='\0';

      if (help_file != NULL && *help_file)
        strcpy(scratch+len, eq_help);

      strcat(scratch, listanswer_right);
    }

  }

  isnllb=! *linebuf;

  #define NO_RET (byte)'\xff'
    
  retval=NO_RET;

  while (retval==NO_RET)
  {
    ch=Input_Char(type | (isnllb ? CINPUT_PROMPT : 0),
                         (isnllb ? scratch : NULL));

    if (ch <= 255)
      ch=toupper(ch);

    for (p=list; *p; p++)
    {
      if (ch==(int)toupper(*p) ||
         (ch=='\r' && *p=='|') ||
         (ch=='|' && *p=='\r') ||
         (isupper(*p) && (ch=='\r' || ch=='\x00' || ch=='|')))
      {
        retval=(byte)toupper(*p);
        break;
      }
    }

    if (*p=='\0') /* If we didn't match anything */
    {
      /* If we are allowed to return any response, simply return this       *
       * one without displaying an error.                                   */

      if (type & CINPUT_ANY)
      {
        retval=ch;
        break;
      }

      if (invalid_response && *invalid_response)
        Printf("\r%s", invalid_response);

      Clear_KBuffer();
      isnllb=TRUE;
      retval=NO_RET;
    }
    else if (*(p+1)=='\0' && help_file && *help_file)
    {
      Putc('\n');

      Display_File(0, NULL, help_file);

      if (!rtnhelp)
        retval=NO_RET;
    }
  }

  /* If this has been recursed, we can junk the buffer to conserve memory */

  if (lvl > 0)
  {
    free(aszListString[lvl]);
    aszListString[lvl]=NULL;
  }

  --lvl;
  return retval;
}




#ifdef NEVER /* notused */

int MoreYn(void)
{
  int c;

  Puts(CLEOL);
  c=GetYnAnswer(more_prompt,CINPUT_NOLF);

  if (usr.video==GRAPH_TTY)
    Puts(moreyn_blank);
  else Puts(CLEOL);

  return(c);
}

#endif


byte MoreYnns(void)
{
  byte c;

  Puts(CLEOL);
  c=GetYnnsAnswer(more_prompt,CINPUT_NOLF | CINPUT_DISPLAY);

  if (usr.video==GRAPH_TTY)
    Puts(moreynns_blank);
  else Puts(CLEOL);

  return(c);
}



signed int timeleft(void)
{
  static signed long temptime;

  if ((temptime=(signed long)time(NULL)) > (signed long)timeoff)
    return -1;
  else return ((int)(((signed long)timeoff-temptime)/60L)+1);
}

signed int timeonline(void)
{
  return ((int)((time(NULL)-(signed long)timeon)/60L));
}


/* Tells the user to press enter */

void Press_ENTER(void)
{
  int ch;
  int timer2;
  char *prompt=press_enter_s;
  char *p;
  
  /* If an <enter> has been queued */


  if (*linebuf && (*linebuf=='|' || strchr(cmd_delim, *linebuf)))
  {
    p = linebuf+1;

    while (*p && strchr(cmd_delim, *p))
      p++;

    strocpy(linebuf, p);
    return;
  }

  Puts(prompt);
  vbuf_flush();

  /* Start the input timer */
  
  timer2=FALSE;
  input_timeout=timerset(timeout_tics);

  for (;;)
  {
    if (Mdm_kpeek() != -1)
    {
      ch=Mdm_getcw();
      
      if (ch==K_RETURN)
      {
        EatNulAfterCr();
        break;
      }
      else if (ch==0)
      {
        if (loc_peek()==K_ALTJ)
        {
          loc_getch();
          Shell_To_Dos();
        }
      }
    }
    
    if (halt())
      break;

    Check_Time_Limit(&input_timeout, &timer2);
    Check_For_Message(NULL, NULL);
    Giveaway_Slice();
  }

  Puts(press_enter_c);

  *linebuf='\0';
  display_line=1;
}



/* Do a more y/n/= only when required, keeping track of the current line,
   HotFlash state, etc.  Nonstop should be a pointer to a char in the
   calling program, which indicates whether or not we're supposed to be
   running in non-stop mode.  (You shoukd initialize it to FALSE.)         */

int MoreYnBreak(char *nonstop,char *colour)
{
  byte ch;
  
  if (display_line >= (byte)TermLength() && (usr.bits2 & BITS2_MORE))
  {
    if (*nonstop)
    {
      display_line=1;
      vbuf_flush();
      return FALSE;
    }

    if (colour != NULL)
      Puts(colour);

    if ((ch=MoreYnns())==YES)
      ;
    else if (ch==M_NONSTOP)
      *nonstop=TRUE;
    else if (ch==NO)
    {
      display_line=1;
      return TRUE;
    }

    display_line=1;
  }

  return FALSE;
}


/* EatNulAfterCr - if we are running an incoming telnet session,
 * eat NULs that may be present after incoming CRs.  This is so that
 * the remote user does not need to toggle the "crmod" setting
 * to interact properly with Maximus.
 */

void EatNulAfterCr(void)
{
#if defined(OS_2) || defined(UNIX)
  long t;

  if (GetConnectionType()==CTYPE_TELNET)
  {
    /* Wait for a tenth of a second */

    t = timerset(10);

    while (!timeup(t))
    {
      if (Mdm_kpeek()==0)
      {
        Mdm_getcw();
        break;
      }

      Giveaway_Slice();
    }
  }
#endif
}


