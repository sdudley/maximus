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
 * Miscellaneous utility routines
 *
 * Created: 06/Jan/92
 * Updated: 24/Dec/99
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

#ifndef UNIX
 #include <io.h>
 #include <dos.h>
 #include <process.h>
 #include "ffind.h"
#else
 // huh?
 static int errno;
#endif
 #include <fcntl.h>
 #include "sqafix.h"

/*
 * This routine calculates hash value for the given string
 */

 ULONG APPENTRY CalcHash(PSZ psz)
 {
   ULONG fix, hash = 0l;
   PCH pch;

   ASSERT(psz != NULL);

   // Calculate hash value for the given string

   for (pch = psz; *pch; pch++) {
     hash = (hash << 4) + (ULONG)tolower(*pch);
     if ((fix = (hash & 0xf0000000l)) != 0l) {
       hash|= fix >> 24;
       hash|= fix;
     }
   }

   // Strip off the high bit

   return (hash & 0x7FFFFFFFlu);
 }

/*
 * This routines are stubs for DW-TEXT memory allocation calls
 */
#ifndef __DPMI__
  PVOID APIENTRY MemAlloc(size_t cb, USHORT fs)
  {
    PVOID p;

    if ((p = malloc(cb)) == NULL) return NULL;
    if (fs & MA_CLEAR) xmemset(p, 0, cb);

    return p;
  }

  PVOID APIENTRY MemRealloc(PVOID p, size_t cb)
  {
    // Reallocate specified memory block and check if ok

    if ((p = realloc(p, cb)) == NULL)
      return NULL;

    return p;
  }

  VOID  APIENTRY MemFree(PVOID p)
  {
    if (p != NULL) free(p);
  }
#endif

/*
 * This routine skips over the leading spaces and returns first nonspace
 */

 CHAR APPENTRY SkipSpaces(PCH * ppch)
 {
   ASSERT(ppch != NULL);
   ASSERT(*ppch != NULL);

   while (isspace(**ppch)) (*ppch)++;
   return **ppch;
 }

/*
 * This routine scans the Squish area flags and checks if it's a *.SQ? area
 */

 BOOL APPENTRY IsSquishArea(PSZ pszFlags)
 {
   // Scan in all the area options if any and check for the '-$' flag

   if (pszFlags != NULL)
     while (*pszFlags && SkipSpaces(&pszFlags) == '-') {
       if (*++pszFlags == '$') return TRUE;
       while (*pszFlags && !isspace(*pszFlags) && *pszFlags != '-') pszFlags++;
     }

   return FALSE;
 }

/*
 * This routine checks if given string for special command line characters
 */

 BOOL APPENTRY IsSpecCmdChars(PSZ psz)
 {
#if defined __OS2__ || defined __W32__
   static CHAR achSpec[] = "<>|&";
#else
   static CHAR achSpec[] = "<>|";
#endif

   ASSERT(psz != NULL);

   // Scan the given string

   while (*psz)
     if (xstrchr(achSpec, *psz) != NULL)
       return TRUE;
     else
       psz++;

   return FALSE;
 }

/*
 * This routine scans the Squish area flags and checks if it's passthru area
 */

 BOOL APPENTRY IsPassThruArea(PSZ pszFlags)
 {
   // Scan in all the area options if any and check for the '-0' flag

   if (pszFlags != NULL)
     while (*pszFlags && SkipSpaces(&pszFlags) == '-') {
       if (*++pszFlags == '0') return TRUE;
       while (*pszFlags && !isspace(*pszFlags) && *pszFlags != '-') pszFlags++;
     }

   return FALSE;
 }

/*
 * This routine checks for idle passthru area
 */

 BOOL APPENTRY IsIdlePassThruArea(PAREA parea)
 {
   USHORT iLink;
   PLINK plink;

   ASSERT(parea != NULL);

   // Check if area is not a passthru area or there are no links at all

   if (!IsPassThruArea(parea->pszSqshFlags) || parea->plink == NULL)
     return FALSE;

   // Calculate number of links which are not present in ignore passthru
   // nodes list

   for (plink = parea->plink, iLink = 0; plink != NULL; plink = plink->plinkNext) {

     // Check if we have to ignore passive links and this link is the one

     if ((cfg.fl & FL_IDLEIGNOREPASSIVE) && !(plink->fs & LF_ACTIVE))
       continue;

     // Check if this link is in the ignore node list

     if (IsAddrInList(cfg.anetIdleNode, numbof(cfg.anetIdleNode), &plink->netAddr))
       continue;

     // Increment links count

     iLink++;
   }

   // Idle if only one link for this area

   return iLink == 1;
 }

/*
 * This routine checks if the group character is in string
 */

 BOOL APPENTRY CheckGroup(PSZ pszGroups, CHAR chGroup, BOOL * pfReadOnly)
 {
   BOOL fLower, fUpper;

   // Check if there is a group specification

   if (pszGroups == NULL) return FALSE;

   // Check if lower or upper case group specification is present

   fLower = (xstrchr(pszGroups, tolower(chGroup)) != NULL);
   fUpper = (xstrchr(pszGroups, toupper(chGroup)) != NULL);

   // Check if we need to set readonly flag

   if (pfReadOnly != NULL) *pfReadOnly = fLower;

   // Return positive on either lower or upper group specification

   return (fLower || fUpper);
 }

/*
 * This routine allocates memory and sets in string there
 */

 PSZ APPENTRY AllocString(PCH pch, SHORT cch)
 {
   PSZ psz;

   // Check if null string specified

   if (pch == NULL) return NULL;

   // Check if asciiz string and get its length

   if (cch == (SHORT)-1) cch = xstrlen(pch);

   // Allocate memory and copy the string in

   if ((psz = MemAlloc(cch + 1, 0)) != NULL) {
     xmemcpy(psz, pch, cch); psz[cch] = '\0';
   }

   return psz;
 }

/*
 * This routine adds a string to the array of string pointers
 */

 BOOL APPENTRY AddApsz(PSZ apsz[], USHORT cpsz, PSZ psz)
 {
   USHORT cch, ipsz;

   ASSERT(apsz != NULL);
   ASSERT(psz != NULL);

   // Check if we already have this string defined

   for (ipsz = 0; ipsz < cpsz && apsz[ipsz] != NULL; ipsz++)
     if (!xstricmp(apsz[ipsz], psz))
       return TRUE;

   // Check if we have room for one more string pointer

   if (ipsz >= cpsz)
     return FALSE;

   // Allocate memory to hold the new string and set it in

   if ((cch = xstrlen(psz)) > 0)
     if ((apsz[ipsz] = MemAlloc(cch + 1, MA_CLEAR)) != NULL)
       xstrcpy(apsz[ipsz], psz);
     else
       return FALSE;

   return TRUE;
 }

/*
 * This routine checks if a given string is in the string list
 */

 BOOL APPENTRY IsStringInList(PSZ apsz[], USHORT cpsz, PSZ psz)
 {
   USHORT ipsz;

   ASSERT(apsz != NULL);
   ASSERT(psz != NULL);

   // Scan the specified string list looking for a match

   for (ipsz = 0; ipsz < cpsz && apsz[ipsz] != NULL; ipsz++)
     if (!xstricmp(apsz[ipsz], psz))
       return TRUE;

   return FALSE;
 }

/*
 * This routine checks if a given string prefix is in the string list
 */

 BOOL APPENTRY IsPrefixInList(PSZ apsz[], USHORT cpsz, PSZ psz)
 {
   PCH pch1, pch2;
   USHORT ipsz;

   ASSERT(apsz != NULL);
   ASSERT(psz != NULL);

   // Scan the specified string list looking for a match

   for (ipsz = 0; ipsz < cpsz && apsz[ipsz] != NULL; ipsz++) {
     for(pch1 = psz, pch2 = apsz[ipsz]; *pch1 && *pch2 &&
         toupper(*pch1) == toupper(*pch2); pch1++, pch2++);
     if (!*pch2) return TRUE;
   }

   return FALSE;
 }

/*
 * This routine checks if a given string matches any mask in list
 */

 BOOL APPENTRY IsMatchInList(PSZ apsz[], USHORT cpsz, PSZ psz)
 {
   USHORT ipsz;

   ASSERT(apsz != NULL);
   ASSERT(psz != NULL);

   // Scan the specified string list looking for a match

   for (ipsz = 0; ipsz < cpsz && apsz[ipsz] != NULL; ipsz++)
     if (GrepSearch(psz, apsz[ipsz], FALSE))
       return TRUE;

   return FALSE;
 }

/*
 * This routine adds a string to a linked list of strings
 */

 PLSZ APPENTRY AddLsz(PLSZ * pplsz, PCH pch, SHORT cch, SHORT ipos)
 {
   PLSZ plsz;

   ASSERT(pch != NULL);

   // Check if null terminated string and get its length

   if (cch == (SHORT)-1) cch = xstrlen(pch);

   // Create new string list element and check if ok

   if ((plsz = (PLSZ) LstCreateElement(sizeof(LSZ) + cch)) == NULL)
     return NULL;

   // Link the new string list element at the specified position and
   // check if ok. If there is no list head specification, then just
   // create an element assuming it would be linked by the caller

   if (pplsz != NULL)
     if (LstLinkElement((PPLE) pplsz, (PLE) plsz, ipos) == LST_ERROR) {
       LstDestroyElement((PLE) plsz);
       return NULL;
     }

   // Set in the string and its length

   xmemcpy(plsz->ach, pch, cch);
   plsz->cch = cch;
   plsz->hash = CalcHash(plsz->ach);

#ifdef DEBUG
//fprintf(STDAUX, "AddLsz: pplsz=[%09Fp], '%s'\r\n", pplsz, plsz->ach);
#endif

   return plsz;
 }

/*
 * This routine gets a string from a linked list of strings
 */

 PLSZ APPENTRY GetLsz(PLSZ plsz, PSZ psz)
 {
   ULONG hash;

   ASSERT(psz != NULL);

   // Get the hash value for the search string

   hash = CalcHash(psz);

   // Search the list

   while (plsz != NULL)
     if (plsz->hash == hash && !xstricmp(plsz->ach, psz))
       return plsz;
     else
       plsz = plsz->plszNext;

   return NULL;
 }

/*
 * This routine deletes the linked list of strings
 */

 VOID APPENTRY DelLszList(PLSZ * pplsz)
 {
   PLSZ plsz;

   ASSERT(pplsz != NULL);

   // Delete the linked list of strings

   while (*pplsz != NULL) {
     plsz = (PLSZ) LstUnlinkElement((PPLE) pplsz, 0);
     ASSERT(plsz != NULL);

#ifdef DEBUG
//fprintf(STDAUX, "DelLszList: pplsz=[%09Fp], '%s'\r\n", pplsz, plsz->ach);
#endif

     LstDestroyElement((PLE) plsz);
   }

   ASSERT(LstQueryElementCount((PLE) *pplsz) == 0);
 }

/*
 * This routine will return reply message attributes for the node
 */

 USHORT APPENTRY GetNodeMsgAttr(PNODE pnode)
 {
   USHORT attr = MSGLOCAL | MSGPRIVATE;
   USHORT fs;

   // Check if node is known and choose default node flags if not

   fs = (pnode != NULL) ? pnode->fs : cfg.fsDefNodeFlags;

   // Set message attributes according to the node flags

   if (fs & NF_KILLSENT)  attr|= MSGKILL;
   if (fs & NF_SENDHOLD)  attr|= MSGHOLD;
   if (fs & NF_SENDCRASH) attr|= MSGCRASH;

   return attr;
 }

/*
 * This routine will return sysop name for the given node
 */

 PSZ APPENTRY GetNodeSysop(PNODE pnode)
 {
   return (pnode != NULL && pnode->pszSysop != NULL) ? pnode->pszSysop : "SysOp";
 }

/*
 * This routine returns node list element pointer for the given node address
 */

 PNODE APPENTRY GetNodeFromAddr(NETADDR * pnetAddr)
 {
   PNODE pnode;

   ASSERT(pnetAddr != NULL);

   // Check if this node is known to the SqaFix

   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext)
     if (!xmemcmp(&pnode->netAddr, pnetAddr, sizeof(NETADDR)))
       return pnode;

   return NULL;
 }

/*
 * This routine returns uplink list element pointer for the given node address
 */

 PUPLINK APPENTRY GetUplinkFromAddr(NETADDR * pnetAddr)
 {
   PUPLINK puplink;

   ASSERT(pnetAddr != NULL);

   // Scan through the uplinks looking for the node address match

   for (puplink = cfg.puplinkFirst; puplink != NULL; puplink = puplink->puplinkNext)
     if (!xmemcmp(&puplink->pnode->netAddr, pnetAddr, sizeof(NETADDR)))
       return puplink;

   return NULL;
 }

/*
 * This routine returns uplink list element pointer for the given idle area
 */

 PUPLINK APPENTRY GetIdleAreaUplink(PAREA parea)
 {
   PUPLINK puplink;
   PLINK plink;

   ASSERT(parea != NULL);

   // Scan through all the area links skipping the ones which are in
   // ignore idle passthru nodes list

   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext) {

     // Check if we have to ignore passive links and this link is the one

     if ((cfg.fl & FL_IDLEIGNOREPASSIVE) && !(plink->fs & LF_ACTIVE))
       continue;

     // Check if the link is in the ignore list

     if (IsAddrInList(cfg.anetIdleNode, numbof(cfg.anetIdleNode), &plink->netAddr))
       continue;

     // Scan through the uplinks looking for the node address match

     for (puplink = cfg.puplinkFirst; puplink != NULL; puplink = puplink->puplinkNext)
       if (!xmemcmp(&puplink->pnode->netAddr, &plink->netAddr, sizeof(NETADDR)))
         return puplink;
   }

   return NULL;
 }

/*
 * This routine returns uplink list element pointer for the given node
 */

 PUPLINK APPENTRY GetUplinkFromNode(PNODE pnode)
 {
   PUPLINK puplink;

   ASSERT(pnode != NULL);

   // Scan through the uplinks looking for the node match

   for (puplink = cfg.puplinkFirst; puplink != NULL; puplink = puplink->puplinkNext)
     if (puplink->pnode == pnode)
       return puplink;

   return NULL;
 }

/*
 * This routines scan network address specification
 */

 static BOOL SUBENTRY DoHasAddrChar(PCH pch, SHORT ch)
 {
   ASSERT(pch != NULL);

   while (*pch && !isspace(*pch))
     if (*pch == ch)
       return TRUE;
     else
       pch++;

   return FALSE;
 }


 static BOOL SUBENTRY DoIsAddrChar(SHORT ch)
 {
   return (ch && xstrchr("0123456789:/.", ch));
 }


 PCH APPENTRY ScanNetAddr(NETADDR * pnetAddr, PSZ psz)
 {
   PCH pch, pchEnd;

   ASSERT(pnetAddr != NULL);
   ASSERT(psz != NULL);

   // Skip through the leading spaces

   for (pch = psz; isspace(*pch); pch++);
   pchEnd = pch;

   // Scan in the zone if any

   if (DoIsAddrChar(*pch) && DoHasAddrChar(pch, ':')) {
     while (isdigit(*pchEnd)) pchEnd++;
     if (*pchEnd != ':' || pch == pchEnd) return NULL;
     pnetAddr->zone = atoi(pch);
     pch = ++pchEnd;
     if (!isdigit(*pch) || !DoHasAddrChar(pch, '/')) return NULL;
   }

   // Scan in the net if any

   if (DoIsAddrChar(*pch) && DoHasAddrChar(pch, '/')) {
     while (isdigit(*pchEnd)) pchEnd++;
     if (*pchEnd != '/' || pch == pchEnd) return NULL;
     pnetAddr->net = atoi(pch);
     pch = ++pchEnd;
     if (!isdigit(*pch)) return NULL;
   }

   // Scan in the node if any

   if (DoIsAddrChar(*pch) && *pch != '.') {
     while (isdigit(*pchEnd)) pchEnd++;
     if (*pchEnd != '.' && *pchEnd != '@' && !isspace(*pchEnd) && *pchEnd) return NULL;
     pnetAddr->node = atoi(pch);
     pch = pchEnd;
   }

   // Scan in the point if any

   if (*pch != '.') {
     if (*pch == '@') while (*pch && !isspace(*pch)) pch++;
     if (!isspace(*pch) && *pch) return NULL;
     pnetAddr->point = 0;
   } else {
     for (pchEnd = ++pch; isdigit(*pchEnd); pchEnd++);
     if (*pchEnd == '@') while (*pchEnd && !isspace(*pchEnd)) pchEnd++;
     if (!isspace(*pchEnd) && *pchEnd) return NULL;
     pnetAddr->point = atoi(pch);
     pch = pchEnd;
   }

   // Check if zone or net is zero and return

   return (pnetAddr->zone && pnetAddr->net) ? pch : NULL;
 }

