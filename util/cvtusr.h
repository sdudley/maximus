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

#define MAX_id 0x2158414dL /* "MAX!" */
#define OPUS   0x5355504fL


#define BASIC(s)    BASIC_String(s,sizeof(s))

void Dump(char *find);
void Format(void);
/* void Convert_Max30a1(void); */
void Convert_MaxPriv(void);
void Convert_Opus110(void);
void ConvertTo_Opus110(void);
void Convert_Quick(void);
void Convert_Opus103(void);
void Convert_Max(int c);
void Convert_Max102(void);
void Check_If_Exist(void);
long CRC(char *s);
char * BASIC_String(char *s,int len);
void Blank_User(struct _usr *usr);
void Convert_RBBS(char *name);
void Reverse_Max200(void);

byte MaxToOpusPriv(int mpriv);
int OpusToMaxPriv(byte opriv);


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

   #define MAXLREAD   0x0100  /* Number of msg area pointers to maintain      */
   #define HITECH     0x0000      /* 1-line, top-of-screen, bar-style menus */

/*-------------------------------------------------------------------------*/
/* 'Bits' -- User Option Flag Meanings                                     */
/*-------------------------------------------------------------------------*/

   #define  NO_IBMCHAR  0x0004 /* Cannot receive IBM textmode graphics     */
   #define  USE_LORE    0x0008 /* Wants LORE editor instead of OPed        */
   #define  MORE_PROMPT 0x0010 /* Wants the page break "MORE?" question    */
   #define  ANSI        0x0020 /* Can handle ANSI video                    */
   #define  CONFIG_SET  0x0040 /* OPUS logon questions answered            */
   #define  FORMFEED    0x0080 /* SET=xmit clearscreen, CLEAR=eat formfeed */
   #define  AVATAR      0x0100 /* Can handle AVATAR (aka "oANSI") video    */

   #define  USE_PHONE   0x0400 /* User Modem Tel# to Call Back    (future) */
   #define  USE_ALIAS   0x0800 /* Use ALIAS in var user functions (future) */

   #define  NO_NAME     0x1000 /* Don't List User's Name in User List      */
   #define  NO_TIME     0x2000 /* Don't List User's Last-On in User List   */
   #define  NO_CITY     0x4000 /* Don't List User's City in User List      */

   #define  CURSOR_CONTROL (ANSI|AVATAR)



   struct   _usr110
   {
      char name[36];       /* Caller's first and last names                */
      char city[36];       /* Caller's location                            */
      char pwd[16];        /* Password                                     */
      char usrtel[16];     /* User Tel# for ref or future callback         */
      char alias[32];      /* User alias if ID is not it                   */
      word times;          /* Number of previous calls to this system      */
      byte ClassPriv;      /* User Access Privilege                        */
      byte help;           /* Help level (see below)                       */
      byte tabs;           /* 0=transmit <sp> instead of <tab>             */
      byte language;       /* Lang set no. (1-6) as defined in .PRM file.  */
      word nulls;          /* Number of Nulls (delays) after <cr>          */
      word msg;            /* Last message area accessed                   */
      word Bits;           /* User option flags (See Below)                */

      /* Each of the 32 bits within the following 4-byte (long) item       */
      /* represents a single user 'key'.   To have access to any Opus      */
      /* function that has any of 32 'lock' bits set for it, there has to  */
      /* be at least the matching key bits set in this item.  Having other */
      /* key bits sit does not matter.                                     */
      /*                                                                   */
      /* For maintenance and discussion, key and lock bits are referred to */
      /* by the characters A-Z and 0-5 where 'A' is bit 0 (least           */
      /* significant) and '5' refers to bit 31 (most significant).         */

      long ClassLock;      /* 32 User 'Keys'                               */
                           /* Each bit of this long object corresponds to  */
                           /* each of 32 possible user 'keys' which are    */
                           /* referred to by the character (A-Z,0-5) where */
                           /* the least significant) bit is called 'A' and */
                           /* the most significant nit is called '5'.      */

      long ludate;         /* Date of user's Last Call to system expressed */
                           /* as seconds since 01-Jan-70 in GMT/UTC time.  */

      int  time;           /* Time on-line so-far today                    */

      word flag;           /* User file management flags (see below)       */

      long upld;           /* Total kilobytes uploaded to date.            */
      long dnld;           /* Total kilobytes downloaded to date.          */
      int  dnldl;          /* Total kilobytes downloaded last/current call */
      word files;          /* Last file area accessed                      */
      byte width;          /* Width of the caller's monitor                */
      byte len;            /* Height of the caller's                       */
      word credit;         /* FidoNet usage credit in cents                */
      word debit;          /* FidoNet usage in cents                       */

      char spcoec[8];      /* Special OECC to show after logon             */
                           /* Good for clubs, stores, user groups          */
                           /* Eg, "Welcome to the dBase User Group"        */

      byte saccnt[5];      /* Array of 5 counters which correspond to the  */
                           /* files SPANN#.BBS which reside on the path    */
                           /* given by 'spcldir' in the .PRM file. If any  */
                           /* is > 0 than the user is shown the matching   */
                           /* announcement and is decremented.  When 0, it */
                           /* is no longer displayed.                      */

      byte exflag;         /* Expiration behavior flags (see below)        */

      long xdate;          /* Expiry date always rounded to (0000 hrs) and */
                           /* as secs since 01-Jan-70 in *LOCAL* time for  */
                           /* reasons of cross-zone accounting equivelnce. */

      long crmin;          /* Total minutes given to user                  */
      long dbmin;          /* Total minutes used  by user                  */

      char ulikes[32];     /* Reserved for future feature                  */
      long fudate;         /* First Call Date(secs since 01-Jan-70 UTC/GMT)*/

      byte reserved[16];   /* Filler, reserved.                            */

      /* The following item contains for each of the 256 possible areas,   */
      /* the last message number that the user read in that area.  A value */
      /* of zero means the area was not accessed by the user yet.          */

      word lastmsg[MAXLREAD];


      long OPUS_id;           /* Opus Record ID ... Must always be 'OPUS'  */


      /* The next 7 sets of 'id' and 'inf' data are for external utilities */
      /* that have been registered with OpusInfo to use for the storage    */
      /* and independent maintenance of auxiliary information.  The first  */
      /* (long) array of 7 items holds the registered ID's of utilities    */
      /* that make use of the 7 matching 32-byte data blocks that follow.  */
      /*                                                                   */
      /* Any utility may make use of any of the 7 data blocks providing    */
      /* they ensure that the desired blocks are not already used as would */
      /* be indicated by an ID in the matching ID positions.  If free,     */
      /* then the utility claims them by writing its registered ID in the  */
      /* matching ID positions.  It is up to the utility to ensure that    */
      /* enough positions are free in all of the user records that would   */
      /* potentially be accessed by it.                                    */
      /*                                                                   */
      /* Currently registered ID's ........................................*/
      /*                                                                   */
      /* 0x57555646 .. FVIEW ... File viewing utility by Doug Boone        */
      /* 0x4F4D414E .. OMAN  ... Opus system manager by Tom Kashuba        */


      long extern_id[7];      /* LONG ID number for external programs      */
                              /* Must be registered with OpusInfo          */

      byte extern_inf[7][32]; /* 7 32-byte external util data blocks       */

   };


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



