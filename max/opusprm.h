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

/*# name=Opus .PRM structure.
    credit=This module extracted from OPUS.H, which was written (and
    credit=probably copyrighted by) Wynn Wagner III.
*/

#ifdef MAX_EXTERN
#undef MAX_EXTERN
#endif

#define  MAX_EXTERN         8       /* max. number of external programs     */
#define  MAXCLASS          12       /* number of possible priv levels       */
#define  ALIAS_CNT         15       /* number of matrix addresses           */

               /*-----------------------------------------------------------*/
               /* Information about a class of users.  Warning: This is NOT */
               /* a stable structure.                                       */
               /*-----------------------------------------------------------*/
struct   class_rec
            {
               sword        priv;
               sword        max_time;      /* max cume time per day         */
               sword        max_call;      /* max time for one call         */
               sword        max_dl;        /* max dl bytes per day          */
               word         ratio;         /* ul:dl ratio                   */
               word         min_baud;      /* speed needed for logon        */
               word         min_file_baud; /* speed needed for file xfer    */
            };

   struct   class_rec_110 /* For V17 .PRM */
   {
      byte  ClassPriv;
      byte  class_fill;
      sword max_time;               /* max cume time per day               */
      sword max_call;               /* max time for one call               */
      sword max_dl;                 /* max dl bytes per day                */
      word  ratio;                  /* ul:dl ratio                         */
      word  min_baud;               /* speed needed for logon              */
      word  min_file_baud;          /* speed needed for file xfer          */
   };


/*--------------------------------------------------------------------------*/
/* Registers to pass to a FOSSIL appendage                                  */
/*--------------------------------------------------------------------------*/
struct _FOSREGS
   {
   word  ax;
   word  bx;
   word  cx;
   word  dx;
   };

/*--------------------------------------------------------------------------*/
/* FLAGS                                                                    */
/*--------------------------------------------------------------------------*/
#define LOGECHO    0x0001  /* log echomail areas        0000 0000 0000 0001 */
#define STEADY     0x0002  /* never change baud rate    0000 0000 0000 0010 */
#define ECHOSCAN   0x0004  /* automatically scan echo's 0000 0000 0000 0100 */
#define ECHO_GUARD 0x0008  /* no toss un-pwd echo arc   0000 0000 0000 1000 */
#define NO_FWD     0x0010  /* No IN TRANSIT netmail   * 0000 0000 0001 0000 */



struct pointers_14 {

               /*-----------------------------------------------------------*/
               /* DATA                                                      */
               /*-----------------------------------------------------------*/
         byte  version;        /* for safety                                */
         byte  testmode;       /* input from keyboard, not modem            */
         long  total_callers;  /* total number of callers to system         */
         long  quote_position; /* last position used in last quote file     */
         byte  multitasker;    /* flag for DoubleDos (see below)            */
         byte  snooping;       /* Local monitor active                      */
         byte  edit_exit;      /* 1= use new user questionaire              */
         byte  verbose;        /* wordy sysop log                           */
         byte  terse;          /* brief sysop log                           */
         byte  trace;          /* log trace mode                            */
         byte  show_areapath;  /* use path not Dir.Bbs                      */
         byte  task_num;       /* for multi-tasking systems                 */
         byte  exit_val;       /* ERRORLEVEL to use after caller            */
         byte  val_outside;    /* ERRORLEVEL for O)utside                   */
         byte  val_zero;       /* ERRORLEVEL for sysop 0 command            */
         byte  no_crashmail;   /* 1 = don't accept crashmail                */
         byte  auto_kill;      /* RECD PVT msgs. 0=no 1=ask 2=yes           */
         byte  crashexit;      /* non-zero= ErrorLevel exit                 */
         byte  unpack_arc;     /* 1=unpack incomming arcmail                */
         byte  toss_echo;      /* 1=toss incomming echomail                 */
         byte  arc_exit;       /* ErrorLevel for after incomming ARCmail    */
         byte  old_dtr;        /* 1 drop dtr to look busy, 0 go off hook    */
         sword carrier_mask;
         sword handshake_mask;
         sword ctla_priv;      /* Priv to see CONTROL-A lines in messages   */
         sword max_baud;       /* fastest speed we can use                  */
         sword min_baud;       /* minimum baud to get on-line               */
         sword speed_graphics; /* min baud for graphics                     */
         sword com_port;       /* Com1=0, Com2=1                            */
         sword logon_priv;     /* Access level for new users                */
         sword date_style;     /* Used for FILES.BBS display                */
         sword seenby_priv;    /* Min priv to see SEEN_BY line              */

