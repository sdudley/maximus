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
static char rcs_id[]="$Id: cvtusr.c,v 1.1.1.1 2002/10/01 17:57:17 sdudley Exp $";
#pragma on(unreferenced)

/*# name=USER.BBS conversion utility
*/

#define MAX_INCL_VER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mem.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "max.h"
#include "max_oldu.h"
#include "cvtusr.h"
#include "userapi.h"


char user_name[]="user";
char user_bbs[]="user.bbs";
char user_idx[]="user.idx";
char user_bak[]="user.bak";
char user_102[]="user.102";
char user_200[]="user.200";
char user_dol[]="user$$$";
char user_poo[]="user$$$.bbs";
char user_poo2[]="user$$$.idx";
char user_dat[]="user.dat";
char users_bbs[]="users.bbs";
char rec_num[]="Record %04d\r";

void _fast Adjust_User_Record(struct _usr *user);

int _stdc c_main(int argc,char *argv[])
{
  int x;

  Hello("CVTUSR", "Maximus User File Conversion Utility", VERSION,
        "1989, " THIS_YEAR);

  if (argc < 2)
    Format();

  for (x=1;x < argc;x++)
  {
    if (*argv[x]=='-' || *argv[x]=='/')
    {
      switch(tolower(argv[x][1]))
      {
        case 'd':
          Dump(argv[x]+2);
          break;

        case '3':
      /*  Convert_Max30a1(); */
          Convert_MaxPriv();
          break;

        case 'o':
          Convert_Opus110();
          break;

        case 'q':
          Convert_Quick();
          break;

        case 'l':
          Convert_Lread();
          break;

        case 'n':
          Convert_Max102();
          break;

        case 'p':
          Convert_Max200();
          break;

        case 's':
          Reverse_Max300();
          break;

        default:
         printf("\aInvalid command-line parameter:  `%s'!\n",argv[x]);
         Format();
      }
    }
    else
    {
      printf("\aInvalid command-line parameter:  `%s'!\n",argv[x]);
      Format();
    }
  }

  return 0;
}




void Format(void)
{
  printf("Command-line format:\n\n");

  printf("  CVTUSR [[-x...]\n\n");

  printf("where \"-x\" is one of the following switches:\n\n");

  printf("  -q        Convert a QBBS/SBBS/RA USERS.BBS file to Max " VERSION " format.\n");
  printf("  -o        Convert Opus 1.1 (and some Opus 1.7) USER.DAT to Max " VERSION " format.\n");
  printf("  -n        Convert a Max 1.0x user file to Max " VERSION ".\n");
  printf("  -p        Convert a Max 2.0x user file to Max " VERSION ".\n");
  printf("  -s        Swap the 'alias' and 'name' fields in a Max " VERSION " user file.\n");
  printf("  -l        Fix the lastread pointers in a Max " VERSION " user file.\n");

  exit(1);
}

word newpriv(word oldpriv)
{
  switch (oldpriv)
  {
    case HIDDEN:    return (word)-1;
    case TWIT:      return 0;
    case DISGRACE:  return 10;
    case LIMITED:   return 20;
    case NORMAL:    return 30;
    case WORTHY:    return 40;
    case PRIVIL:    return 50;
    case FAVORED:   return 60;
    case EXTRA:     return 70;
    case CLERK:     return 80;
    case ASSTSYSOP: return 90;
    case SYSOP:     return 100;
  }
  return oldpriv;
}



