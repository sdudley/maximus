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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: max_misc.c,v 1.11 2004/06/06 21:48:51 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Miscellaneous routines
*/

#define MAX_LANG_max_init
#define MAX_LANG_max_log
#define MAX_LANG_max_main
#define MAX_LANG_f_area
#define MAX_INCL_COMMS

#include <stdio.h>
#include <time.h>
#include <dos.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <mem.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <share.h>
#ifdef UNIX
# include <errno.h>
#endif

#ifdef OS_2
#define INCL_DOS
#include "pos2.h"
#endif

#include "prog.h"
#include "alc.h"
#include "max_msg.h"
#include "max_file.h"
#include "display.h"
#include "max_menu.h"


  /* These values override the default values in the usr record
   * for the screen length and width. This is useful for RIP
   * where the text window size is known at both ends (usually
   * set by the host
   */

static int win_cols =0;
static int win_rows =0;
static int rip_font =0;
int loc_cols =0;
int loc_rows =0;
static int rip_x=0;
static int rip_y=0;

int RipOriginX()
{
  return rip_x;
}

int RipOriginY()
{
  return rip_y;
}

#ifndef ORACLE
void GetLocalSize()
{
  loc_rows = WinGetNumRows(win);
  loc_cols = WinGetNumCols(win);
}
#endif

  /* Return the screen width & length as seen by the user
   * This is 'local smart' so for local logins, the
   * current screen dimensions override the usr record
   * in local mode
   */

int TermWidth(void)
{
  if (local)
  {
    if (loc_cols==0)
      GetLocalSize();
    return loc_cols;
  }
  if (win_cols)
    return win_cols;
  if (hasRIP())
  {
    switch (rip_font)
    {
    case 0: return 80;
    case 1: return 90;
    case 2: return 80;
    case 3: return 90;
    case 4: return 40;
    }
  }
  return usr.width;
}


int TermLength(void)
{
  if (local)
  {
    if (loc_rows==0)
      GetLocalSize();
    return loc_rows;
  }
  if (win_rows)
    return win_rows;
  if (hasRIP())
  {
    switch (rip_font)
    {
    case 0: return 42; /* Return length-1 because some users may have */
    case 1: return 42; /* status bar enabled (and there is no easy way */
    case 2: return 24; /* to tell this!). */
    case 3: return 24;
    case 4: return 24;
    }
  }
  return usr.len;
}

  /* Sets (or resets) the dimensions of the current user's
   * display window. This is invoked via the mecca [textsize]
   * token
   */

void SetTermSize(int icols, int irows)
{
  win_cols = (icols < 20 || icols > 132) ? 0 : icols;
  win_rows = (irows < 1  || irows > 200) ? 0 : irows;
}

void SetRipFont(int ifontno)
{
  rip_font=ifontno;
}

void SetRipOrigin(int ix, int iy)
{
  rip_x=ix;
  rip_y=iy;
}


/*
 * Pauses for <duration> hudredths of seconds, while
 * giving away timeslice.
 */

void Delay(unsigned int duration)
{
  long tim;

  vbuf_flush();

  tim=timerset(duration);

  while (! timeup(tim))
    Giveaway_Slice();
}


void Giveaway_Slice(void)
{
  #ifndef ORACLE
    static long cd_loss=-1L;
    static int pvar=-1;
    static long last_check=-1L;

    /* A lot of this stuff only needs to be checked once a second */

    if (last_check != time(NULL))
    {
      last_check=time(NULL);

      if (dspwin && timeup(dspwin_time))
      {
        WinClose(dspwin);

        if (in_wfc)
          VidHideCursor();

        dspwin=NULL;
      }

      if (!local)
        Draw_StatusLine(STATUS_NORMAL);

      if (!in_wfc && !finished && !local)
      {
        if (pvar==-1)
        {
          if (port != -1)
            pvar=port;
        }
        else if (port != pvar)
        {
          logit(log_pvchg);
          LogClose();

          brkuntrap();
          uninstall_24();

          _exit(ERROR_CRITICAL);
        }
      }

      if (!local)
      {
        if (carrier())
          cd_loss=-1L;
        else if (!in_wfc && !finished)
        {
          if (cd_loss==-1L)
            cd_loss=time(NULL);
          else if (cd_loss != -2 && time(NULL) > cd_loss+ROBO_TIME)
          {
            cd_loss=-2;
            mdm_hangup();
          }
        }
      }

      LogFlush();
    }

    #ifdef MCP_VIDEO
      mcp_out_flush();
      mcp_sleep();
    #endif
  #endif /* !ORACLE */

#if defined(OS_2)
  DosSleep(1L);
#elif defined(NT)
  Sleep(1L);
#elif defined(__MSDOS__)
  if (sleeper)
    (*sleeper)();
#elif defined(UNIX)
# ifdef _REENTRANT
  sched_yield();
# else
  sleep(0);
# endif
#else
  #error Unknown OS!
#endif
}



