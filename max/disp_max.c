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
static char rcs_id[]="$Id: disp_max.c,v 1.1 2002/10/01 17:50:51 sdudley Exp $";
#pragma on(unreferenced)

/*# name=.BBS-file display routines (Max-specific)
*/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "prog.h"
#include "max_msg.h"
#include "max_file.h"
#include "max_menu.h"
#include "display.h"

static int near MsgAreaControl(DSTK *d);
static int near ParseTime(DSTK *d);
static word near DispCheckExpire(DSTK *d);
static word near DispHandleKeys(DSTK *d);
static word near DispHandleXtern(DSTK *d);
static word near DispLink(DSTK *d);
static word near DispCheckBaud(DSTK *d);
static word near DispDoAPB(DSTK *d);
static word near DispComment(void);
static word near DispSetPriv(DSTK *d);
static word near DispChatAvail(DSTK *d);
static word near DispCheckHelp(DSTK *d);
static void near DispDoNodeDisp(DSTK *d);
static word near DispNodeOnline(DSTK *d);
static word near DispNodeAvailable(DSTK *d);


/* AllowAttribute
 *
 * This function returns TRUE if the specified attribute number is
 * allowable in the area referenced by pmah, taking into account
 * the user's access level and other permissions.
 */

int AllowAttribute(PMAH pmah, int i)
{
  int fAllowedArea;       /* True if attribute allowed in this area */
  word priv=(word)-1;     /* Privilege level required for user to use attrib */

  /* If the user wants to send a file, and we are in an area that allows
   * local attaches, pretend that it was the local attach key.
   */

  if (i==MSGKEY_FILE && (pmah->ma.attribs & MA_ATTACH))
    i = MSGKEY_LATTACH;

  /* Allow any attributes in netmail.
   * Allow "read" in any area.
   * Allow "private" in any area.
   * Allow file attaches in any area that has the file attach bit set.
   */

  fAllowedArea = (pmah->ma.attribs & MA_NET) ||
                 i==MSGKEY_READ ||
                 i==MSGKEY_PRIVATE ||
                 ((i==MSGKEY_LATTACH) && (pmah->ma.attribs & MA_ATTACH));

  switch (i)
  {
    case MSGKEY_LATTACH:
      if (prm.attach_base && (prm.attach_path || MAS(*pmah, attachpath)))
        priv = prm.msg_localattach;
      break;

    case MSGKEY_PRIVATE:
      if ((pmah->ma.attribs & (MA_PVT|MA_PUB)) == (MA_PVT|MA_PUB))
        priv=usr.priv;
      break;

    default:
      priv = prm.msg_ask[i];
      break;

  }

  return (fAllowedArea &&
          (GEPriv(usr.priv, priv) || mailflag(CFLAGM_ATTRANY)));
}