void Convert_Opus110(void)
{
  struct _usr110 usr110;
  struct _usr usr;

  int infile, num;
  HUF huf;



  Check_If_Exist();

  if ((infile=open(user_dat, O_RDONLY | O_BINARY))==-1)
  {
    printf("Error opening USER.DAT for read!  Aborting...\n");
    exit(1);
  }

  if ((huf=UserFileOpen(user_name, O_CREAT))==NULL)
  {
    printf("Error opening USER.DB for write!  Aborting...\n");
    exit(1);
  }

  for (num=0; read(infile, (char *)&usr110, sizeof usr110)==sizeof usr110; )
  {
    Blank_User(&usr);

    strcpy(usr.name,usr110.name);
    strcpy(usr.alias,usr110.alias);

    strcpy(usr.city,usr110.city);
    strcpy(usr.pwd,usr110.pwd);
    strcpy(usr.phone,usr110.usrtel);

    usr.times=usr110.times;

    if (usr110.help==HITECH)
      usr.help=NOVICE;
    else usr.help=usr110.help;

    if (usr110.tabs)
      usr.bits |= BITS_TABS;
    else usr.bits &= ~BITS_TABS;
    
    usr.nulls=(byte)usr110.nulls;

    sprintf(usr.msg,"%d",usr110.msg);
    sprintf(usr.files,"%d",usr110.files);

    if ((usr110.Bits & NO_IBMCHAR)==0)
      usr.bits2 |= BITS2_IBMCHARS;

    if (usr110.Bits & AVATAR)
      usr.video=GRAPH_AVATAR;
    else if (usr110.Bits & ANSI)
      usr.video=GRAPH_ANSI;
    else usr.video=GRAPH_TTY;

    if (usr110.Bits & USE_LORE)
      usr.bits2 |= BITS2_BORED;
    else usr.bits2 &= ~BITS2_BORED;

    if (usr110.Bits & MORE_PROMPT)
      usr.bits2 |= BITS2_MORE;
    else usr.bits2 &= ~BITS2_MORE;

    if (usr110.Bits & CONFIG_SET)
      usr.bits2 |= BITS2_CONFIGURED;
    else usr.bits2 &= ~BITS2_CONFIGURED;

    if (usr110.Bits & FORMFEED)
      usr.bits2 |= BITS2_CLS;
    else usr.bits2 &= ~BITS2_CLS;
    
    usr.xkeys=usr110.ClassLock;
    usr.max2priv = OpusToMaxPriv(usr110.ClassPriv);
    usr.priv=newpriv(usr.max2priv);
    usr.struct_len=sizeof(struct _usr)/20;

    Get_Dos_Date(&usr.ludate);
    
    usr.time=usr110.time;
    usr.up=usr110.upld;
    usr.down=usr110.dnld;
    usr.downtoday=(long)usr110.dnldl;

    usr.width=80;
    usr.len=24;
    usr.credit=usr110.credit;
    usr.debit=usr110.debit;

    usr.lastread_ptr=num;

    if ((++num % 32)==0)
    {
      printf(rec_num, num);
      fflush(stdout);
    }

    if (! UserFileCreateRecord(huf, &usr, TRUE))
      printf("Error writing user record for '%s'\n", usr.name);
  }

  printf(rec_num, num);
  printf("\nDone!\n");

  close(infile);
  UserFileClose(huf);
}


void Dump(char *find)
{
  HUF huf;
  HUFF huff;

  if ((huf=UserFileOpen(user_name, 0))==NULL)
  {
    printf("Error opening USER.DB for read!\n");
    exit(1);
  }


  printf("Name                  City                      Priv   Lread\n");
  printf("--------------------- ------------------------- ------ -----\n");

  if ((huff=UserFileFindOpen(huf,
                             *find ? find : NULL,
                             *find ? find : NULL)) != NULL)
  {
    do
    {
      printf("%-21.21s %-25.25s %-6u %d\n",
             huff->usr.name,
             huff->usr.city,
             huff->usr.priv,
             huff->usr.lastread_ptr);

//      if (*find)
//        break;
    }
    while (UserFileFindNext(huff, NULL, NULL));

    UserFileFindClose(huff);
  }

  UserFileClose(huf);
}



