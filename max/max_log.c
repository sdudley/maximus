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
static char rcs_id[]="$Id: max_log.c,v 1.6 2004/01/11 19:43:21 wmcbrine Exp $";
#pragma on(unreferenced)

/*# name=Log-on routines and new-user junk
*/

#define MAX_LANG_max_init
#define MAX_LANG_max_chat
#define MAX_LANG_max_main
#define MAX_INCL_COMMS

#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <ctype.h>
#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max_msg.h"
#include "display.h"
#include "userapi.h"
#include "trackm.h"
#include "ued.h"
#include "md5.h"

#ifdef EMSI
#include "emsi.h"
#endif

static void near doublecheck_rip(void);
static void near doublecheck_ansi(void);
static void near Calc_Timeoff(void);
static void near Logo(char *key_info);
static void near Banner(void);
static int near GetName(void);
static int near Find_User(char *username);
static void near NewUser(char *username);
static void near Get_AnsiMagnEt(void);
static void near Check_For_User_On_Other_Node(void);
static void near Set_OnOffTime(void);
static void near Write_Active(void);
static int near InvalidPunctuation(char *string);
static int near checkterm(char *prompt, char *helpfile);

static char szBadUserName[]="%sbad_user";

#define USR_BLOCK 9

void Login(char *key_info)
{
  signed int left;
  int newuser;
  
  Calc_Timeoff();
  
  if (! local && !waitforcaller)
    logit(log_caller_bps, baud);

  caller_online=TRUE;

  if (! local)
    mdm_baud(current_baud);

  Mdm_Flow_On();

  if (!local)
    Logo(key_info);

  /* Check overall lowest baud rate */

  if (!local && baud < prm.min_baud)
  {
    logit(ltooslow, baud, prm.min_baud);

    Display_File(0, NULL, PRM(tooslow));
    mdm_hangup();
  }

  Banner();

  strcpy(usrname, us_short);
  ChatSetStatus(FALSE, cs_logging_on);

  newuser=!GetName();

  Validate_Runtime_Settings();
  Set_OnOffTime();
  logit(log_given, left=timeleft());


#ifdef MAX_TRACKER
  /* Initialize the message tracking subsystem */

  InitTracker();
#endif

  /* We're OK!  Let's write the logon files now... */

  fLoggedOn=TRUE;

  Write_LastUser();
  Write_Active();

  ci_login();
  ci_loggedon();

  TagReadTagFile(&mtm);

  /* Set the current user's chat availability */

  if (usr.bits & BITS_NOTAVAIL)
    ChatSetStatus(FALSE, cs_notavail);
  else ChatSetStatus(TRUE, cs_avail);

  mdm_attr=curattr=-1;
  Puts(CYAN);

  /* If this is an existing user and there is a file to display for users
   * of this class, then display it now.
   */

  if (!newuser && *ClassFile(cls))
    Display_File(0, NULL, ClassFile(cls));

  /* If user flubbed password on last logon attempt */

  if (usr.bits2 & BITS2_BADLOGON)
  {
    Display_File(0, NULL, PRM(bad_logon));
    usr.bits2 &= ~BITS2_BADLOGON;
  }

  date_newfile=usr.date_newfile;

  switch (usr.times+1)
  {
    case 1:
      Display_File(DISPLAY_PCALL,NULL,PRM(newuser2));
      break;

    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      Display_File(DISPLAY_PCALL,NULL,PRM(rookie));
      break;

    default:
      Display_File(DISPLAY_PCALL,NULL,PRM(welcome));
      break;
  }

  bstats.today_callers++;
  bstats.num_callers++;
  usr.times++;

#ifdef NEVER /* Opus 1.03 compatibility feature no longer supported */
  if (prm.flags & FLAG_lbaud96)
    usr.ludate=next_ludate;
#endif

  if (usr.time)
    Display_File(0, NULL, PRM(timewarn));
}