#define TP_to_Cstr(tps,cs) (memmove(cs,tps+1,(int)*tps),cs[*tps]='\0')
#define C_to_TPstr(tps,cs) (memmove(tps+1,cs,strlen(cs)),*tps=strlen(cs))

struct _qbbs_user   /* QBBS 2.6x user structure */
{
  char name[36],
       city[26],
       pwd[16],
       data_phone[13],
       home_phone[13],
       last_time[6],
       last_date[9];

  byte attrib;

  char flags[4];

  int credit,
      pending,
      times_posted,
      highmsgread,
      seclvl,
      times,
      ups,
      downs,
      upk,
      downk,
      todayk,
      elapsed,
      len;

  int combinedptr;

  byte age;

  byte rsvd1[5];
};

/* Attrib:

    0x01: Deleted
    0x02: Formfeeds
    0x04: MORE prompt
    0x08: ANSI
    0x10: No kill
    0x20: Ignore download hours
    0x40: ANSI full-screen editor
    0x80: Reserved
*/



/* Bits for usr.flag, below. */

#define UFLAG_deleted   0x01
#define UFLAG_permanent 0x02

/* Bits for usr.bits, below */

#define BITS_hotkeys    0x01    /* Hotkeys, independent of HOTFLASH level */



#define hotkeys(usr)  hot_keys(&usr)


