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
 * Request executor routines
 *
 * Created: 06/Jan/92
 * Updated: 12/Jan/00
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 * Update log:
 *
 * 30 Jul 2003  rfj     Updated to use new crc32.c code.
 * 
 */

#ifndef UNIX
 #include <io.h>
 #include <dos.h>
 #include <process.h>
#endif
 #include "sqafix.h"

/* DELETE THIS SECTION ONCE NEW CRC 32 ROUTINE IS WORKING OK! *?
 */
#ifdef OLD_CRC_CODE
 #define INCL_CRC32
 #include "crc1632.h"
#else
 #include "crc32.h"
#endif
#include "sqalng.h"

#include "pathdef.h"

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   d e c l a r a t i o n s                                   //
/////////////////////////////////////////////////////////////////////////////

 // Miscellaneous defines

#define SQSH_BASENAME "@"                        // Squish msgbase file name
#define AREA_THRESHOLD (1*(cfg.cchMaxMsgLine/2)) // Areatag display threshold

 // File system special characters

#define SPEC_CHARS_HPFS "\\?*/:<>|&"
#define SPEC_CHARS_NTFS "\\?*/:<>|&"
#define SPEC_CHARS_TREE "\\?*/:<>|&"

#ifdef UNIX
// Huh?
#define P_WAIT 100
#endif


 // Module local variables

 static CHAR achLine[512];
 static PCH  pchLine = achLine;
 static SHORT cchMaxAreaLeng;
 static PSZ pszExecAreaMaskSuffix;

 // Local function prototypes

 static BOOL SUBENTRY DoSendAreaRules(PNODE pnode, PSZ pszArea, BOOL fReport);

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   s u b r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine returns length of the longest area tag known
 */

 static SHORT SUBENTRY DoGetMaxAreaLeng(VOID)
 {
   SHORT cch, cchMax = AREA_THRESHOLD;
   PAREA parea;

   // Check if this is the first time and scan only once

   if (cchMaxAreaLeng == 0)
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
       if ((cch = xstrlen(parea->achTag)) > cchMaxAreaLeng)
         cchMaxAreaLeng = cch;

   // Check if longest areatag exceeds threshold and adjust max area leng

   if (cchMaxAreaLeng > cchMax)
     cchMaxAreaLeng = cchMax;

   return cchMaxAreaLeng;
 }

/*
 * This subroutine sets the line to the formatted text
 */

 static SHORT DoSetLine(PSZ pszFormat, ...)
 {
   va_list argptr;
   SHORT cch;

   va_start(argptr, pszFormat);

   cch = GetTextLeng(argptr, pszFormat);
   if (cch < sizeof(achLine))
     pchLine = achLine + vsprintf(achLine, pszFormat, argptr);
   else {
     pchLine = achLine; cch = 0;
   }

   va_end(argptr);

   return cch;
 }

/*
 * This subroutine adds the formatted text to the line
 */

 static SHORT DoAddLine(PSZ pszFormat, ...)
 {
   va_list argptr;
   SHORT cch;

   va_start(argptr, pszFormat);

   cch = GetTextLeng(argptr, pszFormat);
   if ( (PSZ) pchLine + cch < (PSZ) achLine + sizeof(achLine))
   {
     pchLine+= vsprintf(pchLine, pszFormat, argptr);
   }
   else
   {
     cch = 0;
   }
   
   va_end(argptr);

   return cch;
 }

/*
 * This sets line to the area tag and appends dots and optional mode char
 */

 static SHORT SUBENTRY DoSetAreaName(PSZ pszArea)
 {
   SHORT cch = DoSetLine("%s ", pszArea);
   SHORT cchMax = DoGetMaxAreaLeng();
   SHORT cchDot = cchMax - cch + lengof(" ...");

   // Make up dotted padding

   if (cchDot > 0) {
     while (cchDot--)
       cch+= DoAddLine(".");
   } else {
     DoAddLine("\n");
     for (cch = 0; cch < cchMax; )
       cch+= DoAddLine(".");
     cch+= DoAddLine("....");
   }

   return cch;
 }

/*
 * This appends line with the area info
 */

 static VOID SUBENTRY DoAddAreaInfo(PAREA parea, PNODE pnode,
                                    PSZ pszStatus, BOOL fShowGroup)
 {
   BOOL fReadOnly = FALSE;
   USHORT cch, cchIndent;
   PCH pch, pchSpace;
   CHAR chGroup;
   PSZ psz;

   // Make up the group character

   if (parea->chGroup) {
     CheckGroup(pnode->pszGroup, parea->chGroup, &fReadOnly);
     chGroup = fReadOnly ? tolower(parea->chGroup) : toupper(parea->chGroup);
   } else
     chGroup = ' ';

   // Write out the area tag and group specification if requested

   if (pszStatus != NULL) {
     if (fShowGroup)
       DoAddLine(" %-10.10s [%c] ", pszStatus, chGroup);
     else
       DoAddLine(" %s ", pszStatus);
   } else
     DoAddLine(" ");

   // Calculate the area description indent

   if ((psz = xstrrchr(achLine, '\n')) == NULL) {
     cchIndent = xstrlen(achLine);
   } else {
     cchIndent = xstrlen(psz + 1);
   }

   // Check if there is no description and terminate the line if so

   if (!parea->pszDescr || !parea->pszDescr[0])
     DoAddLine("\n");
   else
     // Check if the are description fits on the single line or
     // line wrapping is disabled

     if (cfg.cchMaxMsgLine < cchIndent + 10 ||
         cfg.cchMaxMsgLine > cchIndent + xstrlen(parea->pszDescr)) {
       DoAddLine("\"%s\"\n", parea->pszDescr);
     } else {

       // Write out the leading double quota and loop through all the
       // area description writing it out on several lines

       DoAddLine("\"");
       for (psz = parea->pszDescr;;) {

         // Scan until max line length or end of line storing the latest
         // space offset

         for (cch = 0, pch = psz, pchSpace = NULL;
             *pch && cchIndent + cch < cfg.cchMaxMsgLine;
              cch++, pch++) if (isspace(*pch)) pchSpace = pch;

         // Check if this is not the first turn and indent the line

         if (psz != parea->pszDescr) DoAddLine("%*s ", cchIndent, "");

         // Check if finished in the middle of the discription and write
         // out the truncated line then advance to the next line, otherwise
         // write the line and the traling doublequota and cr

         if (*pch) {
           if (pchSpace != NULL) cch = (USHORT) (pchSpace - psz);
           DoAddLine("%.*s\n", cch, psz);
           psz = (pchSpace != NULL) ? pchSpace : pch;
           SkipSpaces(&psz);
         } else {
           DoAddLine("%s\"\n", psz);
           break;
         }
       }
     }
 }

/*
 * This subroutine writes out a report header
 */

 static VOID SUBENTRY DoWriteHeader(NETADDR * pnetAddr, PSZ pszHeader)
 {
   SHORT cch;

   // Write out header and a dashed line below

   cch = WriteMsg("\n%s %s\n", pszHeader, FormatNetAddr(pnetAddr));
   WriteMsgLine(cch - 2, '-', TRUE);

   // Reset cached max area length so that it will be recalculated

   cchMaxAreaLeng = 0;
 }

/*
 * This subroutine checks the area group status for the given node
 */

 static BOOL SUBENTRY DoCheckGroupStatus (PNODE pnode, CHAR chGroup, 
					     BOOL fGroupSpec)
 {
   // Check if node access override

   if (cfg.fl & FL_OVERRIDEGROUP) return TRUE;

   // Check if we need to look in the existing groups list

   if (!fGroupSpec)
     return CheckGroup(pnode->pszGroup, chGroup, NULL);

   // Otherwise check if the specified group list exist and the group
   // can be found there. If not, check if the specified grup list
   // refers to default group list we also need to check and leave if not

   if (pnode->pszGroupSpec) {
     if (CheckGroup(pnode->pszGroupSpec, chGroup, NULL))
       return TRUE;
     if (!CheckGroup(pnode->pszGroupSpec, '*', NULL))
       return FALSE;
   }

   // Finally check the default group list

   return (CheckGroup(cfg.pszDefNodeGroups, chGroup, NULL));
 }

/*
 * This subroutine checks the area status for the given node
 */

 static PLINK SUBENTRY DoCheckAreaStatus(PAREA parea, PNODE pnode,
                                         PBOOL pfAllowed, BOOL fGroupSpec)
 {
   PLINK plink;

   // Check if node is already linked and thus should be shown this area

   plink = GetAreaLink(parea, &pnode->netAddr);

   // Assume group is allowed for this node

   *pfAllowed = TRUE;

   // Check if node access override and assume not allowed

   if (cfg.fl & FL_OVERRIDEGROUP) return plink;

   // Check if node has sufficient access level and specified group list
   // contains this area group

   if (pnode->level < parea->level ||
      !DoCheckGroupStatus(pnode, parea->chGroup, fGroupSpec))
     *pfAllowed = FALSE;

   // Return link pointer if any

   return plink;
 }

/*
 * This subroutine checks if a specified area is not restricted
 */

 static BOOL SUBENTRY DoCheckAreaAccess(PAREA parea, PNODE pnode)
 {
   // Check if node access override

   if (cfg.fl & FL_OVERRIDEGROUP) return TRUE;

   // Check if node has sufficient access level

   if (pnode->level < parea->level) return FALSE;

   // Check if node has this group allowed

   return CheckGroup(pnode->pszGroup, parea->chGroup, NULL);
 }

/*
 * This subroutine checks if a specified area exists and is allowed
 */

 static PAREA SUBENTRY DoCheckArea(PNODE pnode, PSZ pszArea, PBOOL pfAllowed)
 {
   PAREA parea;

#ifndef UNIX
   if ((parea = GetAreaFromTag(xstrupr(pszArea))) != NULL) {
#else
   if ((parea = GetAreaFromTag(pszArea)) != NULL) {
#endif
     *pfAllowed = DoCheckAreaAccess(parea, pnode);
   } else {
     *pfAllowed = FALSE;
     if (!IsWildGrep(pszArea))
       WriteMsg("\nArea %s doesn't exist\n", pszArea);
     else
       WriteMsg("\nNo areas matching %s exist\n", pszArea);
   }

   return parea;
 }

/*
 * This subroutine makes area group list by removing not available areas
 */

 static PSZ SUBENTRY DoMakeAreaGroupList(PNODE pnode, PSZ pszGroup)
 {
   PCH pch;

   // Check if there is an area group list specified and if no
   // substitute all the allowed groups for this node

   if (pszGroup == NULL)
     return pnode->pszGroup;

   // Check if we're in override group access mode

   if (cfg.fl & FL_OVERRIDEGROUP)
     return pszGroup;

   // Check if the node is allowed to all the groups it requests
   // and reset those which are not allowed

   for (pch = pszGroup; *pch; pch++)
     if (!CheckGroup(pnode->pszGroup, *pch, NULL)) {
       if (!ISGROUP(*pch))
         WriteMsg("\nInvalid group: %c\n", *pch);
       else
         WriteMsg("\nGroup %c is restricted for node %s\n",
                  *pch, FormatNetAddr(&pnode->netAddr));

       *pch = ' ';
     }

   return pszGroup;
 }

/*
 * This subroutine locates the free space within the string
 */

 static PSZ SUBENTRY DoGetFreeSpace(PSZ psz, USHORT cch)
 {
   USHORT ich;

   while (*psz)
     if (isspace(*psz)) {
       for (ich = 0; isspace(psz[ich]); ich++);
       if (ich >= cch)
         return psz;
       else
         psz+= ich;
     } else
       psz++;

   return NULL;
 }

/*
 * This subroutine removes trailing spaces in the given string
 */

 static VOID SUBENTRY DoDelEndSpaces(PSZ psz)
 {
   USHORT ich;

   for (ich = xstrlen(psz); ich > 0 && isspace(psz[ich - 1]);
        psz[--ich] = '\0');
 }

/*
 * This subroutine checks if area is readonly for a node
 */

 static BOOL SUBENTRY DoCheckReadOnly(PAREA parea, PNODE pnode)
 {
   BOOL fReadOnly;
   CheckGroup(pnode->pszGroup, parea->chGroup, &fReadOnly);
   return fReadOnly;
 }

/*
 * This subroutine locates readonly flag in Squish flags string
 */

 static PSZ SUBENTRY DoGetReadOnlyFlag(PAREA parea, PNODE pnode)
 {
   NETADDR netAddr;
   PSZ psz;
   PCH pch;

   // Check if there are no squish flags at all

   if ((psz = parea->pszSqshFlags) == NULL) return NULL;

   // Scan the squish flags looking for the -x<addr> flag for this node

   while (SkipSpaces(&psz))
     if (psz[0] != '-') {
       WriteLog("- Area %s has invalid Squish flag: %s\n", parea->achTag, psz);
       return NULL;
     } else {
       switch (tolower(psz[1])) {
         case 'x': // ReadOnly node address
                   netAddr = cfg.anetAddr[0];
                   if ((pch = ScanNetAddr(&netAddr, &psz[2])) == NULL) {
                     WriteLog("- Area %s has invalid Squish -x flag address: %s\n", parea->achTag, psz);
                     return NULL;
                   } else
                   if (!xmemcmp(&netAddr, &pnode->netAddr, sizeof(NETADDR))) {
                     return psz;
                   } else {
                     psz = pch;
                   }
                   break;
         default : // Skip all others
                   while (*psz && !isspace(*psz)) psz++;
                   break;
       }
     }

   return NULL;
 }

/*
 * This subroutine adds readonly flag in Squish flags string
 */

 static BOOL SUBENTRY DoAddReadOnlyFlag(PAREA parea, PNODE pnode)
 {
   CHAR ach[64];
   USHORT cch;
   PCH pch;

   // Check if -x<addr> Squish flag is already there

   if ((pch = DoGetReadOnlyFlag(parea, pnode)) != NULL)
     return TRUE;

#ifdef DEBUG
fprintf(STDAUX, "DoAddReadOnlyFlag: %s for %s (%s)\r\n", FormatNetAddr(&pnode->netAddr), parea->achTag, parea->pszSqshFlags);
#endif

   // Format the -x<addr> Squish flag

   cch = sprintf(ach, " -x%s", FormatNetAddr(&pnode->netAddr));

   // Add the readonly flags for this node to squish flags of this area

   if (parea->pszSqshFlags == NULL) {
     if ((parea->pszSqshFlags = AllocString(ach, cch)) == NULL)
       goto Fail;
   } else

   // Then trying to squeeze it in the existing blank space

   if ((pch = DoGetFreeSpace(parea->pszSqshFlags, cch)) != NULL) {
     xmemcpy(pch, ach, cch);
   } else

   // And finally try to reallocate memory block

   if ((pch = MemAlloc(xstrlen(parea->pszSqshFlags) + cch + 1, 0)) != NULL) {
     xstrcpy(pch, parea->pszSqshFlags);
     MemFree(parea->pszSqshFlags); parea->pszSqshFlags = pch;
     DoDelEndSpaces(parea->pszSqshFlags);
     xstrcat(parea->pszSqshFlags, ach);
   } else {
Fail:  WriteLog("! Can't make %s readonly for %s (no memory)\n",
                FormatNetAddr(&pnode->netAddr), parea->achTag);
   }

#ifdef DEBUG
fprintf(STDAUX, "-----------------: %s for %s (%s)\r\n", FormatNetAddr(&pnode->netAddr), parea->achTag, parea->pszSqshFlags);
#endif

   // Force flags and links update

   parea->fs|= AF_SQSHFLAGSCHANGED | AF_ACTIVECHANGED;

   return TRUE;
 }

/*
 * This subroutine deletes readonly flag in Squish flags string
 */

 static BOOL SUBENTRY DoDelReadOnlyFlag(PAREA parea, PNODE pnode)
 {
   USHORT ich;
   PCH pch;

   // Check if -x<addr> Squish flag is there

   if ((pch = DoGetReadOnlyFlag(parea, pnode)) == NULL)
     return TRUE;

#ifdef DEBUG
fprintf(STDAUX, "DoDelReadOnlyFlag: %s for %s (%s)\r\n", FormatNetAddr(&pnode->netAddr), parea->achTag, parea->pszSqshFlags);
#endif

   // Remove the -x<addr> squish flag and trailing spaces if any

   for (ich = 0; pch[ich] && !isspace(pch[ich]); ich++);
   xstrcpy(pch, pch + ich);
   DoDelEndSpaces(parea->pszSqshFlags);

#ifdef DEBUG
fprintf(STDAUX, "-----------------: %s for %s (%s)\r\n", FormatNetAddr(&pnode->netAddr), parea->achTag, parea->pszSqshFlags);
#endif

   // Force flags and links update

   parea->fs|= AF_SQSHFLAGSCHANGED | AF_ACTIVECHANGED;

   return TRUE;
 }

/*
 * This subroutine creates a message to force area creation if necessary
 */

 static BOOL SUBENTRY DoSendAreaCreate(PNODE pnode, PAREA parea)
 {
   CHAR achText[256];

   // Check if we have to force area creation

   if (!(pnode->fs & NF_SENDCREATE) || (cfg.fl & FL_IGNORESENDCREATE) ||
       IsMyAka(&pnode->netAddr))
     return FALSE;

   // Format the message text

   if (!parea->pszDescr || !parea->pszDescr[0])
     sprintf(achText, "%%CREATE %.100s\r",
             parea->achTag);
   else
     sprintf(achText, "%%CREATE %.100s \"%.128s\"\r",
             parea->achTag, parea->pszDescr);

   // Send the message

   SendMsg(GetAddrMatch(&pnode->netAddr), SQAFIX_HOST,
          &pnode->netAddr, SQAFIX_NAME, GetNodeMsgAttr(pnode),
           pnode->achPassword, achText);

   return TRUE;
 }

/*
 * This subroutine send an area rules if necessary
 */

 static BOOL SUBENTRY DoSendAutoRules(PNODE pnode, PAREA parea)
 {
    // Check if we need to send rules

    if (!(pnode->fs & NF_SENDRULES) || !(parea->fs & AF_SENDRULES))
      return FALSE;

    // Send rules for a given area

    return DoSendAreaRules(pnode, parea->achTag, FALSE);
 }

/*
 * This subroutine links node to the area
 */

 static VOID SUBENTRY DoLinkNode(PNODE pnode, PAREA parea, BOOL fActive)
 {
   BOOL fReadOnly = DoCheckReadOnly(parea, pnode);
   PCH pch;

   if (AddAreaLink(parea, &pnode->netAddr, fActive,
                   fReadOnly ? "(readonly)" : "")) {
     AddToDone(parea, DN_LINKED);

     // Add the readonly flag if necessary for this node and area

     if (fActive && fReadOnly) DoAddReadOnlyFlag(parea, pnode);

     // Check if this node is one of my akas

     if (IsMyAka(&pnode->netAddr)) {

       // Check if this is active link to passthru area and if so,
       // remove the Squish passthru flag and force flags update

       if (fActive && parea->pszSqshFlags != NULL &&
          (pch = xstrstr(parea->pszSqshFlags, "-0")) != NULL) {
         pch[0] = ' '; pch[1] = ' ';
         WriteLog("- Area %s has been made non passthru\n", parea->achTag);
         parea->fs|= AF_SQSHFLAGSCHANGED | AF_ACTIVECHANGED;
       }

     } else
     if (fActive) {

       // Force area creation if +F node flag is in effect

       DoSendAreaCreate(pnode, parea);

       // Send area rules if +I node and area flags are in effect

       DoSendAutoRules(pnode, parea);
     }
   }
 }

/*
 * This subroutine unlinks node from the area
 */

 static VOID SUBENTRY DoUnlinkNode(PNODE pnode, PAREA parea, BOOL fActive)
 {
   PLINK plink;
   PCH pch;

   // Delete area link and check if ok

   if (!DelAreaLink(parea, &pnode->netAddr)) return;

   // Delete the readonly flag if necessary

   if (fActive) DoDelReadOnlyFlag(parea, pnode);

   // Report operation

   AddToDone(parea, DN_UNLINKED);

   WriteLog("- Kill %s in %s list for %s\n",
             FormatNetAddr(&pnode->netAddr),
             fActive ? "active" : "passive",
             parea->achTag);

   // Check if this active unlinked node is one of my akas

   if (!fActive || !IsMyAka(&pnode->netAddr))
     return;

   // Scan through all other links to see if any of them is also my AKA

   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext)
     if (IsMyAka(&plink->netAddr))
       return;

#ifdef DEBUG
fprintf(STDAUX, "DoUnlinkNode: my AKA %s for %s (%s)\r\n", FormatNetAddr(&pnode->netAddr), parea->achTag, parea->pszSqshFlags);
#endif

   // Check if this is nonpassthru area and if so, add the Squish
   // passthru flag by first checking if there are no flags at all

   if (parea->pszSqshFlags == NULL) {
     if ((parea->pszSqshFlags = AllocString(" -0", -1)) == NULL)
       return;
   } else

   // Then trying to squeeze it in the existing blank space

   if ((pch = xstrstr(parea->pszSqshFlags, "    ")) != NULL) {
     pch[1] = '-'; pch[2] = '0';
   } else

   // And finally try to reallocate memory block

   if ((pch = MemAlloc(xstrlen(parea->pszSqshFlags) + sizeof(" -0"), 0)) != NULL) {
     xstrcpy(pch, parea->pszSqshFlags);
     MemFree(parea->pszSqshFlags); parea->pszSqshFlags = pch;
     DoDelEndSpaces(parea->pszSqshFlags);
     xstrcat(parea->pszSqshFlags, " -0");
   } else
     return;

#ifdef DEBUG
fprintf(STDAUX, "------------: my AKA %s for %s (%s)\r\n", FormatNetAddr(&pnode->netAddr), parea->achTag, parea->pszSqshFlags);
#endif

   // Log the operation and force flags and links update

   WriteLog("- Area %s has been made passthru\n", parea->achTag);
   parea->fs|= AF_SQSHFLAGSCHANGED | AF_ACTIVECHANGED;
 }