         sword msg_ask[16];    /* Array of privs. for message attr ask's    */
         sword msg_assume[16]; /* Array of privs. for message attr assume's */
         sword msg_fromfile;   /* Priv. for doing message from file         */

         byte  watchdog;       /* 1=Set Fossil to reboot during outside     */
         byte  video;          /* 0=Dos, 1=Fossil 2=IBM                     */

         byte  filler[8];

         word  logon_time;     /* time to give for logons                   */

         byte  echo_exit;      /* ERRORLEVEL for after inbound echomail     */

         byte  bfill;
         byte  Flags;          /* See "FLAGS" below                         */

/*       byte  junk1; */

         word  our_zone;
         word  matrix_mask;

         struct class_rec cls[MAXCLASS];
         struct _ndi alias[ALIAS_CNT];


               /*-----------------------------------------------------------*/
               /* OFFSETS                                                   */
               /*-----------------------------------------------------------*/
         sword m_init;         /* modem initialization string               */
         sword predial;        /* modem dial command sent before number     */
         sword postdial;       /* modem command sent after dialed number    */
         sword timeformat;
         sword dateformat;
         sword fkey_path;      /* path to `F-key files'                     */
         sword parm_outside;   /* prog/parms for O)utside                   */
         sword parm_zero;      /* parm for sysop 0 command                  */
         sword sys_path;       /* path to SYSTEM?.BBS files                 */
         sword user_file;      /* path/filename of User.Bbs                 */
         sword net_info;       /* path to NODELIST files                    */
         sword sched_name;     /* name of file with _sched array            */
         sword logo;           /* first file shown to a caller              */
         sword welcome;        /* shown after logon                         */
         sword bulletin;       /* shown after the welcome file              */
         sword editorial;
         sword quote;
         sword question;       /* Questionnaire available on the main menu  */
         sword request_list;   /* list of files approved for file requests  */
         sword newuser1;
         sword newuser2;
         sword rookie;
         sword application;    /* new user questionnaire                    */
         sword avail_list;     /* file to send when FILES is file requested */
         sword hlp_editor;     /* Intro to msg editor for novices.          */
         sword hlp_replace;    /* Explain the Msg.Editor E)dit command      */
         sword msg_inquire;    /* Explain the Msg. I)nquire command         */
         sword hlp_locate;     /* Explain the Files L)ocate command         */
         sword hlp_contents;   /* Explain the Files C)ontents command       */
         sword out_leaving;    /* Bon Voyage                                */
         sword out_return;     /* Welcome back from O)utside                */
         sword daylimit;       /* Sorry, you've been on too long...         */
         sword timewarn;       /* warning about forced hangup               */
         sword sysop;          /* sysop's name                              */
         sword tooslow;        /* explains minimum logon baud rate          */
         sword xferbaud;       /* explains minimum file transfer baud rate  */
         sword msgarea_list;   /* dump file... used instead of Dir.Bbs      */
         sword file_area_list; /* dump file... used instead of Dir.Bbs      */
         sword maillist_file;  /* default "nodelist" file                   */
         sword byebye;         /* file displayed at logoff                  */
         sword protocols[MAX_EXTERN]; /* external file protocol programs    */
         sword local_editor;
         sword file_mgt;       /* external file section management          */
         sword hold_area;      /* path to pending outbound matrix traffic   */
         sword barricade;
         sword badaccess;
         sword msg_mgt;        /* external message section maintenance      */

         sword mailpath;       /* path to inbound bundles                   */
         sword filepath;       /* path for inbound matrix files             */
         sword oped_help;      /* help file for the full-screen editor      */
         sword temppath;       /* place to put temporary files              */
         sword m_busy;         /* mdm cmd to take modem off hook            */
         sword system_name;    /* board's name                              */
         sword freq_about;     /* File Request: ABOUT file                  */