void Convert_MaxPriv(void)
{
  HUF huf;
  HUFF huff;
  int num=0;

  if ((huf=UserFileOpen(user_name, O_RDWR))==NULL)
  {
    printf("Error opening user.bbs for write!\n");
    exit(1);
  }

  if ((huff=UserFileFindOpen(huf,NULL,NULL))==NULL)
  {
    printf("Error attemping user find\n");
    UserFileClose(huf);
    exit(1);
  }


  do
  {
    struct _usr *pusr=&huff->usr;

    if ((++num % 32)==0)
    {
      printf(rec_num, num);
      fflush(stdout);
    }

    pusr->priv=newpriv(pusr->max2priv);

    if (pusr->xp_flag & XFLAG_DEMOTE)
      pusr->xp_priv=newpriv(pusr->xp_priv);

    if (!UserFileUpdate(huf, pusr->name, pusr->alias, pusr))
      printf("Error writing record for '%s'\n", pusr->name);
  }
  while (UserFileFindNext(huff,NULL,NULL));

  printf(rec_num, num);
  printf("\nDone!\n");
  UserFileClose(huf);
}


void Convert_Quick(void)
{
  struct _qbbs_user qbu;
  struct _usr usr;

  int infile, num;
  HUF huf;

  Check_If_Exist();

  if ((infile=open(users_bbs,O_RDONLY | O_BINARY))==-1)
  {
    printf("Error opening USERS.BBS for read!  Aborting...\n");
    exit(1);
  }

  if ((huf=UserFileOpen(user_name, O_CREAT | O_TRUNC))==NULL)
  {
    printf("Error opening USER.DB for write!  Aborting...\n");
    exit(1);
  }

  for (num=0; read(infile,(char *)&qbu, sizeof qbu)==sizeof qbu; )
  {
    Blank_User(&usr);

    TP_to_Cstr(qbu.name, usr.name);
    TP_to_Cstr(qbu.city, usr.city);
    TP_to_Cstr(qbu.pwd, usr.pwd);

    if (strchr(usr.pwd, ' '))
      *strchr(usr.pwd, ' ')='\0';

    TP_to_Cstr(qbu.home_phone, usr.phone);
    TP_to_Cstr(qbu.data_phone, usr.dataphone);

    usr.credit=qbu.credit;
    usr.times=qbu.times;

    usr.up=qbu.upk;
    usr.down=qbu.downk;
    usr.downtoday=qbu.todayk;

    usr.nup=qbu.ups;
    usr.ndown=qbu.downs;

    usr.len=(byte)qbu.len;
    usr.width=80;

    usr.help=NOVICE;

    strcpy(usr.msg, "1");
    strcpy(usr.files, "1");

    usr.struct_len=sizeof(struct _usr)/20;

    usr.priv=qbu.seclvl;
    usr.max2priv = TWIT;
#if 0
    if (qbu.seclvl >= 100)
      usr.priv=SYSOP;
    else if (qbu.seclvl >= 90)
      usr.priv=ASSTSYSOP;
    else if (qbu.seclvl >= 80)
      usr.priv=CLERK;
    else if (qbu.seclvl >= 70)
      usr.priv=EXTRA;
    else if (qbu.seclvl >= 60)
      usr.priv=FAVORED;
    else if (qbu.seclvl >= 50)
      usr.priv=PRIVIL;
    else if (qbu.seclvl >= 40)
      usr.priv=WORTHY;
    else if (qbu.seclvl >= 20)
      usr.priv=NORMAL;
    else usr.priv=DISGRACE;
#endif

    usr.lastread_ptr=num;

    if ((++num % 32)==0)
    {
      printf(rec_num, num);
      fflush(stdout);
    }

    if (! UserFileCreateRecord(huf, &usr, TRUE))
      printf("Error writing user record for '%s'\n", usr.name);
  }

  printf(rec_num, num);
  printf("\nDone!\n");

  close(infile);
  UserFileClose(huf);
}


void Convert_Lread(void)
{
  HUF huf;
  HUFF huff;
  int num;

  if ((huf=UserFileOpen(user_name, O_RDWR))==NULL)
  {
    printf("Error opening USER.DB for read/write!  Aborting...\n");
    exit(1);
  }

  num=0;

  if ((huff=UserFileFindOpen(huf, NULL, NULL)) != NULL)
  {
    do
    {
      huff->usr.lastread_ptr=num;
      huff->usr.struct_len=sizeof(struct _usr)/20;

      if (!UserFileUpdate(huf, huff->usr.name, huff->usr.alias, &huff->usr))
        printf("Error updating user record for \"%s\"!\n", huff->usr.name);

      if ((++num % 32)==0)
      {
        printf(rec_num, num);
        fflush(stdout);
      }
    }
    while (UserFileFindNext(huff, NULL, NULL));

    UserFileFindClose(huff);
  }

  printf(rec_num, num);
  printf("\nDone!\n");

  UserFileClose(huf);
}

