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

/* $Id: msgapi.h,v 1.4 2003/06/11 14:03:07 wesgarland Exp $ */

#ifndef __SQAPI_H_DEFINED
#define __SQAPI_H_DEFINED
#define __MSGAPI_H_DEFINED

#include <stdio.h>
#include <stdlib.h>
#include "stamp.h"
#include "typedefs.h"
#include "compiler.h"

#define MSGAPI
#define MSGAPI_VERSION  1

struct _msgapi;
struct _msgh;
struct _xmsg;


/* Compatibility with MSGAPI v0.0 */

#if !defined(MSGAPI_NO_OLD_TYPES) && !defined(_WINDOWS_)
  typedef struct _msgapi MSG;
  typedef struct _msgh MSGH;
#endif


/* Handle special code for OS/2 DLLs */

#if defined(OS_2)

  #ifdef __FLAT__ /* OS/2 2.0 */
    #ifndef EXPENTRY
      #define EXPENTRY _System
    #endif

    #define OS2LOADDS
    #define OS2FAR

    #ifndef MSGAPI_NO_OLD_TYPES
      #define MSG MSG
      #define MSGH MSGH
    #endif
  #else
    #ifndef EXPENTRY
      #define EXPENTRY pascal far _loadds
    #endif

    #define OS2LOADDS _loadds
    #define OS2FAR far

    #ifndef MSGAPI_NO_OLD_TYPES
      #define MSG MSG far
      #define MSGH MSGH far
    #endif
  #endif

#elif defined(NT)
    #define OS2LOADDS
    #define OS2FAR

    #define EXPENTRY pascal
#else
#if !defined(UNIX)
 #define EXPENTRY pascal
#else
 #define EXPENTRY
#endif
  #define OS2LOADDS
  #define OS2FAR
#endif

#ifdef __WATCOMC__
  #if defined(OS_2) && !defined(__FLAT__)
    #define MAPIENTRY far pascal  /* WC segmented model doesn't need loadds */
  #else
    #define MAPIENTRY EXPENTRY
  #endif
#else
  #define MAPIENTRY EXPENTRY
#endif

/* Definitions for MsgAPI 0.4+ */

typedef struct _msgapi OS2FAR *HAREA;
typedef struct _msgh OS2FAR *HMSG;

typedef struct _xmsg XMSG;
typedef struct _xmsg OS2FAR *PXMSG;

typedef dword UMSGID;


/* MsgOpenArea(name, MODE, type); */

#define MSGAREA_NORMAL  0x00
#define MSGAREA_CREATE  0x01
#define MSGAREA_CRIFNEC 0x02


/* MsgOpenArea(name, mode, TYPE); */

#define MSGTYPE_SDM     0x01
#define MSGTYPE_SQUISH  0x02
#define MSGTYPE_ECHO    0x80


/* MsgOpenMsg(mh, mode, N); */

#define MSGNUM_CUR      (dword)-1L
#define MSGNUM_PREV     (dword)-2L
#define MSGNUM_NEXT     (dword)-3L

#define MSGNUM_current  MSGNUM_CUR
#define MSGNUM_previous MSGNUM_PREV
#define MSGNUM_next     MSGNUM_NEXT

/* MsgOpenMsg(mh, MODE, n); */

#define MOPEN_CREATE    0
#define MOPEN_READ      1
#define MOPEN_WRITE     2
#define MOPEN_RW        3

/* MsgUidToMsgn(mh, umsgid, TYPE) */

#define UID_EXACT     0x00
#define UID_NEXT      0x01
#define UID_PREV      0x02

/* Values for 'msgapierr' */