/*
 * This subroutine moves node to passive list for the given the area
 */

 static VOID SUBENTRY DoMakeNodePassive(PNODE pnode, PAREA parea, PLINK plink)
 {
   if (SetAreaLink(parea, plink, PASSIVE)) {
     AddToDone(parea, DN_PASSIVE);
     WriteLog("- Pasv %s for %s\n",
               FormatNetAddr(&pnode->netAddr),
               parea->achTag);
   }
 }

/*
 * This subroutine moves node to active list for the given the area
 */

 static VOID SUBENTRY DoMakeNodeActive(PNODE pnode, PAREA parea, PLINK plink)
 {
   if (SetAreaLink(parea, plink, ACTIVE)) {
     AddToDone(parea, DN_ACTIVE);
     WriteLog("- Actv %s for %s\n",
               FormatNetAddr(&pnode->netAddr),
               parea->achTag);
   }
 }

/*
 * This subroutine creates new area message base tree
 */

 static BOOL SUBENTRY DoCreateDirTree(PSZ pszPath)
 {
#ifndef UNIX 
   SHORT iSaveDisk = 0;
#endif   
   SHORT fDone = FALSE;
   CHAR achSaveDir[MAXPATH];
   PCH pch, pchEnd;

#ifdef DEBUG
//fprintf(STDAUX, "DoCreateDirTree: %s\r\n", pszPath);
#endif

   // Preserve the current disk and switch to the requested disk
   // if specified setting the beginning of the path specification

#ifndef UNIX
   iSaveDisk = getdisk();
   if (pszPath[1] == ':') {
     pch = pszPath + 2;
     setdisk(toupper(*pszPath) - 'A');
     if (getdisk() != toupper(*pszPath) - 'A') {
       setdisk(iSaveDisk);
       WriteLog("$ Can't set current drive to %c:\n", toupper(*pszPath));
       return FALSE;
     }
   } else
#endif
     pch = pszPath;

   // Preserve the current directory on the specified drive and check if ok

   if (getcwd(achSaveDir, sizeof(achSaveDir)) == NULL) {
#ifdef UNIX
     WriteLog("$ Can't get current dir\n" );
#else
     WriteLog("$ Can't get current dir on %c:\n", 'A' + getdisk());
     setdisk(iSaveDisk);
#endif
     return FALSE;
   }

   // Check if have absolute path specification and change to the root
   // directory skipping over its backslash

//   if (*pch == '\\')
   if (*pch == PATH_DELIM)
   {
//     if (chdir("\\")) {
     if (chdir(PATH_DELIMS)) {
#ifdef UNIX
       WriteLog("$ Can't change to root dir\n");
       chdir(achSaveDir);
#else
       WriteLog("$ Can't change to root dir on %c:\n", 'A' + getdisk());
       chdir(achSaveDir);
       setdisk(iSaveDisk);
#endif
       return FALSE;
     } else
     {
       pch++;
     }

   }
   // Loop through the specified path directory tree creating dirs

   for (; *pch && !fDone; pch++) {

     // Get the next backslash and if there is no we're done,
     // otherwise temporarily replace it with null

//     if ((pchEnd = xstrchr(pch, '\\')) == NULL) {
     if ((pchEnd = xstrchr(pch, PATH_DELIM)) == NULL) {
       fDone = TRUE;
       break;
     } else
       *pchEnd = '\0';

     // Attempt to create directory and switch to it

     fDone = mkdir(pch);
     fDone = chdir(pch);

     // Restore the zeroed backslash and advance to the next directory

//     *pchEnd = '\\'; pch = pchEnd;
     *pchEnd = PATH_DELIM; pch = pchEnd;
   }

   // Restore the current directory on the requested drive and
   // the current drive

#ifndef UNIX
   chdir(achSaveDir);
   setdisk(iSaveDisk);
#endif

   return (!fDone || fDone == (SHORT)-1) ? FALSE : TRUE;
 }

/*
 * This subroutine returns target file system max file size
 */
#ifndef __GNUC__ 
 #pragma argsused
#endif 
 static USHORT SUBENTRY DoGetMaxFileSize(PSZ pszPath)
 {
#if defined __OS2__
   CHAR achPath[MAXPATH];
   FILESTATUS fstat;
   USHORT code;

   // Check if target file system supports long file names, generating sample
   // long file name and checking if it's valid

   if (pszPath[0] == '\0') {
//     xstrcpy(achPath, "\\");
     xstrcpy(achPath, PATH_DELIMS);
   } else
	   // Note:  Since unix doesn use :\\, the following doesn't need change for PATH_DELIMS processing....
   if (pszPath[1] == ':') {
     achPath[0] = pszPath[0]; xstrcpy(&achPath[1], ":\\");
   } else {
//     xstrcpy(achPath, "\\");
     xstrcpy(achPath, PATH_DELIMS);
   }

   // Append the sample long file name

   xstrcat(achPath, "LongFileName.Ext");

   // Verify the file name

   code = DosQPathInfo(achPath, FIL_NAMEISVALID, &fstat, sizeof(fstat), 0);

   // Check if long file name is ok and return the default OS/2 file name
   // size, otherwise return plain FAT max file name size

   return (code == 0) ? MAXFILE : 8 + 1;
#elif defined(__W32__) || defined(UNIX)

   // Under Win32 or UNIX we always assume long file naming

   return MAXFILE;
#else

   // Under DOS we always assume 8x3 file naming

   return MAXFILE;
#endif
 }

/*
 * This subroutine returns area base file name given the area tag for NTFS
 */
/***************************************************************************
 * NTFS    - makes up the base file name out of the areatag after removing *
 *           characters not valid in Win32 NTFS file names:                 *
 *                                                                         *
 *          <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC\*.MSG or                 *
 *          <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC.SQ?                      *
 ***************************************************************************/

#if defined(__W32__) || defined(UNIX)
 static BOOL SUBENTRY DoMakeNAPathNTFS(PSZ pszArea, PNEWAREA pnewarea,
                                       PSZ pszFile, BOOL lowerCase)
 {
   SHORT ich, cch, cchMax;
   PSZ psz;

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   cch = xstrlen(pszFile);
   cchMax = MAXPATH - 1 - cch;

   // Compose the area base file name from the area tag by taking the first
   // maxfile characters of it without those not allowed in the os2 file names

   for (ich = 0, psz = pszArea; ich < cchMax && *psz; psz++)
     if (!xstrchr(SPEC_CHARS_NTFS, *psz)) {
       if (lowerCase)
          pszFile[cch + ich] = tolower(*psz);
       else
         pszFile[cch + ich] = *psz;
       ich++;
     }

   // Check if we already have the message base with the same path

   return !GetAreaFromPath(pszFile);
 }
#endif

/*
 * This subroutine returns area base file name given the area tag for NTFSDIR
 */
/***************************************************************************
 * NTFSDIR - just like NTFS but creates a directory to put the squish      *
 *           message base in:                                              *
 *                                                                         *
 *           <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC\*.MSG or                *
 *           <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC\@.SQ?                   *
 ***************************************************************************/

#if defined(__W32__) || defined(UNIX)
 static BOOL SUBENTRY DoMakeNAPathNTFSDIR(PSZ pszArea, PNEWAREA pnewarea,
                                          PSZ pszFile, BOOL lowerCase)
 {
   BOOL fSquishArea = IsSquishArea(pnewarea->pszFlags);
   SHORT ich, cch, cchMax;
   PSZ psz;

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   cch = xstrlen(pszFile);
   cchMax = MAXPATH - 1 - cch;
//   if (fSquishArea) cchMax-= sizeof("\\"SQSH_BASENAME);
   if (fSquishArea) cchMax-= sizeof(PATH_DELIMS SQSH_BASENAME);

   // Compose the area base file name from the area tag by taking the first
   // maxfile characters of it without those not allowed in the os2 file names

   for (ich = 0, psz = pszArea; ich < cchMax && *psz; psz++)
     if (!xstrchr(SPEC_CHARS_NTFS, *psz)) {
       if (lowerCase)
	 pszFile[cch + ich] = tolower(*psz);
       else
         pszFile[cch + ich] = *psz;
       ich++;
     }

   // Append the trailing backslash and squish message base name

//   if (fSquishArea) xstrcat(pszFile, "\\"SQSH_BASENAME);
   if (fSquishArea) xstrcat(pszFile, PATH_DELIMS SQSH_BASENAME);

   // Check if we already have the message base with the same path

   return !GetAreaFromPath(pszFile);
 }
#endif

/*
 * This subroutine returns area base file name given the area tag for HPFS
 */
/***************************************************************************
 * HPFS    - makes up the base file name out of the areatag after removing *
 *           characters not valid in OS/2 HPFS file names:                 *
 *                                                                         *
 *          <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC\*.MSG or                 *
 *          <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC.SQ?                      *
 ***************************************************************************/

#if defined(__OS2__) || defined(UNIX)
 static BOOL SUBENTRY DoMakeNAPathHPFS(PSZ pszArea, PNEWAREA pnewarea,
                                       PSZ pszFile, BOOL lowerCase)
 {
   SHORT ich, cch, cchMax;
   PSZ psz;

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   cch = xstrlen(pszFile);
   cchMax = MAXPATH - 1 - cch;

   // Compose the area base file name from the area tag by taking the first
   // maxfile characters of it without those not allowed in the os2 file names

   for (ich = 0, psz = pszArea; ich < cchMax && *psz; psz++)
     if (!xstrchr(SPEC_CHARS_HPFS, *psz)) {
       if (lowerCase)
	 pszFile[cch + ich] = tolower(*psz);
       else
         pszFile[cch + ich] = *psz;
       ich++;
     }

   // Check if we already have the message base with the same path

   return !GetAreaFromPath(pszFile);
 }
#endif

/*
 * This subroutine returns area base file name given the area tag for HPFSDIR
 */
/***************************************************************************
 * HPFSDIR - just like HPFS but creates a directory to put the squish      *
 *           message base in:                                              *
 *                                                                         *
 *           <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC\*.MSG or                *
 *           <NewAreaPath>\UUCP.COMP.BINARY.IBM.PC\@.SQ?                   *
 ***************************************************************************/

#if defined(__OS2__) || defined(UNIX)
 static BOOL SUBENTRY DoMakeNAPathHPFSDIR(PSZ pszArea, PNEWAREA pnewarea,
                                          PSZ pszFile, BOOL lowerCase)
 {
   BOOL fSquishArea = IsSquishArea(pnewarea->pszFlags);
   SHORT ich, cch, cchMax;
   PSZ psz;

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   cch = xstrlen(pszFile);
   cchMax = MAXPATH - 1 - cch;
//   if (fSquishArea) cchMax-= sizeof("\\"SQSH_BASENAME);
   if (fSquishArea) cchMax-= sizeof(PATH_DELIMS SQSH_BASENAME);

   // Compose the area base file name from the area tag by taking the first
   // maxfile characters of it without those not allowed in the os2 file names

   for (ich = 0, psz = pszArea; ich < cchMax && *psz; psz++)
     if (!xstrchr(SPEC_CHARS_HPFS, *psz)) {
       if (lowerCase)
	 pszFile[cch + ich] = tolower(*psz);
       else
         pszFile[cch + ich] = *psz;
       ich++;
     }

   // Append the trailing backslash and squish message base name

//   if (fSquishArea) xstrcat(pszFile, "\\"SQSH_BASENAME);
   if (fSquishArea) xstrcat(pszFile, PATH_DELIMS SQSH_BASENAME);

   // Check if we already have the message base with the same path

   return !GetAreaFromPath(pszFile);
 }
#endif

/*
 * This subroutine returns area base file name given the area tag
 */