void Reverse_Max300(void)
{
  struct _usr usrold;
  HUF huf;
  HUFF huff;
  int num;
    
  if ((huf=UserFileOpen(user_name, O_RDWR))==NULL)
  {
    printf("Error opening USER.DB for read/write!  Aborting...\n");
    exit(1);
  }

  num=0;

  if ((huff=UserFileFindOpen(huf, NULL, NULL)) != NULL)
  {
    do
    {
      usrold=huff->usr;

      printf("Updating user '%s' (%s)\n", huff->usr.name, huff->usr.alias);

      if (*huff->usr.alias)
      {
        strcpy(huff->usr.name, usrold.alias);
        strcpy(huff->usr.alias, usrold.name);
      }

      if ((++num % 32)==0)
      {
        printf(rec_num, num);
        fflush(stdout);
      }

      if (!UserFileUpdate(huf, usrold.name, usrold.alias,
                          &huff->usr))
        printf("Error updating user '%s'!\n", usrold.name);
    }
    while (UserFileFindNext(huff, NULL, NULL));

    UserFileFindClose(huff);
  }

  printf(rec_num, num);
  printf("\nDone!\n");

  UserFileClose(huf);
}

void Convert_Max102(void)
{
  struct _usr usr;
  struct _usr102 usr102;
  char temp[PATHLEN];

  HUF huf;
  int infile;
  int num;

  if ((infile=open(user_bbs, O_RDONLY | O_BINARY))==-1)
  {
    printf("Error opening USER.BBS for read!  Aborting...\n");
    exit(1);
  }

  if ((huf=UserFileOpen(user_dol, O_CREAT | O_TRUNC))==NULL)
  {
    printf("Error opening USER$$$.BBS for write!  Aborting...\n");
    exit(1);
  }

  num=0;

  while (read(infile, (char *)&usr102, sizeof usr102)==sizeof usr102)
  {
    Blank_User(&usr);

    strcpy(usr.name, usr102.name);

    if (num==0 && usr102.struct_len==sizeof(struct _usr)/20)
    {
      printf("Warning!  USER.BBS appears to already be a Max " VERSION " format user\n"
             "file.  Do you still wish to proceed with the conversion [y,N]? ");

      fgets(temp,80,stdin);

      if (*temp != 'y' && *temp != 'Y')
      {
        printf("Aborted.\n");
        UserFileClose(huf);
        unlink(user_poo);
        unlink(user_poo2);
        exit(1);
      }
    }

    strcpy(usr.city,usr102.city);
    strcpy(usr.alias,usr102.realname);
    strcpy(usr.phone,usr102.phone);
    usr.lastread_ptr=num;
    usr.timeremaining=0;
    strcpy(usr.pwd,usr102.pwd);
    usr.times=usr102.times;
    
    if (usr102.ansi)
      usr.video=GRAPH_ANSI;
    else if (usr102.avatar)
      usr.video=GRAPH_AVATAR;
    else usr.video=GRAPH_TTY;
    
    usr.nulls=usr102.nulls;

    if (usr102.bits & BITS_hotkeys)
      usr.bits=BITS_HOTKEYS;
    else usr.bits=0;

    usr.bits2=0;
    
    usr.help=usr102.help;

    if (usr.help==0x20)
      usr.help=NOVICE;

    if (usr102.bad_logon)
      usr.bits2 |= BITS2_BADLOGON;
    
    if (usr102.ibmchars)
      usr.bits2 |= BITS2_IBMCHARS;
    
    if (usr102.tabs)
      usr.bits |= BITS_TABS;
    
    if (usr102.use_lore)
      usr.bits2 |= BITS2_BORED;
    
    if (usr102.more)
      usr.bits2 |= BITS2_MORE;
    
    if (usr102.kludge)
      usr.bits2 |= BITS2_CONFIGURED;
    
    if (usr102.formfeed)
      usr.bits2 |= BITS2_CLS;
    
    usr.priv=newpriv(usr102.priv);
    usr.max2priv = usr102.priv;
    
    usr.struct_len=sizeof(struct _usr)/20;
    usr.time=usr102.time;
    
    usr.delflag=0;
    
    if (usr102.flag & UFLAG_deleted)
      usr.delflag |= UFLAG_DEL;
    
    if (usr102.flag & UFLAG_permanent)
      usr.delflag |= UFLAG_PERM;
    
    usr.width=usr102.width;
    usr.len=usr102.len;
    usr.credit=usr102.credit;
    usr.debit=usr102.debit;

    ASCII_Date_To_Binary(usr102.ldate,&usr.ludate);
    
    usr.xkeys=(long)usr102.key;
    usr.lang=0;
    usr.def_proto=PROTOCOL_NONE;
    
    usr.up=usr102.upld;
    usr.down=usr102.dnld;
    usr.downtoday=usr102.dnldl;

    strcpy(usr.msg, Area_Name(usr102.msg));
    strcpy(usr.files, Area_Name(usr102.files));

    usr.sex=SEX_UNKNOWN;

    /* Fill in defaults for the new-file-search and password change */

    Get_Dos_Date(&usr.date_newfile);
    Get_Dos_Date(&usr.date_pwd_chg);

    
    if ((++num % 32)==0)
    {
      printf(rec_num, num);
      fflush(stdout);
    }

    /* Max 1.0x compatibility */
    Adjust_User_Record(&usr);

    if (!UserFileCreateRecord(huf, &usr, TRUE))
      printf("Error writing user record for %s\n", usr.name);
  }

  printf(rec_num, num);
  printf("\nDone!\n");

  close(infile);
  UserFileClose(huf);

  unlink(user_bak);
  unlink(user_idx);
  rename(user_bbs, user_bak);
  rename(user_poo, user_bbs);
  rename(user_poo2, user_idx);
}


