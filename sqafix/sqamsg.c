/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/*
 * SQAFIX: Squish Area Fix Utility For Squish Echo Mail Processor
 *
 * Message management routines
 *
 * Created: 06/Jan/92
 * Updated: 12/Jan/00
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

#ifndef UNIX
 #include <dos.h>
#endif
 #include "sqafix.h"

#include "pathdef.h"

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   d e c l a r a t i o n s                                   //
/////////////////////////////////////////////////////////////////////////////

 // Miscellaneous defines

 #define CONT_PREV_MSG  "\r( continued from the previous message )\r\r"
 #define CONT_NEXT_MSG  "\r( continued in the next message )\r"

 // Module local variables

 static XMSG msgLong;           // Message header and control info
 static HMSG hmsgLong;          // Message handle
 static UMSG umsgLong;          // Message uid
 static BOOL fLongMsg;          // TRUE if writing long message
 static BOOL fLongFrame;        // TRUE if need to write continuation frames
 static USHORT imsgLongCont;    // Message continuation counter
 static USHORT ichLongSubj;     // Message subject line length

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   s u b r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine sets message creation date/stamp
 */

 static VOID SUBENTRY DoMsgSetDttm(XMSG * pmsg)
 {
   struct tm * pTime;
   time_t sec;

   // Get the current time

   sec = time(NULL); pTime = localtime(&sec);

   // Set message creating date time stamp

   pmsg->date_written.date.yr = pTime->tm_year - 80;
   pmsg->date_written.date.mo = pTime->tm_mon + 1;
   pmsg->date_written.date.da = pTime->tm_mday;

   pmsg->date_written.time.hh = pTime->tm_hour;
   pmsg->date_written.time.mm = pTime->tm_min;
   pmsg->date_written.time.ss = pTime->tm_sec / 2;

   // Set message arrival date time stamp to be the same as message
   // creation so that those msg purgers will not bark

   pmsg->date_arrived = pmsg->date_written;
 }

/*
 * This subroutine makes up a message header
 */

 static VOID SUBENTRY DoMsgHeader(XMSG * pmsg,
                                  NETADDR * pnetFromAddr, PSZ pszFrom,
                                  NETADDR * pnetToAddr, PSZ pszTo,
                                  USHORT attr, PSZ pszSubj)
 {
   xmemset(pmsg, 0, sizeof(XMSG));
   pmsg->attr = attr;
   xstrncpy(pmsg->to,   pszTo,   sizeof(pmsg->to)   - 1);
   xstrncpy(pmsg->from, pszFrom, sizeof(pmsg->from) - 1);
   xstrncpy(pmsg->subj, pszSubj, sizeof(pmsg->subj) - 1);
   xmemcpy(&pmsg->orig, pnetFromAddr, sizeof(pmsg->orig));
   xmemcpy(&pmsg->dest, pnetToAddr,   sizeof(pmsg->dest));
 }

/*
 * This subroutine formats the ^aMSGID kludge
 */

 static USHORT SUBENTRY DoFormatMSGID(XMSG * pmsg, PCH pchCtrl)
 {
   static ULONG lStampPrev;
   ULONG lStamp, lSecs, lHund, lSecStart = (ULONG) time(NULL);
#if defined (__OS2__)
   static BOOL fInfoSeg = FALSE;
   static PGINFOSEG pgis;
   static PLINFOSEG plis;
   SEL selgis, sellis;
#elif defined (__W32__)
    W32SYSTEMTIME w32st;
#elif defined (UNIX)
    #include <time.h>
    time_t t;
#else
   union REGS regs;
#endif

   // Under OS2 get pointers to the global and local info segments once

#ifdef __OS2__
   if (!fInfoSeg) {
     DosGetInfoSeg(&selgis, &sellis);
     pgis = MAKEPGINFOSEG(selgis);
     plis = MAKEPLINFOSEG(sellis);
     fInfoSeg = TRUE;
   }
#endif

   // Make up time stamp out of number of seconds since Jan 1, 1970
   // shifted 7 bits to the left OR'ed with current system clock and
   // loop untill we get a new stamp

   do {
#if defined (__OS2__)
     lSecs = (ULONG) pgis->time;
     lHund = (ULONG) pgis->hundredths;
     DosSleep(0);
#elif defined (__W32__)
     W32GetSystemTime(&w32st);
     lSecs = (ULONG) time(NULL);
     lHund = (ULONG) w32st.wMilliseconds / 10;
#elif defined (UNIX)
     t = time(NULL);
     lSecs = (unsigned) t;
     // BO: I really doubt about that one 
     lHund = (unsigned) t / 100;
#else
     lSecs = (ULONG) time(NULL);
     regs.h.ah = 0x2c; intdos(&regs, &regs);
     lHund = (ULONG) regs.h.dl;
#endif
     lStamp = (lSecs << 7) | (lHund & 0x07f);
   } while ((lStampPrev >= lStamp) && ((ULONG) time(NULL) < lSecStart + 5));

   // Check if we finally have unique ascending ^aMSGID kludge stamp and
   // if not, use incremented largest stamp value

   if (lStampPrev >= lStamp) lStamp = lStampPrev + 1;

   // Format the ^aMSGID kludge and return its length

   return sprintf(pchCtrl, "\x01""MSGID: %s %08lx",
                  FormatNetAddr(&pmsg->orig),
                  lStampPrev = lStamp);
 }

