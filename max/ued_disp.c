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
static char rcs_id[]="$Id: ued_disp.c,v 1.1.1.1 2002/10/01 17:53:20 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Internal user editor (screen-display routines)
*/

#define MAX_LANG_max_ued
#define MAX_INCL_COMMS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "mm.h"
#include "ued.h"
#include "ued_disp.h"

static int near MKD(void);


/*                1               2               3               4
   123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
1  User ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Last call                   ÄÄÄÄÄÄÄÄÄÄÄÄ¿
2   ³ Name      123456789012345678 Alias      1234567890123456789012345         ³
3   ³ City      Kingston           VoicePhone 123456789012345 Sex  Female       ³
4   À Password  1234567890123456   DataPhone  (613)634-3058   Bday 74-03-24     Ù
5  A)ccess ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿    S)ettings ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
6   ³ Priv level AsstSysop   Credit    65500   ³     ³ Width      80            ³
7   ³ Keys       12345678901 Debit     7952    ³     ³ Length     25            ³
8   ³ Group Num  0           Used Pts  12345678³     ³ Nulls      0             ³
9   ³ Alloc Pts  1234567890  ShowUlist YES     ³     ³ Msg Area   MAX.MUFFIN    ³
a   À Nerd       NO                            Ù     ³ File Area  12345678901234³
b  I)nformation ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ ³ Video Mode AVATAR        ³
c   ³ DL (all) 800K / 52222    Cur. time  12345    ³ ³ Help       REGULAR       ³
d   ³ Today DL 0K / 0          Added time 12345    ³ ³ !Language  English       ³
e   ³ Uploads  1234567K / 2233 # calls    7723     ³ ³ Protocol   Zmodem        ³
f   ³ PostMsgs 1234            ReadMsgs   22       ³ À Compress   1234567890123 Ù
10  À 1stCall  03/22/94        !PwdDate   03/22/94 Ù
11 F)lags ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ E)xpiry ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
12  ³ Hotkeys YES  IBM Chars    YES  ScrnClr   YES ³  ³ Expire by 1234567890    ³
13  ³ MaxEd   YES  Pause (More) YES  AvailChat  NO ³  ³ Action    12345678901234³
14  ³ Tabs    YES  CalledBefore YES  FSR       YES ³  À Date      NONE          Ù
15  À RIP      NO                                  Ù
16
17  Select: Aasdf)
18          Foo)
*/


static char * near Expire_By(struct _usr *user)
{
  if (user->xp_flag & XFLAG_EXPDATE)
    return ued_xp_date;
  else if (user->xp_flag & XFLAG_EXPMINS)
    return ued_xp_mins;
  else
    return ued_xp_none;
}

static char * near Expire_Action(struct _usr *user,char *temp)
{
  if (user->xp_flag & XFLAG_DEMOTE)
  {
    char ptmp[16];

    sprintf(temp, ued_xp_demote, privstr(user->xp_priv,ptmp));
    return temp;
  }

  if (user->xp_flag & XFLAG_AXE)
    return ued_xp_hangup;

  return ued_xp_none;
}

static char * near Expire_At(struct _usr *user,char *temp)
{
  if (user->xp_flag & XFLAG_EXPDATE)
  {
    FileDateFormat(&user->xp_date, temp);
    return temp;
  }

  if (user->xp_flag & XFLAG_EXPMINS)
  {
    sprintf(temp, ued_xp_minutes, user->xp_mins);
    return temp;
  }

  return ued_xp_none;
}



/* Display all of the ued_ssxxx strings from the language file */

void DrawUserScreen(void)
{
  int i;
  char *str;

  for (i=0; *(str=s_ret(n_ued_ss1+i)) != 0; i++)
    Puts(str);
}



static char * near Sex(byte sex)
{
  switch (sex)
  {
    case SEX_MALE:    return sex_male;
    case SEX_FEMALE:  return sex_female;
    default:          return sex_unknown;
  }
}




