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

#ifndef __MEX_MAX_H_DEFINED
#define __MEX_MAX_H_DEFINED


#define CIT_DAY_TIME      0
#define CIT_CALL_TIME     1
#define CIT_DL_LIMIT      2
#define CIT_RATIO         3
#define CIT_MIN_BAUD      4
#define CIT_MIN_XFER_BAUD 5

struct mex_instancedata
{
    word  instant_video;
    word  task_num;
    word  local;
    word  port;
    dword speed;
    word  alias_system;
    word  ask_name;
    word  use_umsgid;
} __attribute__((packed));

struct mex_cstat
{
    word  task_num;
    word  avail;
    IADDR username;
    IADDR status;
} __attribute__((packed));

struct mex_date
{
  byte  day;                    /* Day of month.   1 = first of month */
  byte  month;                  /* Month of year.  1 = January */
  byte  year;                   /* Year - 1980.    0 = 1980 */
} __attribute__((packed));

struct mex_time
{
  byte  hh;                     /* Hour */
  byte  mm;                     /* Minute */
  byte  ss;                     /* Second */
} __attribute__((packed));

struct mex_stamp
{
  struct mex_date date;
  struct mex_time time;
} __attribute__((packed));

struct mex_usr
{
  IADDR  name;                  /* Caller's name */
  IADDR  city;                  /* Callers city and state/province */
  IADDR  alias;                 /* User's alias */
  IADDR  phone;                 /* User's phone number */
  word   lastread_ptr;          /* Offset in LASTREAD.BBS/areaname.SQL */

/*word  time_remain;*/          /* Time left for current call - obsolete */

  IADDR  pwd;                   /* User's password */
  word   times;                 /* Number of previous calls to system */
  byte   help;                  /* Help level.  Novice=6, regular=4, */
                                /* expert=2, hotflash=32. */
  byte    video;                /* Video mode.  0=tty, 1=ansi, 2=avatar */
  byte    nulls;                /* Number of NULs (delays) after <cr> */

  /* Bits 1 */

  byte    hotkeys;              /* 1 = Hotkeys enabled */
  byte    notavail;             /* 1 = Not available for chat */
  byte    fsr;                  /* 1 = Full-screen reader is enabled */
  byte    nerd;                 /* 1 = User is a nerd (cannot yell) */
  byte    noulist;              /* 1 = User does not show in userlist */
  byte    tabs;                 /* 1 = User can handle tab characters */
  byte    encrypted;            /* 1 = Password is encrypted */
  byte    rip;                  /* 1 = RIP graphics enabled */

  /* Bits 2 */

  byte    badlogon;             /* 1 = Last logon attempt was bad */
  byte    ibmchars;             /* 1 = User can handle IBM characters */
  byte    bored;                /* 1 = BORED.  0 = MaxEd */
  byte    more;                 /* 1 = More prompt enabled */
  byte    configured;           /* 1 = City/pwd/phone fields have been filled */
  byte    cls;                  /* 1 = User can handle clearscreen */

  word     priv;                /* User's priv level */

/**/  IADDR dataphone;

  word  time;                   /* Length of time user has been on-line */
                                /* today for previous calls */

/*long    baud;*/               /* This caller's speed is - obsolete */

  /* Delflag */

  byte    deleted;              /* This user has been deleted. */
  byte    permanent;            /* This user is permanent. */

/**/  dword   msgs_posted;
/**/  dword   msgs_read;
  byte    width;                /* Width of caller's screen */
  byte    len;                  /* Height of caller's screen */

  word     credit;              /* Netmail credit, in cents */
  word     debit;               /* Netmail debit, in cents */

  /* Expiration controls */

  sword     xp_priv;            /* Demote user to this priv when */
                                /* subscription expires.  (Same encoding */
                                /* as usr.priv) */

  struct mex_stamp  xp_date;    /* If it is past this date, the user has */
                                /* expired. */

  long    xp_mins;              /* The user has this many minutes left */
                                /* before expiring. */

  /* XPflag */

  byte    expdate;              /* 1 = Expire user based on date in xp_date */
  byte    expmins;              /* 1 = Expire user based on time in xp_mins */
  byte    expdemote;            /* 1 = When expired, demote to xp_priv */
  byte    expaxe;               /* 1 = When expired, hang up */

  byte    sex;                  /* User's sex -- see SEX_xxx definitions */
  struct mex_stamp  ludate;     /* Date of user's last call */

  IADDR  xkeys;                 /* User's keys, as a string */

  byte    lang;                 /* User's current language. 0=first lang, */
                                /* 1=second lang, etc. */

  byte    def_proto;            /* User's default protocol. */
                                /*      -1 = No default protocol */
                                /*      -2 = Xmodem */
                                /*      -3 = Telink */
                                /*      -4 = Xmodem-1K */
                                /*      -5 = SEAlink */
                                /*      -6 = Zmodem */
                                /*    0-16 = external protocol */

  long    up;                   /* Kilobytes uploaded, for all calls */
  long    down;                 /* Kilobytes downloaded, for all calls */
  long    downtoday;            /* Kilobytes downloaded today */

  IADDR   msg;                  /* Current message area */
  IADDR   files;                /* Last message area */

  byte    compress;             /* Default compression program */

/**/  IADDR   dob;                   /* Date of birth */
/**/  struct mex_stamp date_1stcall;
/**/  struct mex_stamp date_pwd_chg;
/**/  dword   nup;
/**/  dword   ndown;
/**/  dword   ndowntoday;
/**/  word    time_added;
/**/  dword   point_credit;
/**/  dword   point_debit;
/**/  struct mex_stamp date_newfile;

  word    call;                 /* Number of previous calls today */

} __attribute__((packed));


struct mex_marea
{
  IADDR name;
  IADDR descript;
  IADDR path;
  IADDR tag;
  IADDR attach_path;
  IADDR barricade;
  word  division;
  word  type;
  word  attribs;
} __attribute__((packed));

struct mex_farea
{
  IADDR name;
  IADDR descript;
  IADDR downpath;
  IADDR uppath;
  IADDR filesbbs;
  IADDR barricade;
  word  division;
  word  attribs;
} __attribute__((packed));

struct mex_msg
{
  dword current;
  dword high;
  dword num;
  word direction;
} __attribute__((packed));

struct mex_sys
{
  word current_row;
  word current_col;
  word more_lines;
} __attribute__((packed));

struct mex_ffind
{
  struct _ffind * finddata;
  IADDR   filename;
  long    filesize;
  struct mex_stamp filedate;
  word    fileattr;
} __attribute__((packed));

struct mex_callinfo
{
  IADDR   name;
  IADDR   city;
  struct mex_stamp login;
  struct mex_stamp logoff;
  word    task;
  word    flags;
  word    logon_priv;
  IADDR   logon_xkeys;
  word    logoff_priv;
  IADDR   logoff_xkeys;
  word    filesup;
  word    filesdn;
  word    kbup;
  word    kbdn;
  word    calls;
  word    read;
  word    posted;
  word    paged;
  word    added;
} __attribute__((packed));


#endif /* __MEX_MAX_H_DEFINED */

