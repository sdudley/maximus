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
static char rcs_id[]="$Id: max_chng.c,v 1.1.1.1 2002/10/01 17:51:31 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Change Setup menu options
*/

#define MAX_LANG_max_bor
#define MAX_LANG_max_chng

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "mm.h"
#include "arc_def.h"
#include "md5.h"

static int near Invalid_City(char *usr_city);
static int near Invalid_Name(char *usr_name);
static int near Invalid_Phone(char *usr_phone);

static void near Chg_RIP(void)
{
  usr.bits ^= BITS_RIP;

  if (usr.bits & BITS_RIP)
  {
    usr.bits  |= (BITS_HOTKEYS|BITS_FSR);

    usr.bits2 |= BITS2_CLS;

    if (!autodetect_rip())
      logit(log_rip_enabled_ndt);
  }
  else
  {
    RipReset();
    Puts("\r!|*");
  }

  Set_Lang_Alternate(hasRIP());
}

static void near Chg_Hotkeys(void)
{
  if (usr.bits & BITS_RIP)      /* Hitkeys must be left on in this case */
    usr.bits |= BITS_HOTKEYS;
  else
    usr.bits ^= BITS_HOTKEYS;
}

static void near Chg_FSR(void)
{
  usr.bits ^= BITS_FSR;

  if (usr.bits & BITS_RIP)      /* FSR must be left on in this case */
  {
    if ((usr.bits & BITS_FSR)==0)
    {
      usr.bits |= BITS_FSR;

      Puts(chg_need_fsr);
      Press_ENTER();
    }
  }

  if (usr.bits & BITS_FSR)
  {
    if (usr.video==GRAPH_TTY)   /* FSR requires graphics */
      usr.bits &= ~BITS_FSR;
    else
      usr.bits2 |= BITS2_MORE;  /* Also requires more prompting */
  }
}

static void near Chg_Userlist(void)
{
  usr.bits ^= BITS_NOULIST;
}

static void near Chg_Protocol(void)
{
  sword protocol;

  Puts(chose_default_proto);
  usr.def_proto = (byte)((File_Get_Protocol(&protocol, TRUE, FALSE)==-1)
                     ? PROTOCOL_NONE : protocol);
}

static void near Chg_Ibm(void)
{
  usr.bits2 ^= BITS2_IBMCHARS;
}

static void near Chg_Clear(void)
{
  if (usr.bits & BITS_RIP)
    usr.bits2 += BITS2_CLS;
  else
    usr.bits2 ^= BITS2_CLS;
}

static void near Chg_More(void)
{
  usr.bits2 ^= BITS2_MORE;
  
  /* User can't have both FSR and More on at the same time. */

  if ((usr.bits2 & BITS2_MORE)==0 && (usr.bits & BITS_FSR))
    usr.bits &= ~BITS_FSR;
}

static void near Chg_Tabs(void)
{
  usr.bits ^= BITS_TABS;
}


static void near Chg_Password(void)
{
  char string[BUFLEN];        /* string entered by the user */
#ifdef CANENCRYPT
  byte abMd5[MD5_SIZE];       /* MD5 of string entered by user */
#endif
  int fMatch;                 /* TRUE if correct pwd entered */
  int tries;                  /* Number of invalid passwords entered */

  tries=0;

  logit(log_ch_pwd);

  WhiteN();

  for (;;)
  {
    if (tries != 0)
    {
      Clear_KBuffer();
      logit(log_inv_pwd, string);
      Printf(wrong_pwd, tries);
      Putc('\n');

      if (tries==3)
      {
        logit(l_invalid_pwd);
        Puts(invalid_pwd);
        ci_ejectuser();
        mdm_hangup();
      }
    }

    *string='\0';

    while (! *string)
    {
      InputGetsLLe(string,BUFLEN,'.',current_pwd);

      if (tries==0 && ! *string)                /* Abort if first attempt */
        return;
    }

#ifdef CANENCRYPT
    if (usr.bits & BITS_ENCRYPT)
    {
      string_to_MD5(strlwr(string), abMd5);

      fMatch = (memcmp(abMd5, usr.pwd, MD5_SIZE)==0);
    }
    else
#endif
      fMatch = (stricmp(cfancy_str(string), usr.pwd)==0);

    if (fMatch)
    {
      Get_Pwd();
      *string='\0';
      break;
    }

    tries++;
  }
}



