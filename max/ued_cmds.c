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
static char rcs_id[]="$Id: ued_cmds.c,v 1.2 2003/06/29 19:26:11 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Internal user editor (miscellaneous commands)
*/

#define MAX_LANG_max_ued
#define MAX_LANG_max_chng
#define MAX_LANG_max_init

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "ffind.h"
#include "mm.h"
#include "max_area.h"
#include "ued.h"
#include "userapi.h"
#include "md5.h"

static void near Strip_Bad_Stuff(byte *s,int m);
static int fAddUser=FALSE;

#define MAX_PLR 200   /* Max # of lread ptrs to be purged in one go */

int Init_Ued(void)
{
  if (!usr.video || (usr.bits2 & BITS2_CLS)==0)
  {
    Puts(ued_need_graphics_cls);
    Press_ENTER();
    return -1;
  }

  if (TermLength() < 24)
  {
    Puts(ued_need_more_lines);
    Press_ENTER();
    return -1;
  }

  changes=FALSE;
  fAddUser=FALSE;

  /* Allocate memory for some stuff */

  if ((find_name=malloc(PATHLEN * sizeof(char)))==NULL)
  {
    logit(log_nomem_del);
    return -1;
  }

  if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
  {
    cant_open(PRM(user_file));

    free(find_name);
    return -1;
  }

  DrawUserScreen();
  
  return 0;
}



/* Find the next available lastread pointer */

int Find_Next_Lastread(HUF huf)
{
  HUFFS huffs;
  int rc;

  Printf(searching_for_user);
  vbuf_flush();

  FindLR_Start(UserFileSize(huf));

  if ((huffs=UserFileFindSeqOpen(huf)) != NULL)
  {
    do
    {
      FindLR_AddOne(huffs->usr.lastread_ptr, huffs->usr.name);
    }
    while (UserFileFindSeqNext(huffs));

    UserFileFindSeqClose(huffs);
  }

  rc=FindLR_GetFreePtr();
  FindLR_Stop();

  Printf(searching_for_user_back);

  return rc;
}



/* Remove nasty characters from strings */

static void near Strip_Bad_Stuff(byte *s, int m)
{
  *(s+m)='\0';

  while (*s)
  {
    if (*s < ' ')
      *s=' ';

    s++;
  }
}



/* This is called whenever we read in a new user */

static void near Read_New_User(void)
{
  changes=FALSE;
  olduser=user;

  if (eqstri(user.name, usr.name))
    user=usr;

  Strip_Bad_Stuff(user.name, 35);
  Strip_Bad_Stuff(user.city, 35);

#ifdef CANENCRYPT
  if ((user.bits & BITS_ENCRYPT)==0)
#endif
    Strip_Bad_Stuff(user.pwd, 15);

  fAddUser=FALSE;
}

/* Go to the last user */

void UedLast(void)
{
  Update_User();
  UedFindLastUser();
}


/* Go to the next user */

void UedPlus(void)
{
  HUFF huff;
  struct _usr tryusr;
  int rc=FALSE;

  /* To make sure that Sysop can't accidentally "+++" his modem into *
   * command mode!                                                   */

  Putc('\r');

  Update_User();

  if ((huff=UserFileFindOpen(huf, user.name, NULL)) != NULL)
  {
    /* Save a copy of the first good user, in case we are at
     * eof and can't get a "next" user.
     */

    tryusr=huff->usr;

    rc=UserFileFindNext(huff, NULL, NULL);

    if (rc)
      user=huff->usr;
    else
      user=tryusr;

    UserFileFindClose(huff);
  }
  else
    UedFindFirstUser();

  Read_New_User();
}


/* Go to the prior user */

void UedMinus(void)
{
  HUFF huff;
  int rc=FALSE;

  Update_User();

  if ((huff=UserFileFindOpenR(huf, user.name, NULL)) != NULL)
  {
    rc=UserFileFindPrior(huff, NULL, NULL);

    if (rc)
      user=huff->usr;

    UserFileFindClose(huff);
  }

  if (!rc)
    UedFindFirstUser();

  Read_New_User();
}







void UedGetShowUlist(void)
{
  user.bits ^= BITS_NOULIST;
  changes=TRUE;
}

void UedGetDebit(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_debit);

  if (*temp)
    user.debit=atoi(temp);

  changes=TRUE;
}