/***************************************************************************
 * TREE    - builds a directory tree out of the dot separated areatag,     *
 *           for example area with tag UUCP.COMP.BINARY.IBM.PC will be     *
 *           placed in:                                                    *
 *                                                                         *
 *           <NewAreaPath>\UUCP\COMP\BINARY\IBM\PC\*.MSG or                *
 *           <NewAreaPath>\UUCP\COMP\BINARY\IBM\PC.SQ?                     *
 ***************************************************************************/

 static BOOL SUBENTRY DoMakeNAPathTREE(PSZ pszArea, PNEWAREA pnewarea,
                                       PSZ pszFile)
 {
   CHAR achArea[MAX_AREA_LENG];
   USHORT ich, cch, cchMax;
   PCH pch;
   PSZ psz;

   // Find out max target file system file name size

   cchMax = DoGetMaxFileSize(pnewarea->achPath);

   // Make up the area tag clean from invalid characters

   for (ich = 0; *pszArea; pszArea++) {
     if (!xstrchr(SPEC_CHARS_TREE, *pszArea))
     {
       if (ich < lengof(achArea) - 1)
       {
         achArea[ich++] = toupper(*pszArea);
       }
       else
       {
         break;
       }
     }
   }
   achArea[ich] = '\0';

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   ich = xstrlen(pszFile);

   // Skip over leading dots if any

   for (psz = achArea; *psz == '.'; psz++);

   // Loop through all the characters in the tag specification

   while (*psz) {

     // Check if there is no point ahead or its too far for the
     // current file naming scheme and if so take the maxfile
     // characters of what's left

     if ((pch = xstrchr(psz, '.')) == NULL ||
         (cch = (USHORT)(pch - psz)) > cchMax - 1)
       cch = min(cchMax - 1, xstrlen(psz));

     // Copy the element of the path and append backslash

     if (ich + cch + 1 < MAXPATH - 1) {
       xmemcpy(pszFile + ich, psz, cch);
//       ich+= cch; pszFile[ich++] = '\\';
       ich+= cch; pszFile[ich++] = PATH_DELIM;
     } else
       return FALSE;

     // Advance the areatag pointer skipping dots if necessary

     for (psz+= cch; *psz == '.'; psz++);
   }

   // Remove the trailing backslash

   if (ich > 0) pszFile[ich - 1] = '\0';

   // Check if we already have the message base with the same path

   return !GetAreaFromPath(pszFile);
 }

/*
 * This subroutine returns area base file name given the area tag
 */
/***************************************************************************
 * TREEDIR - builds a directory tree out of the dot separated areatag,     *
 *           for example area with tag UUCP.COMP.BINARY.IBM.PC will be     *
 *           placed in:                                                    *
 *                                                                         *
 *           <NewAreaPath>\UUCP\COMP\BINARY\IBM\PC\*.MSG or                *
 *           <NewAreaPath>\UUCP\COMP\BINARY\IBM\PC\@.SQ?                   *
 ***************************************************************************/

 static BOOL SUBENTRY DoMakeNAPathTREEDIR(PSZ pszArea, PNEWAREA pnewarea,
                                          PSZ pszFile)
 {
   CHAR achArea[MAX_AREA_LENG];
   USHORT ich, cch, cchMax;
   PCH pch;
   PSZ psz;

   // Find out max target file system file name size

   cchMax = DoGetMaxFileSize(pnewarea->achPath);

   // Make up the area tag clean from invalid characters

   for (ich = 0; *pszArea; pszArea++) {
     if (!xstrchr(SPEC_CHARS_TREE, *pszArea))
     {
       if (ich < lengof(achArea) - 1)
       {
         achArea[ich++] = toupper(*pszArea);
       }
       else
       {
         break;
       }
     }
   }
   achArea[ich] = '\0';

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   ich = xstrlen(pszFile);

   // Skip over leading dots if any

   for (psz = achArea; *psz == '.'; psz++);

   // Loop through all the characters in the tag specification

   while (*psz) {

     // Check if there is no point ahead or its too far for the
     // current file naming scheme and if so take the maxfile
     // characters of what's left

     if ((pch = xstrchr(psz, '.')) == NULL ||
         (cch = (USHORT)(pch - psz)) > cchMax - 1)
       cch = min(cchMax - 1, xstrlen(psz));

     // Copy the element of the path and append backslash

     if (ich + cch + 1 < MAXPATH - 1) {
       xmemcpy(pszFile + ich, psz, cch);
//       ich+= cch; pszFile[ich++] = '\\';
       ich+= cch; pszFile[ich++] = PATH_DELIM;
     } else
       return FALSE;

     // Advance the areatag pointer skipping dots if necessary

     for (psz+= cch; *psz == '.'; psz++);
   }

   // Remove the trailing backslash for the msg style areas or
   // append the message base file name for the squish areas

   if (IsSquishArea(pnewarea->pszFlags)) {
     xstrcat(pszFile, SQSH_BASENAME);
   } else {
     if (ich > 0) pszFile[ich - 1] = '\0';
   }

   // Check if we already have the message base with the same path

   return !GetAreaFromPath(pszFile);
 }

/*
 * This subroutine returns area base file name given the area tag for CRC
 */
/***************************************************************************
 * CRC     - makes up an 8 character base name using hexadecimal CRC32     *
 *         value of areatag. Useful for those who have huge amount         *
 *         of echo areas, usually from uunet.                              *
 ***************************************************************************/

 static BOOL SUBENTRY DoMakeNAPathCRC(PSZ pszArea, PNEWAREA pnewarea,
                                      PSZ pszFile)
 {
   ULONG crc = -1lu;
   USHORT ich;
#ifdef OLD_CRC_CODE
   PCH pch;
#endif

   // Make up the area path

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   ich = xstrlen(pszFile);

   // Calculate crc32 of the areatag

#ifdef OLD_CRC_CODE
   for (pch = pszArea; *pch; crc = CRC32(*pch++, crc));
#else
   /* crc = crc32_init(); */
   crc = crc32_compute(pszArea, crc);
#endif

   // Loop until we get the new path formatting the crc area file name and
   // checking if we already have the message base with the same path in which
   // case we just increment crc value

   do {
     sprintf(pszFile + ich, "%08lX", crc++);
   } while (GetAreaFromPath(pszFile) != NULL);

   return TRUE;
 }

/*
 * This subroutine returns area base file name given the area tag
 */
/***************************************************************************
 * 8x3     - makes up an 8 character base name truncating areatag          *
 *           and removing characters not valid in dos file names           *
 ***************************************************************************/

 static BOOL SUBENTRY DoMakeNAPath8x3(PSZ pszArea, PNEWAREA pnewarea,
                                      PSZ pszFile, BOOL lowerCase)
 {
   SHORT ich, cch;
   PSZ psz;

   // Set the new area path and calculate its length

   xstrncpy(pszFile, pnewarea->achPath, MAXPATH - 1);
   cch = xstrlen(pszFile);

   // Compose the area base file name from the area tag by taking the first
   // eight characters of it without those not allowed in the dos file names
   // and check if we already have area base with the same name. If so, try
   // building another base name skipping first characters of the area tag

   loop {

     // Make up the base file name

     xmemset(pszFile + cch, 0, MAXPATH - cch);
     for (ich = 0, psz = pszArea; ich < 8 && *psz; psz++)
       if (!xstrchr(".?*\\/:<>|+", *psz)) {
	 if (lowerCase)
           pszFile[cch + ich] = tolower(*psz);
	 else
           pszFile[cch + ich] = *psz;
         ich++;
       }

     // Check if we have this base already and if yes, skip the
     // first character of the area tag and try once more...

     if (GetAreaFromPath(pszFile) == NULL)
       break;
     else
       if (!*++pszArea)
         return FALSE;
   }

   return TRUE;
 }

/*
 * This subroutine returns area base file name given the area tag
 */

 static PSZ SUBENTRY DoMakeNewAreaPath(PSZ pszArea, PNEWAREA pnewarea)
 {
   BOOL fSquishArea = IsSquishArea(pnewarea->pszFlags);
   CHAR achPath[MAXPATH];
   BOOL fOk = FALSE;
   PSZ psz;
   BOOL forceLowerCase = FALSE;

   // Clean up the target path

   xmemset(achPath, 0, sizeof(achPath));

   // Set flag for making new names lower case....
   forceLowerCase = pnewarea->fs & NA_CONVERTLOWER;
#ifdef DEBUG
   printf("DEBUG: forceLowerCase is %s\n",forceLowerCase ? "TRUE" : "FALSE");
   WriteLog("DEBUG: forceLowerCase is %s\n", forceLowerCase ? "TRUE" : "FALSE");
#endif
   // Make up the area base name according to new area flags

   switch (pnewarea->fs & NA_CONVERTMASK) {
#if defined(__W32__) || defined(UNIX)
     case NA_CONVERTNTFS:    fOk = DoMakeNAPathNTFS(pszArea, pnewarea, achPath, forceLowerCase); break;
     case NA_CONVERTNTFSDIR: fOk = DoMakeNAPathNTFSDIR(pszArea, pnewarea, achPath, forceLowerCase); break;
#endif
#if defined(__OS2__) || defined(UNIX)
     case NA_CONVERTHPFS:    fOk = DoMakeNAPathHPFS(pszArea, pnewarea, achPath, forceLowerCase); break;
     case NA_CONVERTHPFSDIR: fOk = DoMakeNAPathHPFSDIR(pszArea, pnewarea, achPath, forceLowerCase); break;
#endif
     case NA_CONVERTTREE:    fOk = DoMakeNAPathTREE(pszArea, pnewarea, achPath); break;
     case NA_CONVERTTREEDIR: fOk = DoMakeNAPathTREEDIR(pszArea, pnewarea, achPath); break;
     case NA_CONVERTCRC:     fOk = DoMakeNAPathCRC(pszArea, pnewarea, achPath); break;
     default:                fOk = DoMakeNAPath8x3(pszArea, pnewarea, achPath, forceLowerCase); break;
   }

   // Check if failed and leave

   if (!fOk) return NULL;

   // Create the directory tree and check if ok. Temporarily add the
   // trailing backslash and dummy file name for the msg style areas
   // so that the last directory will be created too. Squish could do
   // this on its own later, but we want to make sure now that the
   // directory may be created

//   if (!fSquishArea) xstrcat(achPath, "\\-");
   if (!fSquishArea) xstrcat(achPath, PATH_DELIMS "-");

   if (!DoCreateDirTree(achPath)) {
     WriteLog("$ Can't make dir %s\n", achPath);
     return NULL;
   }
//   if (!fSquishArea) *xstrrchr(achPath, '\\') = '\0';
   if (!fSquishArea) *xstrrchr(achPath, PATH_DELIM) = '\0';

   // Allocate the area base name and check if ok

   if ((psz = AllocString(achPath, -1)) == NULL) {
     WriteLog("! Insufficient memory (new area path)\n");
     exit(EXIT_FAILURE);
   }

   return psz;
 }

/*
 * This subroutine processes the new area description specification
 */

 static PSZ SUBENTRY DoGetNewAreaDescr(PSZ pszArea, BOOL fTailDescr)
 {
   PSZ pszDescr;
   PCH pch;

   // Check if area description is specified and if not, substitute the
   // default area description unless we locate it in the area desc list

   if (!fTailDescr || *(pszDescr = xstrchr(pszArea, 0) + 1) == 0) {
     if ((pszDescr = GetAreaDescrFromTag(pszArea)) == NULL)
       return NULL;
   } else {

     // Check for the leading double quota and skip over it

     if (*pszDescr == '"') {
       pszDescr++;
     } else {
       WriteMsg("\nMissing leading double quota in area description\n");
       return NULL;
     }

     // Check for the trailing double quota and remove it

     for (pch = pszDescr; *pch != '"'; pch++)
       if (!*pch) {
         WriteMsg("\nMissing trailing double quota in area description\n");
         return NULL;
       }
     *pch = '\0';
   }

   return AllocString(pszDescr, -1);
 }

/*
 * This subroutine returns the new area info for the given tag and node
 */

 static PNEWAREA SUBENTRY DoGetNewAreaInfo(PSZ pszArea, PNODE pnode)
 {
   PNEWAREA pnewarea;

   // Check if there are specific settings for this node

   for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
        pnewarea = pnewarea->pnewareaNext)
     if (!CompNetAddr(&pnode->netAddr, &pnewarea->netAddr))
       return pnewarea;

   // Check if there are specific settings for this area

   for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
        pnewarea = pnewarea->pnewareaNext)
     if (GrepSearch(pszArea, pnewarea->pszArea, FALSE))
       return pnewarea;

   // Otherwise assume default new area info

   return &newareaDef;
 }

/*
 * This subroutine sets the new area access level using some weird heuristics
 */

 static BOOL SUBENTRY DoSetNewAreaLevel(PAREA parea, NETADDR * pnetAddr,
                                        USHORT minLinkLevel)
 {
   BOOL fFound = FALSE;
   USHORT level = 0;
   PUPLINK puplink;
   PLSZ plsz;

   // Scan through the uplinks looking for the node address match

   for (puplink = cfg.puplinkFirst; puplink != NULL; puplink = puplink->puplinkNext)
     if (!xmemcmp(&puplink->pnode->netAddr, pnetAddr, sizeof(NETADDR))) {

       // Check if this uplink entry could be used by the lowest level
       // freq node linked and skip it if not

       if (minLinkLevel < puplink->level) continue;

       // Check if this uplink entry could be used to freq this area tag and
       // skip it if not

       for (plsz = puplink->plszMask; plsz != NULL; plsz = plsz->plszNext)
         if (GrepSearch(parea->achTag, plsz->ach, FALSE))
           break;

       if (plsz == NULL) continue;

       // Check if this uplink entry is the lowest level one

       if (level < puplink->level) {
         level = puplink->level; fFound++;
       }
     }

   // Check if uplink level has been found and set it in

   if (fFound) parea->level = level;

#ifdef DEBUG
fprintf(STDAUX, "DoSetNewAreaLevel: %s level=%u, minLinkLevel=%u\r\n", parea->achTag, parea->level, minLinkLevel);
#endif

   return fFound;
 }