static void near Chg_Help(void)
{
  char *hk=help_keys;
  int ch;

  ch='\x00';

  while (ch != hk[0] && ch != hk[1] && ch != hk[2])
  {
    if (! *linebuf)
      Puts(help_menu);

    ch=toupper(KeyGetRNP(help_prompt));

    if (ch==hk[0]) /* novice */
      usr.help=NOVICE;
    else if (ch==hk[1])  /* regular */
      usr.help=REGULAR;
    else if (ch==hk[2])  /* expert */
      usr.help=EXPERT;
    else Clear_KBuffer();
  }
}


static void near Chg_Nulls(void)
{
  char string[BUFLEN];

  WhiteN();

  InputGets(string, num_nulls);

  if ((usr.nulls=(byte)atoi(string)) > 200)
    usr.nulls=0;
}


static void near Chg_Width(void)
{
  char string[BUFLEN];
  extern int loc_cols;

  WhiteN();

  for (;;)
  {
    InputGets(string, mon_width);

    usr.width=(byte)atoi(string);

    if (usr.width < 20 || usr.width > 132)
    {
      usr.width=80;
      Puts(bad_width);
      Clear_KBuffer();
    }
    else
    {
      if (! *linebuf)
        Printf(draw_line, usr.width);

      if (GetYnAnswer(check_x, 0)==YES)
      {
        if (local)
          loc_cols = usr.width;

        return;
      }
      else
      {
        Puts(incorrect_width);
        Clear_KBuffer();
      }
    }
  }
}


static void near Chg_Length(void)
{
  char string[BUFLEN];
  extern int loc_rows;
  byte x;

  if (hasRIP())
    Puts("\r!|*|#|#|#\n");  /* Kludge, but required */

  WhiteN();

  if (! *linebuf)
    for (x=CHANGE_SCREENLEN;x >= 2;x--)
      Printf("%d\n", x);

  x=usr.help;       /* Temporarily change help level, so HFLASH doesn't */
  usr.help=NOVICE;  /* screw us up! */

  InputGets(string, top_num);

  usr.help=x;

  if ((usr.len=(byte)atoi(string)) < 8)
    usr.len=8;
  else if (usr.len > 200)
    usr.len=200;

  if (local)
    loc_rows = usr.len;
}



static void near Chg_Graphics(void)
{
  int ch;

  ch=0;

  if (local || baud >= prm.speed_graphics)
  {
    char *vk=video_keys;

    while (ch != vk[0] && ch != vk[1] && ch != vk[2])
    {
      if (! *linebuf)
        Puts(video_menu);

      ch=toupper(KeyGetRNP(select_p));

      if (ch==vk[0])
        usr.video=GRAPH_TTY;
      else if (ch==vk[1])
        usr.video=GRAPH_ANSI;
      else if (ch==vk[2])
        usr.video=GRAPH_AVATAR;
      else Clear_KBuffer();
    }
  }
  else if ((unsigned int)prm.speed_graphics==38400u)
  {
    Puts(no_colour);
    Press_ENTER();
  }
  else
  {
    Printf(col_too_slow, prm.speed_graphics);
    Press_ENTER();
  }
}



static void near Chg_Edit(void)
{
  if (usr.video==GRAPH_TTY && (usr.bits2 & BITS2_BORED))
  {
    Puts(req_graph);
    Press_ENTER();
  }
  else if ((prm.flags & FLAG_no_magnet) && (usr.bits2 & BITS2_BORED))
  {
    Puts(unavailable);
    Press_ENTER();
  }
  else usr.bits2 ^= BITS2_BORED;
}



void Chg_City(void)
{
  char temp[PATHLEN];

  do
  {
    *linebuf='\0';
    WhiteN();

    InputGetsL(temp, 35, enter_city);
  }
  while (Invalid_City(temp));

  strcpy(usr.city, fancier_str(temp));
}

void Chg_Alias(void)
{
  char temp[PATHLEN];

  do
  {
    int l;

    *linebuf='\0';
    WhiteN();

    InputGetsL(temp, sizeof(usr.alias)-1, enter_name);

    /* Strip trailing blanks from the alias entered by the user */

    l = strlen(temp);

    while (l && temp[l-1]==' ')
      temp[--l]=0;

    /* If nothing left, default to the user's name */

    if (! *temp)
      strnncpy(temp, usr.name, sizeof(usr.alias)-1);
  }
  while (Invalid_Name(temp));

  temp[sizeof(usr.alias)-1]='\0';

  cfancy_str(temp);

  strncpy(usr.alias, *temp ? temp : usr.name, sizeof(usr.alias)-1);
  usr.alias[sizeof(usr.alias)-1]='\0';

  SetUserName(&usr, usrname);
}

void Chg_Phone(void)
{
  char temp[PATHLEN];

  do
  {
    *linebuf='\0';
    WhiteN();

    InputGetsL(temp, 50, enter_phone);
  }
  while (Invalid_Phone(temp));

  temp[14]='\0';
  strcpy(usr.phone, temp);
}




