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
static char rcs_id[]="$Id: max_locl.c,v 1.2 2003/06/04 23:37:33 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Local command functions
*/

#define MAX_LANG_max_log
#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <mem.h>
#include <time.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "keys.h"
#include "mm.h"


#ifndef ORACLE

static void near Fake_Noise(void);
static int near Priv_Adjust_Start(void);
static int near Parse_Local_Normal(int ch);
static int near Parse_Priv_Adjust(int ch);
static void near Local_Stats_Display(void);


VWIN *privwin=NULL;
static VPICK *vp;
PLIST * pl_privs =NULL;

int SetLocalKeystate(int setto)
{
  int wasstate = keyboard;
  keyboard = ( setto || local ) ? 1 : 0;
  return wasstate;
}

int LocalKeystate(void)
{
  return keyboard;
}

/* Parse keystrokes from the local console */

void Parse_Local(int ch)   
{
  static int keystate;

  if (in_wfc)
    return;

  switch(keystate)
  {
    case 0:
      keystate=Parse_Local_Normal(ch);
      break;

    case 1:
      keystate=Parse_Priv_Adjust(ch);
      break;
  }
}
  


static dword newkeys=0L;


static void near ShowWinKeys(void)
{
#ifdef TTYVIDEO
  if (displaymode==VIDEO_IBM)
  {
#endif
    WinCleol(privwin, 7, 0, col.pop_text);

    WinPutstra(privwin, 7, 4, col.pop_high, priv_keys);
    WinPutstra(privwin, 7,10, col.pop_text, Keys(newkeys));
#ifdef TTYVIDEO
  }
  else
  {
    Lputs(priv_keys);
    Lputs(Keys(newkeys));
    Lputc('\n');
  }
#endif
}




static int near Priv_Adjust_Start(void)
{
  int i, wid;
  newkeys=usr.xkeys;

#ifdef TTYVIDEO
  if (displaymode != VIDEO_IBM)
  {
    Lputs(type_keys_to_toggle);
    ShowWinKeys();
    return 1;
  }
#endif

  if (!pl_privs)
    return 0;

  wid=0;
  for (i=0 ; pl_privs[i].name; ++i)
  {
    int l=strlen(pl_privs[i].name);
    if (l > wid)
      wid=l;
  }
  wid+=2;

  if ((privwin=WinOpen(0, 0, 11, 35+wid, BORDER_DOUBLE, col.pop_text,
                       col.pop_border,
                       WIN_CENTRE | WIN_NOCSYNC | WIN_NODRAW))==NULL)
  {
    return 0;
  }
  
  WinPutstr(privwin,2,wid+2,priv_1);
  WinPutstr(privwin,3,wid+2,priv_2);
  WinPutstr(privwin,4,wid+2,priv_3);

  ShowWinKeys();

  if ((vp=WinCreatePickList(privwin, 1, 1, 5, col.pop_list,
                            col.pop_lselect, pl_privs,
                            usr.priv)) == NULL)
  {
    WinClose(privwin);
    privwin=NULL;
    return 0;
  }

  WinSync(privwin, FALSE);
  return 1;
}