#ifndef ORACLE

  static VWIN *swin=NULL;

  void Draw_StatusLine(int clearit)
  {
    /* These are declared static, so they won't use any stack space. */

    static char *bufr;
    static char ptmp[16];
    static long secs_left;
    static long last=0L;


    if (no_video)
      return;

    /* Only create status line if mode==VIDEO_ibm and if it's enabled */

    if (! (displaymode==VIDEO_IBM && (prm.flags & FLAG_statusline)))
      return;

    /* Don't display status line on WFC screen, either */

    if (in_wfc)
      return;

    if (clearit==STATUS_REMOVE)
    {
      if (swin)
      {
        WinCls(swin,CGRAY);
        WinSync(swin,FALSE);
        WinClose(swin);
        swin=NULL;
      }

      return;
    }


    if (local)
      return;

    if (! swin)
    {
      swin=WinOpen(VidNumRows()-1, 0, 1, VidNumCols(), BORDER_NONE,
                   col.status_bar, CGRAY, WIN_NOCSYNC);

      /* Make sure that the status line is on the "bottom" of the linked list */
      WinToBottom(swin);

      if (!swin)
        return;
    }

    secs_left=((signed long)timeoff-(signed long)time(NULL));

    if (secs_left < 0L)
      secs_left=0L;

    /* Time limit should stay stable while in CHAT mode, and line          *
     * should only be updated once per second.                             */

    if ((clearit != STATUS_FORCE && (inchat || secs_left==last)) ||
        (bufr=malloc(81))==NULL)
      return;

    last=secs_left;

    WinGotoXY(swin, 0, 0, FALSE);
  /* 1234567890123456 1234567890123456 12345678901234 123456789/12345678 1234:12 C K*/
    sprintf(bufr,"%-16.16s %-16.16s %-14.14s %9.9s%s%-8.8s %4ld:%02ld",
            usr.name,
            usr.alias,
            usr.phone,
            privstr(usr.priv, ptmp),
            usr.xkeys ? "/" : " ",
            Keys(usr.xkeys),
            secs_left/60L,secs_left % 60L);

    WinSetAttr(swin, col.status_bar);
    WinPuts(swin, bufr);

    free(bufr);

    WinPutc(swin, ' ');

    if (chatreq)
      WinSetAttr(swin, col.status_cht);

    WinPutc(swin, (byte)(chatreq ? 'C' : ' '));

    WinSetAttr(swin, (byte)(fFlow ? col.status_cht : col.status_bar));

    WinPutc(swin, (byte)(fFlow ? 'F' : ' '));

    WinSetAttr(swin, col.status_key);
    WinPutc(swin, (byte)((!local && keyboard) ? 'K' : ' '));

    WinCleol(swin, WinGetRow(swin), WinGetCol(swin), col.status_bar);
    WinSync(swin, FALSE);
  }



  /* Strip out any ESC characters from the string s.  Used mainly to strip
     ANSI crap from the To/From/Subj fields, but it is used for a few other
     miscellaneous items.                                                   */

  #ifdef KEY

    char * Strip_Ansi(char *s, char *area, long msgnum)
    {
      extern void (pascal far *pstrip_ansi)(int len, char far *ptr, char far * far *ret, int far *got);
      char *rc=(char *)0x1296;
      int got=TRUE;

      (*pstrip_ansi)(strlen(s), s, &rc, &got);

      if (got && area && msgnum != 0)
        logit(log_got_ansi, area, msgnum);

      return rc;
    }

  #else

    char * Strip_Ansi(char *s, char *area, long msgnum)
    {
      char *orig_s;

      orig_s=s;

      while (*s)
      {
        if (*s=='\x1b')
        {
          if (area && msgnum != 0)
            logit(log_got_ansi, area, msgnum);

          *s='<';
        }

        s++;
      }

      return orig_s;
    }
  #endif /* !KEY */


  /* Convert the "Format Date" and "Format Time" strings into something      *
   * that we can pass to strftime()...                                       */

  char * Timestamp_Format(char *format,union stamp_combo *sstamp,char *out)
  {
    struct tm localt;

    char *sfmt;
    char *p;

    if (sstamp->ldate==0L)
    {
      *out='\0';
      return out;
    }

    if ((sfmt=malloc(255))==NULL)
    {
      *out='\0';
      return out;
    }

    strcpy(p=sfmt,format);

    while (*p)
    {
      if (*p=='%')
      {
        switch(*++p)
        {
          case '\0':      /* Guard against pilot booboo */
            *p='\0';
            break;

          case 'A':
            *p='p';
            break;

          case 'B':
            *p='m';
            break;

          case 'C':
            *p='b';
            break;

          case 'D':
            *p='d';
            break;

          case 'E':
            *p='I';
            break;

          case 'H':
            *p='H';
            break;

          case 'M':
            *p='M';
            break;

          case 'S':
            *p='S';
            break;

          case 'Y':
            *p='y';
            break;

          default:
          case '%':
            *p='%';
            break;
        }
      }

      p++;
    }

    DosDate_to_TmDate(sstamp,&localt);

  #ifdef __WATCOMC__
    /* WC produces "01-01-101" for Jan 1st 2001 instead of "01-01-01" */

    if (localt.tm_year > 99)
      localt.tm_year %= 100;
  #endif

    strftime(out, BUFLEN, sfmt, &localt);

    free(sfmt);

    return out;
  }



  /* Determines whether or not a string has any real text */

  int isblstr(char *s)
  {
    while (s && *s)
      if (isalnum(*s++))
        return FALSE;

    return TRUE;
  }



  /* Add 'seconds' more seconds to our user's time limit, as long as we      *
   * don't overrun the command-line '-t' parameter, if any.                  */

  long Add_To_Time(long seconds)
  {
    unsigned long save_toff;
    long added;

    save_toff=min(timeoff, getoff);

    timeoff=min(getoff, timeoff+(unsigned long)seconds);

    /* Return how many seconds were actually added... */

    added=timeoff-save_toff;

    if (usr.xp_flag & XFLAG_EXPMINS)
      usr.xp_mins += added/60;

    usr.time_added += added/60;

    return added;
  }




  char * Help_Level(int help)
  {
    if (help==EXPERT)
      return (exper);
    else if (help==REGULAR)
      return (regul);
    else return (novic);
  }


  /* Returns an ASCIIZ string containing the numbers of the keys that the    *
   * current user has.                                                       */

  char * Keys(long key)
  {
    static char keys[MAX_KEYS+1];
    char *s;
    int x;

    s=keys;

    for (x=0;x < MAX_KEYS;x++)
      if (key & (1L << x))
      {
        if (x <= 7)
          *s++=(char)('1'+x);
        else *s++=(char)('A'+(x-8));
      }

    *s='\0';

    return keys;
  }


  /* FinishUp() *must* be declared cdecl because it is called by atexit()... *
   * It just makes a normal call to the *real* FinishUp() function, which is *
   * kept in another overlay to save space.  Since we can't do indirect      *
   * (ie. via pointers) function calls between overlays, we have to do it    *
   * this way...                                                             */

  void _stdc FinishUp(void)
  {
        FinishUp2(TRUE);
  }