/*
 * This routine scans network address mask specification
 */

 #define ISALL(pch, pchAll)  (!memicmp((pch), (pchAll), lengof(pchAll)))

 PCH APPENTRY ScanNetAddrMask(NETADDR * pnetAddr, PSZ psz)
 {
   static CHAR achWORLD[] = "World";
   static CHAR achALL[] = "All";
   static CHAR achALLsALL[] = "All/All";
   static CHAR achALLpALL[] = "All.All";
   static CHAR achALLsALLpALL[] = "All/All.All";
   PCH pch, pchEnd;

   ASSERT(pnetAddr != NULL);
   ASSERT(psz != NULL);

   // Skip through the leading spaces

   for (pch = psz; isspace(*pch); pch++);
   pchEnd = pch;

   // Check for the World mask

   if (!memicmp(pch, achWORLD, lengof(achWORLD))) {
     pch+= lengof(achWORLD);
     pnetAddr->zone = pnetAddr->net = -1;
     pnetAddr->node = pnetAddr->point = -1;
     if (!*pch || isspace(*pch)) return pch;
     return NULL;
   }

   // Check for the leading All mask and character past it

   if (ISALL(pch, achALL)) {
     pch+= lengof(achALL);
     switch (*pch) {

       case ':': // All:
                 pch++;
                 pnetAddr->zone = pnetAddr->net = -1;
                 pnetAddr->node = pnetAddr->point = -1;

                 if (ISALL(pch, achALLsALLpALL))        // ALL:all/all.all ?
                   pch+= lengof(achALLsALLpALL);
                 else
                 if (ISALL(pch, achALLsALL))            // ALL:all/all ?
                   pch+= lengof(achALLsALL);
                 else
                 if (ISALL(pch, achALL))                // ALL:all ?
                   pch+= lengof(achALL);
                 else
                   return NULL;                         // ALL:*

                 return pch;

       case '/': // All/
                 pch++;
                 pnetAddr->net = pnetAddr->node = pnetAddr->point = -1;

                 if (ISALL(pch, achALLpALL))            // ALL/all.all ?
                   pch+= lengof(achALLpALL);
                 else
                 if (ISALL(pch, achALL))                // ALL/all ?
                   pch+= lengof(achALL);
                 else
                   return NULL;                         // ALL:*

                 if (!*pch || isspace(*pch)) goto Done;
                 return NULL;

       default : // All?
                 pnetAddr->zone = pnetAddr->net = -1;
                 pnetAddr->node = pnetAddr->point = -1;

                 if (!*pch || isspace(*pch)) return pch;
                 return NULL;
     }
   }

   // Scan in the zone if any

   if (DoIsAddrChar(*pch) && DoHasAddrChar(pch, ':')) {
     while (isdigit(*pchEnd)) pchEnd++;
     if (*pchEnd != ':' || pch == pchEnd) return NULL;
     pnetAddr->zone = atoi(pch);
     pch = ++pchEnd;

     // Check if <zone>:All... ?

     if (!ISALL(pch, achALL)) {
       if (!isdigit(*pch) || !DoHasAddrChar(pch, '/')) return NULL;
     } else {
       pnetAddr->net = -1; pnetAddr->node = pnetAddr->point = -1;

       if (ISALL(pch, achALLsALLpALL))  // <zone>:All/All.All ?
         pch+= lengof(achALLsALLpALL);
       else
       if (ISALL(pch, achALLsALL))      // <zone>:All/All ?
         pch+= lengof(achALLsALL);
       else
         pch+= lengof(achALL);          // <zone>:All

       if (!*pch || isspace(*pch)) goto Done;
       return NULL;
     }
   }

   // Scan in the net if any

   if (DoIsAddrChar(*pch) && DoHasAddrChar(pch, '/')) {
     while (isdigit(*pchEnd)) pchEnd++;
     if (*pchEnd != '/' || pch == pchEnd) return NULL;
     pnetAddr->net = atoi(pch);
     pch = ++pchEnd;

     // Check if <net>/All... ?

     if (!ISALL(pch, achALL)) {
       if (!isdigit(*pch)) return NULL;
     } else {
       pnetAddr->node = pnetAddr->point = -1;

       if (ISALL(pch, achALLpALL))      // <net>/All.All ?
         pch+= lengof(achALLpALL);
       else
         pch+= lengof(achALL);          // <net>/All

       if (!*pch || isspace(*pch)) goto Done;
       return NULL;
     }
   }

   // Scan in the node if any

   if (DoIsAddrChar(*pch) && *pch != '.') {
     while (isdigit(*pchEnd)) pchEnd++;
     if (*pchEnd != '.' && !isspace(*pchEnd) && *pchEnd) return NULL;
     pnetAddr->node = atoi(pch);
     pch = pchEnd;
   }

   // Scan in the point if any

   if (*pch != '.') {
     if (!isspace(*pch) && *pch) return NULL;
     pnetAddr->point = 0;
   } else
   if (!ISALL(pch + 1, achALL)) {
     for (pchEnd = ++pch; isdigit(*pchEnd); pchEnd++);
     if (!isspace(*pchEnd) && *pchEnd) return NULL;
     pnetAddr->point = atoi(pch);
     pch = pchEnd;
   } else {
     pch+= 1 + lengof(achALL);
     if (!*pch || isspace(*pch)) goto Done;
     return NULL;
   }

   // Check if zone or net is zero and return

Done: return (pnetAddr->zone && pnetAddr->net) ? pch : NULL;
 }

/*
 * This routine formats network address string
 */

 PSZ APPENTRY FormatNetAddr(NETADDR * pnetAddr)
 {
   static USHORT iAddr;
   static struct { CHAR achAddr[32]; } aAddr[4];
   USHORT cch = 0;
   PSZ psz;

   ASSERT(pnetAddr != NULL);

   // Locate the next slot to return formatted network address in

   if (iAddr >= numbof(aAddr)) iAddr = 0;
   psz = aAddr[iAddr++].achAddr;

   // Check if the zone is not zero and format it

   if (pnetAddr->zone)
     cch+= sprintf(psz + cch, "%u:", pnetAddr->zone);

   // Format the net and node numbers

   cch+= sprintf(psz + cch, "%u/%u", pnetAddr->net, pnetAddr->node);

   // Check if the point is not zero and format it

   if (pnetAddr->point)
     cch+= sprintf(psz + cch, ".%u", pnetAddr->point);

   return psz;
 }

/*
 * This routine compares two addresses
 */

 SHORT APPENTRY CompNetAddr(NETADDR * pnetAddr1, NETADDR * pnetAddr2)
 {
   ASSERT(pnetAddr1 != NULL);
   ASSERT(pnetAddr2 != NULL);

   if (pnetAddr1->zone  < pnetAddr2->zone ) return -1;
   if (pnetAddr1->zone  > pnetAddr2->zone ) return +1;
   if (pnetAddr1->net   < pnetAddr2->net  ) return -2;
   if (pnetAddr1->net   > pnetAddr2->net  ) return +2;
   if (pnetAddr1->node  < pnetAddr2->node ) return -3;
   if (pnetAddr1->node  > pnetAddr2->node ) return +3;
   if (pnetAddr1->point < pnetAddr2->point) return -4;
   if (pnetAddr1->point > pnetAddr2->point) return +4;

   return 0;
 }

/*
 * This routine creates a sequential minimized netaddr list
 */

 PSZ APPENTRY MakeNetAddrList(NETADDR * pnetAddr, NETADDR * pnetAddrPrev)
 {
   static CHAR achText[32];
   SHORT fComp;

   ASSERT(pnetAddr != NULL);
   ASSERT(pnetAddrPrev != NULL);

   // Check if we have to normalize subsequent addresses

   if (cfg.fl & FL_USEFULLNETADDR)
     fComp = 1;
   else
     fComp = CompNetAddr(pnetAddr, pnetAddrPrev);

   // Store zone, net and node -- whatever is needed

   switch (mod(fComp)) {

     case 1:    // different zone -- zone:net/node
          sprintf(achText, " %u:%u/%u", pnetAddr->zone, pnetAddr->net, pnetAddr->node);
          break;

     case 2:    // different net  -- net/node
          sprintf(achText, " %u/%u", pnetAddr->net, pnetAddr->node);
          break;

     case 3:    // different node -- node
          sprintf(achText, " %u", pnetAddr->node);
          break;

     case 4:    // different point -- .point
          sprintf(achText, " .%u", pnetAddr->point);
          break;

     default: xstrcpy(achText, " ");
   }

   // Append the point if it's not zero

   if (pnetAddr->point && mod(fComp) != 4)
     sprintf(xstrchr(achText, 0), ".%u", pnetAddr->point);

   // Store the current address for the next address compare

   xmemcpy(pnetAddrPrev, pnetAddr, sizeof(NETADDR));

   return achText;
 }

/*
 * This routine checks if net address is in the specified list
 */

 BOOL APPENTRY IsAddrInList(NETADDR anetAddr[], USHORT cnetAddr,
                            NETADDR * pnetAddr)
 {
   USHORT iAddr;

   ASSERT(anetAddr != NULL);
   ASSERT(pnetAddr != NULL);

   // Scan the specified address list looking for a match

   for (iAddr = 0; iAddr < cnetAddr && anetAddr[iAddr].zone; iAddr++)
     if (!xmemcmp(&anetAddr[iAddr], pnetAddr, sizeof(NETADDR)))
       return TRUE;

   return FALSE;
 }

/*
 * This routine checks if given address has address masks
 */

 BOOL APPENTRY IsAddrMask(NETADDR * pnetAddr)
 {
   ASSERT(pnetAddr != NULL);

   if (pnetAddr->zone  == (USHORT)-1) return TRUE;
   if (pnetAddr->net   == (USHORT)-1) return TRUE;
   if (pnetAddr->node  == (USHORT)-1) return TRUE;
   if (pnetAddr->point == (USHORT)-1) return TRUE;

   return FALSE;
 }

/*
 * This routine checks if given address is one of my akas
 */

 BOOL APPENTRY IsMyAka(NETADDR * pnetAddr)
 {
   ASSERT(pnetAddr != NULL);

   return IsAddrInList(cfg.anetAddr, numbof(cfg.anetAddr), pnetAddr);
 }

/*
 * This routine returns the aka which matches the given address zone
 */

 NETADDR * APPENTRY GetAddrMatch(NETADDR * pnetAddr)
 {
   SHORT iAddr, iAddrMax, iCmp, iCmpMax;

   ASSERT(pnetAddr != NULL);

   // Scan the aka address list looking for the closest aka

   for (iAddr = 0, iAddrMax = 0, iCmpMax = 0;
        iAddr < numbof(cfg.anetAddr) && cfg.anetAddr[iAddr].zone;
        iAddr++)
     if ((iCmp = CompNetAddr(&cfg.anetAddr[iAddr], pnetAddr)) == 0) {
       iAddrMax = iAddr;
       break;
     } else
     if (mod(iCmp) > iCmpMax) {
       iCmpMax = mod(iCmp);
       iAddrMax = iAddr;
     }

#ifdef DEBUG
//fprintf(STDAUX, "GetAddrMatch: use %s for %s\r\n", FormatNetAddr(&cfg.anetAddr[iAddrMax]), FormatNetAddr(pnetAddr));
#endif

   // Return the address with max match

   return &cfg.anetAddr[iAddrMax];
 }

/*
 * This routine creates a new area list element pointer given the area tag
 */

 PAREA APPENTRY AddArea(PSZ pszArea, CHAR chGroup)
 {
   SHORT cch = xstrlen(pszArea);
   PAREA parea = cfg.pareaFirst;
   SHORT iArea = 0;

   ASSERT(pszArea != NULL);

   // Check if we have to sort the area list by area group then by tag

   if ((cfg.fl & FL_SORTAREAGROUP) && (cfg.fl & FL_SORTAREATAG)) {
     while (parea != NULL && parea->chGroup < chGroup) {
       parea = parea->pareaNext; iArea++; }
     while (parea != NULL && parea->chGroup == chGroup &&
            xstricmp(parea->achTag, pszArea) < 0) {
       parea = parea->pareaNext; iArea++; }
   } else

   // Check if we have to sort the area list by area group only

   if (cfg.fl & FL_SORTAREAGROUP) {
     while (parea != NULL && parea->chGroup <= chGroup) {
       parea = parea->pareaNext; iArea++; }
   } else

   // Check if we have to sort the area list by area tag only

   if (cfg.fl & FL_SORTAREATAG) {
     while (parea != NULL && xstricmp(parea->achTag, pszArea) < 0) {
       parea = parea->pareaNext; iArea++; }
   } else
     iArea = LST_END;

   // Create the new area list element and initialize it with the
   // new echo area tag and the default parameters

   if ((parea = (PAREA) LstCreateElement(sizeof(AREA) + cch)) == NULL) {
     WriteLog("! Insufficient memory (arealist)\n");
     exit(EXIT_FAILURE);
   } else {
     LstLinkElement((PPLE) &cfg.pareaFirst, (PLE) parea, iArea);
     xmemcpy(parea->achTag, pszArea, cch);
     parea->hash = CalcHash(parea->achTag);
     parea->pszDescr = cfg.pszDefAreaDescr;
     parea->fs = cfg.fsDefAreaFlags;
     parea->level = cfg.usDefAreaLevel;
     parea->chGroup = chGroup;
   }

   return parea;
 }

/*
 * This routine deletes the given area from the list
 */

 PDELAREA APPENTRY DelArea(PAREA parea)
 {
   PDELAREA pdelarea;
   USHORT iArea;

   ASSERT(parea != NULL);

   // Get the specified area descriptor index in the existing area list

   if ((iArea = LstIndexFromElement((PLE) cfg.pareaFirst, (PLE) parea)) == LST_ERROR)
     return NULL;

   // Create the new deleted area list element and check if ok

   if ((pdelarea = (PDELAREA) LstCreateElement(sizeof(DELAREA))) == NULL)
     return NULL;

   // Unlink specified area from the existing areas list and link it into the
   // deleted areas list with all of its data as it is now

   LstUnlinkElement((PPLE) &cfg.pareaFirst, iArea);
   LstLinkElement((PPLE) &cfg.pdelareaFirst, (PLE) pdelarea, LST_END);
   pdelarea->parea = parea;

   // Force the config files update

   cfg.fl|= FL_REWRITECFG;

   return pdelarea;
 }

/*
 * This routine deletes area for a SqaFix server
 */

 BOOL APPENTRY AutoDelArea(PAREA parea)
 {
   PDELAREA pdelarea;
   BOOL fSquishArea;

   ASSERT(parea != NULL);

   // Destroy the area and report

   if ((pdelarea = DelArea(parea)) == NULL)
     return FALSE;
   else
     pdelarea->pnode = NULL;

   // Destroy the area message base if necessary... woops!

   if (!(cfg.fl & FL_PRESERVEDESTRAREAS) && !(cfg.fl & FL_TESTMODE))
     DelAreaMsgBase(parea);

   // Log the area deletion

   fSquishArea = IsSquishArea(parea->pszSqshFlags);
   WriteLog("* Area %s destroyed in %s%s\n",
             parea->achTag,
             parea->pszPath, fSquishArea ?  ".SQ?" : "\\*.MSG");

   if (parea->pszDescr != NULL && parea->pszDescr[0])
     WriteLog("* Desc \"%s\"\n", parea->pszDescr);

   return TRUE;
 }