static int near Parse_Priv_Adjust(int ch)
{
  switch (ch)
  {
    case 0:
#ifdef TTYVIDEO
      if (displaymode==VIDEO_IBM)
#endif
      {
        switch (loc_getch())
        {
          case K_UP:
            WinPickAction(vp,PICK_UP);
            break;

          case K_DOWN:
            WinPickAction(vp,PICK_DOWN);
            break;
        }
      }
      break;

    case '\r':
#ifdef TTYVIDEO
      if (displaymode==VIDEO_IBM)
      {
#endif
        usr.priv=(word)WinClosePickList(vp);

        WinClose(privwin);
        privwin=NULL;
#ifdef TTYVIDEO
      }
      else
      {
        ShowWinKeys();
        Lputs(done_ex);
      }
#endif
      usr.xkeys=newkeys;

      Find_Class_Number();
      return 0;

    case '\x1b':
#ifdef TTYVIDEO
      if (displaymode==VIDEO_IBM)
      {
#endif
        WinClosePickList(vp);
        WinClose(privwin);
        privwin=NULL;
#ifdef TTYVIDEO
      }
      else
      {
        Lputs(sys_aborted);
      }
#endif
      return 0;

    default:
      ch=toupper(ch);

        /* Handle key toggling */

      if (ch >= '1' && ch <= '8')
        newkeys ^= (1L << (ch-'1'));
      else if (ch >= 'A' && ch <= 'X')
        newkeys ^= (1L << (ch-'A'+8));
      else
      {
#ifdef TTYVIDEO
        if (displaymode==VIDEO_IBM)
#endif
          WinPickAction(vp,ch);
#ifdef TTYVIDEO
        else Lputs(type_keys_to_toggle);
#endif
      }
        
      ShowWinKeys();

#ifdef TTYVIDEO
      if (displaymode==VIDEO_IBM)
#endif
        WinSync(privwin, FALSE);
      break;
  }

  return 1;
}




static int near Parse_Local_Normal(int ch)
{
  int c;
  char ptmp[16];

  switch(toupper(ch))
  {
    case K_ONEMORE:
      c=loc_getch();

#ifdef UNIX
      if (c == K_ESC)
        goto realEscape;

      /* wes -- he flips out of symbol labels and into real scan codes here. so will we.. */
      if (c >= K_F1 && c <= K_F12)
      {
        Parse_FKey((c = c - K_F1 + 59));
        break;
      }
      if (c >= K_SF1 && c <= K_SF12)
      {
        Parse_FKey((c = c - K_SF1 + 84));
        break;
      }
#else
      if ((c >= 59 && c <= 68) || (c >= 84 && c <= 113))
      {
        Parse_FKey(c);
        break;
      }
#endif 
      else switch (c)
      {
        case K_ALTC:
          chatreq=FALSE;
          ChatMode();      /* else built-in chatmode */

          if (inmagnet)
            Fix_MagnEt();

          input_timeout=timerset(timeout_tics);
          break;

        case K_ALTJ:
          Shell_To_Dos();
          break;

        case K_ALTD:
          Fake_Noise();
          break;
          
        case K_ALTN:
          usr.bits ^= BITS_NERD;
          LocalMsg(nerd_toggled, (usr.bits & BITS_NERD) ? sys_on : sys_off);
          if (usr.bits & BITS_NERD)
            ci_nerd();
          break;

        case K_UP:
        case K_PGUP:
          timeoff += (c==K_UP) ? 60 : 300;
          sent_time_almostup=FALSE;
          sent_time_5left=FALSE;
          LocalMsg(min_add,c==K_UP ? 1 : 5,
                   c==K_UP ? blank_str : "s",timeleft());
          break;

        case K_DOWN:
        case K_PGDN:
          timeoff -= (c==K_DOWN) ? 60 : 300;
          sent_time_almostup=FALSE;
          sent_time_5left=FALSE;
          LocalMsg(min_less,c==K_DOWN ? 1 : 5,
                   c==K_DOWN ? blank_str : "s",timeleft());
          break;
      }
      break;

    case K_CTRLC:
      brk_trapped++;
      break;

    case K_CTRLX:
      logit(log_axe);
      ci_ejectuser();
      mdm_dump(DUMP_ALL);
      mdm_hangup();
      break;

#ifndef UNIX
    case K_ESC:
#else
    case -2:
    realEscape:     
#endif
      if (displaymode==VIDEO_IBM && dspwin)
      {
        WinClose(dspwin);
        dspwin=NULL;
        dspwin_time=0L;
      }
      break;

    case 'A':
      LocalMsg(key_on);
      if (!snoop)
      {
        snoop=TRUE;
        Lputs(CLS);
      }
      
      keyboard=TRUE;
      break;

    case 'L':           /* Lock priv level */
      lockpriv=realpriv();
      locked=TRUE;
      LocalMsg(pv_lock,privstr(realpriv(), ptmp));
      break;

    case 'N':
      LocalMsg(on_snoop);
      
      if (!snoop)
      {
        snoop=TRUE;
        Lputs(CLS);
      }
      break;

    case 'O':
      if (snoop)
      {
        Lputs(CLS);
        vbuf_flush();
        snoop=FALSE;
      }
      
      LocalMsg(off_snoop);
      break;

    case 'S':           /* Set priv level to 'x' */
      return (Priv_Adjust_Start());

    case 'U':           /* Lock priv level */
      if (locked)
      {
        locked=FALSE;
        usr.priv=lockpriv;
        LocalMsg(pv_unlock);
        Find_Class_Number();
      }
      break;

    case 'Z':
      LocalMsg(cume_zero);
      usr.time=0;
      timeon=time(NULL);
      break;

    case '+':
      Priv_Up();

      LocalMsg(pv_change,privstr(usr.priv, ptmp));
      logit(log_pv_change,privstr(usr.priv, ptmp));

      Find_Class_Number();
      break;

    case '-':
      Priv_Down();

      LocalMsg(pv_change,privstr(usr.priv, ptmp));
      logit(log_pv_change,privstr(usr.priv, ptmp));

      Find_Class_Number();
      break;

    case '!':
      Toggle_Yell_Noise();
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    {
      int key;

      key=(ch-'1');

      if (UserHasKey(key))
        UserKeyOff(key);
      else UserKeyOn(key);

      LocalMsg(togkey, ch, UserHasKey(key) ? sys_on : sys_off);
      break;
    }

    case '=':
#ifdef CANENCRYPT
      LocalMsg(pwd_is,
               (usr.bits & BITS_ENCRYPT)
                 ? brackets_encrypted
                 : cfancy_str(usr.pwd));
#else
      LocalMsg(pwd_is, cfancy_str(usr.pwd));
#endif
      break;

    default:
      Local_Stats_Display();

      if (ch=='?')
        snoop=FALSE;
      break;
  }

  input_timeout=timerset(timeout_tics);
  vbuf_flush();

  return 0;
}