word DisplayMaxCode(DSTK *d)
{
  char *p;
  union stamp_combo st;
  sword ch;

  switch (DispSlowGetChar(d))
  {
    case '\x01':
      Puts(sc_time(&usr.ludate, d->scratch));
      break;

    case '\x02':
      return DispCheckBaud(d);

    case '\x03':
      Puts(PRM(system_name));
      break;
      
    case '\x04':
      Puts(PRM(sysop));
      break;

    case '\x05':
      Puts(last_readln);
      break;
      
    case '\x06':
      ch=DispSlowGetChar(d);
      switch (ch)
      {
        case 'd': /* The current subdivision within the file area */
          if ((p=strrchr(usr.files,'.')) != NULL)
          {
            Puts(++p);
            break;
          } /*Fallthru*/

        case 'A': /* Current file area */
          Puts(usr.files);
          break;

        case 'D': /* Everything except the last subdivision in the file area */
          if ((p=strrchr(usr.files,'.')) != NULL)
            Printf("%0.*s", p-usr.files, usr.files);
          break;

        case 'N': /* Current file area description */
          Puts(FAS(fah, descript));
          break;
      }
      break;

    case '\x07':
      Local_Beep(1);
      break;

    case '\x0b':
      Puts(bstats.lastuser);
      break;

    case '\x0d':      /* Message-area controls */
      return MsgAreaControl(d);

    case '\x0e':
      if ((ch=DispSlowGetChar(d))=='C')
        Printf(pu, usr.credit);
      else if (ch=='D')
        Printf(pu, usr.debit);
      else if (ch=='B')
        Printf(pl, (long)usr.credit-(long)usr.debit);
      break;

    case '\x10':
      Puts(usr.phone);
      break;

    case '\x12':
      Puts(usr.name);
      break;

    case '8':     /* Only display if user's screen is >= 79 cols */
      if (TermWidth() < 79)
        return SKIP_LINE;
      break;
      
    case 'a':     /* all-points bulletin */
      return DispDoAPB(d);

    case '^':
#ifdef ORACLE
      return SKIP_LINE;
#else
      {
        int fn, i=FALSE;

        FENTRY fent;

        for (fn=0; !i && GetFileEntry(fn, &fent); ++fn)
          i=!!(fent.fFlags & FFLAG_TAG);
        return i ? SKIP_NONE : SKIP_LINE;
      }
#endif

    case 'A':     /* Add line to system log */
      if (*DispGetString(d, d->scratch, PATHLEN))
      {
        Parse_Outside_Cmd(d->scratch, d->temp);
        logit(percent_s, d->temp);
      }
      break;
      
    case 'b': /* iftask */
      DispGetToBlank(d, d->scratch);
      
      if (task_num != (byte)atoi(d->scratch))
        return SKIP_LINE;
      break;

    case 'B': /* iflang */
      DispGetToBlank(d, d->scratch);
      
      if (usr.lang != (byte)atoi(d->scratch))
        return SKIP_LINE;
      break;

    case 'c':
      return DispChatAvail(d);

    case 'C': /* msg_checkmail */
      {
        byte was_no_output=no_local_output;
        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;
        Msg_Checkmail(NULL);
        no_local_output=was_no_output;
      }
      break;
      
    case 'd': /* ibmchars */
      if ((usr.bits2 & BITS2_IBMCHARS)==0)
        return SKIP_LINE;
      break;

    case 'D':
      /* Unlink (delete) a specific file */
      DispGetToBlank(d, d->scratch);

      Convert_Star_To_Task(d->scratch);
      Parse_Outside_Cmd(d->scratch,d->temp);

      if (fexist(d->temp))
        unlink(d->temp);
      break;

    case 'e':
      DispGetToBlank(d, d->scratch);
      
       if (! eqstri(d->scratch,last_readln))
        return SKIP_LINE;
      break;
      
    case 'E':     /* Sysop wants out NOW! */
      *d->onexit='\0';
      d->ret=DRET_EXIT;
      return SKIP_FILE;

    case 'f':
      DispGetToBlank(d, d->scratch);

      FileDate(d->scratch, &st);

      if (GEdate(&st, &usr.ludate))
        return SKIP_NONE;

      return SKIP_LINE;

    case 'F':     /* Check for new files */
      strcpy(linebuf, "*");
      {
        byte was_no_output=no_local_output;
        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;
        File_Locate();
        no_local_output=was_no_output;
      }
      break;

    case 'g':     /* Fix remote terminal size (temporarily) */
      ch=DispSlowGetChar(d);
      SetTermSize(ch,DispSlowGetChar(d));
      break;

    case 'G':     /* Abort line if keypress */
      if (Mdm_keyp())
        return SKIP_LINE;
      break;

    case 'h':     /* If user has hotkeys */
      if ((usr.bits & BITS_HOTKEYS)==0)
        return SKIP_LINE;
      break;

    case 'H':
      return DispCheckHelp(d);
      
    case 'i': /* ifexist */
      DispGetToBlank(d, d->scratch);

      Convert_Star_To_Task(d->scratch);
      Parse_Outside_Cmd(d->scratch,d->temp);

      if (! fexist(d->temp))
        return SKIP_LINE;
      break;
      
    case 'I':
      if (((ch=DispSlowGetChar(d))=='L' && !local) ||
          (ch=='R' && local))
      {
        return SKIP_LINE;
      }
      break;
      
    case 'j': /* multinode stuff */
      if ((ch=DispSlowGetChar(d))=='D')
        DispDoNodeDisp(d);
      else if (ch=='O')
        return (DispNodeOnline(d));
      else if (ch=='A')
        return (DispNodeAvailable(d));
      else if (ch=='N')
        Printf("%d",task_num);
      break;

    /* Download an explicit filename */

    case 'J':
      DispGetToBlank(d, d->temp);

#ifndef ORACLE
      {
        word flags=FFLAG_OK | FFLAG_THIS1 | FFLAG_TAG;
        char *p;

        if (strchr(d->temp, '%'))
          Parse_Outside_Cmd(d->temp, p=d->scratch);
        else p=d->temp;

        while (*p=='!' || *p=='@')
        {
          if (*p=='!')
            flags |= FFLAG_NOBYTES;
          else flags |= FFLAG_NOTIME;

          p++;
        }

        if (fexist(p))
        {
          upper_fn(p);
          AddFileEntry(p, flags, -1L);
        }
      }
#endif
      break;

    case 'k': /* key setting logic */
      return (DispHandleKeys(d));

    case 'K':
      {
        int ret;
        byte was_no_output=no_local_output;
        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;
        ret=DispComment();
        no_local_output=was_no_output;
        return ret;
      }

    case 'l':
#ifndef ORACLE
      {
        byte was_no_output=no_local_output;
        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;

        if ((ch=DispSlowGetChar(d))=='M')
          ListMsgAreas(NULL, FALSE, FALSE);
        else if (ch=='F')
          ListFileAreas(NULL, FALSE);

        no_local_output=was_no_output;
      }
#endif
      break;

    case 'L':     /* Link to another file */
      return DispLink(d);

    case 'm':
      if (! inmagnet)
        return SKIP_LINE;
      break;

    case 'M':     /* Change the menupath to something else */
      DispGetToBlank(d, d->scratch);
      strcpy(menupath, d->scratch);
      break;

    case 'n':     /* Run a MEX program */
    {
      byte was_no_output=no_local_output;
#ifndef ORACLE
      char *pszArgs;
#endif

      if (d->type & DISPLAY_NOLOCAL)
        no_local_output=FALSE;

      /* Parse "%" translation characters */

      DispGetString(d, d->scratch, sizeof d->scratch);

#ifndef ORACLE
      Parse_Outside_Cmd(d->scratch, d->temp);

      /* Now get the arguments out of the string */

      if ((pszArgs=firstchar(d->temp, " ", 2))==NULL)
        pszArgs="";
      else
      {
        pszArgs[-1]=0;
      }

      MexStartupIntrin(d->temp, pszArgs, 0 /*VMF_DEBEXE | VMF_DEBHEAP*/);
#endif
      no_local_output=was_no_output;
      break;
    }

    case 'O':
      *linebuf='\0';
      break;

    case 'p':
      switch(ch=DispSlowGetChar(d))
      {
        case 'D':
      #ifndef ORACLE
          Priv_Down();
      #endif
          break;

        case 'U':
      #ifndef ORACLE
          Priv_Up();
      #endif
          break;

        case 'a':
          Puts(ClassAbbrev(cls));
          break;

        case 'd':
          Puts(ClassDesc(cls));
          break;

        case 'k':
          Putc((int)ClassGetInfo(cls,CIT_KEY));
          break;

        case 'l':
          Printf("%u",usr.priv);
          break;
      }
      break;

    case 'P':
      if (strchr(DispGetString(d, d->scratch, MAXLEN), '%'))
        Parse_Outside_Cmd(d->scratch, linebuf+strlen(linebuf));
      else strcat(linebuf, d->scratch);
      break;

    case 'q':
      if ((usr.delflag & UFLAG_PERM)==0)
        return SKIP_LINE;
      break;
      
    case 'Q':   /* Display only if the user hasn't been on today */
      if (usr.time)
        return SKIP_LINE;
      break;

    case 'r':
      /* Get the menu option number from the .bbs file */
      
      ch=DispSlowGetChar(d) << 8;
      ch |= DispSlowGetChar(d);
      
      DispGetString(d, d->scratch, PATHLEN);
      {
        byte was_no_output=no_local_output;
        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;

        #ifndef ORACLE
        BbsRunOpt(ch, d->scratch);
        #endif

        no_local_output=was_no_output;
      }
      break;

    case 'R':
      DispGetToBlank(d, d->scratch);

      if (! stristr(usr.city, d->scratch))
        return SKIP_LINE;
      break;

    case 's':   /* Set priv level */
      return DispSetPriv(d);

    case 'S':
      if (*linebuf)
        return SKIP_LINE;
      break;

    case 't': /* Parse time code */
      return ParseTime(d);

    case 'T': /* toggle one of the old-style keys from 1-8 */
      ch=DispSlowGetChar(d)-'1';

      if (ch >= 0 && ch <= 7)
        if (UserHasKey(ch))
          UserKeyOff(ch);
        else UserKeyOn(ch);
      break;
      
    case 'U':     /* Change the rippath to something else */
      DispGetToBlank(d, d->scratch);
      strcpy(rippath, d->scratch);
      break;

    case 'u':
      vbuf_flush();
      DispGetToBlank(d, d->scratch);
      play_tune(PRM(tune_file), d->scratch, wait_for_it,
                multitasker==MULTITASKER_desqview);
      break;

    case 'V':   /* ripsend - upload an icon/rip file without display */
      DispGetString(d, d->scratch, MAX_FBBS_ENTRY);
#ifndef ORACLE
      if (hasRIP())
        RIP_SendFile(d->scratch,FALSE);
#endif
      break;

    case 'v':   /* ripdisplay - upload an icon/rip file with display */
      DispGetString(d, d->scratch, MAX_FBBS_ENTRY);
#ifndef ORACLE
      if (hasRIP())
         RIP_SendFile(d->scratch,TRUE);
#endif
      break;

    case 'w':
      {
        byte was_no_output=no_local_output;
        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;

        Who_Is_On();

        no_local_output=was_no_output;
      }
      break;

    case 'W':     /* Write a line (with translations) to the questfile */
      Parse_Outside_Cmd(DispGetString(d, d->scratch, PATHLEN), d->temp);

      if (d->questfile)
        fprintf(d->questfile, "%s\n", d->temp);
      break;

    case 'x':
      return DispCheckExpire(d);
     
    case 'X':
      return DispHandleXtern(d);
      
    case 'y':
      switch (DispSlowGetChar(d))
      {
        case 'D':
          if (usr.xp_flag & XFLAG_EXPDATE)
            FileDateFormat(&usr.xp_date, d->scratch);
          else strcpy(d->scratch, gen_none);
          
          Puts(d->scratch);
          break;
          
        case 'T':
          if (usr.xp_flag & XFLAG_EXPMINS)
            sprintf(d->scratch, xp_minutes, usr.xp_mins);
          else strcpy(d->scratch, gen_none);
          
          Puts(d->scratch);
          break;
      }
      break;

    case 'Y':   /* riphasfile - See if remote has a file */
      DispGetToBlank(d, d->scratch);
#ifndef ORACLE
      if (hasRIP())
      {
        long filesize=-1L;
        char *p=strchr(d->scratch,',');   /* Look for filesize */
        if (p!=NULL)
        {
          *p++ = '\0';
          filesize=atol(p);
        }
        if (RIP_HasFile(d->scratch,&filesize)==TRUE)
          return SKIP_NONE;
      }
#endif
      return SKIP_LINE;

    case 'Z':   /* Save tag list */
      DispGetToBlank(d, d->scratch);
#ifndef ORACLE
      save_tag_list(*d->scratch?d->scratch:NULL);
#endif
      break;
      
    case 'z':   /* Restore tag list */
      DispGetToBlank(d, d->scratch);
#ifndef ORACLE
      restore_tag_list(*d->scratch?d->scratch:NULL,FALSE);
#endif
      break;
  }

  return SKIP_NONE;
}

