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
static char rcs_id[]="$Id: max_cmod.c,v 1.2 2003/06/04 23:35:23 wesgarland Exp $";
#pragma on(unreferenced)

#define MAX_LANG_max_init
#define MAX_LANG_f_area
#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "keys.h"
#include "alc.h"
#include "mm.h"

static int pm_getch(void);
static int near Open_Chatlog(void);

void ChatMode(void)
{
  struct _fossil_info finfo;
  char *chatprog;
  char temp[PATHLEN];
  char lastword[BUFLEN];
  char savekb, savesnoop, savedotc;
  word savebits2;
  struct _css css;

  dword add1, add2;

  word lastidx;
  byte col;

  sword last_speaker, break_loop;
  sword ch, len, c;

  long startchat;


  fossil_inf((struct _fossil_info far *)&finfo);
  
  num_yells=0;

  /* Redraw status line to remove flashing 'C' */
  
  Draw_StatusLine(STATUS_FORCE);

  mdm_dump(DUMP_ALL);
  ResetAttr();

  if (inchat)
  {
    LocalMsg(chat_nest);
    return;
  }

  inchat=TRUE;
  
  /* Make the caller unavailable for chat */
  
  ChatSaveStatus(&css);
  
  /* Kludge so that lang file doesn't have to be modified */

  ChatSetStatus(FALSE, cs_notavail);

  savekb=keyboard;
  savesnoop=snoop;
  savedotc=do_timecheck;
  savebits2=usr.bits2;

  display_line=display_col=1;
  
  /* If user hasn't logged on yet, disable IBM chars so that parity         *
   * translates okay.                                                       */

  if (*usr.name=='\0')
    usr.bits2 &= ~BITS2_IBMCHARS;

  chatlog=NULL;
  keyboard=snoop=TRUE;
  do_timecheck=FALSE;

  logit(log_chat, entering, the_chatmode);
  startchat=time(NULL);

  /* If we have a specific prog. to run for CHAT */

  if (*(chatprog=PRM(chat_prog))!='\0')
  {
    if (*chatprog==':')  /* It's a MEX script */
    {
      Mex(chatprog+1);
    }
    else
    {
      Outside(NULL, NULL, OUTSIDE_DOS, chatprog, FALSE, CTL_NONE,
              RESTART_MENU, NULL);
    }
  }
  else
  {

    *lastword='\0';
    lastidx=0;
    last_speaker=-1;
    LocalMsg(chat_on);

    if (*PRM(chat_fbegin))
      Display_File(0,NULL,PRM(chat_fbegin));
    else Puts(chat_start);

    col=1;

    Mdm_Flow_Off();
    vbuf_flush();
    break_loop=FALSE;

    /* Automatically open the chat capture log, if we're supposed to do so. */
    
    if (prm.flags2 & FLAG2_CAPTURE)
      Open_Chatlog();
  
    do
    {
      if (loc_kbhit() || brk_trapped)
      {
        if (last_speaker != 0)
        {
          Puts(YELLOW);
          last_speaker=0;
        }

        if (brk_trapped)
        {
          brk_trapped=0;
          ch=3;
        }
        else ch=loc_getch();

        switch (ch)
        {
          case K_ONEMORE:             /* Scan code... */
            c=pm_getch();

#ifdef UNIX
	    if (c == K_ESC)
              goto realEscape;		/* turn esc-esc into esc */
#endif	
            if ((c >= K_F1 && c <= K_F10) || (c >= K_SF1 && c <= K_SF10))
            {
              Parse_FKey(c);
              col=current_col;
              *lastword='\0';
            }
            else if (c==K_ALTC)     /* Chat buffer toggle */
            {
              if (chatlog)          /* File is already open */
              {
                LocalMsg(chat_capoff);
                Close_Chatlog();
              }
              else                  /* File is closed, open buffer */
              {
                if (Open_Chatlog())
                  LocalMsg(chat_capon);
              }
            }
            else if (c==K_ALTJ)
            {
              /* Jump to dos */
            
              Shell_To_Dos();
              Putc('\n');

              *lastword='\0';
              last_speaker=-1;
            }
            else if (c==K_ALTT)
            {
              /* Display a specific filename */
            
              /* So that input can be only entered locally */
              local=TRUE;

              InputGets(temp, type_which);
            
              local=FALSE;

              display_line=display_col=1;

              Display_File(0,NULL,temp);
            }
            else if (usr.video != GRAPH_TTY)
            {
              /* Handle cursor keys if the user has graphics turned on */
            
              *lastword='\0';
            
              switch (c)
              {
                case K_UP:
                  if (current_line > 1)
                    Puts(UP);
                  break;

                case K_LEFT:
                  if (current_col > 1)
                  {
                    col--;
                    Puts(LEFT);
                  }
                  break;

                case K_RIGHT:
                  if (current_col < TermWidth() &&
                      current_col < (byte)(finfo.width-1))
                  {
                    col++;
                    Puts(RIGHT);
                  }
                  break;

                case K_DOWN:
                  if (current_line < usr.len &&
                      current_line < (byte)(finfo.height))
                  {
                    Puts(DOWN);
                  }
                  break;
              }
            }
          
            vbuf_flush();
            break;

          case 7:
            Putc('\a');
            vbuf_flush();
            ch=0;
            break;

#ifdef UNIX
realEscape:
	  case -2:
#else
          case K_ESC:                /* ESCape */
#endif
            break_loop=TRUE;
            ch=0;
            break;
        }
      }
      else if (mdm_avail())
      {
        if (last_speaker != 1)
        {
          Puts(CYAN);
          last_speaker=1;
        }

        switch (ch=(mdm_ggetcw()& ((usr.bits2 & BITS2_IBMCHARS) ? 0xff : 0x7f)))
        {
          case K_ESC:
            if (((ch=pm_getch()) & 0x7f)=='[' || ch=='O')
            {
              switch (pm_getch() & 0x7f)  /* Strip all other ANSI sequences! */
              {
                case 'A':       /* Up */
                  if (current_line > 1)
                  {
                    Puts(UP);
                    *lastword='\0';
                  }
                  break;

                case 'D':        /* Left */
                  if (current_col > 1)
                  {
                    col--;
                    Puts(LEFT);
                    *lastword='\0';
                  }
                  break;

                case 'C':        /* Right */
                  if (current_col < TermWidth() && 
                      current_col < (byte)(finfo.width-1))
                  {
                    col++;
                    Puts(RIGHT);
                    *lastword='\0';
                  }
                  break;

                case 'B':        /* Down */
                  if (current_line < (byte)(min(usr.len, finfo.height)))
                  {
                    Puts(DOWN);
                    *lastword='\0';
                  }
                  break;
              }
            
              vbuf_flush();
            }

            ch=0;
            break;
        }
      }
      else
      {
        ch=0;
        Check_For_Message(NULL,NULL);

        if (!local && !carrier())
          break;

        Giveaway_Slice();
      }

      if (break_loop)
        break;

      switch (ch)
      {
        case '\x00':                /* If we didn't get anything... */
        case '\x07':                /* or we got a BEL...           */
        case '\x0a':                /* or we got a linefeed...      */
          break;                    /* then don't do anything!      */

        case 127:                   /* VT100 del */
        case '\x08':                /* Backspace */
          if (col > 1)
          {
            col--;
            Puts(bs_sp_bs);

            if (lastidx)
              lastword[--lastidx]='\0';

            if (chatlog)            /* Seek back one */
              fseek(chatlog,ftell(chatlog)-1L,SEEK_SET);
          }

          break;

        case '\x0d':                /* Carriage return */
          Putc('\n');
          lastword[lastidx=0]='\0';
          col=1;

          if (chatlog)
            fputc('\n',chatlog);
        
          break;

        case '\x09':                /* TAB */
          len=9-(current_col % 8);
          col += len;
          Printf("\x19 %c",len);

          if (chatlog)
            while (len--)
              fputc(' ',chatlog);

          lastword[lastidx=0]='\0';
          break;

        case '+':                   /* Guard against "+++" sequence */
          Puts("+\x08");
          /* fall-through */

        default:                    /* Normal character */
          if (chatlog)
            fputc(ch,chatlog);

          Putc(ch);
          col++;

          if (lastidx >= (word)sizeof(lastword)-2)
            lastword[lastidx=0]='\0'; /* If word is too long, zero out */
          else
          {
            lastword[lastidx]=(byte)ch;
            lastword[++lastidx]='\0';
          }

          /* If it's a space, zero out lastword buffer */
        
          if (ch==' ' || ch==',' || ch=='-')  
            lastword[lastidx=0]='\0';

          if (col >= (byte)(TermWidth()-4))
          {
            if (strlen(lastword) > (word)(TermWidth()/2))
            {
              if (chatlog)
                fputc('\n',chatlog);

              Puts(CLEOL "\n");
              lastword[lastidx=0]='\0';
              col=1;
            }
            else
            {
              for (len=strlen(lastword); len--; )  /* Back up over word */
              {
                if (usr.video==GRAPH_TTY)
                  Puts(bs_sp_bs);
                else Putc('\x08');            /* We'll erase later with CLEOL */
              }

              Puts(CLEOL "\n");               /* New line... */
              Puts(lastword);                 /* Drop the word in place */

              if (chatlog)                    /* Back up that many spaces... */
              {
                fseek(chatlog,-(long)strlen(lastword),SEEK_CUR);

                if (chatlog)
                  fputc('\n',chatlog);

                fputs(lastword,chatlog);
              }

              col=(byte)(strlen(lastword)+1);
            }
          }
          break;
      }

      if (ch != 0)
        vbuf_flush();
    }
    while (!break_loop);

    Mdm_Flow_On();

    if (chatlog)
      Close_Chatlog();

    display_line=display_col=1; /* So "More [Y,n,=]?" doesn't happen right   *
                                 * after CHAT finishes.                      */
    if (local || carrier())
    {
      if (*PRM(chat_fend))
        Display_File(0, NULL, PRM(chat_fend));
      else Puts(chat_end);
    }
  }

  logit(log_chat, exiting, the_chatmode);

  /* Time compensation */

  if ((dword)time(NULL) < timestart+(max_time*60L))
  { 
    add1=time(NULL)-startchat;
    add2=(timestart+(max_time*60L))-time(NULL);

    Add_To_Time(min(add1, add2));
  }
  
  /* Even if it would override an event, give the sucker 60 seconds after  *
   * exiting chat, so they don't go <poof> as soon as the SysOp hits <esc> *
   * to exit chat.  (Sometimes, that may happen accidentally.              */
  
  if (timeoff < (dword)(time(NULL)+60L))
    timeoff=time(NULL)+60L;

  do_timecheck=savedotc;

  /* Restore the caller's chat status */
  
  ChatRestoreStatus(&css);

  keyboard=savekb;
  snoop=savesnoop;
  usr.bits2=savebits2;
  inchat=FALSE;

  if (local || carrier())
  {
    Puts(press_enter_s);

    if (inmagnet)
      Fix_MagnEt();
  }
  
  return;
}