#ifdef __MSDOS__
  /* This module is just a shell for the '-r' command-line option, and       *
   * takes care of all the restarting tasks.                                 */

  void Sys_Rst(void)
  {
    char restart_name[PATHLEN];

    current_line=24;

    if (System_Restart(restart_name)==0)
      Display_Options(restart_name, NULL);
    else
    {
      Display_File(!usr.times ? DISPLAY_PCALL : 0, NULL, restart_name);

      if (! usr.times)    /* If newcall */
        usr.times++;

      Display_Options(main_menu, NULL);
    }

    quit(0);
  }
#endif

  int Highbit_Allowed(void)
  {
    /* If we're doing message stuff, then the "high bit" flag in
     * the message area is completely decisive.  Only allow high
     * bit chars in the area if it is enabled.
     */

    if (in_msghibit || in_mcheck)
      return (mah.ma.attribs & MA_HIBIT) && (usr.bits2 & BITS2_IBMCHARS);


    /* If global high bit is turned on, default is to allow high-bit chars */

    if (prm.flags2 & FLAG2_GLOBALHB)
      return TRUE;

    /* If s/he does, then only allow if in right message area type, or if in *
     * the multi-line chat.                                                  */

    if (in_node_chat && (usr.bits2 & BITS2_IBMCHARS))
      return TRUE;

    return FALSE;
  }


  char * fancier_str(char *str)
  {
    byte *s;
    byte cset[80];
    word lower=FALSE;

    s=str;

    strcpy(cset, "abcdefghijklmnopqrstuvwxyz");

    if (prm.charset==CHARSET_SWEDISH)
      strcat(cset, "}{|");

    /* If the name is okay, and properly punctuated */

    if (! ((isupper(*s) || CharsetSwedish(str, s)) &&
        s && strpbrk(s, cset)) && s)
    {
      for (; *s; s++)
      {
        if (ischin(s))
          s++;
        else
        {
          if (lower)
          {
            if (prm.charset==CHARSET_SWEDISH)
              switch (*s)
              {
                case '[':   *s='{'; break;
                case ']':   *s='}'; break;
                case '\\':  *s='|'; break;
                default:    *s=(byte)tolower(*s);
              }
            else *s=(byte)tolower(*s);
          }
          else
          {
            if (prm.charset==CHARSET_SWEDISH)
            {
              switch(*s)
              {
                case '{':   *s='['; break;
                case '}':   *s=']'; break;
                case '|':   *s='\\';  break;
                default:    *s=(byte)toupper(*s);
              }
            }
            else *s=(byte)toupper(*s);
          }

          if (prm.charset==CHARSET_SWEDISH)
            lower=(isalnum(*s) || *s=='{' || *s=='[' || *s==']' || *s=='}'
                   || *s=='\\' || *s=='|');
          else lower=(isalnum(*s));
        }
      }
    }

    return str;
  }


  char * Strip_Trailing_Blanks(char *s)
  {
    char *p;

    p=s+strlen(s);

    while (*(--p)==' ' && p >= s)
      *p='\0';

    return s;
  }

  void AlwaysWhiteN(void)
  {
    Puts(WHITE);

    if (! *linebuf)
      Putc('\n');
  }

  /* Return a pointer past the last file specification in the path */

  char *No_Path(char *orig)
  {
    char *s;

    if ((s=strrstr(orig, pdel_only)) != NULL)
      return (s+1);
    else
      return orig;
  }

  /* Length of a string, takinga avatar colour bytes into account */

  int stravtlen(char *s)
  {
    int count=0;

    while (*s)
      if (*s++=='\x16')
        s += 2;
      else count++;

    return count;
  }




  char *Graphics_Mode(byte video)
  {
    switch (video)
    {
      case GRAPH_ANSI:      return s_ansi;
      case GRAPH_AVATAR:    return s_avatar;
      default:              return s_tty;
    }
  }


  void SetUserName(struct _usr *user, char *username)
  {
    strcpy(username,
           (*user->alias && (prm.flags & FLAG_alias) != 0)
              ? user->alias
              : user->name);

    getword(username, firstname, ctl_delim, 1);
  }


  /* Return the 'n'th archiver, as pointed to by usr.compress */

  struct _arcinfo * UserAri(byte num)
  {
    struct _arcinfo *ar;

    for (ar=ari; ar && --num > 0; ar=ar->next)
      ;

    return ar;
  }

  word CharsetChinese(byte *str, byte *ch)
  {
    NW(str);
    NW(ch);

    /* not implemented */

    return (FALSE);
  }


  char * fbgetsc(char *s,int n,FILE *fp,int *trimmed)
  {
    char *o=NULL;

    int ch=EOF;
    *trimmed=FALSE;

    if (n)
    {
      for (o=s;--n && (ch=fgetc(fp)) != EOF && ch != '\n';)
      {
        if (ch=='\r')
        {
          *trimmed=TRUE;
          if ((ch=fgetc(fp)) != '\n')
            ungetc(ch,fp);

          break;
        }
        else *s++=(char)ch;
      }
    }

    *s='\0';

    return (ch==EOF ? NULL : o);
  }

  char * fbgets(char *s,int n,FILE *fp)
  {
    int trimmed;
    return fbgetsc(s,n,fp,&trimmed);
  }

  /* Returns TRUE if the wildcard 'wc' matches the area name 'name' */

  int BrowseWCMatch(char *wc, char *name)
  {
    char *pwc, *pn;

    for (pwc=wc, pn=name; *pwc; pwc++)
    {
      if (tolower(*pwc)==tolower(*pn))
        pn++;
      else if (*pwc=='*')
      {
        if (*pn != pwc[1])
        {
          while (*pn && *++pn != pwc[1])
            ;
        }
      }
      else
        break;
    }

    /* Return TRUE if we made it to the end of the wildcard string */

    return *pwc==0;
  }