               /*-----------------------------------------------------------*/
               /* Log_Name must always be the last offset in this struct    */
               /* because Bbs_Init uses that symbol to flag the end of      */
               /* the offsets.                                              */
               /*-----------------------------------------------------------*/
         sword log_name;       /* name of the log file                      */
   };




/*-------------------------------------------------------------------------*/
/* The format of the PRM file, VERSION 16                                  */
/*                                                                         */
/* THIS IS AN EXPLOSIVE STRUCTURE.  IT IS SUBJECT TO CHANGE WITH NO NOTICE.*/
/*                                                                         */
/* Offsets to the following item(s) are guaranteed:                        */
/*                                                                         */
/*      byte   version;           OFFSET 0, all versions                   */
/*      byte   task_num;          OFFSET 1, 16+                            */
/*                                                                         */
/*-------------------------------------------------------------------------*/

   struct pointers_16
   {
               /*----------------------------------------------------------*/
               /* DATA                                                     */
               /*----------------------------------------------------------*/

         byte  version;        /* for safety                        STABLE */
         byte  task_num;       /* for multi-tasking systems         STABLE */
         NETADDR alias[ALIAS_CNT];

         byte  video;          /* 0=Dos, 1=Fossil 2=IBM                    */
         byte  testmode;       /* input from keyboard, not modem           */

         word  carrier_mask;
         word  handshake_mask;

         word  max_baud;       /* fastest speed we can use                 */
         word  com_port;       /* Com1=0, Com2=1, FF=keyboard              */

         byte  multitasker;    /* flag for DoubleDos (see below)           */
         byte  mailer_type;    /* 0=Opus, 1=load external, 2=call external */

         byte  ModemFlag;      /* (See MODEM FLAG below)                   */
         byte  LogFlag;        /* (See LOG FLAG below)                     */

         byte  StyleFlag;      /* (See STYLE FLAG below)                   */
         byte  FWDflag;        /* Bits to control IN TRANSIT messages      */

         byte  Flags;          /* See "FLAGS" below                        */
         byte  Flags2;         /* See "FLAGS 2" below                      */

         byte  edit_exit;      /* ERRORLEVEL to use if Matrix area changed */
         byte  exit_val;       /* ERRORLEVEL to use after caller           */

         byte  crashexit;      /* non-zero= ErrorLevel exit                */
         byte  arc_exit;       /* ErrorLevel for after incomming ARCmail   */

         byte  echo_exit;      /* ERRORLEVEL for after inbound echomail    */
         byte  UDB_Flags;      /* User data base flags                     */

         word  min_baud;       /* minimum baud to get on-line              */
         word  color_baud;     /* min baud for graphics                    */
         word  date_style;     /* Used for FILES.BBS display               */

         byte  logon_priv;     /* Access level for new users               */
         byte  seenby_priv;    /* Min priv to see SEEN_BY line             */

         byte  ctla_priv;      /* Priv to see CONTROL-A lines in messages  */
         byte  FromFilePriv;   /* Priv. for doing message from file        */

         byte  AskPrivs[16];   /* Array of privs. for message attr ask's   */
         byte  AssumePrivs[16];/* Array of privs. for message attr assume's*/

         word  logon_time;     /* time to give for logons                  */

         word  matrix_mask;

         word  MinNetBaud;     /* minimum baud rate for remote netmail     */
         word  MaxJanusBaud;   /* fastest baud to use Ianus                */

         struct class_rec_110 cls[MAXCLASS];
         struct _FOSREGS FosRegs[10];

         word  F_Reward;       /* File upload time reward percentage       */
         word  last_area;      /* Highest msg area presumed to exist       */
         word  last_farea;     /* Highest file area presumed to exist      */

         byte  return_secure;  /* 0=ignore LASTUSER, >1 re-read LASTUSER   */

              /*  New for Version 17 */

         byte xuflags;         /* ExtUser Flags: 1=Alias, 2=UserTel        */
         word xlmin;           /* Lead mins for expiry warning             */
         char xlday;           /* Lead days for expiry warning             */
         byte expriv;          /* Expiry privilege                         */
         char totlang;         /* Total languages present (1-6)            */
         byte sylno;           /* Def Sysop Language no.  (0-5)            */
         byte uslno;           /* Def User Language no.   (0-5)            */
         byte fill3b;          /* Byte align filler                        */
         word Scrn_Size;       /* Local CRT's size, HiByte=len, LoByte-Wid */
         word PRM_FILL3[11];   /* Filler                                   */


               /*----------------------------------------------------------*/
               /* OFFSETS                                                  */
               /*----------------------------------------------------------*/

                               /*------------------------------------------*/
                               /* MODEM COMMAND STRINGS                    */
                               /*------------------------------------------*/
         sword MDM_Init;       /* modem initialization string              */
         sword MDM_PreDial;    /* modem dial command sent before number    */
         sword MDM_PostDial;   /* modem command sent after dialed number   */
         sword MDM_LookBusy;   /* mdm cmd to take modem off hook           */

                               /*------------------------------------------*/
                               /* PRIMROSE PATHS                           */
                               /*------------------------------------------*/
         sword misc_path;      /* path to BBS/GBS files                    */
         sword sys_path;       /* path to SYSTEM?.BBS files                */
         sword temppath;       /* place to put temporary files             */
         sword net_info;       /* path to NODELIST files                   */
         sword mailpath;       /* place to put received netmail bundles    */
         sword filepath;       /* place to put received netmail files      */
         sword hold_area;      /* path to pending outbound matrix traffic  */

                               /*------------------------------------------*/
                               /* DATA FILE NAMES                          */
                               /*------------------------------------------*/
         sword user_file;      /* path/filename of User.Bbs                */
         sword sched_name;     /* name of file with _sched array           */
         sword langdir;        /* Langauge file dir. (Was Syl in v16)      */
         sword spcldir;        /* Spcl Ann Text dir. (Was Usl in v16)      */

                               /*------------------------------------------*/
                               /* MISCELLANEOUS TEXT                       */
                               /*------------------------------------------*/
         sword system_name;    /* board's name                             */
         sword sysop;          /* sysop's name                             */
         sword timeformat;
         sword dateformat;
         sword protocols[MAX_EXTERN]; /* external file protocol programs   */

                               /*------------------------------------------*/
                               /* BBS/GBS SUPPORT FILES                    */
                               /*------------------------------------------*/
         sword logo;           /* first file shown to a caller             */
         sword welcome;        /* shown after logon                        */
         sword newuser1;
         sword newuser2;
         sword rookie;

         sword HLP_Editor;     /* Intro to msg editor for novices.         */
         sword HLP_Replace;    /* Explain the Msg.Editor E)dit command     */
         sword HLP_Inquire;    /* Explain the Msg. I)nquire command        */
         sword HLP_Locate;     /* Explain the Files L)ocate command        */
         sword HLP_Contents;   /* Explain the Files C)ontents command      */
         sword HLP_OPed;       /* help file for the full-screen editor     */
         sword OUT_Leaving;    /* Bon Voyage                               */
         sword OUT_Return;     /* Welcome back from O)utside               */
         sword ERR_DayLimit;   /* Sorry, you've been on too long...        */
         sword ERR_TimeWarn;   /* warning about forced hangup              */
         sword ERR_TooSlow;    /* explains minimum logon baud rate         */
         sword ERR_XferBaud;   /* explains minimum file transfer baud rate */
         sword LIST_MsgAreas;  /* dump file... used instead of Dir.Bbs     */
         sword LIST_FileAreas; /* dump file... used instead of Dir.Bbs     */

         sword FREQ_MyFiles;   /* file to send when FILES is file requested*/
         sword FREQ_OKList;    /* list of files approved for file requests */
         sword FREQ_About;     /* File Request: ABOUT file                 */

         sword OEC_Quotes;
         sword byebye;         /* file displayed at logoff                 */
         sword local_editor;   /* text editor to use in keyboard mode      */
         sword barricade;
         sword INMAIL_dir;     /* Where to find INMAIL##.$$$ re multiline  */
         sword mailer;         /* full external mailer command             */
         sword common;         /* File with data common to all tasks       */

              /*  New for Version 17 */

         sword xdwarn;         /* Date Warning OEC BBS                     */
         sword xtwarn;         /* Time Warning OEC BBS                     */
         sword xdexpd;         /* Expired due-to-Date OEC BBS              */
         sword xtexpd;         /* Expired due-to-Time used OEC BBS         */
         sword lang[6];        /* 6 Language File Root Name pointers       */

         sword badpath;        /* Path for unrecognized echo names         */
         sword OFS_Filler[2];  /* Was [13] in V16                          */

               /*----------------------------------------------------------*/
               /* Log_Name must always be the last offset in this struct   */
               /* because Bbs_Init uses that symbol to flag the end of     */
               /* the offsets.                                             */
               /*----------------------------------------------------------*/

         sword log_name;       /* name of the log file                     */
   };


   #define TOTPRIVS 12

   #define _TWIT      0x10 /* Twit .......... Minimum access. Eg, Problems */
   #define _DISGRACE  0x30 /* Disgraced ..... Eg, 1st time callers         */
   #define _LIMITED   0x40 /* Limited ....... Eg, 1st time callers         */
   #define _NORMAL    0x50 /* Normal ........ Eg, Regular callers          */
   #define _WORTHY    0x60 /* Worthy ........ Eg, Approved callers         */
   #define _PRIVEL    0x70 /* Privileged .... Eg, Full access callers      */
   #define _FAVORED   0x80 /* Favored ....... Eg, Friends or Helpers       */
   #define _EXTRA     0x90 /* Extra ......... Eg, Friends or Helpers       */
   #define _CLERK     0xA0 /* Clerk ......... Eg, Occasioanal helper       */
   #define _ASSTSYSOP 0xB0 /* Assistant Sysop High access. Eg, Co-Sysops   */
   #define _SYSOP     0xD0 /* Sysop ......... HIGHEST ACCESS. #1 Sysop     */

   #define _HIDDEN    0xE0 /* Hidden ........ HIDES THINGS / NOT FOR USERS */


   struct _sys110
   {
      /*........ (mostly) Common System Data ..............................*/

      word version;           /* System Record version = 110 = v1.10       */
      word menu;              /* Alternate Menu file extension, 0=MNU      */
      word attrib;            /* Area attributes (see below)               */
      byte fillc1[10];        /* Reserved filler                           */
      byte barrpath[ 40 ];    /* Barricade File path.                      */
      byte fillc2[24];        /* Reserved filler                           */

      /*........ File System Information ..................................*/

      byte filtitle[ 50 ];    /* File Area Title                           */
      byte filepath[ 40 ];    /* Path to the file download directory       */
      byte uppath[   40 ];    /* Path to the file upload directory         */
      byte listpath[ 40 ];    /* Path to FILES.BBS equivalent              */
      byte fillf1[22];        /* Reserved filler                           */

      byte FilePriv;          /* Min priv for file area                    */
      byte DownPriv;          /* If not 0, min priv to download            */
      byte UpPriv;            /* If not 0, min priv to upload              */
      byte FileExtPriv;       /* If not 0, min priv to go Outside          */
      byte fillf2[12];        /* Reserved filler                           */

      long FileLock;          /* Locks for File Area                       */
      long DownLock;          /* If not 0, keys needed to download         */
      long UpLock;            /* If not 0, keys needed to upload           */
      long FileExtLock;       /* If not 0, keys needed to go Outside       */
      byte fillf3[32];        /* Reserved filler                           */

      /*........ Message System Information ...............................*/

      byte msgtitle[ 50 ];    /* Msg  Area Title                           */
      byte msgpath[  40 ];    /* Path to messages                          */
      byte fillm1[ 22 ];      /* Reserved filler                           */

      byte MsgPriv;           /* Min priv for msg area                     */
      byte EditPriv;          /* If not 0, min priv to Enter or Reply      */
      byte MsgExtPriv;        /* If not 0, min priv to go Outside          */
      byte fillm2[13];        /* Reserved filler                           */

      long MsgLock;           /* Locks for Msg Area                        */
      long EditLock;          /* If not 0, keys needed to Enter or Reply   */
      long MsgExtLock;        /* If not 0, keys needed to go Outside       */
      byte fillm3[4];         /* Reserved filler                           */
      byte EchoName[32];      /* Echo Area 'Tag' Name                      */

      /*=================================== Total Record Size   = 512 =====*/
   };