void Convert_Max200(void)
{
  struct _usr usr;
  struct _usr200 usr200;
  char *p;

  int infile;
  HUF huf;
  int num;

  if ((infile=open(user_bbs, O_RDONLY | O_BINARY))==-1)
  {
    printf("Error opening USER.BBS for read!  Aborting...\n");
    exit(1);
  }

  if ((huf=UserFileOpen(user_dol, O_CREAT | O_TRUNC))==NULL)
  {
    printf("Error opening USER$$$.BBS for write!  Aborting...\n");
    exit(1);
  }

  num=0;

  while (read(infile, (char *)&usr200, sizeof usr200)==sizeof usr200)
  {
    static int warned=FALSE;

    if (usr200.struct_len==sizeof usr / 20 && !warned)
    {
      char temp[PATHLEN];

      printf("Warning!  USER.BBS appears to already be a Max " VERSION " format user\n"
             "file.  Do you still wish to proceed with the conversion [y,N]? ");

      fgets(temp,80,stdin);

      if (*temp != 'y' && *temp != 'Y')
      {
        UserFileClose(huf);
        unlink(user_poo);
        unlink(user_poo2);
        printf("Aborted.\n");
        exit(1);
      }

      warned=TRUE;
    }

    /* Zero out the 3.0 structure, then copy in the first half of the       *
     * 2.0 struct.                                                          */

    memset(&usr, 0, sizeof usr);
    memmove(&usr, &usr200, sizeof usr200);

    /* Convert HOTFLASH help to NOVICE */

    if (usr.help==0x20)
      usr.help=NOVICE;

    /* Zero out new fields which are new to Max 3.0 */

    usr.group=0;
    usr.dob_year=0;
    memset(usr.dataphone, 0, sizeof usr.dataphone);
    usr.msgs_posted=usr.msgs_read=0;
    usr.sex=SEX_UNKNOWN;
    memset(usr.rsvd45, 0, sizeof usr.rsvd45);

    memset(&usr.date_1stcall, 0, sizeof usr.date_1stcall);

    /* Fill in defaults for the new-file-search and password change */

    Get_Dos_Date(&usr.date_newfile);
    Get_Dos_Date(&usr.date_pwd_chg);

    /* Copy in the old file areas */

    strcpy(usr.msg, usr200.msg);
    strcpy(usr.files, usr200.files);

    /* Convert dots to underscores */

    while ((p=strchr(usr.msg, '.')) != NULL)
      *p='_';

    /* Convert dots to underscores */

    while ((p=strchr(usr.files, '.')) != NULL)
      *p='_';

    /* Set the new structure size */

    usr.struct_len=sizeof usr / 20;

    usr.priv=newpriv(usr.max2priv);

    if (usr.xp_flag & XFLAG_DEMOTE)
      usr.xp_priv=newpriv(usr.xp_priv);

    /* Display the new record number */

    if ((++num % 32)==0)
    {
      printf(rec_num, num);
      fflush(stdout);
    }

    if (!UserFileCreateRecord(huf, &usr, TRUE))
      printf("Error writing user record for %s\n", usr.name);
  }

  printf(rec_num, num);
  printf("\nDone!\n");

  close(infile);
  UserFileClose(huf);

  unlink(user_bak);
  unlink(user_idx);
  rename(user_bbs, user_bak);
  rename(user_poo, user_bbs);
  rename(user_poo2, user_idx);
}