static void near Logo(char *key_info)
{
#ifndef KEY
  NW(key_info);
#endif

  mdm_dump(DUMP_INPUT);

  Putc('\n');

#ifdef EMSI
  EmsiTxFrame(EMSI_IRQ);
#endif

  Printf("%s v%s ",name,version);

#ifdef KEY
  Printf("\nSystem: %s\n"
           " SysOp: %s",
         key_info+strlen(key_info)+1,
         key_info);
#endif
}


static void near Banner(void)
{
  if ((! *linebuf && !local) || eqstri(linebuf,"-"))
  {
    *linebuf='\0';
    Display_File(0,NULL,PRM(logo));
  }
  else if (! *linebuf)
    strcpy(linebuf,PRM(sysop));
}



static int near GetName(void)
{
  int found_it;     /* True if the user's user record has been found */
  int fMatch;       /* True if the user's password is correct */
  char *fname;      /* The user's first name */
  char *lname;      /* The user's last name */
  char *username;   /* The user's full name */
  char *pwd;        /* The user's password */
  char *quest;      /* The next question to be asked of the user */
  byte save;        /* Saved copy of usr.help */
  char saveb;       /* Saved copy of usr.bits */
  unsigned tries;   /* Number of attempts user has made at password */

  fname   = malloc(BUFLEN);
  lname   = malloc(BUFLEN);
  username= malloc(BUFLEN*3);
  pwd     = malloc(BUFLEN);
  quest   = malloc(PATHLEN);

  if (! (fname && lname && username && pwd && fname && quest))
  {
    logit(mem_none);
    quit(ERROR_CRITICAL);
  }

  for (;;)
  {
    found_it=FALSE;

    *fname=*lname='\0';

    if (!*linebuf)
      Putc('\n');

#if 0 /*SJD Sat  08-01-1992  23:37:45 */ /* a bug in fd makes it puke if we do this */
#ifdef EMSI
{
  extern word fGotICI, fAbort, fEMSI;

    /* If caller is using a pre-2.01 language file, do not do IRQ here */

    if (Mdm_keyp() != '*' && *what_first_name != '\n' &&
        !fAbort && !fEMSI && !fGotICI)
      EmsiTxFrame(EMSI_IRQ);
}
#endif
#endif

    sprintf(quest, what_first_name,
            (prm.flags & FLAG_alias) ? s_alias : blank_str);

    InputGetsWNH(fname, quest);

    if (! *fname)
    {
      if (local)
        quit(0);
      else continue;
    }

    if (!(((byte)toupper(*linebuf)==YES || (byte)toupper(*linebuf)==NO) &&
        (strpbrk(linebuf,cmd_delim)==linebuf+1 || !linebuf[1])))
    {

      /* Try finding on single name first before we ask user
       * unnecessarily for their last name.
       */

      if (!*linebuf)
      {
        strncpy(username, fname, 35);
        username[35]='\0';
        fancier_str(username);

        found_it=Find_User(username);
      }

      if (!found_it && (prm.flags2 & FLAG2_1NAME)==0)
      {
        sprintf(quest, what_last_name,
                (prm.flags & FLAG_alias) ? s_alias : blank_str);

        InputGetsWNH(lname, quest);

        /* If a last name was entered, we need to search again
           with the full name */

      }
    }

    sprintf(username, "%s%s%s", fname, *lname ? " " : blank_str, lname);

    /* Code added in in case people have more than two names! */

    while (*linebuf &&
           !(((byte)toupper(*linebuf)==YES || (byte)toupper(*linebuf)==NO) &&
            (strpbrk(linebuf, cmd_delim)==linebuf+1 || !linebuf[1])))
    {
      strcat(username," ");
      InputGetsWNH(username+strlen(username), blank_str);
    }

    username[35]='\0';

    fancier_str(username);

    if (!found_it)
      found_it=Find_User(username);

    if (!found_it)
    {
      *linebuf='\0';

      if (! *lname && (prm.flags2 & FLAG2_1NAME)==0)
        continue;

      /* Say that we didn't find the user */

      strcpy(usr.name, username);
      SetUserName(&usr, usrname);

      Display_File(0, NULL, PRM(notfound));
    }

    save=usr.help;
    usr.help=NOVICE;

    saveb=usr.bits;
    usr.bits &= ~BITS_HOTKEYS;

    if (! *linebuf)
      Putc('\n');

    if (GetYnAnswer(username,0)==NO)
    {
      if (!local)
        logit(brain_lapse,username);

      Blank_User(&usr);
      continue;
    }

    usr.help=save;
    usr.bits=saveb;

    if (! found_it) /* If we couldn't find him/her in our userfile */
    {
      /* If user wasn't in user file, make sure that there's correct punct. */

      ci_init();

      if (InvalidPunctuation(username))
      {
        Clear_KBuffer();
        Puts(invalid_punct);
        continue;
      }

      logit(so_and_so_calling, username);
      logit(log_not_in_ulist, username);

      Bad_Word_Check(username);

      Clear_KBuffer();

      NewUser(username);

      ci_init();

    }
    else
    {

      ci_init();

      logit(so_and_so_calling, username);

      if (*usr.alias && !eqstri(usr.name,usr.alias))
      {

        /* Tell the sysop who's really calling */

        if (eqstri(username,usr.alias))
          logit(so_and_so_realname, usr.name);
        else
          logit(so_and_so_alias, usr.alias);
      }

      Switch_To_Language();
      Set_Lang_Alternate(hasRIP());

      tries=0;

      do
      {
        if (++tries != 1)
        {
          Clear_KBuffer();

          this_logon_bad=TRUE;

          logit(log_bad_pwd, pwd);

          Printf(wrong_pwd, tries-1);

          if (tries==6)
          {
            ci_ejectuser();
            logit(l_invalid_pwd);
            Puts(invalid_pwd);

            Display_File(0, NULL, "%sbad_pwd", PRM(misc_path));
            mdm_hangup();
          }
        }

        *pwd='\0';

        /* Pressing ENTER now counts as a bad password attempt */

#ifdef CANENCRYPT
        if (*usr.pwd || (usr.bits & BITS_ENCRYPT))
#else
        if (*usr.pwd)
#endif
        {
          if (! *linebuf)
            Putc('\n');

          InputGetseNH(pwd, '.', usr_pwd);
        }

        
        
        /* For "guest" accounts, reconfig at every logon */

#ifdef CANENCRYPT
        if (*usr.pwd==0 && (usr.bits & BITS_ENCRYPT)==0)   
#else
        if (*usr.pwd==0)
#endif
        {
          usr.bits &= ~BITS_RIP;
          usr.bits2 |= BITS2_MORE | BITS2_CLS;
          usr.bits2 &= ~BITS2_CONFIGURED;
          usr.time=0;
          usr.call=0;
          usr.down=0L;
          usr.downtoday=0L;
          usr.up=ultoday=0L;
          usr.nup=usr.ndown=usr.ndowntoday=0L;
          usr.width=80;
          usr.len=24;
        }
      

#ifdef CANENCRYPT
        if (usr.bits & BITS_ENCRYPT)
        {
          byte abMd5[MD5_SIZE];

          string_to_MD5(strlwr(pwd), abMd5);
          fMatch = (memcmp(abMd5, usr.pwd, MD5_SIZE)==0);
        }
        else
#endif
          fMatch = (stricmp(cfancy_str(pwd), usr.pwd)==0);
      }
      while (! fMatch);
    }

    this_logon_bad=FALSE;   /* User made it through, probably just a typo */

    if ((usr.bits2 & BITS2_CONFIGURED)==0)
    {
      if (prm.not_configured)
        Display_File(0, NULL, PRM(not_configured));
      /* Test this again, it might have been changed */
      if ((usr.bits2 & BITS2_CONFIGURED)==0)
        Get_AnsiMagnEt();
    }
    else if (! local)
    {
      doublecheck_ansi();
      doublecheck_rip();
    }

    free(quest);
    free(lname);
    free(username);
    free(pwd);
    free(fname);

    return found_it;
  }
}