/*
 * This routine deletes the area message base files and directories
 */

 BOOL APPENTRY DelAreaMsgBase(PAREA parea)
 {
   BOOL fSqshBase = IsSquishArea(parea->pszSqshFlags);
   CHAR achSaveDir[MAXPATH], achPath[MAXPATH], achMask[MAXFILE+MAXEXT];
   SHORT iSaveDisk;
   PCH pchLastSlash;

#if defined __OS2__
   FILEFINDBUF ffblk;
   HDIR hdir = HDIR_CREATE;
   USHORT cSearch = 1;
#elif defined UNIX
//
#elif defined __W32__
   struct _finddata_t ffblk;
   long hfind;
#else
#ifdef __TURBOC__
   struct ffblk ffblk;
#else
   struct find_t ffblk;
#endif
#endif

   ASSERT(parea != NULL);

   // Copy the path specification and locate the last backslash

   xstrcpy(achPath, parea->pszPath);
   pchLastSlash = xstrrchr(achPath, '\\');

   // Make up the file search mask and the path name

   if (!fSqshBase) {
     xstrcpy(achMask, "*.*");
   } else {
     if (pchLastSlash != NULL) {
       xstrcpy(achMask, pchLastSlash + 1); *pchLastSlash = '\0';
     } else {
       xstrcpy(achMask, achPath); achPath[0] = '\0';
     }
     xstrcat(achMask, ".???");
   }

   // Preserve the current disk and switch to the requested disk
   // if specified

#ifndef UNIX
   iSaveDisk = getdisk();
   if (achPath[1] == ':') {
     setdisk(toupper(achPath[0]) - 'A');
     if (getdisk() != toupper(achPath[0]) - 'A') {
       setdisk(iSaveDisk);
       return FALSE;
     }
   }
#endif
   // Preserve the current directory on the requested disk and check if ok

#ifndef UNIX
   if (getcwd(achSaveDir, sizeof(achSaveDir)) == NULL) {
     setdisk(iSaveDisk);
     return FALSE;
   }

   // Switch to the requested directory and check if ok

   if (achPath[0] && chdir(achPath) == -1) {
     setdisk(iSaveDisk);
     return FALSE;
   }

#endif
   // Log message base deletion

   WriteLog("- Kill files matching %s\\%s\n", achPath, achMask);

   // Delete all files matching mask

#if defined __OS2__ 
   if (!DosFindFirst(achMask, &hdir, FILE_NORMAL, &ffblk, sizeof(ffblk), &cSearch, 0)) {
     do {
       DosDelete(ffblk.achName, 0);
     } while (!DosFindNext(hdir, &ffblk, sizeof(ffblk), &cSearch));
     DosFindClose(hdir);
   }
#elif defined __W32__
   if ((hfind = _findfirst(achMask, &ffblk)) != -1) {
     do {
       unlink(ffblk.name);
     } while (!_findnext(hfind, &ffblk));
     _findclose(hfind);
   }
#elif defined UNIX
   // FIXME: How to fix this?
#else
#ifdef __TURBOC__
   if (!findfirst(achMask, &ffblk, 0)) {
     do {
       unlink(ffblk.ff_name);
     } while (!findnext(&ffblk));
   }
#else
   if (!_dos_findfirst(achMask, 0, &ffblk))
     do {
       unlink(ffblk.name);
     } while (!_dos_findnext(&ffblk));
#endif
#endif

   // Restore the current directory on the requested drive and
   // the current drive

   chdir(achSaveDir); 
#ifndef UNIX
   setdisk(iSaveDisk);
#endif

   // Additional hassle for the *.msg style bases -- kill the directory

   if (!fSqshBase && rmdir(achPath) != -1)
     WriteLog("- Kill msgs directory %s\n", achPath);

   return TRUE;
 }

/*
 * This routine returns alternate area list element pointer given the area tag
 */

 PAREA APPENTRY GetAreaFromTagAlt(PAREA pareaFirstAlt, PSZ pszArea)
 {
   PAREA parea;
   ULONG hash;

   if (pszArea != NULL) {
     hash = CalcHash(pszArea);
     for (parea = pareaFirstAlt; parea != NULL; parea = parea->pareaNext)
       if (parea->hash == hash && !xstricmp(parea->achTag, pszArea))
         return parea;
   }

   return NULL;
 }

/*
 * This routine returns area list element pointer given the area tag
 */

 PAREA APPENTRY GetAreaFromTag(PSZ pszArea)
 {
   return GetAreaFromTagAlt(cfg.pareaFirst, pszArea);
 }

/*
 * This routine returns deleted area list element pointer given the area tag
 */

 PDELAREA APPENTRY GetDelAreaFromTag(PSZ pszArea)
 {
   PDELAREA pdelarea;

   if (pszArea != NULL)
     for (pdelarea = cfg.pdelareaFirst; pdelarea != NULL;
          pdelarea = pdelarea->pdelareaNext)
       if (!xstricmp(pdelarea->parea->achTag, pszArea))
         return pdelarea;

   return NULL;
 }

/*
 * This routine returns area list element pointer given the base file name
 */

 PAREA APPENTRY GetAreaFromPath(PSZ pszPath)
 {
   CHAR achPath[MAXPATH], achPath1[MAXPATH];
   PAREA parea;

   ASSERT(pszPath != NULL);

   // Make up the full path and check if ok

   if (!BuildFullPath(achPath, pszPath)) {
     WriteLog("! Can't make full path: %s\n", pszPath);
     return NULL;
   }

   ASSERT(achPath[1] == ':');

   // Loop through all the existing areas looking for the full path match

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
     if (parea->pszPath == NULL)
       continue;

     // Make up the full path name and check if ok

     if (!BuildFullPath(achPath1, parea->pszPath)) {
       WriteLog("! Can't make full path: %s (area %s)\n", parea->pszPath, parea->achTag);
       continue;
     }

     ASSERT(achPath1[1] == ':');

#ifdef DEBUG
//fprintf(STDAUX, "GetAreaFromPath: %s %s\r\n", achPath, achPath1);
#endif

     // Check if the full path's match

     if (!xstricmp(achPath, achPath1))
       return parea;
   }

   return NULL;
 }

/*
 * This routine returns the echo area posting address as it's known to Squish
 */

 BOOL APPENTRY GetAreaOrigAddr(PAREA parea, NETADDR * pnetAddr)
 {
   PCH pch;

   ASSERT(parea != NULL);
   ASSERT(pnetAddr != NULL);

   // Check if there are any flags specified for this echo area

   if ((pch = parea->pszSqshFlags) != NULL)

     // Skip over the leading spaces and check for the flag prefix

     while (*pch && SkipSpaces(&pch) == '-')

       // Check for the alternate address flag and evaluate it

       if (pch[1] == 'p' || pch[1] == 'P') {
         *pnetAddr = cfg.anetAddr[0];
         if (ScanNetAddr(pnetAddr, pch + 2)) {
           return TRUE;
         } else {
           WriteLog("! Can't evaluate node address in -p<node> flag, area: %s\n",
                     parea->achTag);
           break;
         }
       } else {

         // Advance to the next flag or space

         for (pch++; *pch && !isspace(*pch) && *pch != '-'; pch++);
       }

   // No alternate origination address found so return the primary one

   xmemcpy(pnetAddr, &cfg.anetAddr[0], sizeof(NETADDR));

   return FALSE;
 }

/*
 * This routine adds node to the linked node list for the area
 */

 BOOL APPENTRY AddAreaLink(PAREA parea, NETADDR * pnetAddr, BOOL fActive,
                           PSZ pszLog)
 {
   SHORT ilink, fComp;
   PLINK plink;

   ASSERT(parea != NULL);
   ASSERT(pnetAddr != NULL);

   // Scan the list looking for the place for the new link so that their
   // node addresses will go in acending order. Also check if the link
   // is already there, return error in this case

   for (plink = parea->plink, ilink = 0; plink != NULL;
        plink = plink->plinkNext, ilink++) {
     fComp = CompNetAddr(pnetAddr, &plink->netAddr);
     if (fComp == 0) return FALSE;
     if (fComp < 0) break;
   }

   // Create the new linked nodes list element

   if ((plink = (PLINK) LstCreateElement(sizeof(LINK))) == NULL) {
     WriteLog("! Insufficient memory (arealink)\n");
     exit(EXIT_FAILURE);
   } else {
     LstLinkElement((PPLE)&parea->plink, (PLE) plink, ilink);
     xmemcpy(&plink->netAddr, pnetAddr, sizeof(NETADDR));
     if (fActive) plink->fs|= LF_ACTIVE;
   }

   // Check if we need to log area addition

   if (pszLog != NULL)
     WriteLog("- Link %s to %s list for %s %s\n",
               FormatNetAddr(pnetAddr),
               fActive ? "active" : "passive",
               parea->achTag, pszLog);

   // Show we've changed the list

   parea->fs|= fActive ? AF_ACTIVECHANGED : AF_PASSIVECHANGED;

   return TRUE;
 }

/*
 * This routine removes node from the linked node list for the area
 */

 BOOL APPENTRY DelAreaLink(PAREA parea, NETADDR * pnetAddr)
 {
   PLINK plink;
   SHORT ilink;

   ASSERT(parea != NULL);
   ASSERT(pnetAddr != NULL);

   // Scan through the list looking for a match

   for (plink = parea->plink, ilink = 0;
        plink != NULL && xmemcmp(&plink->netAddr, pnetAddr, sizeof(NETADDR));
        plink = plink->plinkNext, ilink++);

   // Check if the specified link is in the list

   if (plink == NULL) return FALSE;

   // Show we've changed the list

   parea->fs|= (plink->fs & LF_ACTIVE) ? AF_ACTIVECHANGED : AF_PASSIVECHANGED;

   // Remove link list element and destroy it

   LstUnlinkElement((PPLE)&parea->plink, ilink);
   LstDestroyElement((PLE) plink);

   return TRUE;
 }

/*
 * This routine returns link list element pointer for a given node and area
 */

 PLINK APPENTRY GetAreaLink(PAREA parea, NETADDR * pnetAddr)
 {
   PLINK plink;

   ASSERT(parea != NULL);
   ASSERT(pnetAddr != NULL);

   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext)
     if (!xmemcmp(&plink->netAddr, pnetAddr, sizeof(NETADDR)))
       return plink;

   return NULL;
 }

/*
 * This routine sets link passive/active state for the given area
 */

 BOOL APPENTRY SetAreaLink(PAREA parea, PLINK plink, BOOL fActive)
 {
   ASSERT(parea != NULL);
   ASSERT(plink != NULL);

   // Check if link state is not changing and leave, otherwise
   // set it to the requested state

   if (fActive) {
     if (plink->fs & LF_ACTIVE) return FALSE;
     plink->fs|= LF_ACTIVE;
   } else {
     if (!(plink->fs & LF_ACTIVE)) return FALSE;
     plink->fs&=~LF_ACTIVE;
   }

   // Show we've changed the list

   parea->fs|= AF_ACTIVECHANGED | AF_PASSIVECHANGED;

   return TRUE;
 }

/*
 * This routine adds area to the specified done list
 */

 VOID APPENTRY AddToDone(PAREA parea, BOOL fWhat)
 {
   PDONE pdone = cfg.pdoneFirst;

   ASSERT(parea != NULL);

   // Check if the specified area is already in the list

   while (pdone != NULL && pdone->parea != parea)
     pdone = pdone->pdoneNext;

   // Create the list element and link it up

   if (pdone == NULL)
     if ((pdone = (PDONE) LstCreateElement(sizeof(DONE))) == NULL) {
       WriteLog("! Insufficient memory (done list)\n");
       exit(EXIT_FAILURE);
     } else {
       LstLinkElement((PPLE) &cfg.pdoneFirst, (PLE) pdone, LST_END);
       pdone->parea = parea;
     }

   pdone->fWhat = fWhat;
 }

/*
 * This routine appends the extension to the file name
 */

 VOID APPENTRY AppendFileExt(PSZ pszDst, PSZ pszSrc, PSZ pszExt, BOOL fAlways)
 {
   CHAR  achDrive[MAXDRIVE];
   CHAR  achDir  [MAXDIR];
   CHAR  achFile [MAXFILE];
   CHAR  achExt  [MAXEXT];

   // Decompose file name then assemble it back with another extension

   fnsplit(pszSrc, achDrive, achDir, achFile, achExt);
   if (achExt[0] == '\0' || fAlways) xstrcpy(achExt, pszExt);
   fnmerge(pszDst, achDrive, achDir, achFile, achExt);
   xstrupr(pszDst);
 }

/*
 * This routines build a complete path name for the specified file
 */

 static BOOL SUBENTRY DoGetCurDir(PSZ pszCurDir)
 {
   CHAR achCurPath[MAXDRIVE + MAXDIR];

   ASSERT(pszCurDir != NULL);

   if (getcwd(achCurPath, sizeof(achCurPath)) != NULL)
     xstrcpy(pszCurDir, achCurPath + 2);
   else
     return (BOOL)-1;

   return 0;
 }

 BOOL APPENTRY BuildFullPath(PSZ pszDest, PSZ pszSrc)
 {
   CHAR achDrive[MAXDRIVE];
   CHAR achDir[MAXDIR];
   CHAR achFile[MAXFILE];
   CHAR achExt[MAXEXT];
   CHAR achCurDir[MAXDIR];
   USHORT iCurDrive;

   ASSERT(pszDest != NULL);
   ASSERT(pszSrc != NULL);

   // Decompose the supplied path

   fnsplit(pszSrc, achDrive, achDir, achFile, achExt);
#ifndef UNIX

   // Preserve current drive

   iCurDrive = getdisk();

   // Check if drive specified in the supplied path and if not,
   // assume the current one

   if (!achDrive[0]) {
     xstrcpy(achDrive, "A:");
     achDrive[0]+= (CHAR) iCurDrive;
   }

   // Set the current drive to the requested one and check if ok.
   // If failed, restore orginal drive and return error

   setdisk(toupper(achDrive[0]) - 'A');
   if (getdisk() != toupper(achDrive[0]) - 'A') {
     setdisk(iCurDrive);
     return FALSE;
   }

   // Preserve the current directory on the requested drive and
   // check if ok, otherwise restore initial current drive and return


   if (DoGetCurDir(achCurDir)) {
     setdisk(iCurDrive);
     return FALSE;
   }

   // Check if directory specified and make it current

   if (achDir[0]) {

     // Kill trailing back slash if it's not the only character
     // of the directory specification

     if ((achDir[xstrlen(achDir) - 1] == '\\') && (xstrlen(achDir) > 1 ))
       achDir[xstrlen(achDir) - 1] = '\0';

     // Change to the specified directory and check if ok. If failed,
     // restore the inital directory on the requested drive and change
     // to the initial drive

     if (chdir(achDir)) {
       chdir(achCurDir);
       setdisk(iCurDrive);
       return FALSE;
     }
   }

   // So we managed to make a requested directory current on the
   // requested drive. Now get it's full specification and this
   // will be what we're after. If failed, just restore things back

// xstrcpy(achDir, "\\");
// if (getcurdir(0, &achDir[1])) {

   if (DoGetCurDir(achDir)) {
     chdir(achCurDir);
     setdisk(iCurDrive);
     return FALSE;
   }
#endif

   // Compose the fully qualified file name and restore
   // the inital directory on the requested drive and change
   // to the initial drive

   fnmerge(pszDest, achDrive, achDir, achFile, achExt);
#ifndef UNIX
   chdir(achCurDir);
   setdisk(iCurDrive);
#endif

   return TRUE;
 }

/*
 * This routine returns number of characters in the formatted text
 */

 USHORT GetTextLeng(va_list argptr, PSZ pszFormat, ...)
 {
   static FILE * pfile;
   SHORT cch;

   if (!argptr) va_start(argptr, pszFormat);

   // Check if we have a NUL file opened and open it if no

   if (pfile == NULL) pfile = fopen("NUL", "wt");

   // Format and write text to the NUL device getting the count

   cch = vfprintf(pfile, pszFormat, argptr);

   va_end(argptr);

   return cch == EOF ? 0 : cch;
 }

/*
 * This routine writes out the line of the specified length and character
 */

 VOID APPENTRY WriteMsgLine(SHORT cch, CHAR ch, BOOL fNewLine)
 {
   CHAR achText[72];
   PCH pch;

   if (cch > 0) {
     if (cch > sizeof(achText) - 2) cch = sizeof(achText) - 2;
     for (pch = achText; cch--; *pch++ = ch);
     if (fNewLine) *pch++ = '\n';
     *pch = '\0';
     WriteMsg("%s", achText);
   }
 }