#define MERR_NONE   0     /* No error                                       */
#define MERR_BADH   1     /* Invalid handle passed to function              */
#define MERR_BADF   2     /* Invalid or corrupted file                      */
#define MERR_NOMEM  3     /* Not enough memory for specified operation      */
#define MERR_NODS   4     /* Maybe not enough disk space for operation      */
#define MERR_NOENT  5     /* File/message does not exist                    */
#define MERR_BADA   6     /* Bad argument passed to msgapi function         */
#define MERR_EOPEN  7     /* Couldn't close - messages still open           */
#define MERR_NOLOCK 8     /* Base needs to be locked to perform operation   */
#define MERR_SHARE  9     /* Resource in use by other process               */
#define MERR_EACCES 10    /* Access denied (can't write to read-only, etc)  */
#define MERR_BADMSG 11    /* Bad message frame (Squish)                     */
#define MERR_TOOBIG 12    /* Too much text/ctrlinfo to fit in frame (Squish)*/


/* Bitmasks for XMSG.attr */

#define MSGPRIVATE 0x0001
#define MSGCRASH   0x0002
#define MSGREAD    0x0004
#define MSGSENT    0x0008
#define MSGFILE    0x0010
#define MSGFWD     0x0020
#define MSGORPHAN  0x0040
#define MSGKILL    0x0080
#define MSGLOCAL   0x0100
#define MSGHOLD    0x0200
#define MSGXX2     0x0400
#define MSGFRQ     0x0800
#define MSGRRQ     0x1000
#define MSGCPT     0x2000
#define MSGARQ     0x4000
#define MSGURQ     0x8000

#define MSGSCANNED 0x00010000L  /* Message has been exported to the network */
#define MSGUID     0x00020000L  /* xmsg.uid field contains umsgid of msg */


/* Field sizes in XMSG */

#define XMSG_FROM_SIZE  36
#define XMSG_TO_SIZE    36
#define XMSG_SUBJ_SIZE  72

#define XMSG_SIZE (94 + XMSG_FROM_SIZE + XMSG_TO_SIZE + XMSG_SUBJ_SIZE)

/* Number of reply fields in XMSG.replies */

#define MAX_REPLY 9

#ifndef cpp_begin
  #ifdef __cplusplus
    #define cpp_begin()   extern "C" {
    #define cpp_end()     }
  #else
    #define cpp_begin()
    #define cpp_end()
  #endif
#endif



/* Initialization structure */

cpp_begin()
  struct _minf
  {
    /* Version 0 information */

    word req_version;                       /* Should always be MSGAPI_VERSION */
    word def_zone;
    word haveshare;           /* no need to set - filled in by msgapi routines */

    /* Version 1 information */

    void OS2FAR * (MAPIENTRY *palloc)(size_t size);
    void (MAPIENTRY *pfree)(void OS2FAR *ptr);
    void OS2FAR * (MAPIENTRY *repalloc)(void OS2FAR *ptr, size_t size);

    void far * (MAPIENTRY *farpalloc)(size_t size);
    void (MAPIENTRY *farpfree)(void far *ptr);
    void far * (MAPIENTRY *farrepalloc)(void far *ptr, size_t size);
  };
cpp_end()

/* The network address structure.  The z/n/n/p fields are always             *
 * maintained in parallel to the 'ascii' field, which is simply an ASCII     *
 * representation of the address.  In addition, the 'ascii' field can        *
 * be used for other purposes (such as internet addresses), so the           *
 * contents of this field are implementation-defined, but for most cases,    *
 * should be in the format "1:123/456.7" for Fido addresses.                 */

#ifndef __netaddr_defined
#define __netaddr_defined
  struct _netaddr;
  typedef struct _netaddr NETADDR;

  struct _netaddr
  {
    word zone;
    word net;
    word node;
    word point;
  };
#endif /* __netaddr_defined */

typedef NETADDR OS2FAR *PNETADDR;

/* The eXtended message structure.  Translation between this structure, and *
 * the structure used by the individual message base formats, is done       *
 * on-the-fly by the API routines.                                          */

struct _xmsg
{
  dword attr;

  byte from[XMSG_FROM_SIZE];
  byte to[XMSG_TO_SIZE];
  byte subj[XMSG_SUBJ_SIZE];

  NETADDR orig;        /* Origination and destination addresses             */
  NETADDR dest;