/* doublecheck_ansi
 *
 * Ensure that the user really does have ANSI graphics by trying to
 * autodetect, and then warning the user if graphics are not found.
 */

static void near doublecheck_ansi(void)
{
  if ((prm.flags2 & FLAG2_CHKANSI) &&
      (usr.video==GRAPH_ANSI) &&
      !autodetect_ansi() &&
      checkterm(check_ansi, "why_ansi"))
  {
    usr.video=GRAPH_TTY;
    usr.bits &= ~BITS_RIP;
    SetTermSize(0, 0);
  }
}

/* doublecheck_rip
 *
 * Ensure that the user really does have RIP graphics by trying to
 * autodetect, and then warning the user if graphics are not found.
 */

static void near doublecheck_rip(void)
{
  if ((prm.flags2 & FLAG2_CHKRIP) &&
      (usr.bits & BITS_RIP) &&
      !autodetect_rip() &&
      checkterm(check_rip, "why_rip"))
  {
    usr.bits&=~BITS_RIP;
    SetTermSize(0, 0);
  }

  Set_Lang_Alternate(hasRIP());
}


/* Find this user in the user file */

static int near Find_User(char *username)
{
  HUF huf;
  int mode;
  int ret;

  mode=create_userbbs ? O_CREAT : 0;

  /* Open the user file */

  if ((huf=UserFileOpen(PRM(user_file), mode))==NULL)
  {
    cant_open(PRM(user_file));
    Local_Beep(3);
    quit(ERROR_FILE);
  }

  if (!UserFileFind(huf, username, NULL, &usr) &&
      !UserFileFind(huf, NULL, username, &usr))
  {
    ret=FALSE;
  }
  else
  {

    if (usr.delflag & UFLAG_DEL)
      ret = FALSE;
    else
    {
      /* Found the user successfully */

      origusr=usr;
      SetUserName(&usr, usrname);
      ret=TRUE;
    }
  }  

  UserFileClose(huf);
  return ret;
}