static int near MsgAreaControl(DSTK *d)
{
  int ch=DispSlowGetChar(d);
  char *p;

  switch (ch)
  {
    case '#':
#ifdef ORACLE
      Puts("1");
#else
      Printf(pl, MsgNumMsg(sq));
#endif
      break;

    case 'd':
      if ((p = strrchr(usr.msg, '.')) != NULL)
      {
        Printf(p2s, ++p);
        break;
      } /* Fallthru */

    case 'A':
      Printf(p2s, usr.msg);
      break;

    case 'D':
      if ((p = strrchr(usr.msg, '.')) != NULL)
        Printf("%0.*s", p-usr.msg, usr.msg);
      break;

    case 'E':
      if (usr.video!=GRAPH_TTY && usr.width >=79 && usr.len >= 24 && !(usr.bits2 & BITS2_BORED))
        return SKIP_NONE;
      return SKIP_LINE;

    case 'R':
      if (usr.video!=GRAPH_TTY && (usr.bits & BITS_FSR))
        return SKIP_NONE;
      return SKIP_LINE;

    case 'L':
      Printf(pd, last_msg);
      break;

    case 'H':
#ifdef ORACLE
      Puts("1");
#else
      Printf(pd, MsgHighMsg(sq));
#endif
      break;

    case 'N':
      Puts(MAS(mah, descript));
      break;

    case 'B':
      {
        /* Find the offset of the character within the msgattr_keys string */

        char *p=strchr(msgattr_keys, DispSlowGetChar(d));
        int i = (!p) ? -1 : (p-msgattr_keys);

        /* Convert the MSGKEY_FILE key into a "local attach" key
         * for non-netmail areas.
         */

        if (i==MSGKEY_FILE && (mah.ma.attribs & MA_ATTACH)!=0 && (mah.ma.attribs & MA_NET)==0)
          i=MSGKEY_LATTACH;

        return AllowAttribute(&mah, i) ? SKIP_NONE : SKIP_LINE;
      }

    case 'n':
#ifndef ORACLE
      if (((ch=DispSlowGetChar(d))=='N' && last_msg >= MsgHighMsg(sq)) ||
           (ch=='R' && last_msg==0L) ||
           (ch=='M' && MsgNumMsg(sq)==0L))
        return SKIP_NONE;
      else
#endif
        return SKIP_LINE;

    case 'i':
      if (((ch=DispSlowGetChar(d))=='N' &&
           ch=='N' && direction==DIRECTION_NEXT) ||
          (ch=='P' && direction==DIRECTION_PREVIOUS))
        return SKIP_NONE;
      else return SKIP_LINE;

    case 'a':
      if ((ch=DispSlowGetChar(d))=='L')
      {
        if ((mah.ma.attribs & (MA_NET | MA_SHARED))==0)
          return SKIP_NONE;
      }
      else if (ch=='E')
      {
        if (mah.ma.attribs & MA_ECHO)
          return SKIP_NONE;
      }
      else if (ch=='M')
      {
        if (mah.ma.attribs & MA_NET)
          return SKIP_NONE;
      }
      else if (ch=='C')
      {
        if (mah.ma.attribs & MA_CONF)
          return SKIP_NONE;
      }
      return SKIP_LINE;

    case 'F':
#ifndef ORACLE
      if (!Msg_AttachedFiles())
#endif
      return SKIP_LINE;
  }
  
  return SKIP_NONE;
}