struct   _usr102    /* Maximus 1.02 user structure */
   {
      byte name[36];       /* Caller's name                                 */
/*>*/ byte city[36];       /* Caller's location                             */

/**/  byte realname[21];   /* MAX: user's real name                         */
/**/  byte phone[15];      /* MAX: user's phone number                      */
/**/
/**/  word lastread_ptr;   /* MAX: a num which points to offset in LASTREAD */
/**/                       /* file -- Offset of lastread pointer will be    */
/**/                       /* lastread_ptr*sizeof(int).                     */
/**/  word timeremaining;  /* MAX: time left for current call (xternal prog)*/
      byte pwd[16];        /* Password                                      */
      word times;          /* Number of previous calls to this system       */

/**/  byte help;           /* Help level                                    */
/**/  byte rsvd1;

/*>*/ byte tabs;           /* 0=transmit <sp> instead of <tab>              */
/**/  byte rsvd2;

/**/  byte nulls;          /* Number of Nulls (delays) after <cr>           */
/**/  byte bits;           /* More bit flags for user                       */

      word msg;            /* Last message area visited                     */

/**/  bit  bad_logon: 1;   /* MAX: if user's last logon attempt was bad     */
/**/  bit  ibmchars : 1;   /* MAX: if user can receive high-bit chars       */
/**/  bit  avatar   : 1;   /* MAX: if user can handle AVATAR graphics       */
      bit  use_lore : 1;   /* Use the line-oriented editor                  */
      bit  more     : 1;   /* Wants the "MORE?" prompt                      */
      bit  ansi     : 1;   /* OPUS: set=wants Ansi                          */
      bit  kludge   : 1;   /* OPUS: set=used Maximus before                 */
      bit  formfeed : 1;   /* OPUS: set=transmit <ff>, clear=ignore <ff>    */
/**/  bit  key      : 8;   /* MAX: lock/key system                          */

/*>*/ sword priv;          /* Access level                                  */
      byte ldate[19];      /* Date of previous call (AsciiZ string)         */
/**/  byte struct_len;     /* MAX: len of struct, divided by 20. SEE ABOVE! */
      word time;           /* Time on-line so far today                     */

      word flag;           /* Used to hold baud rate for O)utside command   */
                           /* In USER.BBS, usr.flag uses the constants      */
                           /* UFLAG_xxx, defined earlier in this file.      */
      
      word upld;           /* K-bytes uploaded, all calls                   */
      word dnld;           /* K-bytes downloaded, all calls                 */
      word dnldl;          /* K-bytes downloaded, today                     */
      word files;          /* Last file area visited                        */
      byte width;          /* Width of the caller's screen                  */
      byte len;            /* Height of the caller's screen                 */
      word credit;         /* Matrix credit, in cents                       */
      word debit;          /* Current matrix debit, in cents                */
   };



/* RBBS user structure */

struct _rbbsu
{
  char name[31];
  char pwd[15];
  char security[2];
  char logon_opts[14];
  char city[24];
  char rsvd[3];
  char dl_today_num[4];
  char dl_today_bytes[4];
  char dl_total_bytes[4];
  char ul_total_bytes[4];
  char lastcall[14];
  char lastdir[3];
  char dl_total_num[2];
  char ul_total_num[2];
  char time_elapsed[2];
};