#endif /* !ORACLE */

char * Strip_Underscore(char *s)
{
  char *orig;

  orig=s;

  while (*s)
  {
    if (*s=='_')
      *s++=' ';
    else s++;
  }

  return orig;
}



void Blank_User(struct _usr *user)
{
  memset(user,'\0',sizeof(struct _usr));

  user->help=NOVICE;

  user->video=GRAPH_TTY;

  user->bits=BITS_TABS | BITS_FSR;
  user->bits2=BITS2_IBMCHARS | BITS2_BORED |
              BITS2_CLS | BITS2_MORE;

  if (prm.logon_priv==PREREGISTERED)
    user->priv=0;
  else
    user->priv=prm.logon_priv;

  user->width=80;
  user->len=24;
  
  if (offsets)
  {
    char temp[PATHLEN];

    Parse_Outside_Cmd(PRM(begin_msgarea), temp);
    SetAreaName(user->msg, temp);

    Parse_Outside_Cmd(PRM(begin_filearea), temp);
    SetAreaName(user->files, temp);
  }
    
  user->struct_len=sizeof(struct _usr)/20;
  user->def_proto=PROTOCOL_NONE;
}





/* Find the user's priv, and set class number appropriately */

void Find_Class_Number(void)
{

  cls=ClassLevelIndex(usr.priv);
  if (cls==-1)
  {
    if (prm.logon_priv != PREREGISTERED || fLoggedOn)
    {
      /* We couldn't find a record for this priv level in the */
      /* control file, his/her user record must be garbled!   */

      logit(no_class_rec, usr.name);
      mdm_hangup();
    }
    cls=0;
  }

  if (acsflag(CFLAGA_HANGUP))
  {
    logit(turf_hidden);
    mdm_hangup();
  }

  if (!local && (baud < (dword)ClassGetInfo(cls,CIT_MIN_BAUD)))
  {
    logit(ltooslow,baud,ClassGetInfo(cls,CIT_MIN_BAUD));

    Display_File(0,NULL,PRM(tooslow));
    mdm_hangup();
  }

}