/*
 * This routine formats text and writes it to the the message
 */

 USHORT WriteMsg(PSZ pszFormat, ...)
 {
   va_list argptr;
   USHORT cch;

   va_start(argptr, pszFormat);

   // Check if we're in verbose mode and display the message on the
   // console, otherwise just calculate the formatted text length

   if (cfg.fl & FL_DISPLAYGENERATEDMSG)
     cch = vprintf(pszFormat, argptr);
   else
     cch = GetTextLeng(argptr, pszFormat);

   // Check if we're writing a netmail message and this line is
   // going to exhaust the message buffer, then flush buffer and
   // write the line.  Otherwise just add the line to the buffer

   if (cfg.fl & FL_REPORTMSG) {
     if (ichBuf + cch >= cfg.cchMaxMsgPart) {
       SendLongMsgPut(pchBuf);
       ichBuf = 0;
     }

     // Check if the suggested chunk is still too long

     if (ichBuf + cch < cchBuf)
       ichBuf+= vsprintf(pchBuf + ichBuf, pszFormat, argptr);
     else
       ichBuf+= sprintf(pchBuf + ichBuf, "\n*** TEXT IS TOO LONG TO SEND ***\n");
   }

   va_end(argptr);

   return cch;
 }

/*
 * This routine adds area description list element
 */

 PAREADESC APPENTRY AddAreaDescr(PSZ pszArea, PSZ pszDescr)
 {
   static PAREADESC pareadescLast = NULL;
   USHORT cch = xstrlen(pszArea);
   PAREADESC pareadesc;
   PSZ psz;

   // Validate area tag and description

   if (!pszArea || !pszArea[0] || !pszDescr || !pszDescr[0])
     return NULL;

   // Allocate the area description and check if ok

   if ((pszDescr = AllocString(pszDescr, -1)) == NULL) {
     WriteLog("! Insufficient memory (areadesc)\n");
     exit(EXIT_FAILURE);
   }

   // Create the new area description list element and initialize it

   if ((pareadesc = (PAREADESC) LstCreateElement(sizeof(AREADESC) + cch)) == NULL) {
     WriteLog("! Insufficient memory (areadesclist)\n");
     exit(EXIT_FAILURE);
   } else {
     xmemcpy(pareadesc->achTag, pszArea, cch); xstrupr(pareadesc->achTag);
     pareadesc->hash = CalcHash(pareadesc->achTag);
     pareadesc->pszDescr = pszDescr;
   }

   // Fixup double quotas so that they'll not screw up config

   for (psz = pszDescr; (psz = xstrchr(psz, '"')) != NULL; psz++)
     *psz = '\'';

   // Add the area description list element and check if ok. Note that
   // in order to speed up large lists processing we're caching the last
   // added element and link up the next one manually.

   if (pareadescLast == NULL) {
     LstLinkElement((PPLE) &cfg.pareadescFirst, (PLE) pareadesc, LST_END);
     pareadescLast = pareadesc;
   } else {
     pareadescLast->pareadescNext = pareadesc;
     pareadesc->pareadescPrev = pareadescLast;
     pareadescLast = pareadesc;
   }

#ifdef DEBUG
//fprintf(STDAUX, "AddAreaDescr: %s\t\"%s\"\n\r", pareadesc->achTag, pareadesc->pszDescr);
#endif

   return pareadesc;
 }

/*
 * This routine returns area description list element given area tag
 */

 PSZ APPENTRY GetAreaDescrFromTag(PSZ pszArea)
 {
   ULONG hash;
   PAREADESC pareadesc;

#ifdef DEBUG
//fprintf(STDAUX, "GetAreaDescrFromTag: '%s'\r\n", pszArea);
#endif

   ASSERT(pszArea != NULL);

   // Calulate area tag hash

   hash = CalcHash(pszArea);

   // Scan through all the area description list elements looking
   // for the area tag match

   for (pareadesc = cfg.pareadescFirst; pareadesc != NULL;
        pareadesc = pareadesc->pareadescNext)
     if (pareadesc->hash == hash && !xstricmp(pareadesc->achTag, pszArea))
       return pareadesc->pszDescr;

   // Scan through all the area description list elements looking
   // for the area tag mask match

   for (pareadesc = cfg.pareadescFirst; pareadesc != NULL;
        pareadesc = pareadesc->pareadescNext)
     if (GrepSearch(pszArea, pareadesc->achTag, FALSE))
       return pareadesc->pszDescr;

   return NULL;
 }