void Keyboard_Off(void)
{
  LocalMsg(key_off);
  keyboard=FALSE;
  vbuf_flush();
}


int _stdc DspwinPrintf(char *format,...)
{
  char *string;
  va_list var_args;
  int x;

  if ((string=malloc(256))==NULL)
    return 0;
  
  va_start(var_args, format);
  vsprintf(string, format, var_args);
  va_end(var_args);

  /* Translate a "\n"s to a "\r\n" sequence. */

  if (string[x=strlen(string)-1]=='\n')
  {
    string[x]='\0';
    strcat(string,"\r\n");
  }
  
  WinPutsA(dspwin,string);

  free(string);
  return 0;
}

static void near Local_Stats_Display(void)
{
  #ifdef __TURBOC__
  int _stdc (*prfunc)(char *format,...);
  #else
  int (_stdc *prfunc)(char *format,...);
  #endif
  char *temp, ptmp[8];

  if ((temp=malloc(80))==NULL)
    return;
/*
(U#  15) Name: 123456789012345678   Real: 123456789012345  Bps=19200 CHATREQ
         Priv: AsstSysop/12345678  Phone: 123456789012345
         Last: 12 Jan 89 11:22:33   City: 1234567890123456789

   Width: 12345678    Length: 1234 TimeOnline: 12345   TimeLeft:
     Msg: 12            File: A         Calls: 12345  TimeToday: 12345
    Help: HOTFLASH      Tabs: YES       Nulls: 12345  IBM chars: YES
   Video: 12345678     MaxEd: 1234       More: YES          CLS: YES
DL today: 12345678  DL total: 12345  UL total: 12345    Hotkeys: NO
  Credit: 12345        Debit: 12345
*/


  while (loc_kbhit())  /* Drain local keyboard buffer */
    loc_getch();
  

#ifdef TTYVIDEO
  if (displaymode != VIDEO_IBM)
  {
    Lputs(CYAN);
    prfunc=Lprintf;
  }
  else
#endif
  {
    prfunc=DspwinPrintf;
    
    if (dspwin)
      WinClose(dspwin);
    
    if ((dspwin=WinOpen(((VidNumRows()-19) >> 1)-1, 1, 19, 78, BORDER_DOUBLE,
                        col.pop_text, col.pop_border,
                        WIN_NOCSYNC | WIN_NODRAW))==NULL)
    {
      free(temp);
      return;
    }
  
    /* Force background to be BLUE */

    WinPutsA(dspwin, "\x16\x6f");
    WinPutcA(dspwin, (byte)((col.pop_text >> 4) & 0x07));
  }
  
  sprintf(temp,
          "%s%s%s",
          privstr(usr.priv, ptmp),
          usr.xkeys ? "/" : blank_str,
          Keys(usr.xkeys));

#ifdef TTYVIDEO
  if (displaymode != VIDEO_IBM)
    (*prfunc)(stat_0);
#endif

  (*prfunc)(stat_1, usr.name, usr.alias, baud);
  (*prfunc)(stat_2, temp, usr.phone, usr.dob_year, usr.dob_month, usr.dob_day);
  (*prfunc)(stat_3, (displaymode==VIDEO_IBM) ? col.pop_text : curattr,
                    sc_time(&usr.ludate, temp), usr.city,
                    chatreq ? ch_req : blank_str);
  (*prfunc)(stat_0);
  (*prfunc)(stat_4, TermWidth(), TermLength(), timeonline(), timeleft());
  (*prfunc)(stat_5, usr.msg);
  (*prfunc)(stat_6, usr.files, usr.times, usr.time);

  (*prfunc)(stat_7,
            Help_Level(usr.help),
            Sysop_Yes_or_No(usr.bits & BITS_TABS),
            usr.nulls,
            Sysop_Yes_or_No(usr.bits2 & BITS2_IBMCHARS));

  (*prfunc)(stat_8,
            Graphics_Mode(usr.video),
            Sysop_Yes_or_No((usr.bits2 & BITS2_BORED)==0),
            Sysop_Yes_or_No(usr.bits2 & BITS2_MORE),
            Sysop_Yes_or_No(usr.bits2 & BITS2_CLS));

  (*prfunc)(stat_9,
            usr.downtoday,
            usr.down,
            usr.up,
            Sysop_Yes_or_No(usr.bits & BITS_HOTKEYS));

  if (usr.xp_flag & XFLAG_EXPDATE)
    FileDateFormat(&usr.xp_date, temp);
  else if (usr.xp_flag & XFLAG_EXPMINS)
    sprintf(temp, stat_mins, usr.xp_mins);
  else strcpy(temp, proto_none);
  
  (*prfunc)(stat_10,
            usr.credit, usr.debit, 
            (usr.xp_flag & XFLAG_AXE)       ? stat_hangup
            : ((usr.xp_flag & XFLAG_DEMOTE) ? stat_demote
            :                                 proto_none),
            temp);

  (*prfunc)(stat_11,
            Sysop_Yes_or_No(usr.bits & BITS_RIP));

  (*prfunc)(stat_0);
  (*prfunc)(stat_12);
  (*prfunc)(stat_13);
  (*prfunc)(stat_14);
  (*prfunc)(stat_15);
  (*prfunc)(stat_16);

  (*prfunc)(CLEOL);

#ifdef TTYVIDEO
  if (displaymode==VIDEO_IBM)
#endif
    WinSync(dspwin,FALSE);
#ifdef TTYVIDEO
  else Lprintf("\n");
#endif

  /* Stay on-screen for up to 3 seconds */
  dspwin_time=timerset(20*100);

  free(temp);
  return;
}