/* Handle a new user */

static void near NewUser(char *username)
{
  HUF huf;

  Blank_User(&usr);

  strcpy(usr.name,username);
  SetUserName(&usr,usrname);

  if (create_userbbs)
  {
    /* Assume sysop privs */

    usr.priv=(word)ClassGetInfo(ClassLevelIndex((word)-2), CIT_LEVEL);
    usr.lastread_ptr=0;
    usr.credit=65500u;
  }
  else
  {
    usr.priv=prm.logon_priv;  /* Do this up front, just in case s/he       *
                               * somehow gets past this stage              */

    /* Get a lastread pointer for the user here, just in case anything
     * tries to use it.  However, we make the same check again below,
     * just in case we have two users who are logging on at the same
     * time.  Hopefully, the lastread pointer selected here will not
     * be used for anything, but if it is, at least it is guaranteed
     * not to overwrite one of the lastread pointers of the other
     * users in the user file.
     */

    if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
    {
      cant_open(PRM(user_file));
      quit(ERROR_FILE);
    }

    usr.lastread_ptr=Find_Next_Lastread(huf);

    UserFileClose(huf);
  }

  Find_Class_Number();      /* Make sure they have a high enough baud rate *
                             * to log in -- don't bother displaying        *
                             * applic if they don't.                       */

  logit(log_applic);

  if (prm.logon_priv==PREREGISTERED && !create_userbbs)
  {
    if (*PRM(application))
      Display_File(0,NULL,PRM(application));
    else Puts(pvt_system);

    mdm_hangup();
  }

  if (*PRM(application))
    Display_File(0, NULL, PRM(application));

  Chg_City();

  if ((prm.flags & FLAG_ask_name)==0)
    *usr.alias='\0';
  else
  {
    Chg_Alias();

    Bad_Word_Check(usr.alias);
  }

  if (prm.flags & FLAG_ask_phone)
    Chg_Phone();
  else
    *usr.phone='\0';

  /* Store the date of the user's first call */

  Get_Dos_Date(&usr.date_1stcall);

  Display_File(0, NULL, PRM(newuser1));

  Get_Pwd();

  /* Write the user record to disk to make sure that it is registered */

  if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
  {
    cant_open(PRM(user_file));
    quit(ERROR_FILE);
  }

  /* Get a good lastread pointer */

  usr.lastread_ptr=Find_Next_Lastread(huf);

  if (!UserFileCreateRecord(huf, &usr, TRUE))
    logit(cantwrite, PRM(user_file));

  origusr=usr;

  UserFileClose(huf);
}