/*
 * This subroutine formats the ^aREPLY kludge
 */

 static USHORT SUBENTRY DoFormatREPLY(XMSG * pmsg, PCH pchCtrl)
 {
   NETADDR netAddr = {0, 0, 0, 0};

   // Check if the remote request MSGID is present

   if (cfg.pszReqMSGID == NULL) return FALSE;

   // Scan in the destination MSGID address and check if ok

   if (ScanNetAddr(&netAddr, cfg.pszReqMSGID) == NULL) return FALSE;

   // Check if destination message replies to the remote request

   if (xmemcmp(&netAddr, &pmsg->dest, sizeof(NETADDR))) return FALSE;

   // Format the ^aREPLY kludge and return its length

   return sprintf(pchCtrl, "\x01""REPLY: %s", cfg.pszReqMSGID);
 }

/*
 * This subroutine formats the ^aPID kludge
 */

 static USHORT SUBENTRY DoFormatPID(XMSG * pmsg, PCH pchCtrl)
 {
   // Format the ^aPID kludge and return its length

   return sprintf(pchCtrl, "\x01""PID: %s", SQAFIX_NAME" v"VERSION);
 }

/*
 * This subroutine formats the origin line text
 */

 static USHORT SUBENTRY DoFormatTail(XMSG * pmsg, PCH pchTail, BOOL fEchoMail)
 {
   SHORT cchTail;

   // Initialize message tail making sure there is at least trailing null

   cchTail = 1;
   pchTail[0] = '\0';

   // Check if this is echomail or origin line requested and
   // add both tearline and origin if so

   if (fEchoMail || (cfg.fl & FL_ADDORIGINLINE)) {
     cchTail+= sprintf(pchTail, SQAFIX_TEAR
                       " * Origin: %.60s (%s)\r",
                       cfg.achOrigin, FormatNetAddr(&pmsg->orig));
   } else
   if (cfg.fl & FL_ADDTEARLINE) {
     cchTail+= sprintf(pchTail, SQAFIX_TEAR);
   }

   return cchTail;
 }

/*
 * This subroutine creates a new message
 */

 static HMSG SUBENTRY DoCreateNetMailMsg(XMSG * pmsg, LONG cchMsgTotal)
 {
   CHAR achCtrl[128] = "";
   USHORT cchCtrl = 0;
   HMSG hmsg;

   // Open the new message and check if ok

   if ((hmsg = MsgOpenMsg(hNetMail, MOPEN_CREATE, 0)) == NULL) {
     WriteLog("! Can't create message\n");
     return FALSE;
   }

   // Set the message creation date/time stamp

   DoMsgSetDttm(pmsg);

   // Check if we need to force ^aINTL kludge

   if (cfg.fl & FL_FORCEINTLKLUDGE)
     cchCtrl+= sprintf(achCtrl + cchCtrl, "\x01""INTL %u:%u/%u %u:%u/%u",
                       pmsg->dest.zone, pmsg->dest.net, pmsg->dest.node,
                       pmsg->orig.zone, pmsg->orig.net, pmsg->orig.node);

   // Make up the ^aMSGID kludge if necessary

   if (cfg.fl & FL_USEMSGIDKLUDGE)
     cchCtrl+= DoFormatMSGID(pmsg, achCtrl + cchCtrl);

   // Make up the ^aREPLY kludge if necessary

   if (cfg.fl & FL_USEREPLYKLUDGE)
     cchCtrl+= DoFormatREPLY(pmsg, achCtrl + cchCtrl);

   // Make up the ^aPID kludge

   cchCtrl+= DoFormatPID(pmsg, achCtrl + cchCtrl);

   // Write out the new message header and control info including the
   // trailing zero to work around the MsgApi bug

   if (MsgWriteMsg(hmsg, FALSE, pmsg, NULL, 0L, cchMsgTotal,
                   cchCtrl ? cchCtrl + 1 : 0, cchCtrl ? achCtrl : NULL) == -1) {
     WriteLog("! Can't write message header\n");
     MsgCloseMsg(hmsg);
     return FALSE;
   }

   return hmsg;
 }

