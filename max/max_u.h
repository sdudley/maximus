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

/*# name=Maximus USER.BBS structure definition
*/

#ifndef __MAX_U_H_DEFINED
#define __MAX_U_H_DEFINED

#include "typedefs.h"
#include "stamp.h"


#define MAX_ALEN         64     /* Max length of usr.msg[] and usr.files[]  */


/* Masks for usr.bits1, below */

#define BITS_HOTKEYS     0x0001 /* Hotkeys, independent of HOTFLASH level   */
#define BITS_NOTAVAIL    0x0002 /* If set, user is NOT normally available   *
                                 * for chat.                                */
#define BITS_FSR         0x0004 /* Full-screen reading in msg areas         */
#define BITS_NERD        0x0008 /* Yelling makes no noise on sysop console  */
#define BITS_NOULIST     0x0010 /* Don't display name in userlist           */
#define BITS_TABS        0x0020 /* User can handle being sent raw tabs      */
#define BITS_ENCRYPT     0x0040 /* pwd is a MD5 digest, not plaintext       */
#define BITS_RIP         0x0080 /* Remote Imaging Prototocol support        */


/* Masks for usr.bits2, below */

#define BITS2_BADLOGON   0x0001 /* MAX: if user's last logon attempt was bad*/
#define BITS2_IBMCHARS   0x0002 /* MAX: if user can receive high-bit chars  */
#define BITS2_RSVD1      0x0004 /* MAX: *obsolete* 1.02 avatar flag         */
#define BITS2_BORED      0x0008 /* Use the line-oriented editor             */
#define BITS2_MORE       0x0010 /* Wants the "MORE?" prompt                 */
#define BITS2_RSVD2      0x0020 /* OPUS: set=wants Ansi                     */
#define BITS2_CONFIGURED 0x0040 /* OPUS: set=used Maximus before            */
#define BITS2_CLS        0x0080 /* OPUS: set=transmit ^L, clear=ignore ^L   */
#define BITS2_BIT8       0x0100 /* used to be 'usr.keys'                    */
#define BITS2_BIT9       0x0200 /* used to be 'usr.keys'                    */
#define BITS2_BITA       0x0400 /* used to be 'usr.keys'                    */
#define BITS2_BITB       0x0800 /* used to be 'usr.keys'                    */
#define BITS2_BITC       0x1000 /* used to be 'usr.keys'                    */
#define BITS2_BITD       0x2000 /* used to be 'usr.keys'                    */
#define BITS2_BITE       0x4000 /* used to be 'usr.keys'                    */
#define BITS2_BITF       0x8000 /* used to be 'usr.keys'                    */

#define SEX_UNKNOWN       0x0000
#define SEX_MALE          0x0001
#define SEX_FEMALE        0x0002

/* Masks for usr.delflag, below.  WARNING!  Only the first eight bits       *
 * of this flag should be used for compatibility with df_save!              */

#define UFLAG_DEL   0x01
#define UFLAG_PERM  0x02

/* Masks for usr.xp_flag, below */

#define XFLAG_EXPDATE    0x0001 /* Use the xp_date to control access        */
#define XFLAG_EXPMINS    0x0002 /* Use the xp_mins number to control access */
#define XFLAG_DEMOTE     0x0004 /* Demote user to priv level in usr.xp_priv */
#define XFLAG_AXE        0x0008 /* Just hang up on user                     */

/* Constants for usr.video, below */

#define GRAPH_TTY         0x00 /* The current user's graphics setting...    */
#define GRAPH_ANSI        0x01 
#define GRAPH_AVATAR      0x02
#define GRAPH_RIP         0x03

typedef struct _usrndx
{
  dword hash_name;
  dword hash_alias;
} USRNDX;