#if 0
void Convert_RBBS(char *name)
{
  NW(name);

  struct _rbbsu ru;
  struct _usr102 usr;

  int rbbsfile;
  int maxfile;
  int nptr=0;

  if ((rbbsfile=open(name,O_RDONLY | O_BINARY))==-1)
  {
    printf("Error opening `%s' for read!\n",name);
    exit(1);
  }

  if ((maxfile=open(user_bbs,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                    S_IREAD | S_IWRITE))==-1)
  {
    printf("Error opening `USER.BBS' for write!\n");
    exit(1);
  }

  while (read(rbbsfile,(char *)&ru,sizeof(struct _rbbsu))==
          sizeof(struct _rbbsu))
  {
    if (*BASIC(ru.name)=='\0')
      continue;

    memset(&usr,'\0',sizeof(struct _usr102));
    usr.width=80;
    usr.len=24;
    usr.help=NOVICE;

    strcpy(usr.name,fancy_str(BASIC(ru.name)));
    strcpy(usr.pwd,fancy_str(BASIC(ru.pwd)));
    strcpy(usr.city,fancy_str(BASIC(ru.city)));
    usr.dnld=atoi(BASIC(ru.dl_total_bytes));
    usr.upld=atoi(BASIC(ru.ul_total_bytes));
    usr.lastread_ptr=nptr++;
    usr.ibmchars=1;
    usr.more=1;
    usr.formfeed=1;
    usr.tabs=1;
    usr.ansi=0;
    usr.kludge=0;
    usr.priv=newpriv(NORMAL);
    usr.struct_len=sizeof(struct _usr102)/20;

    write(maxfile, (char *)&usr, sizeof(struct _usr102));

    if ((++num % 32)==0)
    {
      printf(rec_num, num);
      fflush(stdout);
    }
  }

  printf(rec_num, num);
  printf("\nDone!\n");
  close(maxfile);
  close(rbbsfile);
}
#endif


void Check_If_Exist(void)
{
  char temp[PATHLEN];

  if (fexist(user_bbs) || fexist(user_idx))
  {
    printf("Warning!  USER.BBS already exists.  Overwrite [y,N]? ");
    fgets(temp,80,stdin);

    printf("\n");

    if (toupper(*temp) != 'Y')
    {
      printf("Aborted!\n");
      exit(1);
    }

    if (fexist(user_bak))
      unlink(user_bak);

    rename(user_bbs, user_bak);
  }
}