/*
 * This subroutine closes the message
 */

 static VOID SUBENTRY DoCloseNetMailMsg(HMSG * phmsg)
 {
   if (MsgCloseMsg(*phmsg) == -1) WriteLog("! Can't close message\n");
   *phmsg = NULL;
 }

/*
 * This subroutine returns msgapi error text
 */

 static PSZ SUBENTRY DoGetOpenError(VOID)
 {
   switch (msgapierr) {
     case MERR_NONE:   return "no error";
     case MERR_BADH:   return "invalid handle";
     case MERR_BADF:   return "base locked or damaged";
     case MERR_NOMEM:  return "not enough memory";
     case MERR_NODS:   return "not enough disk space";
     case MERR_NOENT:  return "base locked or does not exist";
     case MERR_BADA:   return "bad argument passed to function";
     case MERR_EOPEN:  return "messages still open";
     case MERR_NOLOCK: return "base needs to be locked";
     case MERR_SHARE:  return "base in use by other process";
     case MERR_EACCES: return "access denied";
     case MERR_BADMSG: return "bad message frame";
     case MERR_TOOBIG: return "message text too long";
   }

   return "error unknown";
 }

/*
 * This subroutine finishes sending of a long message part
 */

 static BOOL SUBENTRY DoSendLongPart(VOID)
 {
   CHAR achTail[128];
   SHORT cchTail;

   // Check if we're actually sending a long message

   if (hmsgLong == NULL)
     return FALSE;

#ifdef DEBUG
fprintf(STDAUX, "DoSendLongPart: hmsgLong=[%09Fp]\n\r", hmsgLong);
#endif

   // Append message tearline/origin and check if ok

   cchTail = DoFormatTail(&msgLong, achTail, FALSE);

   // Write out the message tail and check if ok

   if (MsgWriteMsg(hmsgLong, TRUE, NULL, achTail, cchTail, 0L, 0L, NULL) == -1)
     WriteLog("! Can't write message tearline/origin\n");

   // Close the message just written

   DoCloseNetMailMsg(&hmsgLong);

   // Report sent message

   WriteLog("- Sent msg #%lu to %s at %s, \"%s\"\n",
             umsgLong, msgLong.to, FormatNetAddr(&msgLong.dest), msgLong.subj);

   // Add it to the sent messages list

   AddSntm(umsgLong);

   // Log message if requested

   if (cfg.fl & FL_VERBOSEMODE)
     LogMsg(hNetMail, NULL, NULL, umsgLong, "NetMail", LM_CTRL | LM_BODY);

   // Show we've changed the folder

   cfg.fExitCode|= EXIT_MAILSENT;

   return TRUE;
 }

/////////////////////////////////////////////////////////////////////////////
// P u b l i c   r o u t i n e s                                           //
/////////////////////////////////////////////////////////////////////////////

