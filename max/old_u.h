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

/***************************************************************************
 *                                                                         *
 *  MAXIMUS-CBCS Source Code, Version 1.02                                 *
 *  Copyright 1989, 1990 by Scott J. Dudley.  All rights reserved.         *
 *                                                                         *
 *  Maximus USER.BBS structure definition                                  *
 *                                                                         *
 *  For complete details of the licensing restrictions, please refer to    *
 *  the licence agreement, which is published in its entirety in MAX.C     *
 *  and LICENCE.MAX.                                                       *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  MAXIMUS-CBCS LICENSING AGREEMENT.  IF YOU DO NOT FIND THE TEXT OF THIS *
 *  AGREEMENT IN ANY OF THE  AFOREMENTIONED FILES, OR IF YOU DO NOT HAVE   *
 *  THESE FILES, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT ONE OF THE   *
 *  ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO USE THIS    *
 *  FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE MAXIMUS-CBCS LICENSING   *
 *  AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE ABLE TO REACH WITH THE   *
 *  AUTHOR.                                                                *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley           FidoNet  1:249/106                              *
 *  777 Downing St.        IMEXnet  89:483/202                             *
 *  Kingston, Ont.         Internet f106.n249.z1.fidonet.org               *
 *  Canada - K7M 5N3       BBS      (613) 389-8315 - HST/14.4K, 24hrs      *
 *                                                                         *
 ***************************************************************************/


/* NOTE:  This structure is semi-stable.  Although it is still compatible  *
 * with the old Opus 1.03 structure, don't expect it to stay that way      *
 * for long.  In a future version, Maximus will be using a dymaic-sized    *
 * user record, making it possible to make additions without breaking      *
 * preexisting software.  You can start to code for this now in your       *
 * software, as the usr.struct_len variable indicates the length of the    *
 * current user structure, divided by twenty.  This allows us to build up  *
 * a base of utilities, and be able to switch to a new format (while still *
 * not breaking anything) in the future.  Also, if usr.sruct_len==0, then  *
 * you MUST assume that the length of the structure is actually 180 bytes  *
 * long, as Opus (and Maximus v1.00 only) did not use this field.  In      *
 * other words:                                                            *
 *                                                                         *
 * len_of_struct=(usr.struct_len ? (usr.struct_len*20) : 180)              *
 *                                                                         *
 * In addition, you can assume that all user records in the user file are  *
 * the SAME size...  ie. You can just read the first user record out of    *
 * the file, and you are assured that the rest of the records in the file  *
 * area also the same size.                                                *
 *                                                                         *
 *                                                                         *
 * Example for reading in the dynamic-sized user structure:                *
 *                                                                         *
 *    {                                                                    *
 *      struct _usr users[MAX_USERS];                                      *
 *                                                                         *
 *      int x,                                                             *
 *          userfile,                                                      *
 *          s_len;                                                         *
 *                                                                         *
 *      if ((userfile=open(ufile_name,O_RDONLY | O_BINARY))==-1)           *
 *        Error();                                                         *
 *                                                                         *
 *      read(userfile,&users[0],sizeof(struct _usr));                      *
 *                                                                         *
 *      s_len=users[0].struct_len ? users[0].struct_len*20 : 180;          *
 *                                                                         *
 *      for (x=0;x < MAX_USERS;x++)                                        *
 *      {                                                                  *
 *        lseek(userfile,(long)x*(long)s_len,SEEK_SET);                    *
 *        read(userfile,&users[x],sizeof(struct _usr));                    *
 *      }                                                                  *
 *                                                                         *
 *      close(userfile);                                                   *
 *    }                                                                    *
 *                                                                         *
 * If anything is added to the user structure, it will be appended to the  *
 * END of the structure, so you can be assured that the offsets of each    *
 * individual variable will NOT change.                                    *
 *                                                                         *
 * Also, when ADDING or DELETING users, certain special operations have    *
 * to be performed, mainly those related to the lastread pointers.  When   *
 * adding a user, the procedure is fairly simple; just make sure that      *
 * usr.lastread_ptr is a unique number, different from all others in       *
 * USER.BBS.  Although Max uses a somewhat complicated algorithm to        *
 * fill gaps in the user file, most utility programs can just read through *
 * USER.BBS, and keep a running tally of the HIGHEST usr.struct_len        *
 * variable.  Once you have that, increment it by one, and stuff it into   *
 * the usr.struct_len of the user to be added.                             *
 *                                                                         *
 * When DELETING users, you must go through the process of "cleansing"     *
 * the lastread pointers for the user you deleted.  The procedure for this *
 * is simple:  For every area listed in AREAS.CTL, open the LASTREAD.BBS   *
 * file for that area, and seek to the offset...                           *
 *                                                                         *
 *    usr.lastread_ptr*(long)sizeof(int)                                   *
 *                                                                         *
 * ...and write *two* NUL bytes (ASCII 00).                                *
 *                                                                         *
 * Please note that you do NOT need to do anything special to sort the     *
 * user file...  Since the lastread offset is stored in usr.lastread_ptr,  *
 * you can sort the user file with impunity, and even use old Opus 1.03    *
 * sort utilities.                                                         */


#define bit   unsigned
#define sword signed int
#define word  unsigned int
#define dword long int
#define byte  unsigned char

/* Bits for usr.flag, below. */

#define UFLAG_deleted   0x01
#define UFLAG_permanent 0x02

/* Bits for usr.bits, below */

#define BITS_hotkeys    0x01    /* Hotkeys, independent of HOTFLASH level */



#define hotkeys(usr)  hot_keys(&usr)


struct   _usr
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