int Convert_Star_To_Task(char *filename)
{
  char temp[10], *p;

  if ((p=strchr(filename,'*')) != NULL)
  {
    sprintf(temp, "%02x", task_num);

    strocpy(p+1, p);
    p[0]=temp[0];
    p[1]=temp[1];

    return TRUE;
  }

  return FALSE;
}




int is_wd(char ch)
{
  if (ch==' ' || ch==',' || ch==';' || ch=='-')
    return TRUE;

  return FALSE;
}


void WhiteN(void)
{
  Puts(WHITE);

  NoWhiteN();
}



void NoWhiteN(void)
{
  if (! *linebuf)
    Putc('\n');
}


int Trim_Line(char *s)
{
  char *p;
  int trimmed=FALSE;
  
  p=s+strlen(s)-1;
  
  while (p >= s && (*p=='\r' || *p=='\n'))
  {
    *p--='\0';
    trimmed=TRUE;
  }
  
  return trimmed;
}




char *SetAreaName(char *usr, char *newarea)
{
  /* Pad the field with zeroes */

  memset(usr, 0, MAX_ALEN);
  strncpy(usr, newarea, MAX_ALEN-1);
  usr[MAX_ALEN-1]='\0';

  return (strupr(usr));
}



char * FileDateFormat(union stamp_combo *stamp,char *temp)
{
  int mo, da, yr;
  
  mo=stamp->msg_st.date.mo;
  da=stamp->msg_st.date.da;
  yr=(stamp->msg_st.date.yr+80) % 100;

  *temp='\0';
  
  switch (prm.date_style)
  {
    case 0:
      sprintf(temp, date_str, mo, da, yr);
      break;

    case 1:
      sprintf(temp, date_str, da, mo, yr);
      break;

    case 2:
      sprintf(temp, date_str, yr, mo, da);
      break;

    case 3:
      sprintf(temp, datestr, yr, mo, da);
      break;
  }
  
  return temp;
}