  union _stampu date_written;   /* When user wrote the msg (UTC)            */
  union _stampu date_arrived;   /* When msg arrived on-line (UTC)           */
  sword utc_ofs;                /* Offset from UTC of message writer, in    *
                                 * minutes.                                 */

  UMSGID replyto;               /* This is a reply to message #x            */
  UMSGID replies[MAX_REPLY];    /* Replies to this message                  */
  dword umsgid;                 /* UMSGID of this message, if (attr&MSGUID) */
                                /* This field is only stored on disk -- it  *
                                 * is not read into memory.                 */

  byte __ftsc_date[20];/* Obsolete date information.  If it weren't for the *
                        * fact that FTSC standards say that one cannot      *
                        * modify an in-transit message, I'd be VERY         *
                        * tempted to axe this field entirely, and recreate  *
                        * an FTSC-compatible date field using               *
                        * the information in 'date_written' upon            *
                        * export.  Nobody should use this field, except     *
                        * possibly for tossers and scanners.  All others    *
                        * should use one of the two binary datestamps,      *
                        * above.                                            */
};





/* This is a 'message area handle', as returned by MsgOpenArea(), and       *
 * required by calls to all other message functions.  This structure        *
 * must always be accessed through the API functions, and never             *
 * modified directly.                                                       */

cpp_begin()
struct _msgapi
{
  #define MSGAPI_ID   0x0201414dL
    
  dword id;                       /* Must always equal MSGAPI_ID */

  word len;                       /* LENGTH OF THIS STRUCTURE! */
  word type;                      /* Type of this base - MSGTYPE_SDM, etc */

  dword num_msg;                  /* Number of msgs in this area */
  dword cur_msg;                  /* Current message number */
  dword high_msg;                 /* Highest message number in this area */
  dword high_water;               /* Message number of high water marker */

  word sz_xmsg;                   /* Size of _xmsg structure */

  byte locked;                    /* Base is locked from use by other tasks */
  byte isecho;                    /* Is this an EchoMail area?              */

  /* Function pointers for manipulating messages within this area.          */
  struct _apifuncs
  {
    sword  (MAPIENTRY *CloseArea)(HAREA mh);
    HMSG   (MAPIENTRY *OpenMsg)  (HAREA mh,word mode,dword n);
    sword  (MAPIENTRY *CloseMsg) (HMSG msgh);
    dword  (MAPIENTRY *ReadMsg)  (HMSG msgh, PXMSG msg, dword ofs,
                                 dword bytes, byte OS2FAR *text, dword cbyt,
                                 byte OS2FAR *ctxt);
    sword  (MAPIENTRY *WriteMsg) (HMSG msgh, word append, PXMSG msg,
                                 byte OS2FAR *text, dword textlen,
                                 dword totlen, dword clen, byte OS2FAR *ctxt);
    sword  (MAPIENTRY *KillMsg)  (HAREA mh, dword msgnum);
    sword  (MAPIENTRY *Lock)     (HAREA mh);
    sword  (MAPIENTRY *Unlock)   (HAREA mh);
    sword  (MAPIENTRY *SetCurPos)(HMSG msgh, dword pos);
    dword  (MAPIENTRY *GetCurPos)(HMSG msgh);
    UMSGID (MAPIENTRY *MsgnToUid)(HAREA mh, dword msgnum);
    dword  (MAPIENTRY *UidToMsgn)(HAREA mh, UMSGID umsgid, word type);
    dword  (MAPIENTRY *GetHighWater)(HAREA mh);
    sword  (MAPIENTRY *SetHighWater)(HAREA mh, dword hwm);
    dword  (MAPIENTRY *GetTextLen)(HMSG msgh);
    dword  (MAPIENTRY *GetCtrlLen)(HMSG msgh);
    UMSGID (MAPIENTRY *GetNextUid)(HAREA harea);
  } OS2FAR *api;

  /* Pointer to application-specific data.  API_SQ.C and API_SDM.C use      *
   * this for different things, so again, no applications should muck       *
   * with anything in here.                                                 */