static int near ParseTime(DSTK *d)
{
  time_t longtime;
  struct tm *lt;

  int operation;
  int hh,h, mm,m;
  int cond=FALSE;

  operation=DispSlowGetChar(d);
  hh=DispSlowGetChar(d)-1;
  mm=DispSlowGetChar(d)-1;

  longtime=time(NULL);
  lt=localtime(&longtime);

  h=lt->tm_hour;
  m=lt->tm_min;

  switch (operation)
  {
    case 1: /* GT */
      if ((h==hh && m > mm) || h > hh)
        cond=TRUE;
      else cond=FALSE;
      break;

    case 2: /* LT */
      if ((h==hh && m < mm) || h < hh)
        cond=TRUE;
      else cond=FALSE;
      break;

    case 3: /* EQ */
      if (h==hh && m==mm)
        cond=TRUE;
      else cond=FALSE;
      break;

    case 4: /* NE */
      if (h==hh && m==mm)
        cond=FALSE;
      else cond=TRUE;
      break;

    case 5: /* GE */
      if ((h==hh && m >= mm) || h > hh)
        cond=TRUE;
      else cond=FALSE;
      break;

    case 6: /* LE */
      if ((h==hh && m <= mm) || h < hh)
        cond=TRUE;
      else cond=FALSE;
      break;
  }

  return cond ? SKIP_NONE : SKIP_LINE;
}