static void near Get_AnsiMagnEt(void)
{
  char string[PATHLEN];
  int x;

  Clear_KBuffer();

  if (local || baud >= prm.speed_graphics)
  {
    NoWhiteN();
    sprintf(string,"%swhy_ansi",PRM(misc_path));

/* TODO: Check up on this.. (Bo) */

    x=autodetect_ansi();

    if (! *linebuf)
      Puts(get_ansi1);

    if (GetListAnswer(x ? CYnq : yCNq, string, useyforyes, 0, get_ansi2)==YES)
    {
      usr.video=GRAPH_ANSI;
      usr.bits |= BITS_FSR;
    }
    else
    {
      usr.video=GRAPH_TTY;
      usr.bits &= ~BITS_FSR;
      usr.bits2 |= BITS2_BORED;
    }

    usr.bits &= ~BITS_RIP;

    if (local || baud >= prm.speed_rip)
    {
      NoWhiteN();
      sprintf(string,"%swhy_rip",PRM(misc_path));

/* TODO: Check up on this.. (Bo) */
      x=autodetect_rip();

      if (GetListAnswer(x ? CYnq : yCNq, string, useyforyes, 0, get_rip)==YES)
      {
        usr.bits  |= (BITS_RIP | BITS_FSR | BITS_HOTKEYS );
        usr.bits2 |= BITS2_CLS;

        if (!x)
        {
          /* Ensure that the user really knows what s/he is doing... */

          logit(log_rip_enabled_ndt);
          doublecheck_rip();
        }
      }
    }

    if (usr.video!=GRAPH_TTY)
    {
      sprintf(string, "%swhy_fsed", PRM(misc_path));

      NoWhiteN();

      if (GetYnhAnswer(string, get_fsed, 0)==YES)
      {
        usr.bits2 &= ~BITS2_BORED;
        usr.bits  |= BITS_FSR;
      }
      else
      {
        usr.bits2 |= BITS2_BORED;
        if (!(usr.bits & BITS_RIP))
          usr.bits  &= ~BITS_FSR;
      }
    }

    sprintf(string, "%swhy_pc", PRM(misc_path));

    NoWhiteN();

    if (GetYnhAnswer(string, get_ibmpc, 0)==YES)
      usr.bits2 |= BITS2_IBMCHARS;
    else
      usr.bits2 &= ~BITS2_IBMCHARS;

    if (usr.bits & BITS_RIP)
    {
      usr.bits |= BITS_HOTKEYS|BITS_FSR;
      usr.bits2 |= BITS2_CLS;
    }
    else
    {
      NoWhiteN();

      sprintf(string, "%swhy_hot", PRM(misc_path));

      if (GetYnhAnswer(string, get_hotkeys, 0)==YES)
        usr.bits |= BITS_HOTKEYS;
      else
        usr.bits &= ~BITS_HOTKEYS;
    }

    Set_Lang_Alternate(hasRIP());

    usr.bits2 |= BITS2_CONFIGURED;   /* We've asked all the kludge stuff. */
  }
  else              /* Too slow for either, leave kludge for later */
  {
    usr.video=GRAPH_TTY;
    usr.bits  &= ~BITS_RIP;
    usr.bits2 |= BITS2_BORED;
  }
}

static int near checkterm(char *prompt, char *helpfile)
{
  char string[PATHLEN];

  sprintf(string, ss, PRM(misc_path), helpfile);

  NoWhiteN();
  return (GetYnhAnswer(string, prompt, 0)==YES);
}