/*
 * This subroutine creates a new area with the specified tag and description
 */

 static BOOL SUBENTRY DoCreateNewArea(PSZ pszArea, PSZ pszDescr, PNODE pnode)
 {
   static CHAR achPassThru[] = "-0 ";
   static CHAR achAreaRefuse[MAX_AREA_LENG];
   static CHAR achAreaFailed[MAX_AREA_LENG];
   BOOL fSquishArea, fPassThruFreq = TRUE;
   USHORT minLinkLevel = (USHORT)-1;
   NETADDR * pnetAddr;
   PNEWAREA pnewarea;
   PSZ psz, pszFile;
   SHORT iAddr, cch;
   PNODE pnodeLink;
   PUPLINK puplink;
   PAREA parea;
   PLINK plink;
   PQUE pque;
   PLSZ plsz;

   // Check if area with this tag should never be created

   if (GetLsz(cfg.plszRefuseCreate, pszArea)) {
     if (xstrcmp(achAreaRefuse, pszArea)) {
       WriteLog("- Area %s can't be created\n", pszArea);
       xstrncpy(achAreaRefuse, pszArea, lengof(achAreaRefuse));
     }

     // Return flag to kill autocreate message

     return (BOOL) -1;
   }

   // Get the new area info for the given node and areatag

   pnewarea = DoGetNewAreaInfo(pszArea, pnode);

   // Check if this area is restricted for creation and if so, skip it

   for (plsz = pnewarea->plszRefuse; plsz != NULL; plsz = plsz->plszNext) {
     if (GrepSearch(pszArea, plsz->ach, FALSE)) {
       if (xstrcmp(achAreaRefuse, pszArea)) {
         WriteLog("- Area %s matches refuse create mask %s\n", pszArea, plsz->ach);
         xstrncpy(achAreaRefuse, pszArea, lengof(achAreaRefuse));
       }

       // Return flag to kill autocreate message

       return (BOOL) -1;
     }
   }

   // Make up the area base file name and check if ok

   if (!xstrcmp(achAreaFailed, pszArea)) {
     return TRUE;
   } else
   if ((pszFile = DoMakeNewAreaPath(pszArea, pnewarea)) == NULL) {
     WriteLog("! Can't make unique file name for new area %s\n", pszArea);
     xstrncpy(achAreaFailed, pszArea, lengof(achAreaFailed));
     return TRUE;
   }

   // Allocate squish area flags and check if ok

   psz = (PSZ) pnewarea->pszFlags ? (PSZ) pnewarea->pszFlags : (PSZ) "";
   if ((psz = AllocString(psz, -1)) == NULL) {
     WriteLog("! Insufficient memory (sqsh area flags)\n");
     exit(EXIT_FAILURE);
   }

   // Create the new area descriptor and initialize it

   parea = AddArea(pszArea, pnewarea->chGroup);
   parea->fs|= AF_AUTOCREATEDAREA;
   parea->pnodeAutoCreate = pnode;
   parea->pszPath = pszFile;
   parea->pszDescr = pszDescr;
   parea->pszSqshFlags = psz;
   fSquishArea = IsSquishArea(pnewarea->pszFlags);

   // Log the new area creation

   WriteLog("* Area %s created by %s in %s%s\n",
             parea->achTag, FormatNetAddr(&pnode->netAddr),
//             pszFile, fSquishArea ?  ".SQ?" : "\\*.MSG");
             pszFile, fSquishArea ?  ".SQ?" : PATH_DELIMS "*.MSG");

   if (parea->pszDescr != NULL && parea->pszDescr[0])
     WriteLog("* Desc \"%s\"\n", parea->pszDescr);

   // Link up the node which created the new area

   AddAreaLink(parea, &pnode->netAddr, ACTIVE, "(autouplink)");

   // Link up all the autolink nodes

   for (iAddr = 0, pnetAddr = pnewarea->anetNode;
        iAddr < numbof(pnewarea->anetNode) && pnetAddr->zone != 0;
        iAddr++, pnetAddr++) {
     AddAreaLink(parea, pnetAddr, ACTIVE, "(autolink)");
   }

   // Check if this area is in freq queue

   if ((pque = GetQueEntry(parea->achTag, QE_FREQ)) != NULL) {

     // Scan through all the freq entry links. Note: the first node listed
     // in freq queue entry is an uplink the request was sent to

     for (plink = pque->plink; plink != NULL; plink = plink->plinkNext) {
       if (plink == pque->plink) {

         // Check if area is created not by the uplink the freq was sent to
         // and report this strange but possible case

         if (xmemcmp(&plink->netAddr, &pnode->netAddr, sizeof(NETADDR)))
           WriteLog("- Area %s created not by freq uplink %s\n",
                     parea->achTag, FormatNetAddr(&plink->netAddr));

         // Check if all freq areas created by this uplink should not
         // be made passthru

         if (fPassThruFreq &&
             (puplink = GetUplinkFromAddr(&plink->netAddr)) != NULL &&
             (puplink->fs & UF_FREQNOPTHRU)) {
           WriteLog("- Area %s is not passthru due to uplink %s flag\n",
                     parea->achTag, FormatNetAddr(&puplink->pnode->netAddr));
           fPassThruFreq = FALSE;
         }

       } else {

         // Link up the freq node

         AddAreaLink(parea, &plink->netAddr, ACTIVE, "(freq node)");

         // Check if this freq node is one of my akas and make area
         // non passthru if so

         if (fPassThruFreq &&
             IsMyAka(&plink->netAddr)) {
           WriteLog("- Area %s is not passthru due to linked aka %s\n",
                     parea->achTag, FormatNetAddr(&plink->netAddr));
           fPassThruFreq = FALSE;
         }

         // Get this freq node descriptor and check if ok

         if ((pnodeLink = GetNodeFromAddr(&plink->netAddr)) != NULL) {

           // Add the readonly flag if necessary for this node and area

           if (DoCheckReadOnly(parea, pnodeLink))
             DoAddReadOnlyFlag(parea, pnodeLink);

           // Force area creation if +F node flag is in effect

           DoSendAreaCreate(pnodeLink, parea);

           // Send area rules if +I node and area flags are in effect

           DoSendAutoRules(pnodeLink, parea);

           // Check if all areas requested by this node should be made
           // non passthru and do so

           if (fPassThruFreq &&
               pnodeLink->fs & NF_FREQNOPTHRU) {
             WriteLog("- Area %s is not passthru due to link %s flag\n",
                       parea->achTag, FormatNetAddr(&pnodeLink->netAddr));
             fPassThruFreq = FALSE;
           }

           // Determine the lowest linked node access level

           if (minLinkLevel > pnodeLink->level)
             minLinkLevel = pnodeLink->level;
         }
       }
     }

     // Set the new area access level to the max uplink level lower than
     // the lowest linked freq node level if any. If there were no links
     // listed in Node keywords just leave the default area group

     if (minLinkLevel != (USHORT)-1)
       DoSetNewAreaLevel(parea, &pnode->netAddr, minLinkLevel);

     // Check if the area matches the no passthru list

     if (fPassThruFreq)
       for (plsz = cfg.plszFReqKeepAreaFirst; plsz != NULL; plsz = plsz->plszNext)
         if (GrepSearch(parea->achTag, plsz->ach, FALSE)) {
           WriteLog("- Area %s is not passthru sinces it matches '%s'\n",
                     parea->achTag, plsz->ach);
           fPassThruFreq = FALSE;
           break;
         }

     // Delete the queue entry

     DelQueEntry(pque->achTag);
     WriteLog("- DelQ %s freq for autocreated area\n", parea->achTag);

     // Check if this area has -0 coming from NewAreaFlags but it
     // sould not be passthru due to freq uplink, node, aka or areatag
     // restrictions and remove -0 flag if so

     if (IsPassThruArea(parea->pszSqshFlags)) {
       if (!fPassThruFreq &&
          (psz = xstrstr(parea->pszSqshFlags, "-0")) != NULL) {
         psz[0] = ' '; psz[1] = ' ';
       }
     } else {

       // This area does not have -0 yet so check if it should be
       // made passthru and add -0 flag if so

       if (fPassThruFreq) {
         cch = xstrlen(parea->pszSqshFlags) + lengof(achPassThru);
         if ((psz = MemAlloc(cch + 1, MA_CLEAR)) == NULL) {
           WriteLog("! Insufficient memory (sqsh area flags -0)\n");
           exit(EXIT_FAILURE);
         } else {
           xstrcpy(psz, achPassThru); xstrcat(psz, parea->pszSqshFlags);
           MemFree(parea->pszSqshFlags);
           parea->pszSqshFlags = psz;
         }
       }
     }
   }

   // Force config files update for this area

   parea->fs|= AF_ACTIVECHANGED | AF_PASSIVECHANGED;

   // Show we've created new area

   cfg.fExitCode|= EXIT_PROCAREA;

   // Return no error condition

   return FALSE;
 }

/*
 * This subroutine reports new areas creation by the given node
 */

 static BOOL SUBENTRY DoReportNewAreas(PNODE pnode, PNEWAREA pnewarea)
 {
   time_t tm = time(NULL);
   USHORT iAddr, iArea, iFile, cArea, cch;
   NETADDR netAddr, * pnetAddr;
   PAREA pareaNotify, parea;
   PNODE pnodeNotify;
   CHAR ach[512];
   FILE * pfile;
   PSZ psz;

   // Compose the notification message new area list for the given node

   for (parea = cfg.pareaFirst, cArea = 0, ichBuf = 0; parea != NULL;
        parea = parea->pareaNext)
     if (parea->fs & AF_AUTOCREATEDAREA && !(parea->fs & AF_AUTOCREATEREPORTED))
       if (!CompNetAddr(&parea->pnodeAutoCreate->netAddr, &pnode->netAddr)) {

         // Format the notification message new area line and description

         psz = IsPassThruArea(parea->pszSqshFlags) ? "\t[passthru]" : "";
         if (!parea->pszDescr || !parea->pszDescr[0])
           cch = sprintf(ach, "%.128s%s\r", parea->achTag, psz);
         else
           cch = sprintf(ach, "%.128s\t\"%.128s\"%s\r", parea->achTag,
                               parea->pszDescr, psz);

         // Check if it fits into the max message and move it in marking this
         // new area as reported, otherwise terminate this list and leave it
         // for the next message

         if (ichBuf + cch < cchBuf) {
           xstrcpy(pchBuf + ichBuf, ach); ichBuf+= cch;
           parea->fs|= AF_AUTOCREATEREPORTED;
           cArea++;
         } else
           break;
       }

   // Notify all the nodes interested if any

   for (iAddr = 0, pnetAddr = pnewarea->anetNote;
        iAddr < numbof(pnewarea->anetNote) && pnetAddr->zone != 0;
        iAddr++, pnetAddr++) {

     // Get the aka address which matches the notified node zone

     xmemcpy(&netAddr, GetAddrMatch(pnetAddr), sizeof(NETADDR));

     // Compose the notification message subject line

     sprintf(ach, "New area%s created at %s by node %s",
                   cArea > 1 ? "s" : "",
                   FormatNetAddr(&netAddr), FormatNetAddr(&pnode->netAddr));

     // Create the node notification message

     pnodeNotify = GetNodeFromAddr(pnetAddr);
     SendMsg(&netAddr, SQAFIX_NAME, pnetAddr,
             GetNodeSysop(pnodeNotify),
             GetNodeMsgAttr(pnodeNotify),
             ach, pchBuf);
   }

   // Post notification message to all the listed notification areas

   for (iArea = 0; iArea < numbof(pnewarea->apszNote) &&
        pnewarea->apszNote[iArea] != NULL; iArea++) {

     // Check if notification area exists

     if ((pareaNotify = GetAreaFromTag(pnewarea->apszNote[iArea])) == NULL) {
       WriteLog("! Can't post notification to an unknown area %s\n",
                 pnewarea->apszNote[iArea]);
       continue;
     }

     // Get the area originating address as it's known to Squish. This may
     // be the primary address or the alternate address specified in the
     // -p<node> flag

     GetAreaOrigAddr(pareaNotify, &netAddr);

     // Compose the notification message subject line

     sprintf(ach, "New area%s created at %s by node %s",
                  cArea > 1 ? "s" : "",
                  FormatNetAddr(&netAddr), FormatNetAddr(&pnode->netAddr));

     // Create the echo area notification message

     PostMsg(pareaNotify, &netAddr, SQAFIX_NAME, &netAddr, "All", MSGLOCAL,
             ach, pchBuf);
   }

   // Write notification message to all the listed notification files

   for (iFile = 0; iFile < numbof(pnewarea->apszFile) &&
        pnewarea->apszFile[iFile] != NULL; iFile++) {

     // Open the notification file for appending and check if ok

     if ((pfile = fopen(pnewarea->apszFile[iFile], "at")) == NULL) {
       WriteLog("$ Can't open file: %s\n", pnewarea->apszFile[iFile]);
       continue;
     } else
       WriteLog("- Make %s\n", pnewarea->apszFile[iFile]);

     // Get the aka address which matches the creation node zone

     xmemcpy(&netAddr, GetAddrMatch(&pnode->netAddr), sizeof(NETADDR));

     // Compose the notification file title

     cch = sprintf(ach, "\nNew area%s created at %s by node %s on %s\n",
                   cArea > 1 ? "s" : "",
                   FormatNetAddr(&netAddr), FormatNetAddr(&pnode->netAddr),
                   FormatSecs(tm));

     xmemset(&ach[cch], '-', cch - 2);
     xstrcpy(&ach[cch + cch - 2], "\n\n");

     // Replace all message body carriage returns with new lines

     for (psz = pchBuf; (psz = xstrchr(psz, '\r')) != NULL; *psz++ = '\n');

     // Write out the notification file title and body

     if (fputs(ach, pfile) == EOF || fputs(pchBuf, pfile) == EOF)
       WriteLog("$ Can't write file: %s\n", pnewarea->apszFile[iFile]);

     // Close the notification file

     fclose(pfile);
   }

   return TRUE;
 }