static word near DispCheckExpire(DSTK *d)
{
  union stamp_combo sc;
  word days;
  time_t now;
  struct tm *tms;
  
  DispGetToBlank(d, d->scratch);
  days=atoi(d->scratch);

  if (usr.xp_flag & XFLAG_EXPDATE)
  {
    now=time(NULL);

    /* 86400 seconds in a day */

    now += (days*86400L);

    tms=localtime(&now);
    TmDate_to_DosDate(tms, &sc);

    return (usr.xp_date.ldate && GEdate(&sc, &usr.xp_date)
              ? SKIP_NONE : SKIP_LINE);
  }
  else
  {
    /* Check the caller's "minutes remaining" */

    return ((sword)usr.xp_mins-timeonline() <= (sword)days 
               ? SKIP_NONE : SKIP_LINE);
  }
}



      
static word near DispHandleKeys(DSTK *d)
{
  int type=DispSlowGetChar(d);
  sword keyn;

  char *p;

  /* Get the list of keys to set/test */

  DispGetToBlank(d, d->scratch);
  cstrupr(d->scratch);

  for (p=d->scratch; *p; p++)
  {
    if (*p >= '1' && *p <= '8')
      keyn=*p-'1';
    else if (*p >= 'A' && *p <= 'X')
      keyn=*p-'A'+8;
    else continue;

    switch (type)
    {
      case 'I': /* display only if key is set */
        if ((usr.xkeys & (1L << keyn))==0)
          return SKIP_LINE;
        break;

      case 'N': /* display only if key is not set */
        if (usr.xkeys & (1L << keyn))
          return SKIP_LINE;
        break;

      case 'O': /* turn key on */
        usr.xkeys |= 1L << keyn;
        break;

      case 'F': /* turn key off */
        usr.xkeys &= ~(1L << keyn);
        break;
    }
  }
  
  return SKIP_NONE;
}