void Validate_Runtime_Settings(void)
{
  BARINFO bi;

  Find_Class_Number();

  /* Check overall lowest baud rate */

  if (!local && baud < (dword)ClassGetInfo(cls,CIT_MIN_BAUD))
  {
    logit(ltooslow, baud, ClassGetInfo(cls,CIT_MIN_BAUD));

    Display_File(0, NULL, PRM(tooslow));
    mdm_hangup();
  }

  Check_For_User_On_Other_Node();

  /* Now validate the run-time settings, in case some external program has
     been screwing around with the user file!                               */

  if (usr.video && baud < prm.speed_graphics && !local)
  {
    usr.video=GRAPH_TTY;
    usr.bits2 |= BITS2_BORED;
  }

  if (prm.flags & FLAG_no_magnet)
    usr.bits2 |= BITS2_BORED;

  if (usr.help != NOVICE && usr.help != REGULAR && usr.help != EXPERT)
    usr.help=NOVICE;

  if (usr.width < 20)
    usr.width=20;

  if (usr.width > 132)
    usr.width=132;

  if (usr.len < 8)
    usr.len=8;

  if (usr.len > 200)
    usr.len=200;
  
  if (usr.lang > prm.max_lang)
    usr.lang=0;

  if (usr.bits & BITS_RIP)
  {
    usr.bits  |= (BITS_HOTKEYS|BITS_FSR);
    usr.bits2 |= BITS2_CLS;
  }

  if (usr.bits & BITS_FSR)
    usr.bits2 |= BITS2_MORE;

#ifdef MUSTENCRYPT
  /* If this flag is set, it means that we are to enforce the
   * "user must have an encrypted password" option.
   * .. unless the sysop has elected not to encrypt. :-)
   */

    /* If this is not a guest account and the user password is
     * not encrypted...
     */

  if (!(prm.flags2 & FLAG2_NOENCRYPT) && *usr.pwd && (usr.bits & BITS_ENCRYPT)==0)
  {

    byte abMd5[MD5_SIZE];

    /* Digest the password, copy it to the user record, and
     * set the encrypted bit.
     */

    string_to_MD5(strlwr(usr.pwd), abMd5);

    memcpy(usr.pwd, abMd5, sizeof usr.pwd);
    usr.bits |= BITS_ENCRYPT;
  }
#endif

  /* Try to ensure that we are currently in a valid file area */

  if (!ValidFileArea(usr.files, NULL, VA_VAL | VA_PWD | VA_EXTONLY, &bi))
  {
    char temp[PATHLEN];

    Parse_Outside_Cmd(PRM(begin_filearea), temp);
    SetAreaName(usr.files, temp);
  }


  /* Try to ensure that we are currently in a valid msg area */

  if (!ValidMsgArea(usr.msg, NULL, VA_VAL | VA_PWD | VA_EXTONLY, &bi))
  {
    char temp[PATHLEN];

    Parse_Outside_Cmd(PRM(begin_msgarea), temp);
    SetAreaName(usr.msg, temp);
  }

  ForceGetFileArea();
  ForceGetMsgArea();
}


int Bad_Word_Check(char *username)
{
  FILE *baduser;

  char usrword[PATHLEN];
  char fname[PATHLEN];
  char line[PATHLEN];
  char *p;

  sprintf(fname, "%sbaduser.bbs", original_path);

  /* If it's not there and can't be opened, don't worry about it */
  
  if ((baduser=shfopen(fname, fopen_read, O_RDONLY))==NULL)
    return FALSE;
  
  while (fgets(line, PATHLEN, baduser))
  {
    Trim_Line(line);

    if (*line=='\0' || *line==';')
      continue;

    strcpy(usrword, username);

    p=strtok(usrword, cmd_delim);

    while (p)
    {

      /* '~' means 'name contains' rather than finding a word match */

      if ((*line=='~' && (stristr(p, line+1) || stristr(username, line+1))) ||
          (*line!='~' && (eqstri(p, line) || eqstri(username, line))))
      {
        fclose(baduser);

        /* Log the problem and hang up */

        logit(bad_uword);
        ci_ejectuser();
        Display_File(0, NULL, szBadUserName, PRM(misc_path));
        mdm_hangup();

        /* This should never return, but... */

        return TRUE;
      }

      p=strtok(NULL,cmd_delim);
    }
  }

  fclose(baduser);
  
  return FALSE;
}