  void *apidata;
};
cpp_end()



/* This is a 'dummy' message handle.  The other message handlers (contained *
 * in API_SQ.C and API_SDM.C) will define their own structures, with some   *
 * application-specified variables instead of other[].  Applications should *
 * not mess with anything inside the _msgh (or MSGH) structure.             */

#define MSGH_ID  0x0302484dL

#if !defined(MSGAPI_HANDLERS) && !defined(NO_MSGH_DEF)
struct _msgh
{
  HAREA ha;
  dword id;

  dword bytes_written;
  dword cur_pos;
};
#endif



#include "api_brow.h"





/* This variable is modified whenever an error occurs with the Msg...()     *
 * functions.  If msgapierr==0, then no error occurred.                     */

#ifdef OS_2 /* Imported .DLL variables are not in DGROUP */
extern word far _stdc msgapierr;
#else
extern word _stdc msgapierr;
#endif

extern struct _minf _stdc mi;



/* Now, a set of macros, which call the specified API function.  These      *
 * will map calls for 'MsgOpenMsg()' into 'SquishOpenMsg()',                *
 * 'SdmOpenMsg()', or '<insert fave message type here>'.  Applications      *
 * should always call these macros, instead of trying to call the           *
 * manipulation functions directly.                                         */

#define MsgCloseArea(mh)       (*(mh)->api->CloseArea) (mh)
#define MsgOpenMsg(mh,mode,n)  (*(mh)->api->OpenMsg)          (mh,mode,n)
#define MsgCloseMsg(msgh)      ((*(((HMSG)msgh)->ha->api->CloseMsg))(msgh))
#define MsgReadMsg(msgh,msg,ofs,b,t,cl,ct) (*(((HMSG)msgh)->ha->api->ReadMsg))(msgh,msg,ofs,b,t,cl,ct)
#define MsgWriteMsg(gh,a,m,t,tl,ttl,cl,ct) (*(((HMSG)gh)->ha->api->WriteMsg))(gh,a,m,t,tl,ttl,cl,ct)
#define MsgKillMsg(mh,msgnum)  (*(mh)->api->KillMsg)(mh,msgnum)
#define MsgLock(mh)            (*(mh)->api->Lock)(mh)
#define MsgUnlock(mh)          (*(mh)->api->Unlock)(mh)
#define MsgGetCurPos(msgh)     (*(((HMSG)msgh)->ha->api->GetCurPos))(msgh)
#define MsgSetCurPos(msgh,pos) (*(((HMSG)msgh)->ha->api->SetCurPos))(msgh,pos)
#define MsgMsgnToUid(mh,msgn)  (*(mh)->api->MsgnToUid)(mh,msgn)
#define MsgUidToMsgn(mh,umsgid,t) (*(mh)->api->UidToMsgn)(mh,umsgid,t)
#define MsgGetHighWater(mh)   (*(mh)->api->GetHighWater)(mh)
#define MsgSetHighWater(mh,n) (*(mh)->api->SetHighWater)(mh,n)
#define MsgGetTextLen(msgh)   (*(((HMSG)msgh)->ha->api->GetTextLen))(msgh)
#define MsgGetCtrlLen(msgh)   (*(((HMSG)msgh)->ha->api->GetCtrlLen))(msgh)
#define MsgGetNextUid(ha)     (*(ha)->api->GetNextUid)(ha)

/* These don't actually call any functions, but are macros used to access    *
 * private data inside the _msgh structure.                                  */

#define MsgCurMsg(mh)         ((mh)->cur_msg)
#define MsgNumMsg(mh)         ((mh)->num_msg)
#define MsgHighMsg(mh)        ((mh)->high_msg)

#define MsgGetCurMsg(mh)      ((mh)->cur_msg)
#define MsgGetNumMsg(mh)      ((mh)->num_msg)
#define MsgGetHighMsg(mh)     ((mh)->high_msg)