void UedGetCredit(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_credit);

  if (*temp)
    user.credit=atoi(temp);

  changes=TRUE;
}


void UedGetAlias(void)
{
  char temp[PATHLEN];
  struct _usr usrtmp;

  InputGetsLL(temp, 20, ued_realname);

  if (*temp)
  {
    if (!eqstri(temp, olduser.name) && !eqstri(temp, olduser.alias) &&
        (UserFileFind(huf, temp, NULL, &usrtmp) ||
         UserFileFind(huf, NULL, temp, &usrtmp)))
    {
      Puts(ued_nameinuse);
      Press_ENTER();
    }
    else
    {
      /* A null alias */

      if (strcmp(temp, " ")==0)
        *temp=0;

      strcpy(user.alias, temp);
    }
  }

  changes=TRUE;
}

void UedGetCity(void)
{
  char temp[PATHLEN];

  InputGetsLL(temp, 35, ued_city);

  if (*temp)
    strcpy(user.city, temp);

  changes=TRUE;
}


void UedGetName(void)
{
  char temp[PATHLEN];
  struct _usr usrtmp;

  InputGetsLL(temp, 35, ued_uname);

  if (*temp)
  {
    if (!eqstri(temp, olduser.name) && !eqstri(temp, olduser.alias) &&
        (UserFileFind(huf, temp, NULL, &usrtmp) ||
         UserFileFind(huf, NULL, temp, &usrtmp)))
    {
      Puts(ued_nameinuse);
      Press_ENTER();
    }
    else
      strcpy(user.name, fancier_str(temp));
  }

  changes=TRUE;
}

void UedGetVoicePhone(void)
{
  char temp[PATHLEN];

  InputGetsLL(temp, 14, ued_phone);

  if (*temp)
    strcpy(user.phone, temp);

  changes=TRUE;
}


void UedGetDataPhone(void)
{
  char temp[PATHLEN];

  InputGetsLL(temp, 14, ued_phone);

  if (*temp)
    strcpy(user.dataphone, temp);

  changes=TRUE;
}


void UedGetSex(void)
{
  byte cmd;

  cmd=(byte)toupper(KeyGetRNP(ued_sex_prompt));

  if (cmd==ued_sex_keys[0])
    user.sex=SEX_MALE;
  else if (cmd==ued_sex_keys[1])
    user.sex=SEX_FEMALE;
  else if (cmd==ued_sex_keys[2])
    user.sex=SEX_UNKNOWN;

  changes=TRUE;
}


/* Get the user's date of bith */

void UedGetBday(void)
{
  char temp[PATHLEN];
  int yy, mm, dd;

  InputGetsLL(temp, 15, ued_dob_prompt, user.dob_year, user.dob_month, user.dob_day);

  if (*temp && sscanf(temp, "%4u-%u-%u", &yy, &mm, &dd)==3)
  {
    /* Support for users who forget the century */

    if (yy < 100)
      yy += 1900;

    user.dob_year=yy;
    user.dob_month=mm;
    user.dob_day=dd;
  }

  changes=TRUE;
}

void UedGetGroup(void)
{
  char temp[PATHLEN];

  InputGetsLL(temp, 6, ued_group_prompt);

  if (*temp)
    user.group=atoi(temp);

  changes=TRUE;
}

void UedGetPointCredit(void)
{
  char temp[PATHLEN];

  InputGetsLL(temp, 25, ued_pcredit_prompt);

  if (*temp)
    user.point_credit=atol(temp);

  changes=TRUE;
}

void UedGetPointDebit(void)
{
  char temp[PATHLEN];

  InputGetsLL(temp, 25, ued_pdebit_prompt);

  if (*temp)
    user.point_debit=atol(temp);

  changes=TRUE;
}


void UedGetHotkeys(void)
{
  user.bits ^= BITS_HOTKEYS;
  changes=TRUE;
}

void UedGetNerd(void)
{
  user.bits ^= BITS_NERD;
  changes=TRUE;
}

void UedGetTabs(void)
{
  user.bits ^= BITS_TABS;
  changes=TRUE;
}


void UedGetRIP(void)
{
  user.bits ^= BITS_RIP;
  changes=TRUE;
}


void UedGetIBMChars(void)
{
  user.bits2 ^= BITS2_IBMCHARS;
  changes=TRUE;
}