static void near Check_For_User_On_Other_Node(void)
{
  int lastuser;
  sword ret;

  struct _usr user;

  char fname[PATHLEN];

  unsigned int their_task;

  FFIND *ff;

  sprintf(fname,active_star,original_path);

  for (ff=FindOpen(fname, 0), ret=0; ff && ret==0; ret=FindNext(ff))
  {
    if (sscanf(cstrlwr(ff->szName), active_x_bbs, &their_task) != 1)
      continue;

    /* Don't process our own task number */
    
    if ((byte)their_task==task_num)
      continue;
        
    sprintf(fname,
            their_task ? lastusxx_bbs : lastuser_bbs,
            original_path, their_task);

    if ((lastuser=shopen(fname, O_RDONLY | O_BINARY))==-1)
      continue;

    read(lastuser, (char *)&user, sizeof(struct _usr));
    close(lastuser);


    /* If we found this turkey on another node... */

    if (eqstri(user.name, usr.name))
    {
      Display_File(0, NULL, "%sACTIVE_2", PRM(misc_path));
      ci_ejectuser();
      mdm_hangup();
    }
  }

  FindClose(ff);
}




/* Calculate the time the user has to be off the system, reset any daily   *
 * download or time limits, and give the user the boot if he's been on too *
 * long                                                                    */

static void near Set_OnOffTime(void)
{
  union stamp_combo today;

  Get_Dos_Date(&today);

  /* If the user has an expiry date, and that date has passed... */

  if ((usr.xp_flag & XFLAG_EXPDATE) && GEdate(&today,&usr.xp_date))
    Xpired(REASON_DATE);


  timeon=time(NULL);

  /* Get today's date, and stuff it in next_ludate.  (This will be          *
   * dumped into the caller's usr.ludate at the end of this call.)          */

  next_ludate=today;

  /* If the two aren't equal, then that time was for some other day,        *
   * so we should reinitalize the user's time and DL quota.                 */

  if (usr.ludate.dos_st.date != next_ludate.dos_st.date)
  {
    usr.time=0;
    usr.call=0;
    usr.time_added=0;
    usr.downtoday=0;
    usr.ndowntoday=0;
  }

  if (usr.time >= (word)ClassGetInfo(cls,CIT_DAY_TIME) ||
      usr.call >= (word)ClassGetInfo(cls,CIT_MAX_CALLS))
  {
    do_timecheck=FALSE;
    
    logit(log_exc_daylimit);
    Display_File(0, NULL, PRM(daylimit));

    Printf(tlimit1, ClassGetInfo(cls,CIT_CALL_TIME));
    Printf(tlimit2, usr.time);
    
    do_timecheck=TRUE;
    ci_ejectuser();

    mdm_hangup();    /* Bye! */
  }

  scRestrict.ldate=0; /* default to no restriction */

  Calc_Timeoff();
}




/* Writes the ACTIVExx.BBS file that signifies the current node has a      *
 * user on-line!                                                           */

static void near Write_Active(void)
{
  int file;
  char fname[PATHLEN];

  sprintf(fname, activexx_bbs, original_path, task_num);

  if ((file=sopen(fname, O_CREAT | O_WRONLY | O_BINARY,
                  SH_DENYWR, S_IREAD | S_IWRITE))==-1)
    cant_open(fname);
  else close(file);
}


/* Inserts the time into the user's .date structure, in the format used    *
 * by Opus!                                                                */


#ifdef NEVER /* notused */
void maketime(char *string)
{
  union stamp_combo dost;

  sc_time(Get_Dos_Date(&dost),string);
}
#endif



/* Checks to see if a string has any invalid punctuation.  This one is     *
 * only used to check the logon name...  InvalidPunctuationS is used       *
 * for stricter checking on everything else.                               */