/*
 * This routine scans area descriptions file in NA format
 */

 BOOL APPENTRY LoadAreaDescrFileNA(PSZ pszFile)
 {
   CHAR achLine[1024];
   ULONG iLine = 0;
   PCH pch, pchEnd;
   FILE * pfile;

   // Verify file name specification

   if (!pszFile || !pszFile[0]) return FALSE;

#ifdef DEBUG
//fprintf(STDAUX, "LoadAreaDescrFileNA: %s\n\r", pszFile);
#endif

   // Open the area description file

   if ((pfile = fopen(pszFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Scanning file: %s\n", pszFile);

   // Scan in the area descriptions file

   achLine[lengof(achLine)] = '\0';

   loop {

     iLine++;

     // Read the subsequent line from the file and check if ok

     if (fgets(achLine, lengof(achLine), pfile) == NULL)
       if (!feof(pfile)) {
         WriteLog("$ Can't read file: %s\n", pszFile);
         exit(EXIT_FAILURE);
       } else
         break;

     // Check if we got the complete line and remove the trailing \n

     if (*(pch = xstrchr(achLine, 0) - 1) == '\n')
       *pch = '\0';
     else
       if (!feof(pfile)) {
         WriteLog("! Line %lu is too long in file '%s'\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }

     // Remove all the trailing spaces if any

     for (--pch; pch >= achLine && isspace(*pch); --pch) *pch = '\0';

     // Skip over leading spaces

     for (pch = achLine; isspace(*pch); pch++);

     // Check if this is an empty line or a comment line

     if (*pch == '\0' || *pch == ';')
       continue;

     // Fix up the end of the area tag specification

     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     while (isspace(*pchEnd)) *pchEnd++ = '\0';

     // Add the area description list element

     AddAreaDescr(pch, pchEnd);
   }

   fclose(pfile);

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszFile);

   return TRUE;
 }

/*
 * This routine saves area descriptions file in NA format
 */

 BOOL APPENTRY SaveAreaDescrFileNA(PSZ pszFile)
 {
   PAREADESC pareadesc;
   CHAR achLine[1024];
   FILE * pfile;

   // Verify file name specification

   if (!pszFile || !pszFile[0]) return FALSE;

#ifdef DEBUG
//fprintf(STDAUX, "SaveAreaDescrFileNA: %s\n\r", pszFile);
#endif

   // Open the area description file

   if ((pfile = fopen(pszFile, "wt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Creating file: %s\n", pszFile);

   // Scan through all the area description list elements looking
   // for the area tag match

   for (pareadesc = cfg.pareadescFirst; pareadesc != NULL;
        pareadesc = pareadesc->pareadescNext) {

     // Format the area description line

     if (pareadesc->pszDescr && pareadesc->pszDescr[0])
       sprintf(achLine, "%.128s\t%.128s\n", pareadesc->achTag, pareadesc->pszDescr);
     else
       sprintf(achLine, "%.128s\n", pareadesc->achTag);

     // Write the subsequent line to the file and check if ok

     if (fputs(achLine, pfile) == EOF) {
       WriteLog("$ Can't write file: %s\n", pszFile);
       exit(EXIT_FAILURE);
     }
   }

   fclose(pfile);

   return TRUE;
 }

/*
 * This routine scans area descriptions file in DZ format
 */

 BOOL APPENTRY LoadAreaDescrFileDZ(PSZ pszFile)
 {
   PSZ pszArea, pszDescr;
   CHAR achLine[1024];
   ULONG iLine = 0;
   FILE * pfile;
   PCH pch;

   // Verify file name specification

   if (!pszFile || !pszFile[0]) return FALSE;

#ifdef DEBUG
//fprintf(STDAUX, "LoadAreaDescrFileDZ: %s\n\r", pszFile);
#endif

   // Open the area description file

   if ((pfile = fopen(pszFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Scanning file: %s\n", pszFile);

   // Scan in the area descriptions file

   achLine[lengof(achLine)] = '\0';

   loop {

     iLine++;

     // Read the subsequent line from the file and check if ok

     if (fgets(achLine, lengof(achLine), pfile) == NULL)
       if (!feof(pfile)) {
         WriteLog("$ Can't read file: %s\n", pszFile);
         exit(EXIT_FAILURE);
       } else
         break;

     // Check if we got the complete line and remove the trailing newline

     if (*(pch = xstrchr(achLine, 0) - 1) == '\n')
       *pch = '\0';
     else
       if (!feof(pfile)) {
         WriteLog("! Line %lu is too long in file '%s'\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }

     // Skip over leading spaces

     for (pch = achLine; isspace(*pch); pch++);

     // Check if this is an empty line or a comment line

     if (*pch == '\0' || *pch == ';')
       continue;

     // Locate and fixup the area tag and description

     if ((pch = xstrchr(pch, ',')) == NULL) continue;
     pszArea = ++pch;

     if ((pch = xstrchr(pszArea, ',')) == NULL) continue;
     *pch = '\0';
     pszDescr = ++pch;

     if ((pch = xstrchr(pszDescr, ',')) != NULL) *pch = '\0';

     // Add the area description list element

     AddAreaDescr(pszArea, pszDescr);
   }

   fclose(pfile);

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszFile);

   return TRUE;
 }

/*
 * This routine saves area list file in NA format
 */

 BOOL APPENTRY SaveAreaListFile(PSZ pszFile)
 {
   CHAR achLine[1024];
   FILE * pfile;
   PAREA parea;

   // Verify file name specification

   if (!pszFile || !pszFile[0]) return FALSE;

#ifdef DEBUG
//fprintf(STDAUX, "SaveAreaListFile: %s\n\r", pszFile);
#endif

   // Open the area list file

   if ((pfile = fopen(pszFile, "wt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Creating file: %s\n", pszFile);

   // Scan through all the area description list elements looking
   // for the area tag match

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {

     // Format the area list line

     if (parea->pszDescr && parea->pszDescr[0])
       sprintf(achLine, "%.128s\t%.128s\n", parea->achTag, parea->pszDescr);
     else
       sprintf(achLine, "%.128s\n", parea->achTag);

     // Write the subsequent line to the file and check if ok

     if (fputs(achLine, pfile) == EOF) {
       WriteLog("$ Can't write file: %s\n", pszFile);
       exit(EXIT_FAILURE);
     }
   }

   fclose(pfile);

   return TRUE;
 }

/*
 * This routine scans area mask file in NA format
 */

 BOOL APPENTRY LoadAreaMaskFileNA(PSZ pszFile, PLSZ * pplszFirst)
 {
   PLSZ plszNext, plszLast = NULL;
   CHAR achLine[1024];
   ULONG iLine = 0;
   PCH pch, pchEnd;
   FILE * pfile;

   // Verify file name specification

   if (!pszFile || !pszFile[0]) return FALSE;

#ifdef DEBUG
//fprintf(STDAUX, "LoadAreaMaskFileNA: %s\n\r", pszFile);
#endif

   // Open the area mask file

   if ((pfile = fopen(pszFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Scanning file: %s\n", pszFile);

   // Scan in the area mask file

   achLine[lengof(achLine)] = '\0';

   loop {

     iLine++;

     // Read the subsequent line from the file and check if ok

     if (fgets(achLine, lengof(achLine), pfile) == NULL)
       if (!feof(pfile)) {
         WriteLog("$ Can't read file: %s\n", pszFile);
         exit(EXIT_FAILURE);
       } else
         break;

     // Check if we got the complete line and remove the trailing \n

     if (*(pch = xstrchr(achLine, 0) - 1) == '\n')
       *pch = '\0';
     else
       if (!feof(pfile)) {
         WriteLog("! Line %lu is too long in file '%s'\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }

     // Remove all the trailing spaces if any

     for (--pch; pch >= achLine && isspace(*pch); --pch) *pch = '\0';

     // Skip over leading spaces

     for (pch = achLine; isspace(*pch); pch++);

     // Check if this is an empty line or a comment line

     if (*pch == '\0' || *pch == ';')
       continue;

     // Fix up the end of the area tag specification

     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     while (isspace(*pchEnd)) *pchEnd++ = '\0';

     // Convert areatg to uppercase

     xstrupr(pch);

     // Add the area mask list element and check if ok. Note that in order
     // to speed up large lists processing we're caching the last added
     // element and link up the next one manually.

     if (*pplszFirst == NULL || plszLast == NULL) {
       if ((plszLast = AddLsz(pplszFirst, pch, -1, LST_END)) == NULL) {
Fail:    WriteLog("! Insufficient memory for line %lu in file '%s'\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }
     } else {
       if ((plszNext = AddLsz(NULL, pch, -1, 0)) == NULL) goto Fail;
       plszLast->plszNext = plszNext;
       plszNext->plszPrev = plszLast;
       plszLast = plszNext;
     }
   }

   fclose(pfile);

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszFile);

   return TRUE;
 }

  // DoGrepSearch() return codes

#define GSR_NOTFOUND ((BOOL) 0)
#define GSR_FOUND    ((BOOL) 1)
#define GSR_ABORT    ((BOOL)-1)

  // DoGrepSearch() declaratins

#define CH(ch)  (fCaseSrch ? (ch) : toupper(ch))

  // Module local variables

  static BOOL fCaseSrch;        // TRUE if case sensitive grep search

/*
 * This subroutine performs a grep-like pattern match
 */

 static BOOL SUBENTRY DoGrepSearch(PSZ pszT, PSZ pszP)
 {
   BOOL fMatch, fInv;
   USHORT chLo;

   ASSERT(pszT != NULL);
   ASSERT(pszP != NULL);

   for (; *pszP; pszT++, pszP++) {
     if (*pszT == '\0' && *pszP != '*') return GSR_ABORT;
     switch (*pszP) {

       case '\\':// Literally match the following character
                 pszP++;
                 // fall through

       default : // Match exactly
                 if (CH(*pszT) != CH(*pszP)) return GSR_NOTFOUND;
                 continue;

       case '?': // Match any character
                 continue;

       case '*': // Match zero or more characters

                 // Consecutive stars act just like one
                 while (*++pszP == '*') continue;

                 // Trailing star matches everything
                 if (*pszP == '\0') return GSR_FOUND;

                 // Optimized recursive match
                 while (*pszT) {
                   fMatch = DoGrepSearch(pszT++, pszP);
                   if (fMatch != GSR_NOTFOUND) return fMatch;
                 }
                 return GSR_ABORT;

       case '[': // Match [abc0-7] or [^abc0-7]
                 fInv = (pszP[1] == '^') ? GSR_FOUND : GSR_NOTFOUND;
                 if (fInv) pszP++;

                 pszP++;

                 fMatch = (CH(*pszP) == CH(*pszT)) ? GSR_FOUND : GSR_NOTFOUND;

                 for (chLo = *pszP; *++pszP && *pszP != ']'; chLo = *pszP) {
                   if (*pszP == '-' &&
                       pszP[1] != ']' ? *pszT <= *++pszP && *pszT >= chLo
                                      : CH(*pszT) == CH(*pszP))
                     fMatch = GSR_FOUND;
                 }

                 if (fMatch == fInv) return GSR_NOTFOUND;
                 continue;
     }
   }

   if (*pszT == '\0') return GSR_FOUND;

   return GSR_NOTFOUND;
 }

/*
 * This routine searches the grep pattern
 */

 BOOL APPENTRY GrepSearch(PSZ pszText, PSZ pszPattern, BOOL fCase)
 {
   // Check if nothing to search for

   if (!pszText || !pszText[0] || !pszPattern || !pszPattern[0])
     return FALSE;

   // Set the case sensitivity flag

   fCaseSrch = fCase;

   // Call the grep search engine and check if found

   return (DoGrepSearch(pszText, pszPattern) == GSR_FOUND);
 }

/*
 * This routine checks if the given pattern has a wild specification
 */

 BOOL APPENTRY IsWildGrep(PSZ pszPattern)
 {
   PCH pch;

   // Check if nothing to search

   if (!pszPattern || !pszPattern[0])
     return FALSE;

   // Scan looking for the special characters

   for (pch = pszPattern; *pch; pch++)
     if (xstrchr("*?[]", *pch))
       return TRUE;

   return FALSE;
 }

/*
 * This routine converts dttm to amount of secs since 01-01-1970
 */

 static BYTE aDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

 time_t APPENTRY DttmToSecs(PDTTM pdttm)
 {
   SHORT i, days, hours;
   time_t secs;

   ASSERT(pdttm != NULL);

   // Set time zone

   tzset();

   // Convert from 1980 to 1970 base date

   secs = 24L * 60L * 60L * 3652L + timezone;
   i = pdttm->yr - 1980;
   secs+= (i >> 2) * (1461L * 24L * 60L * 60L);
   secs+= (i & 3) * (24L * 60L * 60L * 365L);
   if (i & 3) secs+= 24L * 3600L;
   days = 0;

   // Add in months

   i = pdttm->mo - 1;
   while (i > 0) {
     i--;
     days+= aDays[i];
   }
   days+= pdttm->da - 1;

   // Currently in leap year

   if (pdttm->mo > 2 && (pdttm->yr & 3) == 0) days++;

   // Calc hours, minuts and seconds

   hours = days * 24 + pdttm->hh;
   secs+= hours * 3600L;
   secs+= 60L * pdttm->mm + pdttm->ss;

   return (secs);
 }

/*
 * This routine converts amount of secs since 01-01-1970 to dttm
 */

 VOID APPENTRY SecsToDttm(time_t secs, PDTTM pdttm)
 {
   ASSERT(pdttm != NULL);

   // Set time zone

   tzset();

   // Convert from 1970 to 1980 base date

   secs-= 24L * 60L * 60L * 3652L + timezone;
   pdttm->ss = secs % 60;
   secs/= 60;                             // Time in minutes
   pdttm->mm = secs % 60;
   secs/= 60;                             // Time in hours

   // ...

   pdttm->yr = 1980 + (int)((secs / (1461L * 24L)) << 2);
   secs%= 1461L * 24L;
   if (secs >= 366 * 24) {
     secs-= 366 * 24;
     pdttm->yr++;
     pdttm->yr+= (int)(secs / (365 * 24));
     secs%= 365 * 24;
   }

   // ...

   pdttm->hh = secs % 24;
   secs/= 24;                             // Time in days
   secs++;
   if ((pdttm->yr & 3) == 0) {
     if (secs > 60)
       secs--;
     else
     if (secs == 60) {
       pdttm->mo = 2;
       pdttm->da = 29;
       return;
     }
   }

   // ...

   for (pdttm->mo = 0; aDays[pdttm->mo] < secs; pdttm->mo++)
     secs -= aDays[pdttm->mo];
   pdttm->mo++;
   pdttm->da = (SHORT) secs;
 }

/*
 * This routine converts ascii date/time to dttm, returns no of chars scanned
 */

 USHORT APPENTRY ScanDttm(PCH pch, PDTTM pdttm)
 {
   static CHAR ach[] = "1997-03-16@11:48:00";
   // Ascii date/time:  01234567890123456789
   USHORT cch, ich;

   ASSERT(pch != NULL);
   ASSERT(pdttm != NULL);

   // Skip over leading spaces and check for null

   for (cch = 0; isspace(*pch); cch++, pch++);
   if (!*pch) return FALSE;

   // Scan the date time form looking for a mismatch

   for (ich = 0; ach[ich] && pch[ich]; ich++)
     if (isdigit(ach[ich])) {
       if (!isdigit(pch[ich])) return FALSE;
     } else {
       if (ach[ich] != pch[ich]) return FALSE;
     }

   // Check trailing character

   if (ach[ich] == '\0') {
     if (pch[ich] != '\0' && !isspace(pch[ich])) return FALSE;
   } else
     return FALSE;

   // Convert the values

   pdttm->yr = atoi(pch + 0);
   pdttm->mo = atoi(pch + 5);
   pdttm->da = atoi(pch + 8);
   pdttm->hh = atoi(pch + 11);
   pdttm->mm = atoi(pch + 14);
   pdttm->ss = atoi(pch + 17);

   // Validate the values

   if (pdttm->yr < 1980 || pdttm->yr > 2099) return FALSE;
   if (pdttm->mo < 1    || pdttm->mo > 12  ) return FALSE;
   if (pdttm->da < 1    || pdttm->da > 31  ) return FALSE;
   if (pdttm->hh < 0    || pdttm->hh > 23  ) return FALSE;
   if (pdttm->mm < 0    || pdttm->mm > 59  ) return FALSE;
   if (pdttm->ss < 0    || pdttm->ss > 59  ) return FALSE;

   return cch + ich;
 }

/*
 * This routine converts dttm to an ascii date/time
 */

 PSZ APPENTRY FormatDttm(PDTTM pdttm)
 {
   static USHORT iDttm;
   static struct { CHAR achDttm[32]; } aDttm[4];
   PSZ psz;

   ASSERT(pdttm != NULL);

   // Locate the next slot to return formatted dttm in

   if (iDttm >= numbof(aDttm)) iDttm = 0;
   psz = aDttm[iDttm++].achDttm;

   sprintf(psz, "%4.04u-%2.02u-%2.02u@%2.02u:%2.02u:%2.02u",
           pdttm->yr, pdttm->mo, pdttm->da,
           pdttm->hh, pdttm->mm, pdttm->ss);

   return psz;
 }

/*
 * This routine converts secs to an ascii date/time
 */

 PSZ APPENTRY FormatSecs(time_t secs)
 {
   DTTM dttm;

   SecsToDttm(secs, &dttm);

   return FormatDttm(&dttm);
 }

/*
 * This routine adds the node link for the given queue entry
 */

 BOOL APPENTRY AddQueNodeLink(PQUE pque, NETADDR * pnetAddr)
 {
   PLINK plink;

   ASSERT(pque != NULL);
   ASSERT(pnetAddr != NULL);

   // Check if this node is already in the linked node list

   for (plink = pque->plink; plink != NULL; plink = plink->plinkNext)
     if (!xmemcmp(&plink->netAddr, pnetAddr, sizeof(NETADDR)))
       return FALSE;

   // Create the new linked nodes list element

   if ((plink = (PLINK) LstCreateElement(sizeof(LINK))) == NULL) {
     WriteLog("! Insufficient memory (queue node link)\n");
     exit(EXIT_FAILURE);
   }

   // Link it in and initialize

   LstLinkElement((PPLE)&pque->plink, (PLE) plink, LST_END);
   xmemcpy(&plink->netAddr, pnetAddr, sizeof(NETADDR));

   return TRUE;
 }


/*
 * This routine kills all the nodes links for the given queue entry
 */

 VOID APPENTRY DelQueNodeLinks(PQUE pque)
 {
   PLINK plink;

   ASSERT(pque != NULL);

   while ((plink = pque->plink) != NULL) {
     LstUnlinkElement((PPLE) &pque->plink, 0);
     LstDestroyElement((PLE) plink);
   }
 }

/*
 * This routine kills the node link for the given queue entry
 */

 BOOL APPENTRY DelQueNodeLink(PQUE pque, NETADDR * pnetAddr)
 {
   USHORT ilink;
   PLINK plink;

   ASSERT(pque != NULL);
   ASSERT(pnetAddr != NULL);

   // Scan through the queue entry links looking for the address match

   for (plink = pque->plink, ilink = 0; plink != NULL;
        plink = plink->plinkNext, ilink++)
     if (!xmemcmp(&plink->netAddr, pnetAddr, sizeof(NETADDR))) {
       LstUnlinkElement((PPLE) &pque->plink, ilink);
       LstDestroyElement((PLE) plink);
       return TRUE;
     }

   return FALSE;
 }

/*
 * This routine locates a node in the queue entry links
 */

 PLINK APPENTRY GetQueNodeLink(PQUE pque, NETADDR * pnetAddr)
 {
   PLINK plink;

   ASSERT(pque != NULL);
   ASSERT(pnetAddr != NULL);

   for (plink = pque->plink; plink != NULL; plink = plink->plinkNext)
     if (!xmemcmp(&plink->netAddr, pnetAddr, sizeof(NETADDR)))
       return plink;

   return NULL;
 }

/*
 * This routine adds an area to the queue
 */

 PQUE APPENTRY AddQueEntry(PSZ pszArea, USHORT type,
                           time_t time1, time_t time2)
 {
   USHORT cchArea;
   PQUE pque;

   ASSERT(pszArea != NULL);

   // Check if this area is in the queue and delete it's entry if so

   if (GetQueEntry(pszArea, 0)) DelQueEntry(pszArea);

   // Create new queue entry and check if ok

   cchArea = xstrlen(pszArea);
   if ((pque = (PQUE) LstCreateElement(sizeof(QUE) + cchArea)) == NULL) {
     WriteLog("! Insufficient memory (queue entry)\n");
     exit(EXIT_FAILURE);
   }

   // Link the new queue entry at the end of the queue and check if ok

   if (LstLinkElement((PPLE) &cfg.pqueFirst, (PLE) pque, LST_END) == LST_ERROR) {
     WriteLog("! Can't link queue entry\n");
     exit(EXIT_FAILURE);
   }

   // Set new entry area tag and its hash value

   xmemcpy(pque->achTag, pszArea, cchArea);
   pque->hash = CalcHash(pque->achTag);

   // Set queue entry type

   pque->type = (BYTE) type;

   // Set the queue entry times. Note that if time1 is zero, we set
   // it to the current time and treat time2 as a difference

   if (time1) {
     pque->time1 = time1;
     pque->time2 = time2;
   } else {
     pque->time1 = time(NULL);
     pque->time2 = pque->time1 + time2;
   }

   return pque;
 }

/*
 * This routine locates an area in the queue
 */

 PQUE APPENTRY GetQueEntry(PSZ pszArea, USHORT type)
 {
   ULONG hash = CalcHash(pszArea);
   PQUE pque;

   ASSERT(pszArea != NULL);

   // Scan all the queue entires looking for the area tag match

   for (pque = cfg.pqueFirst; pque != NULL; pque = pque->pqueNext)
     if (pque->hash == hash && !xstricmp(pque->achTag, pszArea))
       if (type == 0) return pque;
       else if (pque->type == type) return pque;

   return NULL;
 }

/*
 * This routine deletes an area from the queue
 */

 BOOL APPENTRY DelQueEntry(PSZ pszArea)
 {
   ULONG hash = CalcHash(pszArea);
   SHORT ique;
   PQUE pque;

   ASSERT(pszArea != NULL);

   // Scan all the queue entires looking for the area tag match

   for (pque = cfg.pqueFirst, ique = 0; pque != NULL; pque = pque->pqueNext, ++ique)
     if (pque->hash == hash && !xstricmp(pque->achTag, pszArea)) {

       // Unlink queue entry

       if (!LstUnlinkElement((PPLE) &cfg.pqueFirst, ique)) return FALSE;

       // Kill all the node linke if any and destroy queue entry

       DelQueNodeLinks(pque);
       LstDestroyElement((PLE) pque);

       return TRUE;
     }

   return FALSE;
 }

/*
 * This routine adds idle passthru areas to the queue
 */

 BOOL APPENTRY CheckIdlePassThruAreas(VOID)
 {
   USHORT iarea = 0;
   PAREA parea;
   PQUE pque;
   PLSZ plsz;

   // Check if we're removing idle passthru areas

   if (!cfg.dayIdleTimeout) return FALSE;

   // Scan the areas list

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {

     // Check if this is not an idle passthru area or
     // it's in the deleted areas list

     if (!IsIdlePassThruArea(parea) || GetDelAreaFromTag(parea->achTag))
       continue;

     // Check if it's already in the idle queue

     if ((pque = GetQueEntry(parea->achTag, QE_IDLE)) != NULL)
       continue;

     // Check if this area should not be deleted even if idle passthru

     for (plsz = cfg.plszIdleKeepAreaFirst; plsz != NULL; plsz = plsz->plszNext)
       if (GrepSearch(parea->achTag, plsz->ach, FALSE))
         break;

     if (plsz != NULL) continue;

     // Add idle passthru queue entry

     pque = AddQueEntry(parea->achTag, QE_IDLE, 0,
                        cfg.dayIdleTimeout * SECS_IN_DAY);
     WriteLog("- AddQ %s idle entry expires on %s\n",
               parea->achTag, FormatSecs(pque->time2));
     iarea++;
   }

   return iarea;
 }

/*
 * This routine checks the queue entry
 */

 BOOL APPENTRY CheckQueEntry(PQUE pque)
 {
   ULONG timeNow = time(NULL);
   USHORT ilink, days = (USHORT)((timeNow - pque->time1) / SECS_IN_DAY);
   PAREA parea = GetAreaFromTag(pque->achTag);
   PUPLINK puplink, puplinkNext;
   PLINK plink, plinkNext;
   CHAR achText[512];
   PNODE pnode;
   PNOTE pnote;

   ASSERT(pque != NULL);

   ////////////////////////////////////////
   // Check out the forwarded request entry

   if (pque->type == QE_FREQ) {

     // Scan and verify all the links

     for (plink = pque->plink, ilink = 0; plink != NULL; plink = plinkNext) {
       plinkNext = plink->plinkNext;
       if (GetNodeFromAddr(&plink->netAddr)) {
         ilink++;
       } else {
         WriteLog("- Kill %s in freq queue for %s (unknown node)\n",
                   FormatNetAddr(&plink->netAddr), pque->achTag);
         LstUnlinkElement((PPLE) &pque->plink, ilink);
         LstDestroyElement((PLE) plink);
       }
     }

     // Check if there are no links at all or the first link is not an uplink

     if (pque->plink == NULL)
       WriteLog("- ChkQ %s freq with no uplink and links\n", pque->achTag);
     else
     if ((puplink = GetUplinkFromAddr(&pque->plink->netAddr)) == NULL)
       WriteLog("- ChkQ %s freq node %s is not an uplink\n",
                 pque->achTag, FormatNetAddr(&pque->plink->netAddr));

     // Check if area in freq queue entry exists

     if (parea != NULL) {

       // Link all but the first nodes listed in freq queue entry
       // Note: the first node listed in freq queue entry is an uplink
       // the request was sent to

       for (plink = pque->plink; plink != NULL; plink = plink->plinkNext)
         if (plink != pque->plink)
           AddAreaLink(parea, &plink->netAddr, ACTIVE, "(freq node)");

       // Delete the queue entry and report

       WriteLog("- DelQ %s freq for existing area\n", pque->achTag);
       DelQueEntry(pque->achTag);

       return FALSE;
     }

     // Check if timeout expired for freq queue entry

     if (timeNow >= pque->time2) {

       // Send an unlink areafix request to an uplink just in case
       // this area was actually linked but there were no messages
       // passing through during the timeout period

       if (pque->plink == NULL) {
         WriteLog("- ChkQ %s freq timeout expired, but uplink is not known\n",
                   pque->achTag);
         return TRUE;
       } else
       if ((puplink = GetUplinkFromAddr(&pque->plink->netAddr)) == NULL) {
         WriteLog("- ChkQ %s freq timeout expired, but uplink %s is not known\n",
                   pque->achTag, FormatNetAddr(&pque->plink->netAddr));
         return TRUE;
       } else
       if (!AddAfrq(puplink, pque->achTag, AFRQ_UNLINK)) {
         WriteLog("- ChkQ %s freq timeout expired, but AFix for uplink %s failed\n",
                   pque->achTag, FormatNetAddr(&pque->plink->netAddr));
         return TRUE;
       }

       // Check if we can try alternate uplink

       if ((puplinkNext = GetNextFreqUplink(pque, &pnode)) != NULL) {

         // Log the operation

         WriteLog("* FReq %s to %s (alternate uplink)\n",
                   pque->achTag, FormatNetAddr(&puplinkNext->pnode->netAddr));

         // Add the areafix request and check if ok

         if (!AddAfrq(puplinkNext, pque->achTag, AFRQ_LINK)) {
           WriteLog("! FReq %s to %s failed\n",
                     pque->achTag, FormatNetAddr(&puplinkNext->pnode->netAddr));

           return TRUE;
         }

         // Update the freq queue entry

         pque->time1 = timeNow;
         pque->time2 = timeNow + cfg.dayFReqTimeout * SECS_IN_DAY;
         pque->fs&= ~QF_WARNINGDONE;
         pque->plink->netAddr = puplinkNext->pnode->netAddr;

         // Log the freq queue entry change

         WriteLog("- FixQ %s freq entry expires on %s\n",
                   pque->achTag, FormatSecs(pque->time2));

         // Add the freq has been expired notification

         if ((pnote = AddNote(pque->achTag, NT_FREQ_EXPNOTE)) != NULL) {
           pnote->pszDescr = AllocString(GetAreaDescrFromTag(pque->achTag), -1);
           pnote->puplink = puplink;
         }

         // Add the request forwarding notification entry

         if ((pnote = AddNote(pque->achTag, NT_FREQ_CREATED)) != NULL) {
           pnote->puplink = puplinkNext;
           pnote->pnode = pnode;
         }

         return TRUE;
       }

       // Inform all but the first nodes listed in freq queue entry
       // Note: the first node listed in freq queue entry is an uplink
       // the request was sent to

       for (plink = pque->plink; plink != NULL; plink = plink->plinkNext) {
         if (plink == pque->plink) continue;
         if ((pnode = GetNodeFromAddr(&plink->netAddr)) != NULL) {
           sprintf(achText, "\rForwarded request for area %s has been expired"
                             " after %d days.\r", pque->achTag, days);
           SendMsg(GetAddrMatch(&plink->netAddr), SQAFIX_NAME,
                  &plink->netAddr, GetNodeSysop(pnode), GetNodeMsgAttr(pnode),
                  "Forwarded request timeout has been expired", achText);

         }
       }

       // Add the freq has been expired notification

       if ((pnote = AddNote(pque->achTag, NT_FREQ_EXPNOTE)) != NULL) {
         pnote->pszDescr = AllocString(GetAreaDescrFromTag(pque->achTag), -1);
         pnote->puplink = puplink;
       }

       // Delete the queue entry and report

       WriteLog("- DelQ %s freq timeout expired, %d days elapsed\n",
                 pque->achTag, days);
       DelQueEntry(pque->achTag);

       return FALSE;
     }

     // Check if it's time to send out notifications

     if (cfg.dayFReqWarning > 0 && !(pque->fs & QF_WARNINGDONE) &&
         timeNow >= (pque->time2 - cfg.dayFReqWarning * SECS_IN_DAY) &&
         GetNextFreqUplink(pque, NULL) == NULL) {

       // Add the freq will be expired notification

       if ((pnote = AddNote(pque->achTag, NT_FREQ_EXPWARN)) != NULL) {
         pnote->pszDescr = AllocString(GetAreaDescrFromTag(pque->achTag), -1);
         pnote->time = pque->time2;
         if (pque->plink != NULL)
           pnote->puplink = GetUplinkFromAddr(&pque->plink->netAddr);
       }

       // Set warning sent flag

       pque->fs|= QF_WARNINGDONE;
     }
   } else

   ////////////////////////////////////////
   // Check out the idle passthru area entry

   if (pque->type == QE_IDLE) {

     // Check if area does not exist anymore

     if (parea == NULL) {

       // Delete the queue entry and report

       WriteLog("- DelQ %s idle passthru area doesn't exist\n", pque->achTag);
       DelQueEntry(pque->achTag);

       return FALSE;
     }

     // Check if area is not idle passthru anymore

     if (!IsIdlePassThruArea(parea)) {

       // Delete the queue entry and report

       WriteLog("- DelQ %s not an idle area\n", pque->achTag);
       DelQueEntry(pque->achTag);

       return FALSE;
     }

     // Check if timeout expired for idle passthru queue entry

     if (timeNow >= pque->time2) {

       // Send an unlink request to an uplink node

       if (parea->plink == NULL) {
         WriteLog("- ChkQ %s idle timeout expired, but uplink is not known\n",
                   parea->achTag);
         return TRUE;
       } else
       if ((puplink = GetIdleAreaUplink(parea)) == NULL) {
         WriteLog("- ChkQ %s idle timeout expired, but uplink %s is not known\n",
                   parea->achTag, FormatNetAddr(&parea->plink->netAddr));
         return TRUE;
       } else
       if (!AddAfrq(puplink, parea->achTag, AFRQ_UNLINK)) {
         WriteLog("- ChkQ %s idle timeout expired, but AFix for uplink %s failed\n",
                   parea->achTag, FormatNetAddr(&parea->plink->netAddr));
         return TRUE;
       }

       // Add the idle entry has been expired notification

       if ((pnote = AddNote(pque->achTag, NT_IDLE_EXPNOTE)) != NULL) {
         pnote->pszDescr = AllocString(parea->pszDescr, -1);
       }

       // Delete the queue entry and report

       WriteLog("- DelQ %s idle timeout expired, %d days elapsed\n",
                 pque->achTag, days);
       DelQueEntry(pque->achTag);

       // Add killed passthru queue entry to suppress autocreate

       if (cfg.dayIdleIgnore > 0) {
         pque = AddQueEntry(parea->achTag, QE_KILL, 0,
                            cfg.dayIdleIgnore * SECS_IN_DAY);
         WriteLog("- AddQ %s kill entry expires on %s\n",
                   pque->achTag, FormatSecs(pque->time2));
         if (AddQueNodeLink(pque, &puplink->pnode->netAddr))
           WriteLog("- LnkQ %s to %s (uplink node)\n",
                     FormatNetAddr(&puplink->pnode->netAddr), parea->achTag);
       }

       // Delete idle passthru area

       AutoDelArea(parea);

       return FALSE;
     }

     // Check if it's time to send out notifications

     if (cfg.dayIdleWarning > 0 && !(pque->fs & QF_WARNINGDONE) &&
         timeNow >= (pque->time2 - cfg.dayIdleWarning * SECS_IN_DAY)) {

       // Add the idle will be expired notification

       if ((pnote = AddNote(pque->achTag, NT_IDLE_EXPWARN)) != NULL) {
         pnote->pszDescr = AllocString(parea->pszDescr, -1);
         pnote->time = pque->time2;
       }

       // Set warning sent flag

       pque->fs|= QF_WARNINGDONE;
     }
   } else

   // Check out the killed idle passthru area entry

   if (pque->type == QE_KILL) {

     // Check if there is no uplink

     if (pque->plink == NULL)
       WriteLog("- ChkQ %s kill with no uplink\n", pque->achTag);
     else
     if ((puplink = GetUplinkFromAddr(&pque->plink->netAddr)) == NULL)
       WriteLog("- ChkQ %s kill node %s is not an uplink\n",
                 pque->achTag, FormatNetAddr(&pque->plink->netAddr));

     // Remove all but the first link if any

     if (pque->plink != NULL)
       while ((plink = (PLINK) LstUnlinkElement((PPLE) &pque->plink, 1)) != NULL) {
         WriteLog("- ChkQ %s kill node %s removed\n",
                   pque->achTag, FormatNetAddr(&plink->netAddr));
         LstDestroyElement((PLE) plink);
       }

     // Check if area in kill queue entry exists

     if (parea != NULL) {

       // Delete the queue entry and report

       WriteLog("- DelQ %s kill for existing area\n", pque->achTag);
       DelQueEntry(pque->achTag);

       return FALSE;
     }

     // Check if timeout expired for killed idle passthru queue entry

     if (timeNow >= pque->time2) {

       // Delete the queue entry and report

       WriteLog("- DelQ %s killed idle passthru timeout expired, %d days elapsed\n",
                 pque->achTag, days);
       DelQueEntry(pque->achTag);

       return FALSE;
     }
   }

   return TRUE;
 }

/*
 * This routine scans the queue file line
 */

 static struct {
   USHORT type; PSZ psz;
 } aQE[] = {
   QE_FREQ, "FReq",
   QE_IDLE, "Idle",
   QE_KILL, "Kill",
 };

 BOOL APPENTRY ScanQueFileLine(PSZ pszLine, ULONG iLine)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   BOOL fDone = FALSE, fAll = FALSE;
   USHORT ifld, iqe;
   PCH  pch, pchEnd;
   DTTM dttm;
   PQUE pque;

   ASSERT(pszLine != NULL);

   // Scan through all the queue line fields

   pch = pszLine;
   for (ifld = 0; !fDone && SkipSpaces(&pch); ifld++) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if this is inline comment and leave if so

     if (*pch == ';') break;

     // Scan in the particular field given its position number

     switch (ifld) {
       case 0: // areatag
               pque = AddQueEntry(pch, 0, 0, 0);
               break;
       case 1: // entry type
               for (iqe = 0; iqe < numbof(aQE); iqe++)
                 if (!xstricmp(aQE[iqe].psz, pch)) {
                   pque->type = (BYTE) aQE[iqe].type;
                   break;
                 }
               if (!pque->type) {
                 WriteLog("! Invalid queue entry %lu type: %s\n", iLine, pch);
                 exit(EXIT_FAILURE);
               }
               SETFLAGTO(pque->fs, QF_WARNINGDONE, isupper(*pch))
               break;
       case 2: // event time
               if (!ScanDttm(pch, &dttm)) {
                 WriteLog("! Invalid queue entry %lu time1: %s\n", iLine, pch);
                 exit(EXIT_FAILURE);
               }
               pque->time1 = DttmToSecs(&dttm);
               break;
       case 3: // expiration time
               if (!ScanDttm(pch, &dttm)) {
                 WriteLog("! Invalid queue entry %lu time2: %s\n", iLine, pch);
                 exit(EXIT_FAILURE);
               }
               pque->time2 = DttmToSecs(&dttm);
               if (pque->time1 > pque->time2) {
                 WriteLog("! Invalid queue entry %lu time1 > time2: %s\n", iLine, pch);
                 exit(EXIT_FAILURE);
               }
               fAll = TRUE;
               break;
       default:// node links
               if (!ScanNetAddr(&netAddr, pch)) {
                 WriteLog("! Invalid queue entry %lu node address: %s\n", iLine, pch);
                 exit(EXIT_FAILURE);
               }
               AddQueNodeLink(pque, &netAddr);
               break;
     }

     pch = ++pchEnd;
   }

   // Check if we processed all the required fields

   if (!fAll) {
     WriteLog("! Incomplete queue entry %lu: %s\n", iLine, pszLine);
     exit(EXIT_FAILURE);
   }

   // Check out the queue entry

   CheckQueEntry(pque);

   return TRUE;
 }

/*
 * This routine loads the queue file
 */

 static FILE * pfileQue;

 BOOL APPENTRY LoadQueFile(PSZ pszFile)
 {
   CHAR achLine[1024];
   ULONG iLine = 0;
   PCH pch;

#ifdef DEBUG
//fprintf(STDAUX, "LoadQueFile: %s\n\r", pszFile);
#endif

   ASSERT(pszFile != NULL);

   // Check if queue file not exist and create it so that we can
   // open and lock it for this session

   if (access(pszFile, 0) && (pfileQue = fopen(pszFile, "wt")) != NULL)
     fclose(pfileQue);

   // Open the queue file

   if ((pfileQue = fopen(pszFile, "r+t")) == NULL) {
     WriteLog("$ Can't open queue file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Scanning file: %s\n", pszFile);

   // Scan in the queue file lines

   achLine[lengof(achLine)] = '\0';

   loop {

     iLine++;

     // Read the subsequent line from the file and check if ok

     if (fgets(achLine, lengof(achLine), pfileQue) == NULL)
       if (!feof(pfileQue)) {
         WriteLog("$ Can't read file: %s\n", pszFile);
         exit(EXIT_FAILURE);
       } else
         break;

     // Check if we got the complete line and remove the trailing \n

     if (*(pch = xstrchr(achLine, 0) - 1) == '\n')
       *pch = '\0';
     else
       if (!feof(pfileQue)) {
         WriteLog("! Line %lu is too long in file '%s'\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }

     // Remove all the trailing spaces if any

     for (--pch; pch >= achLine && isspace(*pch); --pch) *pch = '\0';

     // Skip over leading spaces

     for (pch = achLine; isspace(*pch); pch++);

     // Check if this is an empty line or a comment line

     if (*pch == '\0' || *pch == ';')
       continue;

     // Scan in the queue entry line

     ScanQueFileLine(pch, iLine);
   }

   // Don't close the queue file leaving it open and locked for
   // until SaveQueFile() closes it

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszFile);

   return TRUE;
 }

/*
 * This routine saves the queue file
 */

 BOOL APPENTRY SaveQueFile(PSZ pszFile)
 {
   USHORT cch, cchMax, iqe;
   CHAR achLine[1024];
   NETADDR netAddr;
   PLINK plink;
   PQUE pque;
   PSZ psz;

#ifdef DEBUG
//fprintf(STDAUX, "SaveQueFile: %s\n\r", pszFile);
#endif

   ASSERT(pszFile != NULL);

   // Check for idle passthru areas and add them to the idle queue

   CheckIdlePassThruAreas();

   // Check is this is a test run without saving queue file and leave now

   if (cfg.fl & FL_TESTMODEX) {
     fclose(pfileQue);
     return FALSE;
   }

   // Scan through all the queue entries getting the longes areatag

   for (pque = cfg.pqueFirst, cchMax = 0; pque != NULL; pque = pque->pqueNext)
     if ((cch = xstrlen(pque->achTag)) > cchMax)
       cchMax = cch;

   // Make sure the queue file is open and truncate it

   if (pfileQue == NULL ||
#ifndef UNIX
       fseek(pfileQue, 0, SEEK_SET) || chsize(fileno(pfileQue), 0)) {
#else
       fseek(pfileQue, 0, SEEK_SET)) {
#endif
     WriteLog("$ Can't update file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else {
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Updating file: %s\n", pszFile);
   }

   // Scan through all the queue entries

   for (pque = cfg.pqueFirst; pque != NULL; pque = pque->pqueNext) {

     // Format the queue entry area tag

     cch = sprintf(achLine, "%s", pque->achTag);
     for (; cch < cchMax; achLine[cch++] = ' ');

     // Locate queue entry type string

     for (iqe = 0, psz = "????"; iqe < numbof(aQE); iqe++)
       if (aQE[iqe].type == pque->type) {
         psz = aQE[iqe].psz;
         break;
       }

     // Format the queue entry type. Note that if expiration warning
     // was sent, the queue entry type is in upper case, otherwise
     // it's all lowercase

     psz = (pque->fs & QF_WARNINGDONE) ? xstrupr(psz) : xstrlwr(psz);
     cch+= sprintf(achLine + cch, " %s", psz);

     // Format the queue entry event and expiration times

     cch+= sprintf(achLine + cch, " %s", FormatSecs(pque->time1));
     cch+= sprintf(achLine + cch, " %s", FormatSecs(pque->time2));

     // Format the queue entry nodes list starting with full address

     xmemset(&netAddr, 0, sizeof(NETADDR));
     for (plink = pque->plink; plink != NULL; plink = plink->plinkNext) {
       xstrcat(achLine + cch, MakeNetAddrList(&plink->netAddr, &netAddr));
       cch+= xstrlen(achLine + cch);
     }

     // Write the subsequent line to the file and check if ok

     xstrcpy(achLine + cch, "\n");
     if (fputs(achLine, pfileQue) == EOF) {
       WriteLog("$ Can't write file: %s\n", pszFile);
       exit(EXIT_FAILURE);
     }

#ifdef DEBUG
fprintf(STDAUX, "%s\r", achLine);
#endif
   }

   fclose(pfileQue);

   return TRUE;
 }

/*
 * This subroutine is an uplink selection routines common backend
 */

 static PUPLINK SUBENTRY DoGetFreqUplink(PNODE pnode, PSZ pszArea,
                                         NETADDR * pnetAddr)
 {
   BOOL fLastAddr, fUseNext = FALSE;
   PUPLINK puplink;
   PLSZ plsz;

   ASSERT(pnode != NULL);
   ASSERT(pszArea != NULL);

   // Scan through all the uplink nodes looking for the suitable uplink

   for (puplink = cfg.puplinkFirst; puplink != NULL;
        puplink = puplink->puplinkNext) {

     // Check if this node access level is not good enough to
     // send freqs to this uplink

     if (pnode->level < puplink->level) continue;

     // Loop through available area masks for this uplink looking
     // for the pattern match

     for (plsz = puplink->plszMask; plsz != NULL; plsz = plsz->plszNext)
       if (GrepSearch(pszArea, plsz->ach, FALSE))
         break;

     // Check if we got the areamask match for this uplink

     if (plsz != NULL) {

       // Check if we're not looking for an alternate uplink and
       // return this one if so unless it's the same as the requesting node.
       // Otherwise check if this uplink has the same address as the last
       // tried one

       if (pnetAddr == NULL) {
         if (xmemcmp(&puplink->pnode->netAddr, &pnode->netAddr, sizeof(NETADDR)))
           return puplink;
         else
           continue;
       } else
         fLastAddr = !xmemcmp(&puplink->pnode->netAddr, pnetAddr, sizeof(NETADDR));

       // Check if we have scanned past the last tried uplink and if so,
       // use this one unless it has the same node address as the last one or
       // it's address is the same as requesting node's address

       if (fUseNext) {
         if (!fLastAddr)
           if (xmemcmp(&puplink->pnode->netAddr, &pnode->netAddr, sizeof(NETADDR)))
             return puplink;
           else
             continue;
       } else {
         if (fLastAddr) fUseNext = TRUE;
       }
     }
   }

   return NULL;
 }

/*
 * This routine gets the first freq uplink for the given area and node
 */

 PUPLINK APPENTRY GetFreqUplink(PNODE pnode, PSZ pszArea)
 {
   return DoGetFreqUplink(pnode, pszArea, NULL);
 }

/*
 * This routine gets the next uplink for the given freq queue entry
 */

 PUPLINK APPENTRY GetNextFreqUplink(PQUE pque, PNODE * ppnode)
 {
   PNODE pnode;

   ASSERT(pque != NULL);

   // Check if alternate uplinks feature is not enabled

   if (!(cfg.fl & FL_FREQTRYALLUPLINKS))
     return NULL;

   // Make sure this entry has a valid uplink

   if (pque->plink == NULL)
     return NULL;

   // Make sure this entry has a valid downlink

   if (pque->plink->plinkNext == NULL ||
      (pnode = GetNodeFromAddr(&pque->plink->plinkNext->netAddr)) == NULL)
     return NULL;

   // Set the requesting node for caller if necessary

   if (ppnode != NULL) *ppnode = pnode;

   // Get the next suitable uplink

   return DoGetFreqUplink(pnode, pque->achTag, &pque->plink->netAddr);
 }

/*
 * This routine returns areafix request pointer for a given uplink
 */

 PAFRQ APPENTRY GetAfrqForUplink(PUPLINK puplink)
 {
   PAFRQ pafrq;

   ASSERT(puplink != NULL);

   // Scan through the area fix request list looking for the
   // uplink descriptor match

   for (pafrq = cfg.pafrqFirst; pafrq != NULL; pafrq = pafrq->pafrqNext)
     if (pafrq->puplink == puplink)
       return pafrq;

   // Create new areafix request list element and check if ok

   if ((pafrq = (PAFRQ) LstCreateElement(sizeof(AFRQ))) == NULL)
     return NULL;

   // Link up the new areafix list element and set its uplink

   LstLinkElement((PPLE) &cfg.pafrqFirst, (PLE) pafrq, LST_END);
   pafrq->puplink = puplink;

   return pafrq;
 }

/*
 * This routine adds an area to the link areafix request list
 */

 PAFRQ APPENTRY AddAfrq(PUPLINK puplink, PSZ pszArea, USHORT fs)
 {
   PAFRQ pafrq;
   PLSZ plsz;
   PSZ psz;

   ASSERT(puplink != NULL);
   ASSERT(pszArea != NULL);     // end of ASSERTed range

   // Get the areafix request list element for a given uplink and check if ok

   if ((pafrq = GetAfrqForUplink(puplink)) == NULL) {
     WriteLog("! Insufficient memory (afrq list)\n");
     exit(EXIT_FAILURE);
   }

   // Add area to the areafix request list and set its flag

   if ((plsz = AddLsz(&pafrq->plszArea, pszArea, -1, LST_END)) == NULL) {
     WriteLog("! Insufficient memory (afrq area)\n");
     exit(EXIT_FAILURE);
   } else
     plsz->fs = fs;

   // Log the operation

   if (fs & AFRQ_LINK) psz = "link";
   else if (fs & AFRQ_UNLINK) psz = "unlink";
   else psz = "unknown operation";

   WriteLog("- AFix %s %s at %s\n",
             pszArea, psz, FormatNetAddr(&puplink->pnode->netAddr));

   return pafrq;
 }

/*
 * This routine deletes the areafix request list
 */

 VOID APPENTRY DelAfrqList(VOID)
 {
   PAFRQ pafrq;

   // Delete the area fix request list

   while (cfg.pafrqFirst != NULL) {
     pafrq = (PAFRQ) LstUnlinkElement((PPLE) &cfg.pafrqFirst, 0);
     ASSERT(pafrq != NULL);

#ifdef DEBUG
//fprintf(STDAUX, "DelAfrqList: pafrq=[%09Fp] to %s\r\n", pafrq, FormatNetAddr(&pafrq->puplink->pnode->netAddr));
#endif

     DelLszList(&pafrq->plszArea);
     LstDestroyElement((PLE) pafrq);
   }

   ASSERT(LstQueryElementCount((PLE) cfg.pafrqFirst) == 0);
 }

/*
 * This subroutine returns areafix request from address for given uplink
 */

 static NETADDR * SUBENTRY DoGetAfrqFromAddr(PUPLINK puplink)
 {
   // Use primary address unless specific from address is set for this uplink

   return puplink->netAddr.zone ? &puplink->netAddr : &cfg.anetAddr[0];
 }

/*
 * This subroutine returns pointer to the areafix operation string
 */

 static PSZ SUBENTRY DoGetAfrqOp(PUPLINK puplink, PLSZ plsz)
 {
   if (plsz->fs & AFRQ_LINK) {
     if ((puplink->fs & UF_NOPLUSPREFIX) && !(puplink->fs & UF_EXTERNALAFREQ))
       return "";
     else
       return "+";
   } else
     if (plsz->fs & AFRQ_UNLINK)
       return "-";

   return "?";
 }

/*
 * This subroutine sends areafix request
 */

 static BOOL SUBENTRY DoSendAfrq(PAFRQ pafrq)
 {
   PUPLINK puplink = pafrq->puplink;
   PNODE pnode = pafrq->puplink->pnode;
   CHAR achArea[MAX_AREA_LENG + 4];
   NETADDR * pnetAddr;
   USHORT cch;
   PLSZ plsz;

   // Determine the from address

   pnetAddr = DoGetAfrqFromAddr(puplink);

   // Check if we can send request for multiple areas in one message

   if (!(puplink->fs & UF_NOMULTIAREATAGS)) {

     // Start writing of the long message with no continuation lines

     SendLongMsgBeg(pnetAddr, SQAFIX_HOST, &pnode->netAddr, puplink->pszName,
                    GetNodeMsgAttr(pnode), pnode->achPassword, FALSE);

     // Write out areafix request areas

     for (plsz = pafrq->plszArea; plsz != NULL; plsz = plsz->plszNext) {

       // Format the areafix request message line

       cch = sprintf(achArea, "%s%.128s\r", DoGetAfrqOp(puplink, plsz), plsz->ach);
       if (puplink->fs & UF_LOWERCASETAG) xstrlwr(achArea);

       // Check if buffer exceeded and write out message

       if (ichBuf + cch >= cfg.cchMaxMsgPart) {
         SendLongMsgPut(pchBuf);
         ichBuf = 0;
       }

       // Add areafix request message line

       xstrcpy(pchBuf + ichBuf, achArea);
       ichBuf+= cch;
     }

     // Finish writing of the long message

     SendLongMsgEnd();

   } else {

     // Send one areafix request message for every area

     for (plsz = pafrq->plszArea; plsz != NULL; plsz = plsz->plszNext) {

       // Format the areafix request message body

       sprintf(achArea, "%s%.128s\r", DoGetAfrqOp(puplink, plsz), plsz->ach);
       if (puplink->fs & UF_LOWERCASETAG) xstrlwr(achArea);

       // Send the areafix request

       SendMsg(pnetAddr, SQAFIX_HOST, &pnode->netAddr, puplink->pszName,
               GetNodeMsgAttr(pnode), pnode->achPassword, achArea);
     }
   }

   return TRUE;
 }

/*
 * This subroutine executes external areafix requestor
 */

 static BOOL SUBENTRY DoExecAfrq(PAFRQ pafrq, PSZ pszOp, PSZ pszAreaOrFile,
                                 PSZ pszFromAddr, PSZ pszToAddr)
 {
   static CHAR achAFix[] = "AreaFix";
   PUPLINK puplink = pafrq->puplink;
   PNODE pnode = pafrq->puplink->pnode;
   CHAR achCmd[MAXPATH];
   SHORT code;
#ifdef UNIX
   PSZ tmp = NULL;
#endif

   // Log what are we going to execute

   WriteLog("- Exec %s %s %s %s %s %s %s %s %s\n",
             puplink->pszName, pszOp, pszAreaOrFile,
             SQAFIX_HOST, pszFromAddr, achAFix, pszToAddr,
             pnode->achPassword, cfg.achNetMail);

   // Get the operating system command shell name and
   // spawn external program

   GetCommandShellName(achCmd);
   
#ifndef UNIX
   
   code = spawnlp(P_WAIT, achCmd, achCmd, "/C",
                  puplink->pszName, pszOp, pszAreaOrFile,
                  SQAFIX_HOST, pszFromAddr, achAFix, pszToAddr,
                  pnode->achPassword, cfg.achNetMail,
                  NULL);

#else
    /* BO: FIXME! I really don't know what this rutine do! */
#endif

   // Check if spawn failed

   if (code == (SHORT)-1) {
     WriteLog("$ Exec %s failed\n", puplink->pszName);
     return FALSE;
   }

   // Log exit code

   WriteLog("- Exec %s exit code (%u)\n", puplink->pszName, code);

   // Check if it's failure code and return error condition

   return !(code == 255);
 }

/*
 * This subroutine makes areafix request using external requestor
 */

 static BOOL SUBENTRY DoMakeAfrq(PAFRQ pafrq)
 {
   CHAR achArea[MAX_AREA_LENG + 4], achFile[MAXPATH];
   CHAR achFromAddr[32], achToAddr[32];
   PUPLINK puplink = pafrq->puplink;
   PNODE pnode = pafrq->puplink->pnode;
   NETADDR * pnetAddr;
   FILE * pfile;
   PLSZ plsz;

   // Determine the from address

   pnetAddr = DoGetAfrqFromAddr(puplink);

   // Format from and to network addresses

   xstrcpy(achFromAddr, FormatNetAddr(pnetAddr));
   xstrcpy(achToAddr, FormatNetAddr(&pnode->netAddr));

   // Check if we can send request for multiple areas in one message

   if (!(puplink->fs & UF_NOMULTIAREATAGS)) {

     // Make up the temporary file name

     MakeTmpFile(achFile, puplink->pszName);

     // Open the temporary file and check if ok

     if ((pfile = fopen(achFile, "wt")) == NULL) {
       WriteLog("$ Can't create file: %s\n", achFile);
       return FALSE;
     } else
       WriteLog("- Make %s\n", achFile);

     // Write out areafix request areas

     for (plsz = pafrq->plszArea; plsz != NULL; plsz = plsz->plszNext) {

       // Format the areafix request message line

       sprintf(achArea, "%s %.128s\n", DoGetAfrqOp(puplink, plsz), plsz->ach);
       if (puplink->fs & UF_LOWERCASETAG) xstrlwr(achArea);

       // Write out the temporary file line

       if (fputs(achArea, pfile) == EOF) {
         WriteLog("$ Can't write file: %s\n", achFile);
         fclose(pfile);
         return FALSE;
       }
     }

     // Close the temporary file

     fclose(pfile);

     // Execute external areafix requestor

     DoExecAfrq(pafrq, "*", achFile, achFromAddr, achToAddr);

     // Delete temporary file

     DelFile(achFile);

   } else {

     // Send one areafix request message for every area

     for (plsz = pafrq->plszArea; plsz != NULL; plsz = plsz->plszNext) {

       // Add leading and trailing double quotas so that areatags
       // with special characters will not screw up command line

       if (IsSpecCmdChars(plsz->ach))
         sprintf(achArea, "\"%.128s\"", plsz->ach);
       else
         sprintf(achArea, "%.128s", plsz->ach);

       if (puplink->fs & UF_LOWERCASETAG) xstrlwr(achArea);

       // Execute external areafix requestor

       DoExecAfrq(pafrq, DoGetAfrqOp(puplink, plsz), achArea,
                  achFromAddr, achToAddr);
     }
   }

   return TRUE;
 }

/*
 * This routine sends areafix requests
 */

 VOID APPENTRY SendAreaFixReqs(VOID)
 {
   PAFRQ pafrq;

   // Scan through the area fix request list

   for (pafrq = cfg.pafrqFirst; pafrq != NULL; pafrq = pafrq->pafrqNext)
     if (pafrq->plszArea != NULL) {

       // Log the areafix operation

       WriteLog("* Sending AreaFix requests to %s\n",
                 FormatNetAddr(&pafrq->puplink->pnode->netAddr));

       // Call the appropriate areafix requestor

       if (pafrq->puplink->fs & UF_EXTERNALAFREQ) {
         DoMakeAfrq(pafrq);
       } else
       if (pafrq->puplink->fs & UF_AREAFIXPROT) {
         DoSendAfrq(pafrq);
       } else {
         WriteLog("! AFix %s not supported\n", pafrq->puplink->pszName);
       }
     }

   // Delete the area fix request list

   DelAfrqList();
 }

/*
 * This routine adds an element to the send file list
 */

 PSNDF APPENTRY AddSndf(PNODE pnode, PSZ pszFile, PSZ pszSubj)
 {
   USHORT cchFile;
   PSNDF psndf;

   // Get length of the file name and check if ok

   if ((cchFile = xstrlen(pszFile)) == 0)
     return NULL;

   // Make up the default subject string

   if (pszSubj == NULL) pszSubj = "<none>";

   // Scan through send file list looking for an exact match

   for (psndf = cfg.psndfFirst; psndf != NULL; psndf = psndf->psndfNext)
     if (psndf->pnode == pnode &&
        !xstricmp(psndf->pszSubj, pszSubj) &&
        !xstricmp(psndf->achFile, pszFile))
       return psndf;

   // Create send file list element and check if ok

   if ((psndf = (PSNDF) LstCreateElement(sizeof(SNDF) + cchFile)) == NULL)
     return NULL;

   // Set in the file name and node address

   xmemcpy(psndf->achFile, pszFile, cchFile);
   psndf->pnode = pnode;

   // Allocate the subject string and check if ok

   if ((psndf->pszSubj = AllocString(pszSubj, -1)) == NULL) {
     LstDestroyElement((PLE) psndf);
     return NULL;
   }

   // Link the send file list element at the end of the list and
   // check if ok

   if (LstLinkElement((PPLE) &cfg.psndfFirst, (PLE) psndf, LST_END) == LST_ERROR) {
     if (psndf->pszSubj) MemFree(psndf->pszSubj);
     LstDestroyElement((PLE) psndf);
     return NULL;
   }

   // Log the operation

   WriteLog("- File %s \"%s\" to %s\n",
             psndf->achFile, psndf->pszSubj, FormatNetAddr(&pnode->netAddr));

   return psndf;
 }

/*
 * This routine deletes the send file list
 */

 VOID APPENTRY DelSndfList(VOID)
 {
   PSNDF psndf;

   // Delete the area fix request list

   while (cfg.psndfFirst != NULL) {
     psndf = (PSNDF) LstUnlinkElement((PPLE) &cfg.psndfFirst, 0);
     ASSERT(psndf != NULL);

#ifdef DEBUG
//fprintf(STDAUX, "DelSndfList: psndf=[%09Fp] to %s\r\n", psndf, FormatNetAddr(&psndf->pnode->netAddr));
#endif

     if (psndf->pszSubj) MemFree(psndf->pszSubj);
     LstDestroyElement((PLE) psndf);
   }

   ASSERT(LstQueryElementCount((PLE) cfg.psndfFirst) == 0);
 }

/*
 * This subroutine sends file to a given node
 */

 static BOOL SUBENTRY DoSendFile(PNODE pnode, PSZ pszFile, PSZ pszSubj)
 {
   CHAR achLine[512];
   FILE *pfile;

   // Initialize writing the message

   SendLongMsgBeg(GetAddrMatch(&pnode->netAddr), SQAFIX_NAME,
                 &pnode->netAddr, GetNodeSysop(pnode), GetNodeMsgAttr(pnode), pszSubj,
                  TRUE);

   // Open the usage info file and check if ok

   if ((pfile = fopen(pszFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     WriteMsg("\nCan't open file to be sent."
              "\nPlease, inform the SysOp of this error..."
              "\n");
     goto End;
   }

   // Read the file and create a report text

   achLine[sizeof(achLine) - 1] = '\0';
   loop
     if (fgets(achLine, sizeof(achLine) - 1, pfile) != NULL)
       WriteMsg("%s", achLine);
     else
       if (!feof(pfile)) {
         WriteLog("$ Can't read file: %s\n", pszFile);
         WriteMsg("\nProblem occured while reading file to be sent."
                  "\nPlease, inform the SysOp of this error..."
                  "\n");
         fclose(pfile);
         goto End;
       } else
         break;

   // Clean up

   WriteMsg("\n");
   fclose(pfile);

   // Finish report writing

End: SendLongMsgEnd();

   return TRUE;
 }

/*
 * This routine sends files to nodes
 */

 VOID APPENTRY SendFiles(VOID)
 {
   PSNDF psndf;

   // Check if we have something to do

   if (!cfg.psndfFirst) return;

   // Log the operation

   WriteLog("* Sending out files\n");

   // Scan through the send file list

   for (psndf = cfg.psndfFirst; psndf != NULL; psndf = psndf->psndfNext)
     DoSendFile(psndf->pnode, psndf->achFile, psndf->pszSubj);

   // Delete the send file list

   DelSndfList();
 }

/*
 * This routine adds an element to the sent message list
 */

 BOOL APPENTRY AddSntm(UMSG umsg)
 {
   PSNTM psntm;

   // Scan through sent message list looking for a next free slot

   for (psntm = cfg.psntmFirst; psntm != NULL; psntm = psntm->psntmNext)
     if (psntm->imsg < numbof(psntm->aumsg))
       goto SetIt;

   // Create sent message list element and check if ok

   if ((psntm = (PSNTM) LstCreateElement(sizeof(SNTM))) == NULL) {
     WriteLog("! Insufficient memory (send msg list)\n");
     exit(EXIT_FAILURE);
   }

   // Link the sent message list element at the end of the list and
   // check if ok

   if (LstLinkElement((PPLE) &cfg.psntmFirst, (PLE) psntm, LST_END) == LST_ERROR) {
     LstDestroyElement((PLE) psntm);
     return FALSE;
   }

   // Set in the sent message uid

SetIt: psntm->aumsg[psntm->imsg++] = umsg;

   return TRUE;
 }

/*
 * This routine gets an element from the sent message list
 */

 BOOL APPENTRY GetSntm(UMSG umsg)
 {
   PSNTM psntm;
   USHORT imsg;

   // Scan through sent message list looking for a match

   for (psntm = cfg.psntmFirst; psntm != NULL; psntm = psntm->psntmNext)
     for (imsg = 0; imsg < psntm->imsg; imsg++)
       if (psntm->aumsg[imsg] == umsg)
         return TRUE;

   return FALSE;
 }

/*
 * This routine adds an area to the notification list
 */

 PNOTE APPENTRY AddNote(PSZ pszArea, USHORT type)
 {
   USHORT cch = xstrlen(pszArea);
   PNOTE pnote;

   // Create new notification list element and check if ok. Note that
   // we don't terminate the program here just loosing the notifications

   if ((pnote = (PNOTE) LstCreateElement(sizeof(NOTE) + cch)) == NULL)
     return NULL;
   else
     LstLinkElement((PPLE) &cfg.pnoteFirst, (PLE) pnote, LST_END);

   // Set the new notification list element area and type

   xmemcpy(pnote->achTag, pszArea, cch);
   pnote->type = (BYTE) type;

#ifdef DEBUG
//fprintf(STDAUX, "AddNote: %s\t0x%04X\r\n", pnote->achTag, pnote->type);
#endif

   return pnote;
 }

/*
 * This routine deletes the notification list
 */

 VOID APPENTRY DelNoteList(VOID)
 {
   PNOTE pnote;

   // Delete the area fix request list

   while (cfg.pnoteFirst != NULL) {
     pnote = (PNOTE) LstUnlinkElement((PPLE) &cfg.pnoteFirst, 0);
     ASSERT(pnote != NULL);

#ifdef DEBUG
//fprintf(STDAUX, "DelNoteList: pnote=[%09Fp] for '%s'\r\n", pnote, pnote->achTag);
#endif

     if (pnote->pszDescr) MemFree(pnote->pszDescr);
     LstDestroyElement((PLE) pnote);
   }

   ASSERT(LstQueryElementCount((PLE) cfg.pnoteFirst) == 0);
 }

/*
 * This routine returns number of notifications with the given type
 */
/*
 USHORT APPENTRY CalcNotes(USHORT type)
 {
   USHORT iArea = 0;
   PNOTE pnote;

   // Loop through all the notification list elements calculating
   // number of notifications with the given type

   for (pnote = pnoteFirst; pnote != NULL; pnote = pnote->pnoteNext)
     if (type == 0 || pnote->type == type) iArea++;

   return iArea;
 }
*/
/*
 * This routine loads the password list file
 */

 BOOL APPENTRY LoadPwlFile(PSZ pszFile)
 {
   static CHAR achPwd[] = "Password";
   CHAR achLine[1024];
   NETADDR netAddr;
   ULONG iLine = 0;
   PCH pch, pchEnd;
   FILE * pfile;
   USHORT cch;
   PPWL ppwl;

   // Verify file name specification

   if (!pszFile || !pszFile[0]) return FALSE;

#ifdef DEBUG
//fprintf(STDAUX, "LoadPwlFile: %s\n\r", pszFile);
#endif

   // Open the password list file

   if ((pfile = fopen(pszFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Scanning file: %s\n", pszFile);

   // Scan in the password list file

   achLine[lengof(achLine)] = '\0';

   loop {

     iLine++;

     // Read the subsequent line from the file and check if ok

     if (fgets(achLine, lengof(achLine), pfile) == NULL)
       if (!feof(pfile)) {
         WriteLog("$ Can't read file: %s\n", pszFile);
         exit(EXIT_FAILURE);
       } else
         break;

     // Check if we got the complete line and remove the trailing \n

     if (*(pch = xstrchr(achLine, 0) - 1) == '\n')
       *pch = '\0';
     else
       if (!feof(pfile)) {
         WriteLog("! Line %lu is too long in file '%s'\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }

     // Remove all the trailing spaces if any

     for (--pch; pch >= achLine && isspace(*pch); --pch) *pch = '\0';

     // Skip over leading spaces

     for (pch = achLine; isspace(*pch); pch++);

     // Check if this is an empty line or a comment line

     if (*pch == '\0' || *pch == ';')
       continue;

     // Check for the Password keyword and skip over it

     if (!xmemicmp(pch, achPwd, lengof(achPwd))) {
       pch+= lengof(achPwd);
       if (!SkipSpaces(&pch)) {
         WriteLog("! Missing node address in line %lu file: %s\n", iLine, pszFile);
         exit(EXIT_FAILURE);
       }
     }

     // Fix up the end of the address specification

     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     while (isspace(*pchEnd)) *pchEnd++ = '\0';

     // Scan in the address relative to our primary AKA and check if ok

     xmemcpy(&netAddr, &cfg.anetAddr[0], sizeof(NETADDR));
     if (!ScanNetAddr(&netAddr, pch)) {
       WriteLog("! Invalid node address in line %lu file: %s\n", iLine, pszFile);
       exit(EXIT_FAILURE);
     }

     // Fix up the end of the password specification

     for (pch = pchEnd; *pchEnd && !isspace(*pchEnd) && *pchEnd != ',';
     pchEnd++); *pchEnd = '\0';

     // Calculate password string length and check if it's ok

     if ((cch = (USHORT)(pchEnd -pch)) == 0) {
       WriteLog("! Missing password in line %lu file: %s\n", iLine, pszFile);
       exit(EXIT_FAILURE);
     } else
     if (cch > MAX_PASS_LENG) {
       WriteLog("! Password is too long in line %lu file: %s\n", iLine, pszFile);
       exit(EXIT_FAILURE);
     }

     // Check if this node is already present in the password list

     if ((ppwl = GetNodePwl(&netAddr)) != NULL) {

       // Check if passwords are different

       if (xstricmp(ppwl->achPassword, pch)) {
         WriteLog("! Duplicate password for node %s in line %lu file: %s\n", FormatNetAddr(&netAddr), iLine, pszFile);
         exit(EXIT_FAILURE);
       } else
         continue;
     }

     // Add the password list element

     AddNodePwl(&netAddr, pch);
   }

   fclose(pfile);

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszFile);

   return TRUE;
 }

/*
 * This routine gets node and password from the password list
 */

 PPWL APPENTRY GetNodePwl(NETADDR * pnetAddr)
 {
   PPWL ppwl;

   // Scan through the password list looking for the node address match

   for (ppwl = cfg.ppwlFirst; ppwl != NULL; ppwl = ppwl->ppwlNext)
     if (!xmemcmp(&ppwl->netAddr, pnetAddr, sizeof(NETADDR)))
       return ppwl;

   return NULL;
 }

/*
 * This routine adds node and password to the password list
 */

 BOOL APPENTRY AddNodePwl(NETADDR * pnetAddr, PSZ pszPassword)
 {
   USHORT cchPassword;
   PPWL ppwl;

   // Create and link new password list element and check if ok

   cchPassword = xstrlen(pszPassword);
   if ((ppwl = (PPWL) LstCreateElement(sizeof(PWL) + cchPassword)) == NULL) {
     WriteLog("! Insufficient memory (password list)\n");
     exit(EXIT_FAILURE);
   } else
     LstLinkElement((PPLE) &cfg.ppwlFirst, (PLE) ppwl, LST_END);

   // Set in the node address and password

   xmemcpy(&ppwl->netAddr, pnetAddr, sizeof(NETADDR));
   xmemcpy(ppwl->achPassword, pszPassword, cchPassword);

#ifdef DEBUG
//fprintf(STDAUX, "AddNodePwl: ppwl=[%09Fp], %s '%s'\r\n", ppwl, FormatNetAddr(&ppwl->netAddr), ppwl->achPassword);
#endif

   return TRUE;
 }

/*
 * This routine retrieves the operating system shell name
 */

 VOID APPENTRY GetCommandShellName(PSZ pszCmd)
 {
   PSZ psz;

   // Check the COMSPEC environment variable and if not present,
   // use the default command shell name for the current os

   if ((psz = getenv("COMSPEC")) == NULL) {
#if defined(__OS2__)
     psz = "CMD.EXE";
#elif defined(__W32__)
     psz = "CMD.EXE";
#else
     psz = "COMMAND.COM";
#endif
   }

   // Copy the command shell name into the caller's buffer

   xstrcpy(pszCmd, psz);
 }

/*
 * This routine makes the temporary file name given the base
 */

 VOID APPENTRY MakeTmpFile(PSZ pszFile, PSZ pszBase)
 {
   CHAR achPath[MAXPATH], achFile[MAXFILE];
   PSZ psz;

   // Make the base file name

   fnsplit(pszBase, NULL, NULL, achFile, NULL);

   // Locate the temporary directory and if no, use the current one

   if ((psz = getenv("SQAFTEMP")) != NULL ||
       (psz = getenv("TEMP")) != NULL ||
       (psz = getenv("TMP")) != NULL) {
     xstrncpy(achPath, psz, lengof(achPath) - 1);
     achPath[lengof(achPath) - 1] = '\0';
     if ((psz = xstrchr(achPath, 0)) > achPath && *(psz - 1) != '\\')
       xstrcat(achPath, "\\");
   } else
     achPath[0] = '\0';

   // Merge the path and base file name then add extension

   xstrcpy(pszFile, achPath);
   xstrcat(pszFile, achFile);
   xstrcat(pszFile, DEF_TMP_EXT);
 }

/*
 * This routine deletes a specified file
 */

 BOOL APPENTRY DelFile(PSZ pszFile)
 {
   ASSERT(pszFile != NULL);

   // Check if the file exists

   if (access(pszFile, 0))
     return TRUE;

   // Delete file and check if ok

   if (unlink(pszFile)) {
     WriteLog("$ Delete failed: %s\n", pszFile);
     return FALSE;
   }

   // Log operation

   WriteLog("- Kill %s\n", pszFile);

   ASSERT(access(pszFile, 0) != 0);

   return TRUE;
 }

/*
 * This routine renames specified file
 */

 BOOL APPENTRY RenFile(PSZ pszFileOld, PSZ pszFileNew)
 {
   ASSERT(pszFileOld != NULL);
   ASSERT(pszFileNew != NULL);

   if (rename(pszFileOld, pszFileNew)) {
     WriteLog("$ Rename failed: %s to %s\n", pszFileOld, pszFileNew);
     return FALSE;
   }

   // Log operation

   WriteLog("- Renm %s to %s\n", pszFileOld, pszFileNew);

   ASSERT(access(pszFileOld, 0) != 0);
   ASSERT(access(pszFileNew, 0) == 0);

   return TRUE;
 }

/*
 * This routine commits the file associated with the given stream
 */

 SHORT APPENTRY CommitFile(FILE * pfile)
 {
#if 1
   return fflush(pfile);
#else
   int hfile = fileno(pfile);
   union REGS regs;

   fflush(pfile);

   regs.h.ah = 0x68;
   regs.x.bx = hfile;
   intdos(&regs, &regs);

   return regs.x.cflag ? -1 : 0;
#endif
 }

/*
 * This routine formats text and writes it to the log
 */

 VOID WriteLog(PSZ pszFormat, ...)
 {
   time_t tm;
   static FILE * pfile;
   va_list argptr;
   CHAR achText[256];
   BOOL fSysErr = FALSE;
   BOOL fFixEnd = FALSE;
   PCH pch;

   PSZ tmp = (PSZ) malloc(strlen(pszFormat)+1);
   strcpy(tmp, pszFormat);
   pszFormat = (PSZ) malloc(strlen(tmp)+1);
   strcpy(pszFormat, tmp);

   free(tmp);
   tmp = NULL;
   
   va_start(argptr, pszFormat);

   // Check if this is a notification message and we don't have
   // detailed log option in effect and skip it if so

   if (pszFormat[0] == '-' && !(cfg.fl & FL_DETAILEDLOG)) goto Ok;

   // Check if we have the log file open and if not check if
   // there is a log file name defined so we can open it now.
   // If we got it successfully open then write the time/date
   // and version stamp in there

   if (pfile == NULL && cfg.achLogFile[0])
     if ((pfile = fopen(cfg.achLogFile, "at")) != NULL) {
       time(&tm); xstrcpy(achText, asctime(localtime(&tm)));
       if (xstrchr(achText, '\n')) *xstrchr(achText, '\n') = '\0';
       fprintf(pfile, "\n--- %s (v%s)\n\n" , achText, VERSION);
     }

   // Check if this message is an system error alert which
   // we need system error message line to have appended to

   if (pszFormat[0] == '$') {
     pszFormat[0] = '!'; 
     fSysErr = TRUE;
     if (*(pch = xstrchr(pszFormat, 0) - 1) == '\n') {
       *pch = '\0'; fFixEnd = TRUE;
     }
   }

   // Display the message on the console screen and issue
   // the alarm sound for error messages

   if (pszFormat[0] == '!') printf("\a");
   vfprintf(stderr, pszFormat, argptr);
   if (fSysErr) fprintf(stderr, " (%s)\n", sys_errlist[errno]);
   CommitFile(stderr);

   // Check if the log file is open and append a record to it

   if (pfile != NULL) {
     vfprintf(pfile, pszFormat, argptr);
     if (fSysErr) fprintf(pfile, " (%s)\n", sys_errlist[errno]);
     CommitFile(pfile);
   }

   // Restore modified format string since it may be static

   if (fSysErr) pszFormat[0] = '$';
   if (fFixEnd) *pch = '\n';

Ok:va_end(argptr);
 }

/*
 * This routine is called by an assertion macro
 */

 VOID Assert(PSZ pszText, PSZ pszFile, USHORT iLine)
 {
   WriteLog("! Assertion failed: %s, file %s, line %u\n",
             pszText, pszFile, iLine);
 }

/*
 * This directory management stuff is necessary for Microsoft C
 */

#if defined (__MSC__)
#include "stpcpy.c"
#include "fnsplit.c"
#include "fnmerge.c"
#endif

/*
 * End of SQAUTI.C
 */