void UedGetPause(void)
{
  user.bits2 ^= BITS2_MORE;
  changes=TRUE;
}

void UedGetCalledBefore(void)
{
  user.bits2 ^= BITS2_CONFIGURED;
  changes=TRUE;
}

void UedGetScrnClr(void)
{
  user.bits2 ^= BITS2_CLS;
  changes=TRUE;
}

void UedGetAvailChat(void)
{
  user.bits ^= BITS_NOTAVAIL;
  changes=TRUE;
}

void UedGetFSR(void)
{
  user.bits ^= BITS_FSR;
  changes=TRUE;
}

void UedGetMaxed(void)
{
  user.bits2 ^= BITS2_BORED;
  changes=TRUE;
}


void UedGetWidth(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_width);

  if (*temp)
    user.width=(char)atoi(temp);

  changes=TRUE;
}

void UedGetLength(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_length);

  if (*temp)
    user.len=(char)atoi(temp);

  changes=TRUE;
}

void UedGetNulls(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_nulls);

  if (*temp)
    user.nulls=(byte)atoi(temp);

  changes=TRUE;
}

void UedGetMsgArea(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_lastmsg);

  if (*temp)
    SetAreaName(user.msg, temp);

  changes=TRUE;
}


void UedGetFileArea(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_lastfile);

  if (*temp)
    SetAreaName(user.files, temp);

  changes=TRUE;
}



/* Get a new language for the specified user */

void UedGetLanguage(void)
{
  int lang;

  Puts(CLS);

  if ((lang=Get_Language()) != -1)
  {
    user.lang=lang;
    changes=TRUE;
  }

  DrawUserScreen();
}


/* Get a default protocol selection */

void UedGetProtocol(void)
{
  sword proto;

  Puts(CLS);
  Puts(chose_default_proto);

  if (File_Get_Protocol(&proto, TRUE, FALSE) != -1)
    user.def_proto=proto;

  DrawUserScreen();
  changes=TRUE;
}



/* Get a default compressor selection */

void UedGetCompress(void)
{
  byte arc;

  Puts(CLS);

  if ((arc=Get_Archiver()) != 0)
    user.compress=arc;

  DrawUserScreen();
  changes=TRUE;
}

void UedGetDl(void)
{
  char temp[PATHLEN];

  /* Get the number of downloaded kilobytes */

  Inputf(temp, INPUT_WORD | INPUT_NOLF, 0, 0, ued_dlall);
  Puts(CLEOL);

  if (*temp)
    user.down=atol(temp);

  /* Now get the number of downloaded files */

  InputGets(temp, ued_ndlall);

  if (*temp)
    user.ndown=atol(temp);

  changes=TRUE;
}

void UedGetTodayDl(void)
{
  char temp[PATHLEN];

  Inputf(temp, INPUT_WORD | INPUT_NOLF, 0, 0, ued_dltoday);
  Puts(CLEOL);

  if (*temp)
    user.downtoday=atol(temp);

  InputGets(temp, ued_ndltoday);

  if (*temp)
    user.ndowntoday=atol(temp);

  changes=TRUE;
}


void UedGetUploads(void)
{
  char temp[PATHLEN];

  Inputf(temp, INPUT_WORD | INPUT_NOLF, 0, 0, ued_ulall);
  Puts(CLEOL);

  if (*temp)
    user.up=atol(temp);

  InputGets(temp, ued_nulall);

  if (*temp)
    user.nup=atol(temp);

  changes=TRUE;
}

void UedGetPostMsgs(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_msgsposted_prompt);

  if (*temp)
    user.msgs_posted=atol(temp);

  changes=TRUE;
}


void UedGet1stCall(void)
{
  SCOMBO scOld=user.date_1stcall;
  GetNewDateDiscrete(&user.date_1stcall, &scOld, ued_date1stcall_prompt,
                     NULL, NULL, NULL);
  changes=TRUE;
}

void UedGetCurTime(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_time);

  if (*temp)
    user.time=atoi(temp);

  changes=TRUE;
}


void UedGetAddedTime(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_timeadd_prompt);

  if (*temp)
    user.time_added=atoi(temp);

  changes=TRUE;
}

void UedGetNumCalls(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_calls);

  if (*temp)
    user.times=atoi(temp);

  changes=TRUE;
}