/* Display the user's date of birth in a sensible format */

static char * near DOB(struct _usr *pusr, char *out)
{
  switch (prm.date_style)
  {
    default:
      sprintf(out,
              date_str,
              pusr->dob_month,
              pusr->dob_day,
              pusr->dob_year % 100);
      break;

    case 1:
      sprintf(out,
              date_str,
              pusr->dob_day,
              pusr->dob_month,
              pusr->dob_year % 100);
      break;

    case 2:
    case 3:
      sprintf(out,
              prm.date_style==2 ? date_str : datestr,
              pusr->dob_year % 100,
              pusr->dob_month,
              pusr->dob_day);
  }

  return out;
}


/* Display a user record on-scren */

void DisplayUser(void)
{
  struct _arcinfo *ar;
  char temp[125];
  char pwd[17];
  char ptmp[12];

  Puts(WHITE);

  Printf(ued_spermflag,
            eqstri(usr.name, user.name) ? ued_sstatcur
          : (user.delflag & UFLAG_DEL)  ? ued_sstatdel
          : (user.delflag & UFLAG_PERM) ? ued_sstatprm
                                        : ued_sstatblank);

  if (MKD()) goto Dump;
  Printf(ued_slastcall,    sc_time(&user.ludate, temp));
  if (MKD()) goto Dump;
  Printf(ued_sname,        user.name);
  if (MKD()) goto Dump;
  Printf(ued_scity,        user.city);
  if (MKD()) goto Dump;
#ifdef CANENCRYPT
  Printf(ued_spwd,         Show_Pwd((user.bits & BITS_ENCRYPT) ? brackets_encrypted : user.pwd, pwd, (char)(disp_pwd ? 0 : '.')));
#else
  Printf(ued_spwd,         Show_Pwd(user.pwd, pwd, (char)(disp_pwd ? 0 : '.')));
#endif
  if (MKD()) goto Dump;
  Printf(ued_salias,       user.alias);
  if (MKD()) goto Dump;
  Printf(ued_svoicephone,  user.phone);
  if (MKD()) goto Dump;
  Printf(ued_sdataphone,   user.dataphone);
  if (MKD()) goto Dump;
  Printf(ued_ssex,         Sex(user.sex));
  if (MKD()) goto Dump;
  Printf(ued_sdob,         DOB(&user, temp));
  if (MKD()) goto Dump;
  Printf(ued_spriv,        privstr(user.priv,ptmp));
  if (MKD()) goto Dump;
  Printf(ued_skeys,        Keys(user.xkeys));
  if (MKD()) goto Dump;
  Printf(ued_sgroup,       user.group);
  if (MKD()) goto Dump;
  Printf(ued_sallocpts,    user.point_credit);
  if (MKD()) goto Dump;
  Printf(ued_snerd,        Yes_or_No((user.bits  & BITS_NERD)));
  if (MKD()) goto Dump;
  Printf(ued_scredit,      user.credit);
  if (MKD()) goto Dump;
  Printf(ued_sdebit,       user.debit);
  if (MKD()) goto Dump;
  Printf(ued_susedpts,     user.point_debit);
  if (MKD()) goto Dump;
  Printf(ued_sulistshow,   Yes_or_No(!(user.bits & BITS_NOULIST)));
  if (MKD()) goto Dump;

  sprintf(temp, ued_sxfertemplate, user.down, user.ndown);
  Printf(ued_sdlall, temp);
  if (MKD()) goto Dump;

  sprintf(temp, ued_sxfertemplate, user.downtoday, user.ndowntoday);
  Printf(ued_sdltoday,     temp);
  if (MKD()) goto Dump;

  sprintf(temp, ued_sxfertemplate, user.up, user.nup);
  Printf(ued_sup,          temp);
  if (MKD()) goto Dump;

  Printf(ued_sposted,      user.msgs_posted);
  if (MKD()) goto Dump;

  CreateDate(temp, &user.date_1stcall);
  Printf(ued_s1stcall,     temp);
  if (MKD()) goto Dump;

  Printf(ued_stimetoday,   user.time);
  if (MKD()) goto Dump;
  Printf(ued_stimeadded,   user.time_added);
  if (MKD()) goto Dump;
  Printf(ued_stimes,       user.times);
  if (MKD()) goto Dump;
  Printf(ued_sreadmsgs,    user.msgs_read);
  if (MKD()) goto Dump;

  CreateDate(temp, &user.date_pwd_chg);
  Printf(ued_spwdchg,      temp);
  if (MKD()) goto Dump;

  Printf(ued_swidth,       user.width);
  if (MKD()) goto Dump;
  Printf(ued_slength,      user.len);
  if (MKD()) goto Dump;
  Printf(ued_snulls,       user.nulls);
  if (MKD()) goto Dump;
  Printf(ued_slastmarea,   user.msg);
  if (MKD()) goto Dump;
  Printf(ued_slastfarea,   user.files);
  if (MKD()) goto Dump;
  Printf(ued_svideo,       Graphics_Mode(user.video));
  if (MKD()) goto Dump;
  Printf(ued_shelp,        Help_Level(user.help));
  if (MKD()) goto Dump;
  Printf(ued_slang,        PRM(lang_file[user.lang]));
  if (MKD()) goto Dump;
  Printf(ued_sproto,       Protocol_Name(user.def_proto, temp));
  if (MKD()) goto Dump;

  ar=UserAri(user.compress);
  Printf(ued_scompress,    ar ? ar->arcname : proto_none);

  if (MKD()) goto Dump;
  Printf(ued_shotkeys,     Yes_or_No((user.bits  & BITS_HOTKEYS)));
  if (MKD()) goto Dump;
  Printf(ued_smaxed,       Yes_or_No(!(user.bits2& BITS2_BORED)));
  if (MKD()) goto Dump;
  Printf(ued_stabs,        Yes_or_No((user.bits  & BITS_TABS)));
  if (MKD()) goto Dump;
  Printf(ued_srip,         Yes_or_No((user.bits  & BITS_RIP)));
  if (MKD()) goto Dump;
  Printf(ued_sibmchars,    Yes_or_No((user.bits2 & BITS2_IBMCHARS)));
  if (MKD()) goto Dump;
  Printf(ued_spause,       Yes_or_No((user.bits2 & BITS2_MORE)));
  if (MKD()) goto Dump;
  Printf(ued_scalledbefore,Yes_or_No((user.bits2 & BITS2_CONFIGURED)));
  if (MKD()) goto Dump;
  Printf(ued_sscrnclr,     Yes_or_No((user.bits2 & BITS2_CLS)));
  if (MKD()) goto Dump;
  Printf(ued_schatavail,   Yes_or_No(!(user.bits & BITS_NOTAVAIL)));
  if (MKD()) goto Dump;
  Printf(ued_sfsr,         Yes_or_No((user.bits  & BITS_FSR)));
  if (MKD()) goto Dump;
  Printf(ued_sexpireby,    Expire_By(&user));
  if (MKD()) goto Dump;
  Printf(ued_sexpireact,   Expire_Action(&user, temp));
  if (MKD()) goto Dump;
  Printf(ued_sexpiredate,  Expire_At(&user, temp));
  return;

Dump:

  mdm_dump(DUMP_OUTPUT);
  ResetAttr();
  return;
}


static int near MKD(void)
{
  if (Mdm_keyp() && (usr.bits & BITS_HOTKEYS))
    return TRUE;
  else
    return FALSE;
}


/* Display the password, optionally echoing '.'s for each character */

char * Show_Pwd(char *pwd,char *ret,char echo)
{
  char *s, *p;

  for (s=pwd,p=ret;*s;s++)
    if (echo)
      *p++=echo;
    else *p++=*s;

  *p='\0';

  return ret;
}