void cdecl LocalMsg(char *fmt,...)
{
  va_list var_args;
  char *out;

  if ((out=malloc(PATHLEN))==NULL)
    return;

  va_start(var_args,fmt);
  vsprintf(out,fmt,var_args);
  va_end(var_args);

#ifdef TTYVIDEO
  if (displaymode != VIDEO_IBM)
  {
    Lprintf(CYAN "\n%s\n",out);
    free(out);
    return;
  }
#endif

  if (dspwin)
    WinClose(dspwin);

  dspwin=WinMsg(BORDER_DOUBLE, col.pop_text, col.pop_border, out, NULL);

  /* Stay on-screen for up to 3 seconds */
  dspwin_time=timerset(DSPWIN_TIME*100);

  free(out);
  return;
}

/* Shifts a priv up by one level */
void Priv_Up(void)
{
  int classes=(int)ClassGetInfo(0,CIT_NUMCLASSES);
  int ucls=ClassLevelIndex(usr.priv);

  if (++ucls < classes &&
      (ClassGetInfo(ucls, CIT_ACCESSFLAGS) & CFLAGA_HANGUP)==0)
  {
    usr.priv=(word)ClassGetInfo(ucls,CIT_LEVEL);
  }
}


/* Shifts a priv down one level */
void Priv_Down(void)
{
  int ucls=ClassLevelIndex(usr.priv);
  word clevel=(word)ClassGetInfo(ucls,CIT_LEVEL);
  if (clevel!=usr.priv)                   /* Not exactly on a priv level */
    usr.priv=clevel;                  /* Use base priv for current class */
  else if (ucls-- > 0)
    usr.priv=(word)ClassGetInfo(ucls,CIT_LEVEL);
}