/*
 * This subroutine performs the area linkup request forwarding
 */

 static BOOL SUBENTRY DoForwardRequest(PNODE pnode, PSZ pszArea)
 {
   PNEWAREA pnewarea;
   PUPLINK puplink;
   PNOTE pnote;
   PLSZ plsz;
   PQUE pque;

   // Check if requsts forwarding is allowed for this system/node and
   // this is not a wild area specification

   if (!cfg.dayFReqTimeout || (cfg.fl & FL_NOREQUESTFORWARDING) ||
       !(pnode->fs & NF_FORWARDREQ) ||
       IsWildGrep(pszArea))
     return FALSE;

   // Check if this area is restricted for request forwarding

   for (plsz = cfg.plszFReqRefuseAreaFirst; plsz != NULL; plsz = plsz->plszNext) {
     if (GrepSearch(pszArea, plsz->ach, FALSE)) {
       WriteMsg("Request forwarding for areas matching %s is not allowed\n",
                 plsz->ach);
       return FALSE;
     }
   }

   // Check if area with this tag should never be created

   if (GetLsz(cfg.plszRefuseCreate, pszArea)) {
     WriteMsg("Request forwarding for area %s is not allowed\n",
               pszArea);
     return FALSE;
   }

   // Get the first suitable uplink and check if ok

   if ((puplink = GetFreqUplink(pnode, pszArea)) == NULL) {
     WriteMsg("No uplink found to forward this request to\n");
     return FALSE;
   }

   // Check to see if the requested area group is among the specified
   // groups for the requesting node.

   pnewarea = DoGetNewAreaInfo(pszArea, puplink->pnode);
   if (!DoCheckGroupStatus(pnode, pnewarea->chGroup, TRUE)) {
     WriteMsg("Request forwarding for area %s is restricted for node %s\n",
               pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if this area is restricted for creation and if so, skip it

   for (plsz = pnewarea->plszRefuse; plsz != NULL; plsz = plsz->plszNext) {
     if (GrepSearch(pszArea, plsz->ach, FALSE)) {
       WriteMsg("Request forwarding for area %s is not allowed\n", pszArea);
       return FALSE;
     }
   }

   // Check if linkup request for this area is already queued
   // Note: the first node listed in freq queue entry is an uplink
   // the request was sent to

   if ((pque = GetQueEntry(pszArea, QE_FREQ)) != NULL) {

     // Check if there is an uplink node and if not, link the current one.
     // Note that this could create warnings later when some other unkown
     // node creates this area, but this is not a big deal

     if (pque->plink == NULL)
       if (AddQueNodeLink(pque, &puplink->pnode->netAddr))
         WriteLog("- LnkQ %s to %s (assumed uplink node)\n",
                   FormatNetAddr(&puplink->pnode->netAddr), pszArea);

     // Link requesting node to the existing freq entry

     if (AddQueNodeLink(pque, &pnode->netAddr))
       WriteLog("- LnkQ %s to %s freq to %s\n",
                 FormatNetAddr(&pnode->netAddr), pszArea,
                 FormatNetAddr(&pque->plink->netAddr));
   } else {

     // Check if this would exceed the downlink's freq limit if any

     if (pnode->maxFreq && pnode->numFreq >= pnode->maxFreq) {
       WriteMsg("Request forwarding for more than %u areas is not allowed\n",
                 pnode->maxFreq);
       WriteLog("- FReq %s to %s for %s exceeds limit %u\n",
                 pszArea, FormatNetAddr(&puplink->pnode->netAddr),
                 FormatNetAddr(&pnode->netAddr), pnode->maxFreq);
       return FALSE;
     } else
       pnode->numFreq++;

     // Log operation

     WriteLog("* FReq %s to %s for %s\n",
               pszArea, FormatNetAddr(&puplink->pnode->netAddr),
               FormatNetAddr(&pnode->netAddr));

     // Add the areafix request and check if ok

     if (!AddAfrq(puplink, pszArea, AFRQ_LINK)) {
       WriteMsg("Request forwarding failed -- system error, please inform the Sysop...\n");
       WriteLog("! FReq %s to %s for %s failed\n",
                 pszArea, FormatNetAddr(&puplink->pnode->netAddr),
                 FormatNetAddr(&pnode->netAddr));

       return FALSE;
     }

     // Add new freq queue entry

     pque = AddQueEntry(pszArea, QE_FREQ, 0,
                        cfg.dayFReqTimeout * SECS_IN_DAY);
     WriteLog("- AddQ %s freq entry expires on %s\n",
               pszArea, FormatSecs(pque->time2));

     // Link the uplink node to the new freq entry

     if (AddQueNodeLink(pque, &puplink->pnode->netAddr))
       WriteLog("- LnkQ %s to %s (uplink node)\n",
                 FormatNetAddr(&puplink->pnode->netAddr), pszArea);

     // Link requesting node to the new freq entry

     if (AddQueNodeLink(pque, &pnode->netAddr))
       WriteLog("- LnkQ %s to %s (freq node)\n",
                 FormatNetAddr(&pnode->netAddr), pszArea);

     // Add the request forwarding notification entry

     if ((pnote = AddNote(pszArea, NT_FREQ_CREATED)) != NULL) {
       pnote->puplink = puplink;
       pnote->pnode = pnode;
     }
   }

   // Inform remote user

   WriteMsg("Request forwarded to an uplink node\n");

   return TRUE;
 }

/*
 * This subroutine performs the area linkup request unforwarding
 */

 static BOOL SUBENTRY DoUnForwardRequest(PNODE pnode, PSZ pszArea)
 {
   PLINK plink;
   PQUE pque;

   // Check if freq for this area has been queued

   if ((pque = GetQueEntry(pszArea, QE_FREQ)) == NULL)
     return FALSE;

   // Check if this node is present in this freq link list

   if ((plink = GetQueNodeLink(pque, &pnode->netAddr)) == NULL)
     return FALSE;

   // Check if this link is not an unlink

   if (plink == pque->plink)
     return FALSE;

   // Delete the link from the que entry link list

   if (!DelQueNodeLink(pque, &pnode->netAddr))
     return FALSE;
   else
     WriteLog("- UnlQ %s from %s (freq node)\n",
               FormatNetAddr(&pnode->netAddr), pszArea);

   // Inform remote user

   WriteMsg("Request forwarded earlier has been discarded\n");

   return TRUE;
 }

/*
 * This subroutine creates freq expiration notifications
 */

 static BOOL SUBENTRY DoReportFreq(USHORT type)
 {
   time_t tm = time(NULL);
   USHORT iAddr, iArea, iFile, cArea, cch;
   NETADDR netAddr, * pnetAddr;
   PAREA pareaNotify;
   PNODE pnodeNotify;
   ULONG timeNote;
   CHAR ach[512];
   FILE * pfile;
   PNOTE pnote;
   PSZ psz;

   // Compose the notification message

   for (pnote = cfg.pnoteFirst, cArea = 0, ichBuf = 0, timeNote = 0;
        pnote != NULL; pnote = pnote->pnoteNext)
     if (pnote->type == type && !(pnote->fs & NF_REPORTED)) {

       // Set the notification time of minute on the first turn and
       // check for the time within this minute on all the subsequent turns

       if (!timeNote)
         timeNote = (pnote->time / 60) * 60;
       else
         if (pnote->time < timeNote || pnote->time > timeNote + 59)
           continue;

       // Format the notification message line

       if (type == NT_FREQ_CREATED) {
         if (!pnote->pnode->pszSysop || !pnote->pnode->pszSysop[0])
           cch = sprintf(ach, "%.128s\tto %s for %s\r", pnote->achTag,
                         FormatNetAddr(&pnote->puplink->pnode->netAddr),
                         FormatNetAddr(&pnote->pnode->netAddr));
         else
           cch = sprintf(ach, "%.128s\tto %s for %s\t\"%s\"\r", pnote->achTag,
                         FormatNetAddr(&pnote->puplink->pnode->netAddr),
                         FormatNetAddr(&pnote->pnode->netAddr),
                         pnote->pnode->pszSysop);
       } else {
         if (!pnote->pszDescr || !pnote->pszDescr[0])
           cch = sprintf(ach, "%.128s", pnote->achTag);
         else
           cch = sprintf(ach, "%.128s\t\"%.128s\"", pnote->achTag, pnote->pszDescr);

         if (pnote->puplink)
           cch+= sprintf(ach + cch, "\tto %s\r", FormatNetAddr(&pnote->puplink->pnode->netAddr));
         else
           cch+= sprintf(ach + cch, "\r");
       }

       // Check if it fits into the max message and move it in marking this
       // entry as reported, otherwise terminate this list and leave it
       // for the next message

       if (ichBuf + cch < cchBuf) {
         xstrcpy(pchBuf + ichBuf, ach); ichBuf+= cch;
         pnote->fs|= NF_REPORTED;
         cArea++;
       } else
         break;
     }

   // Check if we have anything to report and leave if not

   if (!cArea) return FALSE;

   // Notify all the nodes interested if any

   for (iAddr = 0, pnetAddr = cfg.anetFReqNote;
        iAddr < numbof(cfg.anetFReqNote) && pnetAddr->zone != 0;
        iAddr++, pnetAddr++) {

     // Get the aka address which matches the notified node zone

     xmemcpy(&netAddr, GetAddrMatch(pnetAddr), sizeof(NETADDR));

     // Compose the notification message subject line

     switch (type) {
       case NT_FREQ_CREATED:
            cch = sprintf(ach, "Forwarded request%s at %s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
       case NT_FREQ_EXPWARN:
            cch = sprintf(ach, "Forwarded request%s to be expired at %s%s%s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          timeNote ? " on " : "",
                          timeNote ? FormatSecs(timeNote) : (PSZ) "");
            break;
       case NT_FREQ_EXPNOTE:
            cch = sprintf(ach, "Forwarded request%s %s been expired at %s",
                          cArea > 1 ? "s" : "", cArea > 1 ? "have" : "has",
                          FormatNetAddr(&netAddr));
            break;
       default:
            cch = sprintf(ach, "Forwarded request%s fucked at %s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
     }

     // Create the node notification message

     pnodeNotify = GetNodeFromAddr(pnetAddr);
     SendMsg(&netAddr, SQAFIX_NAME, pnetAddr,
             GetNodeSysop(pnodeNotify),
             GetNodeMsgAttr(pnodeNotify),
             ach, pchBuf);
   }

   // Post notification message to all the listed notification areas

   for (iArea = 0; iArea < numbof(cfg.apszFReqNote) &&
        cfg.apszFReqNote[iArea] != NULL; iArea++) {

     // Check if notification area exists

     if ((pareaNotify = GetAreaFromTag(cfg.apszFReqNote[iArea])) == NULL) {
       WriteLog("! Can't post notification to an unknown area %s\n",
                 cfg.apszFReqNote[iArea]);
       continue;
     }

     // Get the area originating address as it's known to Squish. This may
     // be the primary address or the alternate address specified in the
     // -p<node> flag

     GetAreaOrigAddr(pareaNotify, &netAddr);

     // Compose the notification message subject line

     switch (type) {
       case NT_FREQ_CREATED:
            cch = sprintf(ach, "Forwarded request%s at %s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
       case NT_FREQ_EXPWARN:
            cch = sprintf(ach, "Forwarded request%s to be expired at %s%s%s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          timeNote ? " on " : "",
                          timeNote ? FormatSecs(timeNote) : (PSZ) "");
            break;
       case NT_FREQ_EXPNOTE:
            cch = sprintf(ach, "Forwarded request%s %s been expired at %s",
                          cArea > 1 ? "s" : "", cArea > 1 ? "have" : "has",
                          FormatNetAddr(&netAddr));
            break;
       default:
            cch = sprintf(ach, "Forwarded request%s fucked at %s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
     }

     // Create the echo area notification message

     PostMsg(pareaNotify, &netAddr, SQAFIX_NAME, &netAddr, "All", MSGLOCAL,
             ach, pchBuf);
   }

   // Write notification message to all the listed notification files

   for (iFile = 0; iFile < numbof(cfg.apszFReqFile) &&
        cfg.apszFReqFile[iFile] != NULL; iFile++) {

     // Open the notification file for appending and check if ok

     if ((pfile = fopen(cfg.apszFReqFile[iFile], "at")) == NULL) {
       WriteLog("$ Can't open file: %s\n", cfg.apszFReqFile[iFile]);
       continue;
     } else
       WriteLog("- Make %s\n", cfg.apszFReqFile[iFile]);

     // Get the primary aka address

     netAddr = cfg.anetAddr[0];

     // Compose the notification file title

     switch (type) {
       case NT_FREQ_CREATED:
            cch = sprintf(ach, "\nForwarded request%s at %s on %s\n",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          FormatSecs(tm));
            break;
       case NT_FREQ_EXPWARN:
            cch = sprintf(ach, "\nForwarded request%s to be expired at %s%s%s\n",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          timeNote ? " on " : "",
                          timeNote ? FormatSecs(timeNote) : (PSZ) "");
            break;
       case NT_FREQ_EXPNOTE:
            cch = sprintf(ach, "\nForwarded request%s %s been expired at %s on %s\n",
                          cArea > 1 ? "s" : "", cArea > 1 ? "have" : "has",
                          FormatNetAddr(&netAddr), FormatSecs(tm));
            break;
       default:
            cch = sprintf(ach, "\nForwarded request%s fucked at %s\n",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
     }

     xmemset(&ach[cch], '-', cch - 2);
     xstrcpy(&ach[cch + cch - 2], "\n\n");

     // Replace all message body carriage returns with new lines

     for (psz = pchBuf; (psz = xstrchr(psz, '\r')) != NULL; *psz++ = '\n');

     // Write out the notification file title and body

     if (fputs(ach, pfile) == EOF || fputs(pchBuf, pfile) == EOF)
       WriteLog("$ Can't write file: %s\n", cfg.apszFReqFile[iFile]);

     // Close the notification file

     fclose(pfile);
   }

   return cArea;
 }

/*
 * This subroutine creates idle passthru expiration notifications
 */

 static BOOL SUBENTRY DoReportIdle(USHORT type)
 {
   time_t tm = time(NULL);
   USHORT iAddr, iArea, iFile, cArea, cch;
   NETADDR netAddr, * pnetAddr;
   PAREA pareaNotify;
   PNODE pnodeNotify;
   ULONG timeNote;
   CHAR ach[512];
   FILE * pfile;
   PNOTE pnote;
   PSZ psz;

   // Compose the notification message

   for (pnote = cfg.pnoteFirst, cArea = 0, ichBuf = 0, timeNote = 0;
        pnote != NULL; pnote = pnote->pnoteNext)
     if (pnote->type == type && !(pnote->fs & NF_REPORTED)) {

       // Set the notification time of minute on the first turn and
       // check for the time within this minute on all the subsequent turns

       if (!timeNote)
         timeNote = (pnote->time / 60) * 60;
       else
         if (pnote->time < timeNote || pnote->time > timeNote + 59)
           continue;

       // Format the notification message line

       if (!pnote->pszDescr || !pnote->pszDescr[0])
         cch = sprintf(ach, "%.128s\r", pnote->achTag);
       else
         cch = sprintf(ach, "%.128s\t\"%.128s\"\r", pnote->achTag, pnote->pszDescr);

       // Check if it fits into the max message and move it in marking this
       // entry as reported, otherwise terminate this list and leave it
       // for the next message

       if (ichBuf + cch < cchBuf) {
         xstrcpy(pchBuf + ichBuf, ach); ichBuf+= cch;
         pnote->fs|= NF_REPORTED;
         cArea++;
       } else
         break;
     }

   // Check if we have anything to report and leave if not

   if (!cArea) return FALSE;

   // Notify all the nodes interested if any

   for (iAddr = 0, pnetAddr = cfg.anetIdleNote;
        iAddr < numbof(cfg.anetIdleNote) && pnetAddr->zone != 0;
        iAddr++, pnetAddr++) {

     // Get the aka address which matches the notified node zone

     xmemcpy(&netAddr, GetAddrMatch(pnetAddr), sizeof(NETADDR));

     // Compose the notification message subject line

     switch (type) {
       case NT_IDLE_EXPWARN:
            cch = sprintf(ach, "Idle passthru area%s to be deleted at %s%s%s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          timeNote ? " on " : "",
                          timeNote ? FormatSecs(timeNote) : (PSZ) "");
            break;
       case NT_IDLE_EXPNOTE:
            cch = sprintf(ach, "Idle passthru area%s %s been deleted at %s",
                          cArea > 1 ? "s" : "", cArea > 1 ? "have" : "has",
                          FormatNetAddr(&netAddr));
            break;
       default:
            cch = sprintf(ach, "Idle passthru area%s fucked at %s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
     }

     // Create the node notification message

     pnodeNotify = GetNodeFromAddr(pnetAddr);
     SendMsg(&netAddr, SQAFIX_NAME, pnetAddr,
             GetNodeSysop(pnodeNotify),
             GetNodeMsgAttr(pnodeNotify),
             ach, pchBuf);
   }

   // Post notification message to all the listed notification areas

   for (iArea = 0; iArea < numbof(cfg.apszIdleNote) &&
        cfg.apszIdleNote[iArea] != NULL; iArea++) {

     // Check if notification area exists

     if ((pareaNotify = GetAreaFromTag(cfg.apszIdleNote[iArea])) == NULL) {
       WriteLog("! Can't post notification message to unknown area %s\n",
                 cfg.apszIdleNote[iArea]);
       continue;
     }

     // Get the area originating address as it's known to Squish. This may
     // be the primary address or the alternate address specified in the
     // -p<node> flag

     GetAreaOrigAddr(pareaNotify, &netAddr);

     // Compose the notification message subject line

     switch (type) {
       case NT_IDLE_EXPWARN:
            cch = sprintf(ach, "Idle passthru area%s to be deleted at %s%s%s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          timeNote ? " on " : "",
                          timeNote ? FormatSecs(timeNote) : (PSZ) "");
            break;
       case NT_IDLE_EXPNOTE:
            cch = sprintf(ach, "Idle passthru area%s %s been deleted at %s",
                          cArea > 1 ? "s" : "", cArea > 1 ? "have" : "has",
                          FormatNetAddr(&netAddr));
            break;
       default:
            cch = sprintf(ach, "Idle passthru area%s fucked at %s",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
     }

     // Create the echo area notification message

     PostMsg(pareaNotify, &netAddr, SQAFIX_NAME, &netAddr, "All", MSGLOCAL,
             ach, pchBuf);
   }

   // Write notification message to all the listed notification files

   for (iFile = 0; iFile < numbof(cfg.apszIdleFile) &&
        cfg.apszIdleFile[iFile] != NULL; iFile++) {

     // Open the notification file for appending and check if ok

     if ((pfile = fopen(cfg.apszIdleFile[iFile], "at")) == NULL) {
       WriteLog("$ Can't open file: %s\n", cfg.apszIdleFile[iFile]);
       continue;
     } else
       WriteLog("- Make %s\n", cfg.apszIdleFile[iFile]);

     // Get the primary aka address

     netAddr = cfg.anetAddr[0];

     // Compose the notification file title

     switch (type) {
       case NT_IDLE_EXPWARN:
            cch = sprintf(ach, "\nIdle passthru area%s to be deleted at %s%s%s\n",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr),
                          timeNote ? " on " : "",
                          timeNote ? FormatSecs(timeNote) : (PSZ) "");
            break;
       case NT_IDLE_EXPNOTE:
            cch = sprintf(ach, "\nIdle passthru area%s %s been deleted at %s on %s\n",
                          cArea > 1 ? "s" : "", cArea > 1 ? "have" : "has",
                          FormatNetAddr(&netAddr), FormatSecs(tm));
            break;
       default:
            cch = sprintf(ach, "\nIdle passthru area%s fucked at %s\n",
                          cArea > 1 ? "s" : "", FormatNetAddr(&netAddr));
            break;
     }

     xmemset(&ach[cch], '-', cch - 2);
     xstrcpy(&ach[cch + cch - 2], "\n\n");

     // Replace all message body carriage returns with new lines

     for (psz = pchBuf; (psz = xstrchr(psz, '\r')) != NULL; *psz++ = '\n');

     // Write out the notification file title and body

     if (fputs(ach, pfile) == EOF || fputs(pchBuf, pfile) == EOF)
       WriteLog("$ Can't write file: %s\n", cfg.apszIdleFile[iFile]);

     // Close the notification file

     fclose(pfile);
   }

   return cArea;
 }

/*
 * This subroutine returns an available area status
 */

 static PSZ SUBENTRY DoGetAvailAreaStatus(PAREA pareaFirstSave, PAREA pareaAvail,
                                          PNODE pnode)
 {
   PSZ pszArea = pareaAvail->achTag;
   BOOL fAllowed;
   PAREA parea;
   PLINK plink;
   PQUE pque;
   PSZ psz;

   // First check to see if this uplink area exists at our node

   if ((parea = GetAreaFromTagAlt(pareaFirstSave, pszArea)) != NULL) {

     // Check area visibility and link status for this node

     plink = DoCheckAreaStatus(parea, pnode, &fAllowed, FALSE);

     // Check if node should see this area, that is if it is already
     // linked to it, area is alowed to it or is visible for node
     // Note: same as in CreateAreasReport

     if (plink != NULL || fAllowed || (!fAllowed &&
        (parea->fs & AF_VISIBLE) && (pnode->fs & NF_VISIBLE)))
       return "Receiving";

     // Otherwise return null to hide it

     return NULL;
   }

   // Check if this area is in freq queue and if so, check if this node
   // is going to be linked to it and thus should aloways be shown this
   // area. If not, just set an appropraite string

   if ((pque = GetQueEntry(pszArea, QE_FREQ)) != NULL) {
     if (GetQueNodeLink(pque, &pnode->netAddr))
       return "ReqForYou";
     else
       psz = "Requested";
   } else
     psz =   "Available";

   // Finally check if this node has sufficient access level and
   // this group is in its specified group list so that it can see
   // this area. Note that we have to check specified group list since
   // this new area group might not be present in the EchoArea specs yet

   DoCheckAreaStatus(pareaAvail, pnode, &fAllowed, TRUE);
   if (!fAllowed) return NULL;

   return psz;
 }

/*
 * This subroutine checks if area specification has a rescan suffix
 */

 static PSZ SUBENTRY DoGetAreaSuffix(PSZ * ppszArea)
 {
   static CHAR achArea[MAX_AREA_LENG];
   USHORT cch;
   PCH pch;

   // Check if we're processing areamask and return areamask suffix
   // scanned in ExecExistAreaMask()

   if (cfg.fExecAreaMask) return pszExecAreaMaskSuffix;

   // Look for the comma character and check if found

   if ((pch = xstrchr(*ppszArea, ',')) == NULL)
     return NULL;
   else
     cch = min(lengof(achArea), (USHORT)(pch - *ppszArea));

   // Copy the area tag specification to the local storage and
   // reset callers areatag pointer

   xmemcpy(achArea, *ppszArea, cch); achArea[cch] = '\0';
   *ppszArea = achArea;

   // Return pointer to the area suffix

   return ++pch;
 }

/*
 * This subroutine rescans given area for the specified node
 */

 static BOOL APPENTRY DoRescanArea(PNODE pnode, PSZ pszArea, BOOL fReport)
 {
   CHAR achCmd[MAXPATH], achArea[MAX_AREA_LENG + 4], achNode[40];
   BOOL fAllowed;
   PAREA parea;
   PLINK plink;
   SHORT code;

   // Get the area descriptor pointer and the link for this node

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL) {
     if (fReport)
       WriteMsg("\nArea %s can't be rescanned since it does not exist\n",
                pszArea);
     return FALSE;
   }

   // Check if the node is linked to this area

   if ((plink = GetAreaLink(parea, &pnode->netAddr)) == NULL) {
     if (fReport)
       WriteMsg("\nArea %s is not linked to node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the node is passive for this area

   if (!(plink->fs & LF_ACTIVE)) {
     if (fReport)
       WriteMsg("\nArea %s is not active for node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if this area can be rescanned

   if (!(parea->fs & AF_RESCANOK)) {
     WriteMsg("\nArea %s is not allowed to be rescanned\n",
               pszArea);
     return FALSE;
   }

   // Check if this area is passthru

   if (IsPassThruArea(parea->pszSqshFlags) && !(cfg.fl & FL_RESCANPTHRUAREAS)) {
     WriteMsg("\nArea %s can't be rescanned since it is passthru\n",
               pszArea);
     return FALSE;
   }

   // Check if this node has exceeded allowed rescan request limit

   if (pnode->maxRescan)
     if (pnode->numRescan >= pnode->maxRescan) {
       WriteMsg("\nArea %s can't be rescanned since this would exceed"
                "\nlimit of maximum %u area%s being rescanned for node %s\n",
                 pszArea, pnode->maxRescan, pnode->maxRescan == 1 ? "" : "s",
                 FormatNetAddr(&pnode->netAddr));
       WriteLog("- Scan %s for %s exceeds limit %u\n",
                 pszArea, FormatNetAddr(&pnode->netAddr), pnode->maxRescan);
       return FALSE;
     }

   // Format the node address as an ascii text

   xstrcpy(achNode, FormatNetAddr(&pnode->netAddr));

   // Check if there are special command line characters and enclose
   // string in double quotas so that they will not screw up command line

   if (IsSpecCmdChars(pszArea))
     sprintf(achArea, "\"%.128s\"", pszArea);
   else
     sprintf(achArea, "%.128s", pszArea);

   // Log what are we going to execute

   WriteLog("- Exec %s %s %s\n", cfg.pszRescanCommand, achArea, achNode);

   // Get the operating system command shell name and
   // spawn external program

   GetCommandShellName(achCmd);
   code = spawnlp(P_WAIT, achCmd, achCmd, "/C",
                  cfg.pszRescanCommand, achArea, achNode,
                  NULL);
		  

   // Check if spawn failed and report

   if (code == (SHORT)-1) {
     WriteMsg("\nRescan failed -- system error, please inform the Sysop...\n");
     WriteLog("$ Exec rescan %s for %s failed\n",
              pszArea, FormatNetAddr(&pnode->netAddr));

     return FALSE;
   }

   // Log operation

   WriteLog("- Exec rescan %s for %s, exit code (%d)\n",
            pszArea, FormatNetAddr(&pnode->netAddr), code);

   // Inform remote user

   WriteMsg("\nArea %s rescanned for node %s\n",
             pszArea, FormatNetAddr(&pnode->netAddr));

   // Increment node's rescan counter

   pnode->numRescan++;

   // Show we've created new area

   cfg.fExitCode|= EXIT_PROCAREA;

   return TRUE;
 }

/*
 * This subroutine sends rules for the specified node and area
 */

 static BOOL SUBENTRY DoSendAreaRules(PNODE pnode, PSZ pszArea, BOOL fReport)
 {
   CHAR achRules[MAXPATH], achSubj[128 + MAX_AREA_LENG];
   BOOL fAllowed;
   PAREA parea;

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL) {
     return FALSE;
   }

   // Check if area has the associated rules

   if (!(parea->fs & AF_SENDRULES)) {
     if (fReport)
       WriteMsg("\nRules for area %s are not available at this system"
                "\n",
                 pszArea);
     return FALSE;
   }

   // Make up the rules file name

   if (parea->pszRules != NULL) {
     xstrcpy(achRules, parea->pszRules);
   } else
   if (cfg.pszDefAreaRules != NULL) {
     xstrcpy(achRules, cfg.pszDefAreaRules);
   } else {
     xstrcpy(achRules, parea->pszPath);
     xstrcat(achRules, DEF_RUL_EXT);
   }

   // Check if the rule file exists

   if (access(achRules, 0)) {
     WriteLog("$ Can't locate file: %s\n", achRules);
     WriteMsg("\nRules for area %s were not found at this system"
              "\n",
               pszArea);
     return FALSE;
   }

   // Add the send file list element and check if ok

   sprintf(achSubj, "Rules for area %.128s", pszArea);
   if (!AddSndf(pnode, achRules, achSubj)) {
     WriteMsg("\nSystem error while sending rules info"
              "\n");
     return FALSE;
   }

   // Inform the user

   if (!cfg.fManualMode)
     WriteMsg("\nRules for area %s will be sent in a separate message"
              "\n",
               pszArea);

   return TRUE;
 }

/////////////////////////////////////////////////////////////////////////////
// C o m m a n d   e x e c u t o r   r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/*
 * Run given executor for all existing areas matching the given area mask
 */

 VOID APPENTRY ExecExistAreaMask(PNODE pnode, PSZ pszAreaMask,
                                 PFNEXEC pfnExec)
 {
   PAREA parea;

#ifdef DEBUG
fprintf(STDAUX, "ExecExistAreaMask: '%s'\r\n", pszAreaMask);
#endif

   // Validate area specification

   if (!pszAreaMask || !pszAreaMask[0]) {
     WriteMsg("\nInvalid area specification\n");
     return;
   }

   // Check if this is wild area specification and if no, just
   // run the executor once against the area name, otherwise
   // run the executor for every existing area matching mask

   if (!IsWildGrep(pszAreaMask)) {
     pfnExec(pnode, pszAreaMask);
   } else {

     // Fix up the areamask suffix if any

     pszExecAreaMaskSuffix = DoGetAreaSuffix(&pszAreaMask);

     // Increment area mask execution flag

     cfg.fExecAreaMask++;

     // Scan through all the areas looking for a area tag mask match

     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (GrepSearch(parea->achTag, pszAreaMask, FALSE))
         pfnExec(pnode, parea->achTag);
     }

     // Decrement area mask execution flag and reset areamask suffix

     cfg.fExecAreaMask--;

     pszExecAreaMaskSuffix = NULL;
   }
 }

/*
 * Run given executor for all queue areas matching the given area mask
 */

 VOID APPENTRY ExecQueueAreaMask(PNODE pnode, PSZ pszAreaMask,
                                 PFNEXEC pfnExec)
 {
   PQUE pque;

#ifdef DEBUG
fprintf(STDAUX, "ExecQueueAreaMask: '%s'\r\n", pszAreaMask);
#endif

   // Validate area specification

   if (!pszAreaMask || !pszAreaMask[0]) {
     WriteMsg("\nInvalid area specification\n");
     return;
   }

   // Check if this is wild area specification and if no, just
   // run the executor once against the area name, otherwise
   // run the executor for every existing area matching mask

   if (!IsWildGrep(pszAreaMask)) {
     pfnExec(pnode, pszAreaMask);
   } else {

     // Increment area mask execution flag

     cfg.fExecAreaMask++;

     // Scan through all the areas looking for a area tag mask match

     for (pque = cfg.pqueFirst; pque != NULL; pque = pque->pqueNext) {
       if (GrepSearch(pque->achTag, pszAreaMask, FALSE))
         pfnExec(pnode, pque->achTag);
     }

     // Decrement area mask execution flag

     cfg.fExecAreaMask--;
   }
 }

/*
 * Link node to the specified area
 */

 BOOL APPENTRY ExecLnkNodeArea(PNODE pnode, PSZ pszArea)
 {
   BOOL fRescan = FALSE;
   PAREA parea;
   PLINK plink;
   BOOL fAllowed;
   PCH pch;

   // Fix the area suffix and check if there is a rescan request

   if ((pch = DoGetAreaSuffix(&pszArea)) != NULL) {
     if (toupper(*pch) == 'R') fRescan = TRUE;
   }

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL) {
     DoForwardRequest(pnode, pszArea);
     return FALSE;
   } else
     plink = GetAreaLink(parea, &pnode->netAddr);

   // Check if the node is already linked to this area

   if (plink != NULL) {
     WriteMsg("\nArea %s is already linked to node %s%s\n",
               pszArea, FormatNetAddr(&pnode->netAddr),
               plink->fs & LF_ACTIVE ? "" : " (passive)");
     return FALSE;
   }

   // Check if the node is allowed to maintain this area

   if (!fAllowed) {
     WriteMsg("\nArea %s is restricted for node %s\n",
               pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Link node to the area active list

   DoLinkNode(pnode, parea, ACTIVE);

   // Rescan area if requested

   if (fRescan) ExecRescanArea(pnode, pszArea);

   return TRUE;
 }

/*
 * Link node to all the areas in the specified groups
 */

 BOOL APPENTRY ExecLnkNodeGroup(PNODE pnode, PSZ pszGroup)
 {
   PAREA parea;
   PCH pch;

   // Check out the groups specification

   pszGroup = DoMakeAreaGroupList(pnode, pszGroup);

   // Link node to all areas in the group list

   for (pch = pszGroup; *pch; pch++) {
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (parea->chGroup == toupper(*pch) && DoCheckAreaAccess(parea, pnode))
         DoLinkNode(pnode, parea, ACTIVE);
     }
   }

   return TRUE;
 }

/*
 * Unlink node from the specified area
 */

 BOOL APPENTRY ExecUnlNodeArea(PNODE pnode, PSZ pszArea)
 {
   PAREA parea;
   PLINK plink;
   BOOL fAllowed;

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL) {
     DoUnForwardRequest(pnode, pszArea);
     return FALSE;
   } else
     plink = GetAreaLink(parea, &pnode->netAddr);

   // Check if the node is linked to this area. Note that we're
   // not reporting this for areamask unlink requests

   if (plink == NULL) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is not linked to node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the node is allowed to maintain this area

   if (!fAllowed) {
     WriteMsg("\nArea %s is restricted for node %s\n",
               pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Unlink node from the area

   DoUnlinkNode(pnode, parea, plink->fs & LF_ACTIVE);

   return TRUE;
 }

/*
 * Unlink node from all the areas in the specified groups
 */

 BOOL APPENTRY ExecUnlNodeGroup(PNODE pnode, PSZ pszGroup)
 {
   PAREA parea;
   PLINK plink;
   PCH pch;

   // Check out the groups specification

   pszGroup = DoMakeAreaGroupList(pnode, pszGroup);

   // Unlink node from all areas in the group list

   for (pch = pszGroup; *pch; pch++) {
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (parea->chGroup == toupper(*pch) && DoCheckAreaAccess(parea, pnode))
         if ((plink = GetAreaLink(parea, &pnode->netAddr)) != NULL)
           DoUnlinkNode(pnode, parea, plink->fs & LF_ACTIVE);
     }
   }

   return TRUE;
 }

/*
 * Move node to passive list for the specified area
 */

 BOOL APPENTRY ExecPasNodeArea(PNODE pnode, PSZ pszArea)
 {
   PAREA parea;
   PLINK plink;
   BOOL fAllowed;

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL)
     return FALSE;
   else
     plink = GetAreaLink(parea, &pnode->netAddr);

   // Check if the node is linked to this area. Note that we're
   // not reporting this for areamask unlink requests

   if (plink == NULL) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is not linked to node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the node is already passive to this area. Note that we're
   // not reporting this for areamask unlink requests

   if (!(plink->fs & LF_ACTIVE)) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is already passive for node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the node is allowed to maintain this area

   if (!fAllowed) {
     WriteMsg("\nArea %s is restricted for node %s\n",
               pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Move node to passive list for the specified area

   DoMakeNodePassive(pnode, parea, plink);

   return TRUE;
 }

/*
 * Move node to passive list for all the areas in the specified groups
 */

 BOOL APPENTRY ExecPasNodeGroup(PNODE pnode, PSZ pszGroup)
 {
   PAREA parea;
   PLINK plink;
   PCH pch;

   // Check out the groups specification

   pszGroup = DoMakeAreaGroupList(pnode, pszGroup);

   // Move node to passive list for all areas in the group list

   for (pch = pszGroup; *pch; pch++) {
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (parea->chGroup == toupper(*pch) && DoCheckAreaAccess(parea, pnode))
         if ((plink = GetAreaLink(parea, &pnode->netAddr)) != NULL)
           DoMakeNodePassive(pnode, parea, plink);
     }
   }

   return TRUE;
 }

/*
 * Move node to active list for the specified area
 */

 BOOL APPENTRY ExecActNodeArea(PNODE pnode, PSZ pszArea)
 {
   BOOL fRescan = FALSE;
   BOOL fAllowed;
   PAREA parea;
   PLINK plink;
   PCH pch;

   // Fix the area suffix and check if there is a rescan request

   if ((pch = DoGetAreaSuffix(&pszArea)) != NULL) {
     if (toupper(*pch) == 'R') fRescan = TRUE;
   }

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL)
     return FALSE;
   else
     plink = GetAreaLink(parea, &pnode->netAddr);

   // Check if the node is linked to this area. Note that we're
   // not reporting this for areamask unlink requests

   if (plink == NULL) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is not linked to node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the node is already active to this area. Note that we're
   // not reporting this for areamask unlink requests

   if (plink->fs & LF_ACTIVE) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is already active for node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the node is allowed to maintain this area

   if (!fAllowed) {
     WriteMsg("\nArea %s is restricted for node %s\n",
               pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Move node to active list for the area

   DoMakeNodeActive(pnode, parea, plink);

   // Rescan area if requested

   if (fRescan) ExecRescanArea(pnode, pszArea);

   return TRUE;
 }

/*
 * Move node to active list for all the areas in the specified groups
 */

 BOOL APPENTRY ExecActNodeGroup(PNODE pnode, PSZ pszGroup)
 {
   PAREA parea;
   PLINK plink;
   PCH pch;

   // Check out the groups specification

   pszGroup = DoMakeAreaGroupList(pnode, pszGroup);

   // Move node to active list for all areas in the group list

   for (pch = pszGroup; *pch; pch++) {
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (parea->chGroup == toupper(*pch) && DoCheckAreaAccess(parea, pnode))
         if ((plink = GetAreaLink(parea, &pnode->netAddr)) != NULL)
           DoMakeNodeActive(pnode, parea, plink);
     }
   }

   return TRUE;
 }

/*
 * Send area rules to to given node
 */

 BOOL APPENTRY ExecRulNodeArea(PNODE pnode, PSZ pszArea)
 {
   BOOL fAllowed;
   PAREA parea;
   PLINK plink;

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL)
     return FALSE;
   else
     plink = GetAreaLink(parea, &pnode->netAddr);

   // Check if the node is linked to this area. Note that we're
   // not reporting this for areamask unlink requests

   if (plink == NULL) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is not linked to node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Send area rules

   if (parea->fs & AF_SENDRULES)
     DoSendAreaRules(pnode, parea->achTag, TRUE);

   return TRUE;
 }

/*
 * Rescan given area for the specified node
 */

 BOOL APPENTRY ExecRescanArea(PNODE pnode, PSZ pszArea)
 {
   USHORT iArea = 0;
   PAREA parea;
   PDONE pdone;

   // Check if the sysop tends to avoid rescans at all

   if (cfg.pszRescanCommand == NULL) {
     WriteMsg("\nSorry, this system is not set up for remote rescan requests\n");
     return FALSE;
   }

   // Check if this node is allowed to request rescan

   if (!(pnode->fs & NF_RESCANOK)) {
     WriteMsg("\nNode %s is not allowed to force area rescan\n",
               FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if the area is not specified and rescan all the
   // areas linked up during this session

   if (pszArea == NULL) {
     WriteMsg("\nRescanning all areas linked during this session ...\n");

     // Loop through the done list element

     for (pdone = cfg.pdoneFirst; pdone != NULL; pdone = pdone->pdoneNext)
       if (pdone->fWhat == DN_LINKED || pdone->fWhat == DN_ACTIVE)
         if (DoRescanArea(pnode, pdone->parea->achTag, TRUE))
           ++iArea;

     // Report number of rescanned areas

     if (!iArea)
       WriteMsg("\n... no areas have been rescanned\n");
     else
       WriteMsg("\n... %u area%s been rescanned\n",
                iArea, iArea == 1 ? " has" : "s have");

     return TRUE;
   } else

   // Check if area mask is specified and rescan all matching areas

   if (IsWildGrep(pszArea)) {
     WriteMsg("\nRescanning all linked areas matching mask '%s' ...\n", pszArea);

     // Loop through all the existing areas matching given areamask

     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (GrepSearch(parea->achTag, pszArea, FALSE))
         if (DoRescanArea(pnode, parea->achTag, FALSE))
           ++iArea;
     }

     // Report number of rescanned areas

     if (!iArea)
       WriteMsg("\n... no areas have been rescanned\n");
     else
       WriteMsg("\n... %u area%s been rescanned\n",
                iArea, iArea == 1 ? " has" : "s have");

     return TRUE;
   }

   // Otherwise rescan the specified area

   return DoRescanArea(pnode, pszArea, TRUE);
 }

/*
 * Send rules for a given area to a node
 */

 BOOL APPENTRY ExecSendAreaRules(PNODE pnode, PSZ pszArea)
 {
   USHORT iArea = 0;
   PAREA parea;
   PDONE pdone;

   // Check if the area is not specified and send rules for all
   // areas linked up during this session

   if (pszArea == NULL) {
     WriteMsg("\nSending rules for all areas linked during this session ...\n");

     // Loop through the done list element

     for (pdone = cfg.pdoneFirst; pdone != NULL; pdone = pdone->pdoneNext) {
       if (pdone->fWhat == DN_LINKED || pdone->fWhat == DN_ACTIVE)
         if (DoSendAreaRules(pnode, pdone->parea->achTag, TRUE))
           ++iArea;
     }

     // Report number of areas

     if (!iArea)
       WriteMsg("\n... no rules were sent\n");
     else
       WriteMsg("\n... sent rules for %u area%s\n",
                iArea, iArea == 1 ? "" : "s");

     return TRUE;
   } else

   // Check if area mask is specified and send rules for all matching areas

   if (IsWildGrep(pszArea)) {
     WriteMsg("\nSending rules for all linked areas matching mask '%s' ...\n", pszArea);

     // Loop through all the linked areas matching given areamask

     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
       if (GrepSearch(parea->achTag, pszArea, FALSE))
         if (GetAreaLink(parea, &pnode->netAddr))
           if (DoSendAreaRules(pnode, parea->achTag, TRUE))
             ++iArea;
     }

     // Report number of areas

     if (!iArea)
       WriteMsg("\n... no rules were sent\n");
     else
       WriteMsg("\n... sent rules for %u area%s\n",
                iArea, iArea == 1 ? "" : "s");

     return TRUE;
   }

   // Otherwise send rules for the specified area

   return DoSendAreaRules(pnode, pszArea, TRUE);
 }

/*
 * Create new area on the specified node request
 */

 BOOL APPENTRY ExecCreateArea(PNODE pnode, PSZ pszArea)
 {
   PSZ pszDescr;

   // Check if remote operations allowed for this node

   if (!(pnode->fs & NF_AREACREATE)) {
     WriteMsg("\nArea creation is not allowed for node %s\n",
               FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if we have the area name specification

   if (!pszArea || !pszArea[0]) {
     WriteMsg("\nMissing tag of the area to create\n");
     return FALSE;
   } else
#ifndef UNIX
     xstrupr(pszArea);
#endif

   // Make sure we don't have the area with the same tag

   if (GetAreaFromTag(pszArea) != NULL) {
     WriteMsg("\nArea %s already exists\n", pszArea);
     return FALSE;
   }

   // Get the area description supressing trailing description
   // scanning if in manual mode

   pszDescr = DoGetNewAreaDescr(pszArea, !cfg.fManualMode);

   // Create the new area and check if ok

   if (DoCreateNewArea(pszArea, pszDescr, pnode)) {
     if (pszDescr != NULL && pszDescr[0])
       WriteMsg("\nCan't create area %s \"%s\"\n", pszArea, pszDescr);
     else
       WriteMsg("\nCan't create area %s\n", pszArea);
     return FALSE;
   } else
     if (cfg.fl & FL_REPORTMSG)
     {
       if (pszDescr != NULL && pszDescr[0])
         WriteMsg("\nCreated area %s \"%s\"\n", pszArea, pszDescr);
       else
         WriteMsg("\nCreated area %s\n", pszArea);
     }
   // Show we've created new area

   cfg.fExitCode|= EXIT_PROCAREA;

   return TRUE;
 }

/*
 * Destroy area on the specified node request
 */

 BOOL APPENTRY ExecDestroyArea(PNODE pnode, PSZ pszArea)
 {
   CHAR achText[512];
   PNODE pnodeNotify;
   PDELAREA pdelarea;
   BOOL fSquishArea;
   NETADDR netAddr;
   PAREA parea;
   PLINK plink;

   // Check if remote operations allowed for this node

   if (!(pnode->fs & NF_AREADESTROY)) {
     WriteMsg("\nArea destroy is not allowed for node %s\n",
               FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Check if we have the area name specification

   if (!pszArea || !pszArea[0]) {
     WriteMsg("\nMissing tag of the area to destroy\n");
     return FALSE;
   } else
#ifndef UNIX
     xstrupr(pszArea);
#endif

   // Check if we have an area with the specified tag

   if ((parea = GetAreaFromTag(pszArea)) == NULL) {
     WriteMsg("\nArea %s does not exists\n", pszArea);
     return FALSE;
   }

   // Destroy the area and report

   if ((pdelarea = DelArea(parea)) == NULL) {
     if (!parea->pszDescr || !parea->pszDescr[0])
       WriteMsg("\nCan't destroy area %s\n", pszArea);
     else
       WriteMsg("\nCan't destroy area %s \"%s\"\n", pszArea, parea->pszDescr);
     return FALSE;
   } else {
     if (cfg.fl & FL_REPORTMSG)
     {
       if (!parea->pszDescr || !parea->pszDescr[0])
         WriteMsg("\nDestroyed area %s\n", pszArea);
       else
         WriteMsg("\nDestroyed area %s \"%s\"\n", pszArea, parea->pszDescr);
     }
     pdelarea->pnode = pnode;
   }

   // Destroy the area message base if necessary... woops!

   if (!(cfg.fl & FL_PRESERVEDESTRAREAS) && !(cfg.fl & FL_TESTMODE))
     DelAreaMsgBase(parea);

   // Log the area deletion

   fSquishArea = IsSquishArea(parea->pszSqshFlags);
   WriteLog("* Area %s destroyed by %s in %s%s\n",
             parea->achTag, FormatNetAddr(&pnode->netAddr),
//             parea->pszPath, fSquishArea ?  ".SQ?" : "\\*.MSG");
             parea->pszPath, fSquishArea ?  ".SQ?" : PATH_DELIMS "*.MSG");

   if (parea->pszDescr != NULL && parea->pszDescr[0])
     WriteLog("* Desc \"%s\"\n", parea->pszDescr);

   // Inform all the existing links if any

   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext) {

     // Get the aka address which matches the notified node zone

     xmemcpy(&netAddr, GetAddrMatch(&plink->netAddr), sizeof(NETADDR));

     // Compose the node notification message

     if (parea->pszDescr == NULL || !parea->pszDescr[0])
       sprintf(achText, "\rArea %.128s destroyed at %s by node %s"
                        "\r",
               parea->achTag,
               FormatNetAddr(&netAddr), FormatNetAddr(&pnode->netAddr));
     else
       sprintf(achText, "\rArea %.128s \"%.128s\" destroyed at %s by node %s"
                        "\r",
               parea->achTag, parea->pszDescr,
               FormatNetAddr(&netAddr), FormatNetAddr(&pnode->netAddr));

     // Create the node notification message

     pnodeNotify = GetNodeFromAddr(&plink->netAddr);
     if (pnodeNotify && (pnodeNotify->fs & NF_DONTNOTIFY)) continue;
     SendMsg(&netAddr, SQAFIX_NAME, &plink->netAddr,
             GetNodeSysop(pnodeNotify),
             GetNodeMsgAttr(pnodeNotify),
             "Area destroyed notification", achText);
   }

   return TRUE;
 }

/*
 * Send areafix request to relink existing area linked to given node
 */

 BOOL APPENTRY ExecRLnNodeArea(PNODE pnode, PSZ pszArea)
 {
   static PNODE pnodeRefuse;
   PAREA parea;
   PLINK plink;
   PUPLINK puplink;
   BOOL fAllowed;

   // Check if the given node is an uplink

   if ((puplink = GetUplinkFromNode(pnode)) == NULL) {
     if (pnodeRefuse != pnode) {
       WriteMsg("\nNode %s is not an uplink\n", FormatNetAddr(&pnode->netAddr));
       pnodeRefuse = pnode;
     }
     return FALSE;
   }

   // Check if this area exists

   if ((parea = DoCheckArea(pnode, pszArea, &fAllowed)) == NULL)
     return FALSE;
   else
     plink = GetAreaLink(parea, &pnode->netAddr);

   // Check if this area is in the idle passthru list

   if (GetQueEntry(pszArea, QE_IDLE))
     return FALSE;

   // Check if the node is linked to this area

   if (plink == NULL) {
     if (!cfg.fExecAreaMask)
       WriteMsg("\nArea %s is not linked to node %s\n",
                 pszArea, FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Add the link areafix request

   AddAfrq(puplink, pszArea, AFRQ_LINK);

   return TRUE;
 }

/*
 * Send areafix request to relink frequed area for the given uplink
 */

 BOOL APPENTRY ExecRFrNodeArea(PNODE pnode, PSZ pszArea)
 {
   static PNODE pnodeRefuse;
   PUPLINK puplink;
   PQUE pque;

   // Check if the given node is an uplink

   if ((puplink = GetUplinkFromNode(pnode)) == NULL) {
     if (pnodeRefuse != pnode) {
       WriteMsg("\nNode %s is not an uplink\n", FormatNetAddr(&pnode->netAddr));
       pnodeRefuse = pnode;
     }
     return FALSE;
   }

   // Check if this area exists in freq queue

   if ((pque = GetQueEntry(pszArea, QE_FREQ)) == NULL)
     return FALSE;

   // Check if this area uplink is matching

   if (!pque->plink || CompNetAddr(&pque->plink->netAddr, &pnode->netAddr))
     return FALSE;

   // Add the link areafix request

   AddAfrq(puplink, pszArea, AFRQ_LINK);

   // Update the queue entry expiration time and reset warning sent flag

   pque->time2 = time(NULL) + cfg.dayFReqTimeout * SECS_IN_DAY;
   pque->fs&= QF_WARNINGDONE;

   return TRUE;
 }

/*
 * Send areafix request to reunlink killed area for the given uplink
 */

 BOOL APPENTRY ExecRKlNodeArea(PNODE pnode, PSZ pszArea)
 {
   static PNODE pnodeRefuse;
   PUPLINK puplink;
   PQUE pque;

   // Check if the given node is an uplink

   if ((puplink = GetUplinkFromNode(pnode)) == NULL) {
     if (pnodeRefuse != pnode) {
       WriteMsg("\nNode %s is not an uplink\n", FormatNetAddr(&pnode->netAddr));
       pnodeRefuse = pnode;
     }
     return FALSE;
   }

   // Check if this area exists in kill queue

   if ((pque = GetQueEntry(pszArea, QE_KILL)) == NULL)
     return FALSE;

   // Check if this area uplink is matching

   if (!pque->plink || CompNetAddr(&pque->plink->netAddr, &pnode->netAddr))
     return FALSE;

   // Add the unlink areafix request

   AddAfrq(puplink, pszArea, AFRQ_UNLINK);

   // Update the queue entry expiration time and reset warning sent flag

   pque->time2 = time(NULL) + cfg.dayIdleIgnore * SECS_IN_DAY;
   pque->fs&= QF_WARNINGDONE;

   return TRUE;
 }

/*
 * Process compressor change and report requests
 */

 BOOL APPENTRY ExecCompress(PNODE pnode, PSZ pszArc)
 {
   BOOL fDefPacker, fCurPacker, fListed = FALSE;
   PSZ pszDefPacker = cfg.pszDefPacker;
   PSZ pszCurPacker = NULL;
   PARC parc;
   PLSZ plsz;

   // Check if this system is not set up for packer control

   if (cfg.plszAllowArc == NULL) {
     WriteMsg("\nSorry, this system is not set up for packer operations\n");
     return FALSE;
   }

   // Check if packer control is allowed for this system

   if (!(pnode->fs & NF_COMPRESSOK)) {
     WriteMsg("\nNode %s is not allowed for packer operations\n",
               FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Find out the current packer for this node

   if ((parc = GetArc(&pnode->netAddr, NULL)) != NULL)
     pszCurPacker = parc->achArc;

   // Check if we need to report available packers

   if (pszArc == NULL || pszArc[0] == '?') {

     // Write out header

     DoWriteHeader(&pnode->netAddr, "List of packers available for node");

     // Write out all available packer names

     for (plsz = cfg.plszAllowArc; plsz != NULL; plsz = plsz->plszNext) {

       // Check if default and current packers

       fCurPacker = !pszCurPacker ? FALSE : !xstricmp(plsz->ach, pszCurPacker);
       fDefPacker = !pszDefPacker ? FALSE : !xstricmp(plsz->ach, pszDefPacker);

       // Format and write out packer table line

       if (fCurPacker && fDefPacker) {
         WriteMsg("%s\tCurrent packer for %s (default at %s)\n", plsz->ach,
                   FormatNetAddr(&pnode->netAddr),
                   FormatNetAddr(&cfg.anetAddr[0]));
       } else
       if (fCurPacker) {
         WriteMsg("%s\tCurrent packer for %s\n", plsz->ach,
                   FormatNetAddr(&pnode->netAddr));
       } else
       if (fDefPacker) {
         WriteMsg("%s\tDefault packer at %s\n", plsz->ach,
                   FormatNetAddr(&cfg.anetAddr[0]));
       } else
         WriteMsg("%s\n", plsz->ach);

       fListed = TRUE;
     }

     // Finish the message

     WriteMsg(fListed ? "\n" : "<none>\n");

   } else {

     // Check if the requested packer is allowed

     if (!GetLsz(cfg.plszAllowArc, pszArc)) {
       WriteMsg("\nPacker %s is not available\n", pszArc);
       return FALSE;
     }

     // Check if packer specification exists for this node and remove it

     while ((parc = GetArc(&pnode->netAddr, NULL)) != NULL)
       DelArcLink(parc, &pnode->netAddr);

     // Scan all the packer list entires looking for the place for
     // this node packer specification. We want to reuse existing
     // packer entry unless there are any address masks before it

     for (parc = cfg.parcFirst; parc != NULL; parc = parc->parcNext)
       if (CheckArcAddrMask(parc)) {
         parc = NULL;
         break;
       } else
       if (!xstricmp(parc->achArc, pszArc))
         break;

     // Create the new packer entry if necessary

     if (parc == NULL)
       if ((parc = AddArc(pszArc, 0)) == NULL) {
         WriteMsg("\nCan't create packer %s -- system error\n", parc->achArc);
         WriteLog("! Can't create %s entry for %s\n", parc->achArc,
                  FormatNetAddr(&pnode->netAddr));
         return FALSE;
       }

     // Add the node link to the packer list entry

     if (!AddArcLink(parc, &pnode->netAddr)) {
       WriteMsg("\nCan't change packer to %s -- system error\n", parc->achArc);
       WriteLog("! Can't change packer to %s for %s\n", parc->achArc,
                FormatNetAddr(&pnode->netAddr));
       return FALSE;
     }

     // Report and log operation

     WriteMsg("\nChanged packer to %s\n", parc->achArc);
     WriteLog("- Pack %s for %s\n", parc->achArc,
              FormatNetAddr(&pnode->netAddr));

     // Force packer list update

     cfg.fPackChanged = TRUE;
   }


   return TRUE;
 }

/*
 * Create a link state changes report for the specified node
 */

 BOOL APPENTRY CreateChangesReport(PNODE pnode, PSZ pszNull)
 {
   BOOL fListed = FALSE;
   PSZ psz;

   // Check if we have to report something and write out header

   if (cfg.pdoneFirst == NULL)
     return FALSE;
   else
     DoWriteHeader(&pnode->netAddr, "Summary of echo link changes for node");

   // Write out all the changes made cleaning up the list

   while (cfg.pdoneFirst != NULL) {

     // Store in line the area tag and the dotted line after it

     DoSetAreaName(cfg.pdoneFirst->parea->achTag);

     // Write out the action string

     switch (cfg.pdoneFirst->fWhat) {
       case DN_LINKED:     psz = " Linked\n";           break;
       case DN_UNLINKED:   psz = " Unlinked\n";         break;
       case DN_PASSIVE:    psz = " Passive\n";          break;
       case DN_ACTIVE:     psz = " Active\n";           break;
       default:            psz = " Lost in space!\n";   break;
     }

     // Add to line the action taken and write line out

     DoAddLine(psz);
     WriteMsg("%s", achLine);
     fListed = TRUE;

     // Unlink and destroy the reported action element

     LstDestroyElement(LstUnlinkElement((PPLE)&cfg.pdoneFirst, 0));
   }

   // Finish message

   WriteMsg(fListed ? "\n" : "<none>\n");

   return TRUE;
 }

/*
 * Create a report of all areas linked to the specified node
 */

 BOOL APPENTRY CreateLinkedReport(PNODE pnode, PSZ pszNull)
 {
   BOOL fListed = FALSE;
   PAREA parea;
   PLINK plink;
   PSZ psz;

   // Write out header

   DoWriteHeader(&pnode->netAddr, "List of all areas linked to node");

   // Scan through all areas checking if the node is linked to it

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
     if ((plink = GetAreaLink(parea, &pnode->netAddr)) != NULL) {

       DoSetAreaName(parea->achTag);

       psz = (plink->fs & LF_ACTIVE) ? "Active" : "Passive";

       // Add to line the area state, group and description and write it out

       DoAddAreaInfo(parea, pnode, psz, TRUE);
       WriteMsg("%s", achLine);
       fListed = TRUE;
     }

   // Finish message

   WriteMsg(fListed ? "\n" : "<none>\n");

   return TRUE;
 }

/*
 * Create a report of all unlinked areas for a given node
 */

 BOOL APPENTRY CreateUnlinkedReport(PNODE pnode, PSZ pszNull)
 {
   BOOL fListed = FALSE;
   BOOL fAllowed;
   PAREA parea;
   PLINK plink;
   PSZ psz;

   // Write out header

   DoWriteHeader(&pnode->netAddr, "List of unlinked areas available for node");

   // Scan through all areas checking if the node is linked
   // either to the active or to the passive list

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {

     // Check area visibility and link status for this node

     plink = DoCheckAreaStatus(parea, pnode, &fAllowed, FALSE);

     // Check if node should see this area, that is if it is not
     // linked to it, area is alowed to it or is visible

     if (fAllowed && (plink == NULL || !(plink->fs & LF_ACTIVE))) {

       DoSetAreaName(parea->achTag);

       psz = (plink == NULL) ? "Unlinked" : "Passive";

       // Add to line the area state, group and description and write it out

       DoAddAreaInfo(parea, pnode, psz, TRUE);
       WriteMsg("%s", achLine);
       fListed = TRUE;
     }
   }

   // Finish message

   WriteMsg(fListed ? "\n" : "<none>\n");

   return TRUE;
 }

/*
 * Create a report of existing areas availble for the specified node
 */

 BOOL APPENTRY CreateAreasReport(PNODE pnode, PSZ pszNull)
 {
   BOOL fListed = FALSE;
   BOOL fAllowed;
   PAREA parea;
   PLINK plink;
   PSZ psz;

   // Write out header

   DoWriteHeader(&pnode->netAddr, "List of existing areas available for node");

   // Scan through all areas checking if the node is linked
   // either to the active or to the passive list

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {

     // Check area visibility and link status for this node

     plink = DoCheckAreaStatus(parea, pnode, &fAllowed, FALSE);

     // Check if node should see this area, that is if it is already
     // linked to it, area is alowed to it or is visible for node

     if (plink != NULL || fAllowed || (!fAllowed &&
        (parea->fs & AF_VISIBLE) && (pnode->fs & NF_VISIBLE))) {

       DoSetAreaName(parea->achTag);

       if (plink != NULL)
         psz = (plink->fs & LF_ACTIVE) ? "Active" : "Passive";
       else
         psz = fAllowed ? "Unlinked" : "Restricted";

       // Add to line the area state, group and description and write it out

       DoAddAreaInfo(parea, pnode, psz, TRUE);
       WriteMsg("%s", achLine);
       fListed = TRUE;
     }
   }

   // Finish message

   WriteMsg(fListed ? "\n" : "<none>\n");

   return TRUE;
 }

/*
 * Create a report of all areas availble for the specified node
 */

 BOOL APPENTRY CreateAvailReport(PNODE pnode, PSZ pszNull)
 {
   PAREA parea, pareaFirstSave;
   BOOL fListed = FALSE;
   PNEWAREA pnewarea;
   PUPLINK puplink;
   PLSZ plsz;
   PSZ psz;

   // Check if this system is not set up for request forwarding

   if (!cfg.dayFReqTimeout) {
     WriteMsg("\nSorry, this system is not set up for request forwarding\n");
     return FALSE;
   }

   // Check if requsts forwarding is allowed for this system

   if (!(pnode->fs & NF_FORWARDREQ)) {
     WriteMsg("\nNode %s is not allowed for request forwarding\n",
               FormatNetAddr(&pnode->netAddr));
     return FALSE;
   }

   // Write out header

   DoWriteHeader(&pnode->netAddr, "List of uplink areas available for node");

   // Save the existing area list head pointer and reset it to
   // build the fake list of areas for uplink area report

   pareaFirstSave = cfg.pareaFirst; cfg.pareaFirst = NULL;

   // Scan through all the uplink nodes reporting the available areas

   for (puplink = cfg.puplinkFirst; puplink != NULL;
        puplink = puplink->puplinkNext) {

     // Check if this node access level is not good enough to
     // send freqs to this uplink

     if (pnode->level < puplink->level) continue;

     // Loop through available area masks creating the faked uplink arealist

     for (plsz = puplink->plszMask; plsz != NULL; plsz = plsz->plszNext)
       if (!GetAreaFromTag(plsz->ach)) {
         pnewarea = DoGetNewAreaInfo(plsz->ach, puplink->pnode);
         parea = AddArea(plsz->ach, pnewarea->chGroup);
         parea->level = puplink->level;
         if ((psz = GetAreaDescrFromTag(plsz->ach)) != NULL)
           parea->pszDescr = psz;
       }
   }

   // Loop through all faked uplink arealist and report

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
     if ((psz = DoGetAvailAreaStatus(pareaFirstSave, parea, pnode)) != NULL) {
       DoSetAreaName(parea->achTag);
       DoAddAreaInfo(parea, pnode, psz, TRUE);
       WriteMsg("%s", achLine);
       fListed = TRUE;
     }

   // Delete the list of the uplink areas

   while ((parea = (PAREA) LstUnlinkElement((PPLE) &cfg.pareaFirst, 0)) != NULL)
     LstDestroyElement((PLE) parea);

   // Restore the existing area list head pointer and reset it

   cfg.pareaFirst = pareaFirstSave;

   // Finish message

   WriteMsg(fListed ? "\n" : "<none>\n");

   return TRUE;
 }

/*
 * Create a usage help for the specified node
 */

 BOOL APPENTRY CreateUsageHelp(PNODE pnode, PSZ pszExt)
 {
   CHAR achUseFile[MAXPATH], achDrive[MAXDRIVE], achDir[MAXDIR];
   CHAR achExt[MAXEXT] = DEF_USE_EXT;

   // Check if there is a file extension provided and set it in
   // Note: termination null is in there since the default ext is '.USE'

   if (pszExt != NULL) {
     if (pszExt[0] != '.')
       xstrncpy(achExt + 1, pszExt, lengof(achExt) - 1);
     else
       xstrncpy(achExt, pszExt, lengof(achExt));
   } else {

     // If there is not help file extension specified use the first one
     // listed in the available help file extensions list if any

     if (cfg.plszHelpExtFirst != NULL && cfg.plszHelpExtFirst->ach[0])
       xstrncpy(achExt + 1, cfg.plszHelpExtFirst->ach, lengof(achExt) - 1);
   }

   // Check if extension matches one of the allowed help file extension

   if (!GetLsz(cfg.plszHelpExtFirst, &achExt[1])) {
     WriteMsg("\nThe usage help file with extension %s was not found at this system"
              "\n",
               achExt);
     return FALSE;
   }

   // Build up the usage info file name using the config file path
   // and try to access it... if fails, set in the default extension

   fnsplit(cfg.achCfgSqaf, achDrive, achDir, NULL, NULL);
   fnmerge(achUseFile, achDrive, achDir, SQAFIX_CFG, achExt);

   if (access(achUseFile, 0) && xstricmp(achExt, DEF_USE_EXT)) {
     WriteLog("$ Can't locate file: %s\n", achUseFile);
     WriteMsg("\nThe usage help file with extension %s was not found at this system."
              "\nSending default help file. Please, inform the SysOp of this error..."
              "\n",
               achExt);
     fnmerge(achUseFile, achDrive, achDir, SQAFIX_NAME, DEF_USE_EXT);
   }

   // Add the send file list element and check if ok

   if (!AddSndf(pnode, achUseFile, "SqaFix remote request help")) {
     WriteMsg("\nSystem error while sending usage help info"
              "\n");
     return FALSE;
   }

   // Inform the user

   if (!cfg.fManualMode)
     WriteMsg("\nUsage help will be sent in a separate message"
              "\n");

   return TRUE;
 }

/*
 * Create a new area with the specified tag
 */

 BOOL APPENTRY CreateNewArea(PSZ pszArea, NETADDR * pnetAddr)
 {
   static CHAR achAreaRefuse[MAX_AREA_LENG];
   static NETADDR netAddrRefuse;
   PNODE pnode;
   BOOL fFail;
   PQUE pque;

   // Since zone number is not stored in the PATH kludge the zone
   // specified in the 'netAddr' is set to zero. So in order to get
   // the node descriptor we'll loop through all the nodes which are
   // allowed to create new areas looking for a <net/node> match

   pnetAddr->zone = pnetAddr->point = 0;
   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext)
     if (pnode->fs & NF_AUTOCREATE &&
         pnetAddr->node == pnode->netAddr.node &&
         pnetAddr->net == pnode->netAddr.net) {

       // Check if we already have the zone stored

       if (pnetAddr->zone == 0) {
         pnetAddr->zone = pnode->netAddr.zone;
       } else {
         WriteLog("! Can't evaluate zone: %u/%u is both in zone %u and %u\n",
                  pnetAddr->net, pnetAddr->node,
                  pnetAddr->zone, pnode->netAddr.zone);
         return FALSE;
       }
     }

   // Check if we got the node which is allowed to create new areas

   if ((pnode = GetNodeFromAddr(pnetAddr)) == NULL ||
      !(pnode->fs & NF_AUTOCREATE)) {

     // Check if we just reported this node/area and dont repeat

     if (xmemcmp(&netAddrRefuse, pnetAddr, sizeof(NETADDR)) ||
         xstrcmp(achAreaRefuse, pszArea)) {
       WriteLog("- Node %s is not allowed to create area %s\n",
                FormatNetAddr(pnetAddr), pszArea);
       xmemcpy(&netAddrRefuse, pnetAddr, sizeof(NETADDR));
       xstrncpy(achAreaRefuse, pszArea, lengof(achAreaRefuse));
     }

     return FALSE;
   }

   // Check if this area is in the kill queue since it was recently
   // deleted as idle passthru and this is a left over packet causing
   // just deleted area autocreation

   if ((pque = GetQueEntry(pszArea, QE_KILL)) != NULL) {

     // Check if we just reported this node/area and don't repeat

     if (xmemcmp(&netAddrRefuse, pnetAddr, sizeof(NETADDR)) ||
         xstrcmp(achAreaRefuse, pszArea)) {
       WriteLog("- Area %s was idle passthru, no autocreate until %s for %s\n",
                pszArea, FormatSecs(pque->time2), FormatNetAddr(pnetAddr));
       xmemcpy(&netAddrRefuse, pnetAddr, sizeof(NETADDR));
       xstrncpy(achAreaRefuse, pszArea, lengof(achAreaRefuse));
     }

     // Return flag to kill this message

     goto Kill;
   }

   // Make sure we don't have the area with the same tag

   if (GetAreaFromTag(pszArea) != NULL)
     return FALSE;

   // Attempt to create the new area

   fFail = DoCreateNewArea(pszArea, DoGetNewAreaDescr(pszArea, FALSE), pnode);

   // Check if failed and return appropriate return code

   if (!fFail) return TRUE;
   if (fFail == (BOOL)-1) goto Kill;

   return FALSE;

   // Return flag to kill this message

Kill: return (BOOL)-1;
 }

/*
 * Create a new areas report
 */

 BOOL APPENTRY CreateNewAreasReport(VOID)
 {
   PNEWAREA pnewarea;
   PAREA parea;

   // Scan through all areas to see if there are any not reported
   // autocreated areas

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
     if (parea->fs & AF_AUTOCREATEDAREA &&
       !(parea->fs & AF_AUTOCREATEREPORTED)) {

       // Scan through node specific new area info elements looking for
       // the settings of the node which triggered creation of this area

       for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
            pnewarea = pnewarea->pnewareaNext)
         if (!CompNetAddr(&parea->pnodeAutoCreate->netAddr, &pnewarea->netAddr))
           break;

       // Check if no node specific new area info elements found and
       // assume default new area info

       if (pnewarea == NULL) pnewarea = &newareaDef;

       // Report area creations for the given node

       DoReportNewAreas(parea->pnodeAutoCreate, pnewarea);
     }

   return TRUE;
 }

/*
 * Create an all links report
 */

 BOOL APPENTRY CreateAllLinksReport(NETADDR * pnetAddr)
 {
   CHAR achAddr[64];
   NETADDR netAddr;
   SHORT cchPad;
   PAREA parea;
   PLINK plink;
   BOOL fListed;

   // Write out header

   DoWriteHeader(pnetAddr, "Following is the list of all echo mail links at");

   // Scan through all areas listing it's linked nodes

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {

     // Set the area tag and tab dots into the line

     cchPad = DoSetAreaName(parea->achTag);
     xmemset(&netAddr, 0, sizeof(NETADDR));
     fListed = FALSE;

     for (plink = parea->plink; plink != NULL; plink = plink->plinkNext) {

       // Format the link network address address

       xstrcpy(achAddr, MakeNetAddrList(&plink->netAddr, &netAddr));

       // Check the line margin and if exceeded, write out this line
       // and continue on the next one with the full 4D address

       if (xstrlen(achLine) + xstrlen(achAddr) > cfg.cchMaxMsgLine) {
         DoAddLine("\n"); WriteMsg("%s", achLine);
         DoSetLine("%*s", cchPad, "");
         xmemset(&netAddr, 0, sizeof(NETADDR));
         xstrcpy(achAddr, MakeNetAddrList(&plink->netAddr, &netAddr));
       }

       // Appent linked node address

       DoAddLine(achAddr);
       fListed = TRUE;
     }

     // Append new line character and write out the last line

     DoAddLine(fListed ? "\n" : " <none>\n");
     WriteMsg("%s", achLine);
   }

   return TRUE;
 }

/*
 * This routine creates notifications
 */

 BOOL APPENTRY CreateNotes(VOID)
 {

   // Check if there area any notifications to create

   if (cfg.pnoteFirst == NULL) return FALSE;

   // Log the operation

   WriteLog("* Creating notifications\n");

   // Report all the notification in maxmsg chunks. Note that created
   // freqs are reported after the expired ones so that the alternate
   // uplinks will be in reasonable sequence

   while(DoReportFreq(NT_FREQ_EXPNOTE));
   while(DoReportFreq(NT_FREQ_EXPWARN));
   while(DoReportFreq(NT_FREQ_CREATED));
   while(DoReportIdle(NT_IDLE_EXPNOTE));
   while(DoReportIdle(NT_IDLE_EXPWARN));

   // Delete the notifications list

   DelNoteList();

   return TRUE;
 }

/*
 * This routine syncs SqaFix area creating it in Squish
 */

 BOOL APPENTRY SyncSqafArea(PAREA parea)
 {
   PSZ psz, pszFile, pszArea = parea->achTag;
   NETADDR * pnetAddr;
   PNEWAREA pnewarea;
   USHORT iAddr;

   // Check if there are specific new area settings for this areamask

   for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
        pnewarea = pnewarea->pnewareaNext)
     if (GrepSearch(pszArea, pnewarea->pszArea, FALSE))
       break;

   // Otherwise assume the default new area info

   if (pnewarea == NULL) pnewarea = &newareaDef;

   // Make up the area base file name and check if ok

   if ((pszFile = DoMakeNewAreaPath(pszArea, pnewarea)) == NULL) {
     WriteLog("! Can't make unique file name for area %s\n", pszArea);
     return FALSE;
   }

   // Allocate squish area flags and check if ok

   psz = pnewarea->pszFlags ? pnewarea->pszFlags : (PSZ) "";
   if ((psz = AllocString((PSZ) psz, -1)) == NULL) {
     WriteLog("! Insufficient memory (sqsh area flags)\n");
     exit(EXIT_FAILURE);
   }

   // Make up the squish area info

   parea->pszPath = pszFile;
   parea->pszSqshFlags = psz;

   // Link up all the autolink nodes

   for (iAddr = 0, pnetAddr = pnewarea->anetNode;
        iAddr < numbof(pnewarea->anetNode) && pnetAddr->zone != 0;
        iAddr++, pnetAddr++)
     AddAreaLink(parea, pnetAddr, ACTIVE, "(autolink)");

   return TRUE;
 }

/*
 * End of SQAEXE.C
 */