void UedGetReadMsgs(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_msgsread_prompt);

  if (*temp)
    user.msgs_read=atol(temp);

  changes=TRUE;
}

void UedGetPwdDate(void)
{
  SCOMBO scOld=user.date_pwd_chg;
  GetNewDateDiscrete(&user.date_pwd_chg, &scOld, ued_datepwdchg_prompt,
                     NULL, NULL, NULL);
  changes=TRUE;
}



/* Adjust the user's help level */

void UedGetHelp(void)
{
  byte cmd;

  cmd=(byte)toupper(KeyGetRNP(ued_gethelp));

  if (cmd==ued_help_keys[0])
    user.help=NOVICE;
  else if (cmd==ued_help_keys[1])
    user.help=REGULAR;
  else if (cmd==ued_help_keys[2])
    user.help=EXPERT;

  changes=TRUE;
}


/* Adjust the user's key settings */

void UedGetKeys(void)
{
  char temp[PATHLEN];
  char *p;
  
  changes=TRUE;
  Printf(ued_curkeys, Keys(user.xkeys));
  InputGets(temp, ued_keytoggle);

  for (p=cstrupr(temp); *p; p++)
    if (*p >= '1' && *p <= '8')
      user.xkeys ^= (1L << (*p-'1'));
    else if (*p >= 'A' && *p <= 'X')
      user.xkeys ^= (1L << ((*p-'A')+8));
}

/* Update the user's video mode */

void UedGetVideo(void)
{
  byte cmd;

  changes=TRUE;

  cmd=(byte)toupper(KeyGetRNP(ued_vidmode));

  if (cmd==ued_vid_keys[0])
    user.video=GRAPH_TTY;
  else if (cmd==ued_vid_keys[1])
    user.video=GRAPH_ANSI;
  else if (cmd==ued_vid_keys[2])
    user.video=GRAPH_AVATAR;
}


/* Change the user's password */

void UedGetPwd(void)
{
  char temp[PATHLEN];

  changes=TRUE;
  InputGetsM(temp, 15, usr_pwd);

  if (! *temp)
    if (GetyNAnswer(really_erase, 0)==NO)
      return;

  Get_Dos_Date(&user.date_pwd_chg);

  /* If the sysop entered no password, this is a guest account,
   * so set the password field to zeroes and turn off the encryption bit.
   */

  if (! *temp)
  {
    memset(user.pwd, 0, sizeof user.pwd);
#ifdef CANENCRYPT
    user.bits &= ~BITS_ENCRYPT;
#endif
    return;
  }

#if defined(MUSTENCRYPT) || defined(CHANGEENCRYPT)
  if (!(prm.flags2 & FLAG2_NOENCRYPT))
  {
    byte abMd5[MD5_SIZE];

    string_to_MD5(strlwr(temp), abMd5);

    memcpy(user.pwd, abMd5, sizeof user.pwd);
    user.bits |= BITS_ENCRYPT;
  }
  else
#endif
  {
    strcpy(user.pwd, cfancy_str(temp));
    user.bits &= ~BITS_ENCRYPT;
  }
}



/* Modify the user's priv level */

void UedGetPriv(void)
{
  char temp[PATHLEN];

  InputGets(temp, ued_getpriv);

  if (*temp)
  {
    word lvl;

    if (isdigit(*temp))
      lvl=(word)atol(temp);
    else
      lvl=ClassKeyLevel(toupper(*temp));

    user.priv=lvl;
    user.max2priv = max2priv(user.priv);
    changes=TRUE;
  }
}


/* Display help to the user */

void UedShowHelp(void)
{
  Display_File(0, NULL, "%suedhelp", PRM(misc_path));
  DrawUserScreen();
}



/* Read the first user in the user file */

void UedFindFirstUser(void)
{
  if (!UserFileFind(huf, NULL, NULL, &user))
  {
    logit(cantread, PRM(user_file));
    quit(ERROR_FILE);
  }

  Read_New_User();
}

/* Find the last user in the user file */

void UedFindLastUser(void)
{
  if (!UserFileSeek(huf,-1L,&user,sizeof usr))
    UedFindFirstUser();
  Read_New_User();
}


/* Purge the specified array of lastread pointers from the message area     *
 * lastread files.                                                          */