static word near DispHandleXtern(DSTK *d)
{
  int ch;
  byte was_no_output=no_local_output;
  sword rr;
  char *p;

  if (d->type & DISPLAY_NOLOCAL)
    no_local_output=was_no_output;

  if (d->questfile)
    flush_handle(d->questfile);


  if (d->ck_abort)
    Mdm_flush_ck();
  else Mdm_flush();

  if ((ch=DispSlowGetChar(d))=='E')
    ch=OUTSIDE_ERRORLEVEL;
  else if (ch=='R')
    ch=OUTSIDE_RUN;
  else if (ch=='C')
    ch=OUTSIDE_CHAIN;
  else ch=OUTSIDE_DOS;

  DispGetString(d, d->scratch, MAX_FBBS_ENTRY);

  rst_offset=DispGetPos();
  p=d->scratch;
  rr=0;

  if (*p=='@')
  {
    rr |= OUTSIDE_REREAD;
    ++p;
  }

  Outside(NULL, NULL, ch | rr, p, FALSE, CTL_NONE, RESTART_DOTBBS,
          d->lastfile);

  brk_trapped=0;
  rst_offset=-1L;

  no_local_output=was_no_output;
  return SKIP_NONE;
}


static word near DispLink(DSTK *d)
{
  long tlong;
  int ch;
  
  if (d->questfile)
    flush_handle(d->questfile);

  DispGetToBlank(d, d->scratch);

  /* Here, we turn off the buffering, and free the buffer itself.
     Since we want to get as much mileage as we can out of L)inked
     files, we're freeing the buffer to make room for the next
     one.  If we didn't do this, we'd run out of memory really
     quickly, since each malloc()'d buffer would be stacked on
     top of previous buffers, and they aren't doing any good since
     we're not accessing each file at the same time!                 */

  tlong=DispGetPos();

  free(d->filebufr);

  /* Display the specified file */

  ch=Display_File(d->type & DISPLAY_MENUHELP, NULL, percent_s, d->scratch);

  /* Now, let us reallocate the buffer */

  if ((d->filebufr=(char *)malloc(FILEBUFSIZ))==NULL)
  {
    d->ret=DRET_NOMEM;
    return SKIP_FILE;
  }

  lseek(d->bbsfile, tlong, SEEK_SET);
  d->bufp=d->highp=d->filebufr;

  /* If sysop used an [exit] */

  if (ch==DRET_EXIT || ch==DRET_NOMEM)
  {
    *d->onexit='\0';
    d->ret=DRET_EXIT;
    return SKIP_FILE;
  }
  
  return SKIP_NONE;
}