#define MsgStripDebris(str)          StripNasties(str)
#define MsgCreateCtrlBuf(t, n, l)    CopyToControlBuf(t, n, l)
#define MsgGetCtrlToken(where, what) GetCtrlToken(where, what)
#define MsgCvt4D(c, o, d)            ConvertControlInfo(c, o, d)
#define MsgCvtCtrlToKludge(ctrl)     CvtCtrlToKludge(ctrl)
#define MsgRemoveToken(c, w)         RemoveFromCtrl(c, w)
#define MsgGetNumKludges(txt)        NumKludges(txt)
#define MsgWrite4D(msg, wf, ctrl)    WriteZPInfo(msg, wf, ctrl)
#define MsgInvalidHmsg(mh)           InvalidMsgh(mh)
#define MsgInvalidHarea(mh)          InvalidMh(mh)


cpp_begin()
  sword MAPIENTRY MsgOpenApi(struct _minf OS2FAR *minf);
  sword MAPIENTRY MsgCloseApi(void);

  HAREA MAPIENTRY MsgOpenArea(byte OS2FAR *name, word mode, word type);
  sword MAPIENTRY MsgValidate(word type, byte OS2FAR *name);
  sword MAPIENTRY MsgBrowseArea(BROWSE *b);


  void MAPIENTRY MsgFreeCtrlBuf(char *cbuf);
  void MAPIENTRY MsgFreeCtrlToken(char *cbuf);

  byte * MAPIENTRY StripNasties(byte *str);
  byte OS2FAR * MAPIENTRY CopyToControlBuf(byte OS2FAR *txt, byte OS2FAR * OS2FAR *newtext, unsigned OS2FAR *length);
  byte OS2FAR * MAPIENTRY GetCtrlToken(byte OS2FAR *where, byte OS2FAR *what);
  void MAPIENTRY ConvertControlInfo(byte OS2FAR *ctrl, PNETADDR orig, PNETADDR dest);
  byte OS2FAR * MAPIENTRY CvtCtrlToKludge(byte OS2FAR *ctrl);
  void MAPIENTRY RemoveFromCtrl(byte OS2FAR *ctrl,byte OS2FAR *what);
  word MAPIENTRY NumKludges(char OS2FAR *txt);
  int MAPIENTRY WriteZPInfo(PXMSG msg, void (MAPIENTRY *wfunc)(byte OS2FAR *str), byte OS2FAR *kludges);



  void MAPIENTRY SquishSetMaxMsg(HAREA sq, dword max_msgs, dword skip_msgs, dword age);
  dword MAPIENTRY SquishHash(byte OS2FAR *f);



  HAREA MSGAPI SdmOpenArea(byte OS2FAR *name, word mode, word type);
  sword MSGAPI SdmValidate(byte OS2FAR *name);

  HAREA MSGAPI SquishOpenArea(byte OS2FAR *name, word mode, word type);
  sword MSGAPI SquishValidate(byte OS2FAR *name);

  sword MAPIENTRY InvalidMsgh(HMSG msgh);
  sword MAPIENTRY InvalidMh(HAREA mh);

  void _fast ParseNNN(char *netnode, NETADDR *pn, word all);

  /*sword far pascal farread(sword handle,byte far *buf,word len);
  sword far pascal farwrite(sword handle,byte far *buf,word len);*/

  int far pascal farread(int handle, byte far *buf, unsigned len);
  int far pascal farwrite(int handle, byte far *buf, unsigned len);

  byte * _fast Address(NETADDR *a);


  #ifndef MSGAPI_INIT
    extern void OS2FAR * (MAPIENTRY *palloc)(size_t size);
    extern void (MAPIENTRY *pfree)(void OS2FAR *ptr);
    extern void OS2FAR * (MAPIENTRY *repalloc)(void OS2FAR *ptr, size_t size);

    extern void far * (MAPIENTRY *farpalloc)(size_t size);
    extern void (MAPIENTRY *farpfree)(void far *ptr);
    extern void far * (MAPIENTRY *farrepalloc)(void far *ptr, size_t size);
  #endif

cpp_end()

#endif