static void near PurgeLastreads(int *piOffset, int num_offset)
{
  char temp[PATHLEN];
  MAH ma;
  HAFF haff;
  long where, pos;
  int lrfile, i;
  dword zeroes;

  memset(&ma, 0, sizeof ma);
  Puts(ued_cleansing_lastreads);
  vbuf_flush();

  zeroes=0;

  if ((haff=AreaFileFindOpen(ham, NULL, 0))==NULL)
    return;

  while (AreaFileFindNext(haff, &ma, 0)==0)
  {
    if (! *MAS(ma, path))
      continue;

    Printf(CLEOL " %s\r", cfancy_str(MAS(ma, path)));
    vbuf_flush();

    sprintf(temp, (ma.ma.type & MSGTYPE_SDM) ? ps_lastread : sq_lastread,
            MAS(ma, path));

    if ((lrfile=shopen(temp, O_RDWR | O_BINARY | O_NOINHERIT))==-1)
      continue;

    /* Figure out where the end of the file is */

    pos=lseek(lrfile, 0L, SEEK_END);

    /* Now purge all of the lastread pointers */

    for (i=0; i < num_offset; i++)
    {
      int size;

      if (ma.ma.type & MSGTYPE_SDM)
        size=sizeof(word);
      else size=sizeof(UMSGID);

      where=(long)piOffset[i] * (long)size;

      if (where <= pos)
      {
        lseek(lrfile, where, SEEK_SET);
        write(lrfile, (char *)&zeroes, size);
      }
    }

    close(lrfile);
  }

  DisposeMah(&ma);
}


/* Delete the current user */

void UedDelete(void)
{
  if (eqstri(usr.name, user.name))
  {
    Puts(ued_nodel_current);
    Press_ENTER();
  }
  else
  {
    /* toggle delete bit */

    user.delflag ^= UFLAG_DEL;
    changes=TRUE;
  }
}






/* Write the current user to disk, if necessary */

void Update_User(void)
{
  if (changes)
  {
    /* If we're updating an existing user */

    if (! fAddUser)
    {
      if (UserFileUpdate(huf, olduser.name, olduser.alias, &user))
        changes=FALSE;
      else
        logit(cantwrite, PRM(user_file));
    }
    else
    {
      /* Otherwise, we must be adding a new user */

      if (UserFileCreateRecord(huf, &user, TRUE))
        changes=FALSE;
      else
        logit(cantwrite, PRM(user_file));
    }
  }


  /* Propagate any changes to the current user record */

  if (eqstri(usr.name, user.name))
  {
    byte old_video=usr.video;

    if (usr.priv != user.priv)
    {
      usr=user;
      Find_Class_Number();
    }
    else
    {
      usr=user;
    }

    /* Preserve the video setting (so that screen updates still work) */

    usr.video=old_video;
    usr.bits2 |= BITS2_CLS;

    SetUserName(&usr, usrname);
  }

  Read_New_User();
}



/* Add another user to the user file */

int Add_User(void)
{
  Update_User();

  Blank_User(&user);
  
  user.lastread_ptr=Find_Next_Lastread(huf);
  user.struct_len=sizeof(struct _usr)/20;
  changes=TRUE;

  DrawUserScreen();
  fAddUser=TRUE;
  return 0;
}


static int near try_match(struct _usr *pu, int exact)
{
  if ( (exact && (eqstri(pu->name, find_name) ||
                  eqstri(pu->alias, find_name))) ||
      (!exact && stristr(pu->name, find_name)) ||
      (!exact && stristr(pu->alias, find_name)) ||
      (!exact && stristr(pu->phone, find_name)))
  {
    return TRUE;
  }

  return FALSE;
}


/* Try to find a particular user in the user file */