static int near Invalid_City(char *usr_city)
{
  char *p;

  for (p=usr_city;*p;p++)
    if (isalpha(*p))
      break;

  if (isblstr(usr_city) || *p=='\0')
  {
    Puts(cantskip);
    return TRUE;
  }

  return FALSE;
}


static int near Invalid_Name(char *usr_name)
{
  char *p;

  for (p=usr_name;*p;p++)
  {
    if (isalpha(*p) ||
        isdigit(*p) ||
        CharsetSwedish(usr_name, p) || 
        CharsetChinese(usr_name,p))
    {
      break;
    }
  }

  /* Names need at least one alnum */

  if (! *p)
    return TRUE;

  if (*p && !eqstri(usr_name, usr.name) && !eqstri(usr_name, usr.alias) &&
           (IsInUserList(usr_name, FALSE) || Bad_Word_Check(usr_name)))
  {
    Puts(already_used);
    return TRUE;
  }

  return FALSE;
}


static int near Invalid_Phone(char *usr_phone)
{
  char szNewPhone[PATHLEN];
  char *p, *s;

  /* Strip all spaces if the number is too long */

  if (strlen(usr_phone) <= 14)
    strcpy(szNewPhone, usr_phone);
  else
  {
    for (p=usr_phone, s=szNewPhone; *p; p++)
      if (! isspace(*p))
        *s++=*p;

    *s='\0';
  }

  /* If it's still too long, complain. */

  if (strlen(szNewPhone) > 14)
  {
    Puts(ph_too_long);
    return TRUE;
  }

  /* Now check to see if it contains any alpha chars... */

  for (p=szNewPhone; *p; p++)
    if (isalpha(*p))
      break;

  for (s=szNewPhone; *s; s++)
    if (isdigit(*s) && *s != '-')
      break;

  if (strlen(szNewPhone) < 4 || *p != '\0' || *s=='\0' ||
      strchr(szNewPhone, '>') || strchr(szNewPhone, '<'))
  {
    Puts(inv_phone);
    return TRUE;
  }

  /* Asshole detector */
  if (stristr(szNewPhone, "555-1212") || stristr(szNewPhone, "5551212"))
  {
    Puts(cantskip);
    return TRUE;
  }

  strcpy(usr_phone, szNewPhone);
  return FALSE;
}


static void near Chg_Archiver(void)
{
  byte a=Get_Archiver();
  
  if (a)
    usr.compress=a;
}


/* Get the user to select an archiver */

byte Get_Archiver(void)
{
  char temp[PATHLEN];
  struct _arcinfo *ar;
  byte compressor, cn;
  
  Load_Archivers();

  if (!ari)
    return 0;

  do
  {
    if (*linebuf=='\0')
    {
      Puts(select_def_archiver);
    
      for (cn=1, ar=ari; ar; ar=ar->next, cn++)
        Printf(list_option, cn, ar->arcname);

      WhiteN();
    }
    
    InputGets(temp,select_p);

    compressor=(byte)atoi(temp);
    
    if (!compressor)
      return usr.compress;
  }
  while (compressor==0 ||
         compressor > MAX_ARI || 
         UserAri(compressor)==NULL);

  return compressor;
}


int Exec_Change(int type, char **result)
{
  *result=NULL;

  switch (type)
  {
    case chg_city:      Chg_City();     break;
    case chg_password:  Chg_Password(); break;
    case chg_help:      Chg_Help();     break;
    case chg_nulls:     Chg_Nulls();    break;
    case chg_width:     Chg_Width();    break;
    case chg_length:    Chg_Length();   break;
    case chg_tabs:      Chg_Tabs();     break;
    case chg_more:      Chg_More();     break;
    case chg_video:     Chg_Graphics(); break;
    case chg_editor:    Chg_Edit();     break;
    case chg_clear:     Chg_Clear();    break;
    case chg_ibm:       Chg_Ibm();      break;
    case chg_phone:     Chg_Phone();    break;
    case chg_realname:  Chg_Alias();    break;
    case chg_hotkeys:   Chg_Hotkeys();  break;
    case chg_language:  Chg_Language(); break;
    case chg_userlist:  Chg_Userlist(); break;
    case chg_protocol:  Chg_Protocol(); break;
    case chg_fsr:       Chg_FSR();      break;
    case chg_archiver:  Chg_Archiver(); break;
    case chg_rip:       Chg_RIP();      break;
    default:            logit(bad_menu_opt, type);  return 0;
  }
  
  return 0;
}