struct   _usr
   {
      byte name[36];        /* Caller's name                               0*/
      byte city[36];        /* Caller's location                          36*/

      byte alias[21];       /* MAX: user's alias (handle)                 72*/
      byte phone[15];       /* MAX: user's phone number                   93*/

      word lastread_ptr;    /* MAX: a num which points to offset in      108*/
                            /* file -- Offset of lastread pointer will be   */
                            /* lastread_ptr*sizeof(int).                    */

      word timeremaining;   /* MAX: time left for current call (xtern pro110*/

      byte pwd[16];         /* Password                                  112*/
      word times;           /* Number of previous calls to this system   128*/
      byte help;            /* Help level                                130*/
      word group;           /* Group number (not implemented)            131*/
      byte video;           /* user's video mode (see GRAPH_XXXX)        133*/
      byte nulls;           /* Number of Nulls (delays) after <cr>       134*/

      byte bits;            /* Bit flags for user (number 1)             135*/

      word dob_year;        /* Date of birth: year (1900-)               136*/

      word bits2;           /* Bit flags for user (number 2)             138*/

      word max2priv;        /* Max 2.x priv level (NOT USED)             140*/
      char dataphone[19];   /* Data/business phone number                142*/
      byte struct_len;      /* len of struct, divided by 20. SEE ABOVE!  161*/
      word time;            /* Time on-line so far today                 162*/

      word delflag;         /* Used to hold baud rate for O)utside comman164*/
                            /* In USER.BBS, usr.flag uses the constants     */
                            /* UFLAG_xxx, defined earlier in this file.     */
      
      dword msgs_posted;    /* Total number of messages posted           166*/
      dword msgs_read;      /* Total number of messages read             170*/

      byte width;           /* Width of the caller's screen              174*/
      byte len;             /* Height of the caller's screen             175*/
      word credit;          /* Matrix credit, in cents                   176*/
      word debit;           /* Current matrix debit, in cents            178*/

      word xp_priv;         /* Priv to demote to, when time or minutes ru180*/
                            /* out.                                         */

      union stamp_combo xp_date;  /* Bit-mapped date of when user expires182*/
                                  /* If zero, then no expiry date.          */
 
      dword xp_mins;        /* How many minutes the user has left before 186*
                             * expiring.                                    */

      byte  xp_flag;        /* Flags for expiry.  See above XFLAG_XXX def190*/
      byte  sex;            /* Sex: SEX_MALE or SEX_FEMALE)              191*/

      union stamp_combo ludate;   /* Bit-mapped date of user's last call 192*/

      dword xkeys;          /* User's keys (all 32 of 'em)               196*/
      byte  lang;           /* The user's current language #             200*/
      sbyte def_proto;      /* Default file-transfer protocol            201*/

      dword up;             /* K-bytes uploaded, all calls               202*/
      dword down;           /* K-bytes downloaded, all calls             206*/
      sdword downtoday;     /* K-bytes downloaded, today                 210*/

      byte rsvd45[18];      /* Reserved by Maximus for future use        214*/

      word call;            /* Number of previous calls today            232*/

      byte compress;        /* Default compression program to use        234*/

      byte df_save;         /* Used for storing real usr.delflag in      235*
                             * LASTUSxx.BBS while in a door.                */

      dword extra;                                                     /*236*/
      SCOMBO date_1stcall;  /* Date of first call to system              240*/
      SCOMBO date_pwd_chg;  /* Date of last password change              244*/

      dword nup;            /* Number of files uploaded                  248*/
      dword ndown;          /* Number of files downloaded                252*/
      sdword ndowntoday;    /* Number of files downloaded today          256*/

      word time_added;      /* Time credited to the user for today       260*/
      byte msg[MAX_ALEN];   /* Current message area                      262*/
      byte files[MAX_ALEN]; /* Current file area                         326*/

      byte dob_day;         /* Date of birth: day (1-31)                 390*/
      byte dob_month;       /* Date of birth: month (1-12)               391*/
      dword point_credit;   /* Total points allocated                    392*/
      dword point_debit;    /* Total points used                         396*/
      SCOMBO date_newfile;  /* Date of last new-files check              400*/
      word priv;            /* Privilege level                           404*/
      byte rsvd6[54];       /* Reserved for future use                   406*/
                                                                       /*460*/
      /* user macros?  user address?  user note?  custom question? */
   };

#endif /* __MAX_U_H_DEFINED */