void UedFindUser(int begin,int exact)
{
  HUFF huff;
  struct _usr u;
  int found=FALSE;
  int first=TRUE;

  Update_User();

  if (begin)
    InputGetsLL(find_name, PATHLEN-1, ued_find_who);

  if (*find_name)
  {
    if (exact)
      found=UserFileFind(huf, find_name, NULL, &u) ||
            UserFileFind(huf, NULL, find_name, &u);
    else
    {
      if (begin)
        huff=UserFileFindOpen(huf, NULL, NULL);
      else
        huff=UserFileFindOpen(huf, user.name, NULL);

      if (huff)
      {
        do
        {
          /* If this is the first time through the loop, and
           * we're not beginning a new search, then we have
           * just found the current user.  We want the next
           * user after this one, so we have to skip the
           * try_match call the first time so that we can
           * get the next user.
           */

          if (first && !begin)
            first=FALSE;
          else if (try_match(&huff->usr, exact))
          {
            u=huff->usr;
            found=TRUE;
            break;
          }
        }
        while (UserFileFindNext(huff, NULL, NULL));

        UserFileFindClose(huff);
      }
    }

    if (found)
    {
      user=u;
      Read_New_User();
    }
    else
    {
      Goto(PROMPT_LINE, 1);
      Puts(ued_not_found);
      Press_ENTER();
    }
  }
}


/* Set the current user's expiration date */

void UedGetExpireBy(void)
{
  byte cmd;

  changes=TRUE;

  cmd=(byte)toupper(KeyGetRNP(ued_get_expby));

  if (cmd==ued_get_exp[0])
  {
    user.xp_flag &= ~XFLAG_EXPMINS;
    user.xp_flag |= XFLAG_EXPDATE;
  }
  else if (cmd==ued_get_exp[1])
  {
    user.xp_flag &= ~XFLAG_EXPDATE;
    user.xp_flag |= XFLAG_EXPMINS;
  }
  else if (cmd==ued_get_exp[2])
    user.xp_flag &= ~(XFLAG_EXPMINS | XFLAG_EXPDATE);
}



/* Set the expiry action for this user */

void UedGetExpireAction(void)
{
  byte cmd;

  changes=TRUE;

  cmd=(byte)toupper(KeyGetRNP(ued_get_action));

  if (cmd==ued_get_actionk[0])
  {
    user.xp_flag &= ~XFLAG_DEMOTE;
    user.xp_flag |= XFLAG_AXE;
  }
  else if (cmd==ued_get_actionk[1])
  {
    char temp[PATHLEN];

    user.xp_flag &= ~XFLAG_AXE;
    user.xp_flag |= XFLAG_DEMOTE;

    Goto(PROMPT_LINE, 1);
    Puts(CLEOL);
    
    InputGets(temp, ued_getpriv);
    if (*temp)
    {
      word lvl;

      if (isdigit(*temp))
        lvl=(word)atol(temp);
      else lvl=ClassKeyLevel(toupper(*temp));

      if (lvl==(word)-1)
      {
        Puts(ued_invalidpriv);
        Press_ENTER();
      }
      else
      {
        user.xp_priv=lvl;
        changes=TRUE;
      }
    }
  }
  else if (cmd==ued_get_actionk[2])
    user.xp_flag &= ~(XFLAG_AXE | XFLAG_DEMOTE);
}


/* Set the expiry date for this user */

void UedGetExpireDate(void)
{
  char temp[PATHLEN];

  changes=TRUE;

  if (user.xp_flag & XFLAG_EXPDATE)
  {
    if (GetNewDateDiscrete(&user.xp_date, &user.xp_date, ued_get_date_prompt,
                           NULL, NULL, NULL)==-1)
    {
      Press_ENTER();
      DrawUserScreen();
    }
  }
  else
  {
    InputGets(temp, ued_minutes);

    if (*temp)
      user.xp_mins=atol(temp);
  }
}


/* Undo changes made to the current user */


void UedUndoChanges(void)
{
  user=olduser;
  changes=TRUE;
}


/* Returns TRUE if any other users are currently logged on */

static int near UFileBusy(void)
{
  FFIND *ff;
  char temp[PATHLEN];
  unsigned int node, rc=FALSE;

  sprintf(temp, active_star, original_path);

  if ((ff=FindOpen(temp, 0))==NULL)
    return FALSE;

  do
  {
    if (sscanf(cstrlwr(ff->szName), active_x_bbs, &node) != 1)
      continue; 

    if ((byte)node != task_num)
    {
      Printf(ued_cantdel1, node);
      Printf(ued_cantdel2, node);
      Puts(ued_cantdel3);
      Printf(ued_cantdel4, original_path, node, node);

      Press_ENTER();
      rc=TRUE;
      break;
    }
  }
  while (FindNext(ff)==0);

  FindClose(ff);

  return rc;
}