static word near DispCheckBaud(DSTK *d)
{
  sword ch;

  if (local)
  {
    (void)DispSlowGetChar(d);
    return SKIP_NONE;
  }

  ch=DispSlowGetChar(d);

  if ((ch=='1' && baud < 1200L) ||
      (ch=='2' && baud < 2400L) ||
      (ch=='9' && baud < 9600L))
  {
    return SKIP_LINE;
  }

  return SKIP_NONE;
}


static word near DispDoAPB(DSTK *d)
{
  if (*DispGetString(d, d->scratch, sizeof(d->scratch)-1))
  {
    char *buf=malloc(sizeof(d->scratch));

    Parse_Outside_Cmd(d->scratch, buf);

    #ifndef ORACLE
    ChatAPB(buf);
    #endif

    free(buf);
  }

  return SKIP_NONE;
}

static word near DispNodeOnline(DSTK *d)
{
  word tid;

  DispGetToBlank(d, d->scratch);
  Parse_Outside_Cmd(d->scratch, d->temp);
  tid=(word)atoi(d->temp);
  
  return (ChatFindIndividual((byte)tid, NULL, NULL, NULL) ? SKIP_NONE : SKIP_LINE);
}

static word near DispNodeAvailable(DSTK *d)
{
  byte tid;
  word avail;
  
  DispGetToBlank(d, d->scratch);
  Parse_Outside_Cmd(d->scratch, d->temp);

  tid=(byte)atoi(d->temp);
  
  return ((word)(ChatFindIndividual(tid, NULL, NULL, &avail) && avail) ?
          SKIP_NONE : SKIP_LINE);
}


/* Display a file on another node */

static void near DispDoNodeDisp(DSTK *d)
{
  byte tid;
  char *p;

  DispGetString(d, d->scratch, MAXLEN);
  Parse_Outside_Cmd(d->scratch, d->temp);
  getword(d->temp, d->scratch, ctl_delim, 1);

  /* Find the node number of the other guy */

  tid=(byte)atoi(d->scratch);

  /* Figure out which file to display */

  if ((p=firstchar(d->temp, ctl_delim, 2))==NULL)
    return;

  /* Now tell the other node to display it */

  ChatSendMsg(tid, CMSG_DISPLAY, strlen(p)+1, p);
}



static word near DispComment(void)
{
  return (!Goodbye_Comment() ? SKIP_LINE : SKIP_NONE);
}


static word near DispSetPriv(DSTK *d)
{
  int pidx;
  sword ch;

  ch=DispSlowGetChar(d);
  pidx=ClassKeyIndex(ch);
  if (pidx==-1)
    logit(log_invalid_acs_key,ch);
  else
  {
    usr.priv=ClassKeyLevel(ch);
    Find_Class_Number();
  }

  return SKIP_NONE;
}



static word near DispChatAvail(DSTK *d)
{
  word avail;
  sword ch;

  if (! ChatFindIndividual(task_num, NULL, NULL, &avail))
  {
    (void)DispSlowGetChar(d); /* Else throw away option */
    return SKIP_NONE;
  }

  if (((ch=DispSlowGetChar(d))=='A' && !avail) || (ch=='N' && avail))
    return SKIP_LINE;

  return SKIP_NONE;
}


      
static word near DispCheckHelp(DSTK *d)
{
  sword hlp, ch;

  hlp=(d->type & DISPLAY_MENUHELP) ? menuhelp : usr.help;

  if (((ch=DispSlowGetChar(d))=='N' && hlp != NOVICE) ||
      (ch=='R' && hlp != REGULAR) ||
      (ch=='E' && hlp != EXPERT) ||
      (ch=='H'))
  {
    return SKIP_LINE;
  }
  
  return SKIP_NONE;
}


/* Needs to stay in root because it's called by tune() */

int _stdc wait_for_it(void)
{
  static long lasttime=-1L;
  long curtime;
  
  curtime=time(NULL);
  
  if (curtime != lasttime)
  {
    Mdm_check();
    lasttime=curtime;
  }

  return 0;
}