word halt(void)
{
  if (brk_trapped || mdm_halt())
  {
    brk_trapped=0;
    mdm_dump(DUMP_ALL);
    ResetAttr();

    /* Drain the keyboard buffer */

    while (loc_kbhit())
      loc_getch();

    *linebuf='\0';

    return TRUE;
  }
  
  return FALSE;
}


/* Returns TRUE if the char is queston is a valid Swedish 7bit name char */

word CharsetSwedish(byte *str, byte *ch)
{
  NW(str);
  
  return (prm.charset==CHARSET_SWEDISH &&
          (*ch=='{' || *ch=='}' || *ch=='[' || *ch==']' ||
           *ch=='|' || *ch=='\\'));
}




char * MsgDte(union stamp_combo *st,char *datebuf)
{
#ifdef ORACLE
  NW(st); NW(datebuf);
  strcpy(datebuf, "");
#else
  char temp[BUFLEN+1];
  strcpy(datebuf, Timestamp_Format(PRM(dateformat), st, temp));
  strcat(datebuf, " ");
  strcat(datebuf, Timestamp_Format(PRM(timeformat), st, temp));
#endif

  return datebuf;
}

void Check_Time_Limit(unsigned long *input_timeout, int *timer2)
{
  long left;

  if (finished)
    return;

  if (!local && !carrier())
    Lost_Carrier();

  if (do_timecheck && !in_file_xfer && !in_wfc &&
      !(fLoggedOn && acsflag(CFLAGA_NOTIME)))
  {
    if ((left=timeleft()) <= 0)
      TimeLimit();
    else if (left==2 && !sent_time_almostup)
      TimeAlmostUp();
    else if (left==5 && !sent_time_5left && fLoggedOn)
      Time5Left();
  }

  /* Make sure the user didn't fall asleep... */
  if (input_timeout && timeup(*input_timeout) &&
      (!local || (local && (prm.flags2 & FLAG2_ltimeout))) &&
       !(fLoggedOn && acsflag(CFLAGA_NOTIME)))
  {
    if (*timer2)
    {
      logit(inputtimeout);
      Puts("\n\n\n");
      mdm_dump(DUMP_OUTPUT); 
      mdm_hangup();
    }
    else
    {
      PleaseRespond();    /* Give 'em a warning before we hang up */
      *input_timeout=timerset(RETIMEOUT*100);
      *timer2=TRUE;
    }
  }
}

/* Returns TRUE if the given area is tagged */

#if 0
word AreaTagged(char *name)
{
  byte temp[PATHLEN];
  byte *p;
  byte ch;
  
  temp[0]='\x01';
  strcpy(temp+1, name);
  
  for (p=tma.areas; (p=strstr(p, temp)) != NULL; p++)
    if ((ch=p[strlen(temp)]) < ' ' || ch=='\x00')
      return TRUE;
  
  return FALSE;
}
#endif