void Parse_FKey(int c)
{
  int l;
  char fkfile[PATHLEN];


  l = sprintf(fkfile, "%s%sF%d",PRM(misc_path),
                      (c >=  59 && c <=  68) ? blank_str :
                      ((c >=  84 && c <=  93) ? "S" :
                      ((c >=  94 && c <= 103) ? "C" :
                      ((c >= 104 && c <= 113) ? "A" :
                      blank_str))),
                      (c >=  59 && c <=  68) ? c-58 :
                      ((c >=  84 && c <=  93) ? c-83 :
                      ((c >=  94 && c <= 103) ? c-93 :
                      ((c >= 104 && c <= 113) ? c-103 :
                      1))));

  strcpy(fkfile+l, ".vm");
  if (fexist(fkfile))
  {
    fkfile[l]='\0';
    display_line=display_col=1;
    Mex(fkfile);
  }
  else
  {
    strcpy(fkfile+l,".?bs");
    if (fexist(fkfile))
    {
      fkfile[l]='\0';
      display_line=display_col=1;
      Display_File(0,NULL,percent_s,fkfile);
    }
    else l = 0;
  }

  if (l)
  {
    if (inmagnet && !inchat)
      Fix_MagnEt();

    vbuf_flush();

    input_timeout=timerset(timeout_tics);
  }
}

static void near Fake_Noise(void)
{
  char string[50];
  int x, y, len;

  ci_ejectuser();

  if (mdm_attr > 15)
    Puts(GRAY);

  /* Release all flow constraints in case we accidentally generate a ^s */

  Mdm_Flow(FLOW_OFF);
  
  strcpy(string, line_noise);

  for (y=6; y--;)
  {
    for (x=0, len=strlen(string); x < len; x++)
      string[x] += ((char)time(NULL)) | 0x80;

    Puts(string);

    Delay((unsigned int)(string[x-1]*456L) % 60);
  }

  Lputc('\r');
  logit(log_dump);
  mdm_hangup();
}


#endif /* !ORACLE */