/*
 * This routine opens a netmail folder
 */

 HAREA APPENTRY OpenNetMailFolder(VOID)
 {
   // Check if already opened

   if (hNetMail != NULL)
     return NULL;

   // Open the netmail folder, lock it and check if ok

   if ((hNetMail = MsgOpenArea( cfg.achNetMail
                              , MSGAREA_CRIFNEC
                              , cfg.fl & FL_SQUISHNETMAIL ? MSGTYPE_SQUISH : MSGTYPE_SDM
                              )) == NULL || MsgLock(hNetMail) == -1) {
     WriteLog("! Can't open NetMail folder -- %s\n", DoGetOpenError());
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
//       printf("Opened NetMail folder: %s%s\n", cfg.achNetMail,
//               cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : "\\*.MSG");
       printf("Opened NetMail folder: %s%s\n", cfg.achNetMail,
               cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   return hNetMail;
 }

/*
 * This routine closes a netmail folder
 */

 BOOL APPENTRY CloseNetMailFolder(VOID)
 {
   // Check if opened

   if (hNetMail == NULL)
     return FALSE;

   // Unlock and close the netmail folder and check if ok

   if (MsgUnlock(hNetMail) == -1 || MsgCloseArea(hNetMail) == -1) {
     WriteLog("! Can't close NetMail folder\n");
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Closed NetMail folder: %s%s\n", cfg.achNetMail,
//               cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : "\\*.MSG");
               cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   return TRUE;
 }

/*
 * This routine opens a badmail folder
 */

 HAREA APPENTRY OpenBadMailFolder(VOID)
 {
   HAREA hBadMail;

   // Open the bad mail folder, lock it and check if ok

   if ((hBadMail = MsgOpenArea( cfg.achBadMail
                              , MSGAREA_CRIFNEC
                              , cfg.fl & FL_SQUISHBADMAIL ? MSGTYPE_SQUISH : MSGTYPE_SDM
                              )) == NULL || MsgLock(hBadMail) == -1) {
     WriteLog("! Can't open BadMail folder -- %s\n", DoGetOpenError());
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Opened BadMail folder: %s%s\n", cfg.achBadMail,
//               cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : "\\*.MSG");
               cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   return hBadMail;
 }

/*
 * This routine closes a BadMail folder
 */

 BOOL APPENTRY CloseBadMailFolder(HAREA hBadMail)
 {
   // Check if opened

   if (hBadMail == NULL)
     return FALSE;

   // Unlock and close the netmail folder and check if ok

   if (MsgUnlock(hBadMail) == -1 || MsgCloseArea(hBadMail) == -1) {
     WriteLog("! Can't close BadMail folder\n");
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Closed BadMail folder: %s%s\n", cfg.achBadMail,
//               cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : "\\*.MSG");
               cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   return TRUE;
 }

/*
 * This routine opens an echomail folder
 */

 HAREA APPENTRY OpenEchoMailFolder(PAREA parea)
 {
   HAREA hEchoMail;

   // Open the echo mail folder, lock it and check if ok

   if ((hEchoMail = MsgOpenArea(parea->pszPath
                               , MSGAREA_CRIFNEC
                               , MSGTYPE_ECHO |
                                (IsSquishArea(parea->pszSqshFlags) ?
                                 MSGTYPE_SQUISH : MSGTYPE_SDM)
                               )) == NULL || MsgLock(hEchoMail) == -1) {
     WriteLog("! Can't open %s area mail folder -- %s: '%s%s'\n",
               parea->achTag, DoGetOpenError(), parea->pszPath,
//               IsSquishArea(parea->pszSqshFlags) ? ".SQD" : "\\*.MSG");
               IsSquishArea(parea->pszSqshFlags) ? ".SQD" : PATH_DELIMS "*.MSG");
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Opened EchoMail folder: %s%s\n", parea->pszPath,
//               IsSquishArea(parea->pszSqshFlags) ? ".SQ?" : "\\*.MSG");
               IsSquishArea(parea->pszSqshFlags) ? ".SQ?" : PATH_DELIMS "*.MSG");

   return hEchoMail;
 }

/*
 * This routine closes an echomail folder
 */

 BOOL APPENTRY CloseEchoMailFolder(PAREA parea, HAREA hEchoMail)
 {
   // Check if opened

   if (hEchoMail == NULL)
     return FALSE;

   // Unlock and close the netmail folder and check if ok

   if (MsgUnlock(hEchoMail) == -1 || MsgCloseArea(hEchoMail) == -1) {
     WriteLog("! Can't close echo mail folder for area %s\n", parea->achTag);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Closed EchoMail folder: %s%s\n", parea->pszPath,
//               IsSquishArea(parea->pszSqshFlags) ? ".SQ?" : "\\*.MSG");
               IsSquishArea(parea->pszSqshFlags) ? ".SQ?" : PATH_DELIMS "*.MSG");

   return TRUE;
 }

/*
 * This routine sending writing of a long message
 */

 BOOL APPENTRY SendLongMsgBeg(NETADDR * pnetFromAddr, PSZ pszFrom,
                              NETADDR * pnetToAddr, PSZ pszTo,
                              USHORT attr, PSZ pszSubj,
                              BOOL fFrame)
 {
   // Check if we're already sending a message and increment flag

   if (fLongMsg)
     return FALSE;
   else
     ++fLongMsg;

#ifdef DEBUG
fprintf(STDAUX, "SendLongMsgBeg: to:%s, re:%s\n\r", pszTo, pszSubj);
#endif

   // Set global write message continuation flag

   fLongFrame = fFrame;

   // Reset long message counter and buffer offset

   imsgLongCont = 0;
   ichBuf = 0;

   // Save subject line length so we can add the part counter later on

   ichLongSubj = xstrlen(pszSubj);

   // Make up the new message header

   DoMsgHeader(&msgLong, pnetFromAddr, pszFrom, pnetToAddr, pszTo, attr, pszSubj);

   return TRUE;
 }

/*
 * This routine continues sending of a long message
 */

 BOOL APPENTRY SendLongMsgPut(PSZ pszText)
 {
   LONG cchText = xstrlen(pszText);
   PCH pch;

   // Check if we're sending a message

   if (!fLongMsg)
     return FALSE;

   // Check if have previous message left hanging and if so,
   // append 'continued in the next' and finish it, then update
   // subject line of the next message to have (part n)

   if (imsgLongCont > 0 && hmsgLong != NULL) {
     if (fLongFrame)
       if (MsgWriteMsg(hmsgLong, TRUE, NULL, CONT_NEXT_MSG,
                       lengof(CONT_NEXT_MSG), 0L, 0L, NULL) == -1)
         WriteLog("! Can't write message continuation line\n");
     DoSendLongPart();
     if (fLongFrame)
       sprintf(msgLong.subj + ichLongSubj, " (part %u)", imsgLongCont + 1);
   }

   // Create the message we're going to write in. Note that we reserve
   // some space on a Squish base message frame to leave room for
   // continuation and origin lines

   if ((hmsgLong = DoCreateNetMailMsg(&msgLong, cchText + 256)) == 0)
     return FALSE;
   else
     umsgLong = MsgMsgnToUid(hNetMail, MsgGetHighMsg(hNetMail));

#ifdef DEBUG
fprintf(STDAUX, "SendLongMsgPut: hmsgLong=[%09Fp], umsgLong=%lu\n\r", hmsgLong, umsgLong);
#endif

   // Check if this is not the first message and write the
   // 'continued from the previous' before the message text

   if (fLongFrame && imsgLongCont > 0)
     if (MsgWriteMsg(hmsgLong, TRUE, NULL, CONT_PREV_MSG,
                     lengof(CONT_PREV_MSG), 0L, 0L, NULL) == -1)
       WriteLog("! Can't write message continued line\n");

   // Convert all the \n to \r to conform Fido standards

   for (pch = pszText; *pch; pch++)
     if (*pch == '\n') *pch = '\r';

   // Write out the message body

   if (MsgWriteMsg(hmsgLong, TRUE, NULL, pszText, cchText, 0L, 0L, NULL) == -1)
     WriteLog("! Can't write message body\n");

   // Increment the message counter and exit.  Leave message just written
   // for the case if we would be appending 'continued in the next' text

   imsgLongCont++;

   return TRUE;
 }

/*
 * This routine finishes sending of a long message
 */

 BOOL APPENTRY SendLongMsgEnd(VOID)
 {
   // Check if we're sending a message

   if (!fLongMsg)
     return FALSE;

   // Flush the rest of the report message buffer

   if (ichBuf > 0) SendLongMsgPut(pchBuf);

   // Finish writing the long message part

   DoSendLongPart();

   // Decrement long message flag

   --fLongMsg;

#ifdef DEBUG
fprintf(STDAUX, "SendLongMsgEnd: hmsgLong=[%09Fp]\n\r", hmsgLong);
#endif

   return TRUE;
 }

/*
 * This routine sends a short message to a specified node
 */

 BOOL APPENTRY SendMsg(NETADDR * pnetFromAddr, PSZ pszFrom,
                       NETADDR * pnetToAddr, PSZ pszTo,
                       USHORT attr, PSZ pszSubj, PSZ pszText)
 {
   SHORT cchTail, cchText = xstrlen(pszText);
   CHAR achTail[128];
   UMSG umsg;
   HMSG hmsg;
   XMSG msg;

   // Make up the new message header and format the message tearline/origin

   DoMsgHeader(&msg, pnetFromAddr, pszFrom, pnetToAddr, pszTo, attr, pszSubj);
   cchTail = DoFormatTail(&msg, achTail, FALSE);

   // Create the new message and check if ok

   if ((hmsg = DoCreateNetMailMsg(&msg, cchText + cchTail)) == 0)
     return FALSE;

   // Write out new message text and the origin line. Note that
   // the trailing null is also written at the end of the message
   // tearline/origin

   if (MsgWriteMsg(hmsg, TRUE, NULL, pszText, cchText, 0L, 0L, NULL) == -1 ||
       MsgWriteMsg(hmsg, TRUE, NULL, achTail, cchTail, 0L, 0L, NULL) == -1) {
     WriteLog("! Can't write message\n");
     DoCloseNetMailMsg(&hmsg);
     return FALSE;
   }

   // Close the message just written

   DoCloseNetMailMsg(&hmsg);

   // Report sent message

   WriteLog("- Sent msg #%lu to %s at %s, \"%s\"\n",
             umsg = MsgMsgnToUid(hNetMail, MsgGetHighMsg(hNetMail)),
             msg.to, FormatNetAddr(&msg.dest), msg.subj);

   // Add it to the sent messages list

   AddSntm(umsg);

   // Log message if requested

   if (cfg.fl & FL_VERBOSEMODE)
     LogMsg(hNetMail, NULL, NULL, umsg, "NetMail", LM_CTRL | LM_BODY);

   // Show we've changed the folder

   cfg.fExitCode|= EXIT_MAILSENT;

   return TRUE;
 }

/*
 * This routine posts a message into the echo mail area
 */

 BOOL APPENTRY PostMsg(PAREA parea,
                       NETADDR * pnetFromAddr, PSZ pszFrom,
                       NETADDR * pnetToAddr, PSZ pszTo,
                       USHORT attr, PSZ pszSubj, PSZ pszText)
 {
   CHAR achCtrl[128], achTail[128];
   SHORT cchText = xstrlen(pszText);
   SHORT cchCtrl, cchTail;
   HAREA hEchoMail;
   UMSG umsg;
   HMSG hmsg;
   XMSG msg;

   // Open the echo area mail area folder and check if ok

   if ((hEchoMail = OpenEchoMailFolder(parea)) == NULL)
     return FALSE;

   // Make up the new message header

   DoMsgHeader(&msg, pnetFromAddr, pszFrom, pnetToAddr, pszTo, attr, pszSubj);
   DoMsgSetDttm(&msg);

   // Format additional kludges and message tearline/origin

   cchCtrl = DoFormatMSGID(&msg, achCtrl);
   cchCtrl+= DoFormatPID(&msg, achCtrl + cchCtrl);
   cchTail = DoFormatTail(&msg, achTail, TRUE);

   // Check if this is a *.msg style folder and if so, reset the zone
   // and point info to prevent the address extension kludge generation
   // which are unnecessary in echo areas

   if (!IsSquishArea(parea->pszSqshFlags)) {
     msg.orig.zone = msg.dest.zone = minf.def_zone;
     msg.orig.point = msg.dest.point = 0;
   }

   // Create the new message and check if ok

   if ((hmsg = MsgOpenMsg(hEchoMail, MOPEN_CREATE, 0)) == NULL) {
     WriteLog("! Can't create message in area %s\n", parea->achTag);
     return FALSE;
   }

   // Write out the new message header and control info including the
   // trailing null to work around the MsgApi bug

   if (MsgWriteMsg(hmsg, FALSE, &msg, NULL, 0L, cchText + cchTail,
                   cchCtrl ? cchCtrl + 1 : 0, cchCtrl ? achCtrl : NULL) == -1) {
     WriteLog("! Can't write message header in area %s\n", parea->achTag);
     MsgCloseMsg(hmsg);
     return FALSE;
   }

   // Write out new message text and the tearline/origin

   if (MsgWriteMsg(hmsg, TRUE, NULL, pszText, cchText, 0L, 0L, NULL) == -1 ||
       MsgWriteMsg(hmsg, TRUE, NULL, achTail, cchTail, 0L, 0L, NULL) == -1) {
     WriteLog("! Can't write message in area %s\n", parea->achTag);
     MsgCloseMsg(hmsg);
     return FALSE;
   }

   // Close the message just written

   MsgCloseMsg(hmsg);

   // Report message has been posted

   WriteLog("- Post msg #%lu to %s, \"%s\"\n",
            umsg = MsgMsgnToUid(hEchoMail, MsgGetHighMsg(hEchoMail)),
            parea->achTag, msg.subj);

   // Log message if requested

   if (cfg.fl & FL_VERBOSEMODE)
     LogMsg(hEchoMail, NULL, NULL, umsg, parea->achTag, LM_CTRL | LM_BODY);

   // Close echo area mail folder

   CloseEchoMailFolder(parea, hEchoMail);

   return TRUE;
 }

/*
 * This routine sends a notification message to a given address list
 */
/*
 BOOL APPENTRY SendNotify(NETADDR anetAddr[], USHORT cnetAddr,
                          PSZ pszTitle, PSZ pszText)
 {
   NETADDR netAddr, * pnetAddr;
   USHORT iAddr;
   PNODE pnode;

   // Notify all the nodes listed

   for (iAddr = 0, pnetAddr = anetAddr;
        iAddr < cnetAddr && pnetAddr->zone != 0;
        iAddr++, pnetAddr++) {

     // Get the aka address which matches the notified node zone

     xmemcpy(&netAddr, GetAddrMatch(pnetAddr), sizeof(NETADDR));

     // Create the notification message

     pnode = GetNodeFromAddr(pnetAddr);
     SendMsg(&netAddr, SQAFIX_NAME, pnetAddr, GetNodeSysop(pnode),
             GetNodeMsgAttr(pnode), pszTitle, pszText);
   }

   return iAddr;
 }
*/
/*
 * This routine posts a notification message to a given echoarea list
 */
/*
 BOOL APPENTRY PostNotify(PSZ apszArea[], USHORT cpszArea,
                          PSZ pszTitle, PSZ pszText)
 {
   NETADDR netAddr;
   USHORT iArea;
   PAREA parea;

   // Post notification message to all the listed areas

   for (iArea = 0; iArea < cpszArea && apszArea[iArea] != NULL; iArea++) {
     if ((parea = GetAreaFromTag(apszArea[iArea])) == NULL) {
       WriteLog("! Can't post notification message to unknown area %s\n",
                 apszArea[iArea]);
     } else {

       // Get the area originating address as it's known to Squish. This may
       // be the primary address or the alternate address specified in the
       // -p<node> flag

       GetAreaOrigAddr(parea, &netAddr);

       // Create the echo area notification message

       PostMsg(parea, &netAddr, SQAFIX_NAME, &netAddr, "All", MSGLOCAL,
               pszTitle, pszText);
     }
   }

   return iArea;
 }
*/
/*
 * This routine scans buffer to get a specified kludge associated info
 */

 PSZ APPENTRY GetMsgKludge(PSZ pszKludge, PCH pch, USHORT cch, BOOL fLast)
 {
   SHORT cchText, cchKludge = xstrlen(pszKludge);
   PCH pchText;
   PSZ pszText;

   // Scan buffer to get pointer to the first or last kludge instance

   for (pchText = NULL; *pch && cch > 0; pch++, cch--) {
     if (*pch == '\x01' && !xmemcmp(pszKludge, pch + 1, cchKludge)) {
       pchText = pch + cchKludge + 1; cchText = cch;
       if (!fLast) break;
     }
   }

   // Check if we got the kludge requested and return now if not

   if (pchText == NULL)
     return NULL;

   // Skip over the leading spaces if any and scan past
   // the end of the kludge data to get its length

   while (*pchText == ' ' && cchText > 0) {
     pchText++; cchText--;
   }

   for (pch = pchText, cch = cchText, cchText = 0;
       *pch && cch > 0 && *pch != '\x01' && *pch != '\r' && *pch != '\n';
        pch++, cch--, cchText++);

   // Allocate memory to hold the kludge data and copy it in there

   if ((pszText = MemAlloc(cchText + 1, MA_CLEAR)) != NULL)
     xmemcpy(pszText, pchText, cchText);

   return pszText;
 }

/*
 * This routine gets the node address echo message was exported from
 */

 BOOL APPENTRY GetEchoMsgPath(HMSG hmsg, XMSG * pmsg, UMSG umsg,
                              NETADDR * pnetAddr)
 {
   ULONG cb = MsgGetTextLen(hmsg);
   PCH pch, pchText;
   USHORT cchText;
   PSZ pszPath;

   // Check for huge message

   if (cb > 0x0FF00lu) {
Drop:WriteLog("- Msg# %lu is too long\n", umsg);
     return FALSE;
   }

   // Get short length of the message body and check if ok

   if ((cchText = (USHORT) cb) == 0)
     return FALSE;

   // Check to see if message body fits into the working buffer and if not,
   // try to allocate memory to hold the entire message body

   if (cchText + 1 < cchBuf)
     pchText = pchBuf;
   else
     if ((pchText = MemAlloc(cchText + 1, 0)) == NULL)
       goto Drop;

#ifdef DEBUG
//fprintf(STDAUX, "GetEchoMsgPath: pchBuf=[%09Fp], pchText=[%09Fp], cb=%lu\r\n", pchBuf, pchText, cb);
#endif

   // Read the message body to get the PATH kludges

   if ((cchText = MsgReadMsg(hmsg, NULL, 0L, cchText, pchText, 0L, NULL)) == (USHORT)-1) {
     WriteLog("! Can't read body of msg #%lu\n", umsg);
     if (pchText != pchBuf) MemFree(pchText);
     return FALSE;
   } else {
     pchText[cchText] = '\0';
     cchText = xstrlen(pchText);
   }

   // Get the data associated with the last instance of the PATH kludge

   if ((pszPath = GetMsgKludge("PATH:", pchText, cchText, TRUE)) == NULL) {
     WriteLog("- Msg# %lu has no PATH kludge\n", umsg);
     if (pchText != pchBuf) MemFree(pchText);
     return FALSE;
   }

   // Initialize the network address

   pnetAddr->zone  = (USHORT)-1;
   pnetAddr->net   = 0;
   pnetAddr->node  = 0;
   pnetAddr->point = 0;

   // Scan in the PATH kludge data to get the last node address
   // the message was exported from

   pch = pszPath;
   while (SkipSpaces(&pch))
     if ((pch = ScanNetAddr(pnetAddr, pch)) == NULL) {
       WriteLog("- Msg# %lu has invalid address in PATH kludge\n", umsg);
       if (pchText != pchBuf) MemFree(pchText);
       MemFree(pszPath);
       return FALSE;
     }

   // Clean up and exit

   if (pchText != pchBuf) MemFree(pchText);
   MemFree(pszPath);

   return TRUE;
 }

/*
 * This subroutine logs the message
 */

 BOOL APPENTRY LogMsg(HAREA harea, HMSG hmsg, XMSG * pmsg, UMSG umsg,
                      PSZ pszArea, USHORT fs)
 {
   CHAR ach[MAX_CTRL_LENG], ch;
   BOOL fLocalOpen = FALSE;
   USHORT iAttr, cch;
   PCH pch, pchEnd;
   ULONG ich;
   MSGN msgn;
   XMSG msg;

   static struct {
     ULONG attr;
     PSZ   psz;
   } aAttr[] = {
     MSGPRIVATE,        "Pvt",
     MSGCRASH,          "Crash",
     MSGREAD,           "Recv",
     MSGSENT,           "Sent",
     MSGFILE,           "File",
     MSGFWD,            "Transit",
     MSGORPHAN,         "Orphan",
     MSGKILL,           "Kill",
     MSGLOCAL,          "Local",
     MSGHOLD,           "Hold",
     MSGXX2,            "Rsvd2",
     MSGFRQ,            "Frq",
     MSGRRQ,            "Rrq",
     MSGCPT,            "Cpt",
     MSGARQ,            "Arq",
     MSGURQ,            "Urq",
     MSGSCANNED,        "Scan"
   };

   // Check if we have no message header and get it locally

   if (pmsg == NULL) {
     pmsg = &msg;
     if (harea == NULL) return FALSE;
     if (hmsg == NULL) {
       if ((msgn = MsgUidToMsgn(harea, umsg, UID_EXACT)) == 0) return FALSE;
       if ((hmsg = MsgOpenMsg(harea, MOPEN_READ, msgn)) == NULL) return FALSE;
       fLocalOpen = TRUE;
     }
     if (MsgReadMsg(hmsg, pmsg, 0L, 0L, NULL, 0L, NULL) == -1) {
       if (fLocalOpen) MsgCloseMsg(hmsg);
       return FALSE;
     }
   }

   // Log the message header

   WriteLog("# Msg# %lu in %s\n"
            "# From %s, %s\n"
            "#   To %s, %s\n"
            "# Subj %s\n",
             umsg, pszArea,
             pmsg->from, FormatNetAddr(&pmsg->orig),
             pmsg->to,   FormatNetAddr(&pmsg->dest),
             pmsg->subj
            );

   // Log the message attributes

   WriteLog("# Attr");
   if (!pmsg->attr) {
     WriteLog(" <none>\n");
   } else {
     for (iAttr = 0; iAttr < numbof(aAttr); iAttr++)
       if (pmsg->attr & aAttr[iAttr].attr)
         WriteLog(" %s", aAttr[iAttr].psz);
     WriteLog("\n");
   }

   // Check if we need to log message control info

   if (fs & LM_CTRL) {
     if (MsgReadMsg(hmsg, NULL, 0L, 0L, NULL, lengof(ach), ach) != -1)
       WriteLog("# Ctrl %s\n", ach);
   }

   // Check if we need to log message body

   if (fs & LM_BODY) {
     WriteLog("\n# Body\n");
     for (ich = 0; ; ich+= cch) {
       cch = MsgReadMsg(hmsg, NULL, ich, lengof(ach), ach, 0L, NULL);
       if (cch == 0 || cch == (USHORT)-1) break;
       ach[cch] = '\0';

       // Write out the message body. Note that we'll do this in chunks
       // to prevent overrunning of the output buffer for very long messages

       for (pch = pchEnd = ach; (ch = *pchEnd) != '\0'; pchEnd++)
         if (ch == '\r' || ch == '\n') {
           *pchEnd = '\0';
           WriteLog("%s\n", pch);

           // Advance to the next line skipping 0d/0a sequence if needed

           pch = pchEnd + 1;
           if (ch == '\r' && *pch == '\n') {
             pch++; pchEnd++;
           }
         }

       // Write out the rest of the buffer if any

       if (pch < pchEnd) WriteLog("%s", pch);
     }

     WriteLog("\n# End# %lu in %s\n", umsg, pszArea);
   }

   // Close the message if it was opened locally

   if (fLocalOpen) MsgCloseMsg(hmsg);

   return TRUE;
 }

/*
 * End of SQAMSG.C
 */
