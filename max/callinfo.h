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

/*
 * Structure of Callers.Bbs
 * Maximus (optional) caller information & activity log
 */

#ifndef CALLINFO_H_DEFINED
#define CALLINFO_H_DEFINED

#define CALL_LOGON      0x8000                  /* Caller was logged on! */
#define CALL_CARRIER    0x0001                    /* Caller lost carrier */
#define CALL_EJECTED    0x0002    /* Caller was ejected by sysop or priv */
#define CALL_PAGED_AH   0x0004               /* Caller paged after hours */
#define CALL_DLEXCEED   0x0008         /* Caller attempted to exceed d/l */
#define CALL_EXPIRED    0x0010          /* Caller's subscription expired */
#define CALL_TIMELIMIT  0x0020             /* Caller exceeded time limit */
#define CALL_NERD       0x0040                     /* Caller was nerd'ed */
#define CALL_BARRPWD    0x0080             /* Barricade password failure */

struct callinfo
{
  byte              name[36];     /* User's name/alias                 0*/
  byte              city[36];     /* User's city                      36*/
  union stamp_combo login;        /* Time logged on                   72*/
  union stamp_combo logoff;       /* Time logged off                  76*/
  word              task;         /* Which node                       80*/
  word              flags;        /* Call flags                       82*/
  word              logon_priv;   /* Priv level on login              84*/
  word              logoff_priv;  /* Priv level on logoff             86*/
  dword             logon_xkeys;  /* Keys on login                    88*/
  dword             logoff_xkeys; /* Keys on logoff                   92*/
  word              filesup;      /* Number of files uploaded         96*/
  word              filesdn;      /* Number of files dnloaded         98*/
  word              kbup;         /* kb uploaded                     100*/
  word              kbdn;         /* kb dnloaded                     102*/
  word              calls;        /* Number of previous calls + 1    104*/
  word              read;         /* Number of messages read         106*/
  word              posted;       /* Number of messages posted       108*/
  word              paged;        /* Number of times user paged      110*/
  sword             added;        /* Time added during call          112*/
  byte              reserved[14];                                 /* 114*/
};                                                                /* 128*/


#ifdef ORCACLE

#define ci_init()
#define ci_save()
#define ci_login();
#define ci_filename(b)    (*(b)='\0')

#define ci_paged()
#define ci_read()
#define ci_posted()
#define ci_upload(szk)
#define ci_dnload(szk)
#define ci_timeadd(m)

#define ci_carrier()
#define ci_ejectuser()
#define ci_barricade()
#define ci_paged_ah()
#define ci_dlexceed()
#define ci_expired()
#define ci_timelimit()
#define ci_nerd()

#else

extern struct callinfo sci;

void ci_init(void);
void ci_save(void);
void ci_login(void);
void ci_filename(char *buf);

#define ci_paged()      (sci.paged++)
#define ci_read()       (sci.read++)
#define ci_posted()     (sci.posted++)
#define ci_upload(szk)  (sci.filesup++, sci.kbup+=(szk))
#define ci_dnload(szk)  (sci.filesdn++, sci.kbdn+=(szk))
#define ci_timeadd(m)   (sci.added+=(m))

#define ci_loggedon()   (sci.flags |= CALL_LOGON)
#define ci_carrier()    (sci.flags |= CALL_CARRIER)
#define ci_ejectuser()  (sci.flags |= CALL_EJECTED)
#define ci_barricade()  (sci.flags |= CALL_BARRPWD)
#define ci_paged_ah()   (sci.flags |= CALL_PAGED_AH)
#define ci_dlexceed()   (sci.flags |= CALL_DLEXCEED)
#define ci_expired()    (sci.flags |= CALL_EXPIRED)
#define ci_timelimit()  (sci.flags |= CALL_TIMELIMIT)
#define ci_nerd()       (sci.flags |= CALL_NERD)

#endif

#endif