static int near Open_Chatlog(void)
{
  union stamp_combo stamp;

  char temp[PATHLEN];
  char p1[50];
  char p2[50];


  sprintf(temp,"%sCHATLOG.%03x",original_path,task_num);

  if (fexist(temp))
    chatlog=sfopen(temp, "r+", O_RDWR | O_NOINHERIT, SH_DENYWR);
  else chatlog=sfopen(temp, "w+", O_RDWR | O_CREAT | O_TRUNC | O_NOINHERIT,
                      SH_DENYWR);

  if (temp==NULL)
  {
    cant_open(temp);
    return FALSE;
  }

  fseek(chatlog, 0L, SEEK_END);

  Get_Dos_Date(&stamp);

  strcpy(p1, Timestamp_Format(PRM(dateformat),&stamp,temp));
  strcpy(p2, Timestamp_Format(PRM(timeformat),&stamp,temp));

  fprintf(chatlog, chat_logmsg, p1, p2, usr.name);

  return TRUE;
}


void Close_Chatlog(void)
{
  union stamp_combo stamp;

  char temp[PATHLEN];
  char p1[50];
  char p2[50];

  Get_Dos_Date(&stamp);

  strcpy(p1, Timestamp_Format(PRM(dateformat), &stamp, temp));
  strcpy(p2, Timestamp_Format(PRM(timeformat), &stamp, temp));

  fprintf(chatlog, chat_logend, p1, p2);

  fclose(chatlog);
  chatlog=NULL;
}


void Toggle_Yell_Noise(void)
{
  int fd;

  prm.noise_ok=(char)!prm.noise_ok;
  LocalMsg(yell_toggle, prm.noise_ok ? sys_on : sys_off);

  /* Update it in the *.PRM file */

  if ((fd=shopen(prmname, O_RDWR | O_BINARY | O_NOINHERIT)) != -1)
  {
    write(fd, (char *)&prm, sizeof(struct m_pointers));
    close(fd);
  }
}

static int pm_getch(void)
{
  for (;;)
  {
    if (loc_kbhit())
      return (loc_getch());

    if (mdm_avail())
      return (mdm_ggetcw());
  }
}