#ifdef ORACLE
  int CanAccessFileCommand(PFAH pfah, option opt, char letter, BARINFO *pbi)
  {
    NW(pfah); NW(opt); NW(letter); NW(pbi);
    return FALSE;
  }
#endif



void cant_open(char *fname)
{
  logit(cantopen, fname, errno);
}


/* Convert an ASCIIZ keys string to a bitmask */

dword SZKeysToMask(char *pszKeys)
{
  dword mask=0L;
  char *p;

  /* Now test to see if the user needs to have certain keys */

  if ((p=strchr(pszKeys, '/')) != NULL)
  {
    /* Scan through each key in turn */

    for (p++; isalpha(*p) || isdigit(*p); p++)
    {
      *p=toupper(*p);

      if (*p >= '1' && *p <= '8')
        mask |= (1L << (*p-'1'));
      else if (*p >= 'A' && *p <= 'X')
        mask |= (1L << ((*p-'A')+8));
    }
  }

  return mask;
}




#ifdef DEBUG_OUT

#include "bfile.h"

BFILE b=NULL;

#define LAST_SENT 0
#define LAST_GOT  1

static int lastb=-1;

static void _stdc DebOutFinish(void)
{
  dout_log=FALSE;
  Bwrite(b, "\r\n\r\n**** LOG FINISHED ****\r\n\r\n", 30);
  Bclose(b);
}

void DebOutStart(void)
{
  char str[80];
  SCOMBO sc;

  if ((b=Bopen("io.deb", BO_CREAT | BO_TRUNC | BO_WRONLY, 0, 4096))==NULL)
  {
    logit("!CAN'T OPEN IO.DEB!");
    return;
  }
  
  Get_Dos_Date(&sc);
  
  sprintf(str, "\r\n\r\n**** LOG BEGIN AT %02d:%02d:%02d %02d %s %02d ****\r\n\r\n",
          sc.msg_st.time.hh,
          sc.msg_st.time.mm,
          sc.msg_st.time.ss,
          sc.msg_st.date.da,
          months_ab[sc.msg_st.date.mo-1],
          (sc.msg_st.date.yr+80) % 100);

  Bwrite(b, str, strlen(str));

  atexit(DebOutFinish);
  dout_log=TRUE;
}

void DebOutGotChar(int c)
{
  static char temp[20];
  
  if (lastb != LAST_GOT)
  {
    Bwrite(b, "\r\n\r\nGot: ", 9);
    lastb=LAST_GOT;
  }
  
  sprintf(temp, "%02x ", c);
  Bwrite(b, temp, 3);
}

void DebOutSentChar(int c)
{
  static char temp[20];
  
  if (lastb != LAST_SENT)
  {
    Bwrite(b, "\r\n\r\nSent: ", 9);
    lastb=LAST_SENT;
  }
  
  sprintf(temp, "%02x ", c);
  Bwrite(b, temp, 3);
}


#endif

sword realpriv(void)
{
#ifndef ORACLE
  if (lam && lam->biOldPriv.use_barpriv)
    return lam->biOldPriv.priv;
  else if (laf && laf->biOldPriv.use_barpriv)
    return laf->biOldPriv.priv;
  else
#endif
    return usr.priv;
}


/* Add commas to a number */

char * commaize(long ulNum, char *szBuf)
{
  char *p;
  char cSep=*thousand_sep;

  sprintf(szBuf, "%ld", ulNum);
  p=szBuf+strlen(szBuf);

  if (cSep)
  {
    while (p >= szBuf+4)
    {
      p -= 3;
      strocpy(p+1, p);
      *p=cSep;
    }
  }

  return szBuf;
}


char * _fast stristr_nochin(char *string,char *search)
{
  word last_found=0;
  word strlen_search=strlen(search);

  if (string)
  {
    while (*string)
    {
      if ((tolower(*string))==(tolower(search[last_found])))
        last_found++;
      else
      {
        if (last_found != 0)
        {
          string -= last_found-1;
          last_found=0;
          continue;
        }
      }

      string++;

      if (last_found==strlen_search)
        return(string-last_found);
    }
  }

  return(NULL);
}