static int near InvalidPunctuation(char *string)
{
  char *badstring;
  char *badalias;
  
  if (prm.charset==CHARSET_SWEDISH)
  {
    badstring=",/=@#$%^&()";
    badalias="!*+:<>?~_";
  }
  else
  {
    badstring="\",/\\[]=@#$%^&()";
    badalias="!*+:<>?{|}~_";
  }
  
  if (strpbrk(string, badstring) != NULL)
    return TRUE;
  else
  {
    if (prm.flags & FLAG_alias)
      return FALSE;
    else return ((strpbrk(string, badalias) != NULL));
  }
}


void Get_Pwd(void)
{
  char got[PATHLEN];
  char check[PATHLEN];

  do
  {
    Clear_KBuffer();

    InputGetsLe(got, BUFLEN, '.', get_pwd1);

    if (strlen(got) < 4 || strlen(got) > 15 || strpbrk(got, cmd_delim))
    {
      Puts(bad_pwd1);
      *got=0;
      continue;
    }

    Clear_KBuffer();

    InputGetsLe(check, BUFLEN, '.', check_pwd2);

    if (! eqstri(got, check))
    {
      Printf(bad_pwd2, got, check);
      *got=0;
    }
    else
    {
#if defined(CHANGEENCRYPT) || defined(MUSTENCRYPT)
      usr.bits &= ~BITS_ENCRYPT;
      if (!(prm.flags2 & FLAG2_NOENCRYPT))
      {
        byte abMd5[MD5_SIZE];

        string_to_MD5(strlwr(got), abMd5);

        memcpy(usr.pwd, abMd5, sizeof usr.pwd);
        usr.bits |= BITS_ENCRYPT;
      }
      else
#endif
        strcpy(usr.pwd, cfancy_str(got));

      Get_Dos_Date(&usr.date_pwd_chg);
    }
  }
  while (*got=='\0');
}


static void near Calc_Timeoff(void)
{
  word mins;

  dword min_1;
  dword min_2;
  dword min_3;

  /* Our time limit is the SMALLEST out of the following three numbers:    *
   *                                                                       *
   * Our priv level's maximum time limit for each call                     *
   * Our priv level's maximum time limit for each day, minus the amount    *
   * of time we've been on previously today,                               *
   * The -t parameter specified on the command line                        */

  if (caller_online)
  {
    mins=(word)ClassGetInfo(cls,CIT_DAY_TIME) - usr.time + usr.time_added;
    mins=min(mins, (word)ClassGetInfo(cls,CIT_CALL_TIME));
  }
  else
    mins=prm.logon_time;

  min_1 = timeon + (dword)(mins * 60L);
  min_2 = timestart + (dword)(max_time * 60L);
  
  if (usr.xp_flag & XFLAG_EXPMINS)
    min_3 = timeon + ((long)(usr.xp_mins + 1) * 60L);
  else min_3 = min_2;

  timeoff = min(min_1, min_2);
  timeoff = min(timeoff, min_3);
}


#ifndef ORACLE

  /* ANSI autodetect */

int autodetect_ansi(void)
{
  int x;

  if (local || !ComIsAModem(hcModem))
    return TRUE;

  mdm_dump(DUMP_INPUT);
  Mdm_puts(ansi_autodetect);
  Mdm_flush();

  /* If user's term reports s/he has ANSI */

  if ((x=Mdm_kpeek_tic(200))==27)
    x=TRUE;
  else
    x=FALSE;

  while(Mdm_kpeek_tic(50)!=-1)
    Mdm_getcw();
  mdm_dump(DUMP_INPUT);

  return x;
}

  /* RIP autodetect */

int autodetect_rip(void)
{
  int x;

  if (local || !ComIsAModem(hcModem))
    return FALSE;

  /* RIP autodetect */

  mdm_dump(DUMP_INPUT);
  Mdm_puts(rip_autodetect);
  Mdm_flush();

  /* If user's term reports s/he has RIP */

  if ((x=Mdm_kpeek_tic(200))=='0' || x=='1')
    x= TRUE;
  else
    x=FALSE;

  while(Mdm_kpeek_tic(50)!=-1)
    Mdm_getcw();

  return x;
}

#endif