/* Purge users from the user file */

int UedPurgeUsers(void)
{
  char fname[PATHLEN];
  char fnameidx[PATHLEN];
  int purgelr[MAX_PLR];
  HUF hufnew;
  HUFF huff;
  int pn=0;


  Update_User();
  
  if (UFileBusy())
    return 0;

  Puts(ued_purging);
  vbuf_flush();

  if ((hufnew=UserFileOpen(user_poo, O_CREAT | O_TRUNC))==NULL)
  {
    cant_open(user_poo);
    return 0;
  }

  /* Read all users in current user file */

  if ((huff=UserFileFindOpen(huf, NULL, NULL)) != NULL)
  {
    do
    {
      Strip_Bad_Stuff(huff->usr.name, 35);

      if ((huff->usr.delflag & UFLAG_DEL) &&
          (!*usr.name || !eqstri(huff->usr.name, usr.name)))
      {
        Printf(ued_deleted, huff->usr.name);

        if (pn < MAX_PLR)
          purgelr[pn++]=huff->usr.lastread_ptr;

        vbuf_flush();
      }
      else
      {
        if (!UserFileCreateRecord(hufnew, &huff->usr, FALSE))
        {
          logit(cantwrite, user_poo);
          UserFileFindClose(huff);
          UserFileClose(hufnew);
          return 0;
        }
      }
    }
    while (UserFileFindNext(huff, NULL, NULL));

    UserFileFindClose(huff);
  }

  UserFileClose(hufnew);
  UserFileClose(huf);

  if (fexist(user_bak))
    unlink(user_bak);

  /* Delete the user index */

  strcpy(fnameidx, PRM(user_file));
  strcat(fnameidx, dotidx);

  unlink(fnameidx);

  strcpy(fname, PRM(user_file));
  strcat(fname, dotbbs);

  if (! move_file(fname, user_bak)==0)
    logit(cantmove, fname, user_bak);
  else if (! move_file(user_poo_bbs, fname)==0)
    logit(cantmove, user_poo_bbs, fname);
  else if (! move_file(user_poo_idx, fnameidx)==0)
    logit(cantmove, user_poo_idx, fnameidx);
  else
  {
    if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
      logit(cantopen, PRM(user_file));
    else
    {
      PurgeLastreads(purgelr, pn);

      Puts("\r" CLEOL);
      Puts(done_ex);
      Press_ENTER();

      DrawUserScreen();
      vbuf_flush();

      return 0;
    }
  }

  quit(ERROR_CRITICAL);
  return 0;
}



static char *lastread_used;

static int num_ptr,
           byt,
           usize;

void FindLR_Start(long usr_rec)
{
  usize=(int)usr_rec;

  num_ptr=usize+1;
  byt=max(1,num_ptr/CHAR_BITS);
  num_ptr=num_ptr-CHAR_BITS;

  if ((lastread_used=(char *)malloc(byt))==NULL)
  {
    logit(nomls);
    quit(ERROR_CRITICAL);
  }

  /* Initialize it... */
  memset(lastread_used, '\0', byt);
}


void FindLR_AddOne(int lr_ptr, char *pszName)
{
  int o_byt;

  if (lr_ptr >= num_ptr)
  {
    /* Make sure we don't really screw up via realloc() call... */

/*    if (lr_ptr >= 2048)
      return;*/

    o_byt=byt;

    usize=lr_ptr+50;
    num_ptr=usize+1;
    byt=max(1,num_ptr/CHAR_BITS);
    num_ptr=num_ptr-CHAR_BITS;

    if ((lastread_used=realloc(lastread_used, byt))==NULL)
    {
      logit(nomls);
      quit(ERROR_CRITICAL);
    }

    memset(lastread_used+o_byt, '\0', byt-o_byt);
  }

  /* If user's lastread pointer is cross-linked, warn SysOp */

  if (IsBit(lastread_used, lr_ptr))
    logit(log_lread_xlink, pszName, lr_ptr);
  else
    BitOn(lastread_used, lr_ptr);
}


int FindLR_GetFreePtr(void)
{
  int i;

  for (i=0; i < num_ptr; i++)
    if (! IsBit(lastread_used, i))
      break;

  return i;
}

void FindLR_Stop(void)
{
  free(lastread_used);
  lastread_used=NULL;
}



