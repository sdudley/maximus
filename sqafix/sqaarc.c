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
 * Archiver management routines
 *
 * Created: 22/Apr/97
 * Updated: 14/May/98
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

 #include "sqafix.h"

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   d e c l a r a t i o n s                                   //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   s u b r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// S q a F i x   c o n f i g   p a r s e r   s u b r o u t i n e s         //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// P u b l i c   r o u t i n e s                                           //
/////////////////////////////////////////////////////////////////////////////

/*
 * This routine adds a packer to the packer list
 */

 PARC APPENTRY AddArc(PSZ pszArc, USHORT ipos)
 {
   USHORT cchArc;
   PARC parc;

   // Create new packer list entry and check if ok

   cchArc = xstrlen(pszArc);
   if ((parc = (PARC) LstCreateElement(sizeof(ARC) + cchArc)) == NULL) {
     WriteLog("! Insufficient memory (packer entry)\n");
     exit(EXIT_FAILURE);
   }

   // Link the new packer list entry at the end of the queue and check if ok

   if (LstLinkElement((PPLE) &cfg.parcFirst, (PLE) parc, ipos) == LST_ERROR) {
     WriteLog("! Can't link packer entry\n");
     exit(EXIT_FAILURE);
   } else
     xmemcpy(parc->achArc, pszArc, cchArc);

   return parc;
 }

/*
 * This routine locates packer and link for the given node
 */

 PARC APPENTRY GetArc(NETADDR * pnetAddr, PLINK * pplink)
 {
   PLINK plink;
   PARC parc;

   // Scan all the packer list entires looking the given node

   for (parc = cfg.parcFirst; parc != NULL; parc = parc->parcNext)
     if ((plink = GetArcLink(parc, pnetAddr)) != NULL) {
       if (pplink != NULL) *pplink = plink;
       return parc;
     }

   return NULL;
 }

/*
 * This routine checks if given packer entry has address masks
 */

 BOOL APPENTRY CheckArcAddrMask(PARC parc)
 {
   PLINK plink;

   // Scan all the packer node specifications

   for (plink = parc->plink; plink != NULL; plink = plink->plinkNext)
     if (IsAddrMask(&plink->netAddr))
       return TRUE;

   return FALSE;
 }

/*
 * This routine adds a packer link specification
 */

 PLINK APPENTRY AddArcLink(PARC parc, NETADDR * pnetAddr)
 {
   PLINK plink;

   // Create the new linked nodes list element

   if ((plink = (PLINK) LstCreateElement(sizeof(LINK))) == NULL) {
     WriteLog("! Insufficient memory (packer link)\n");
     exit(EXIT_FAILURE);
   } else {
     LstLinkElement((PPLE)&parc->plink, (PLE) plink, LST_END);
     xmemcpy(&plink->netAddr, pnetAddr, sizeof(NETADDR));
   }

   return plink;
 }

/*
 * This routine gets a packer link specification
 */

 PLINK APPENTRY GetArcLink(PARC parc, NETADDR * pnetAddr)
 {
   PLINK plink;

   // Scan all the packer links entires looking for the node spec match

   for (plink = parc->plink; plink != NULL; plink = plink->plinkNext)
     if (!xmemcmp(&plink->netAddr, pnetAddr, sizeof(NETADDR)))
       return plink;

   return NULL;
 }

/*
 * This routine deletes a packer link specification
 */

 BOOL APPENTRY DelArcLink(PARC parc, NETADDR * pnetAddr)
 {
   USHORT ilink;
   PLINK plink;

   // Get the packer link element and check if ok

   if ((plink = GetArcLink(parc, pnetAddr)) == NULL)
     return FALSE;

   // Get the packer link index and check if ok

   if ((ilink = LstIndexFromElement((PLE) parc->plink, (PLE) plink)) == LST_ERROR)
     return FALSE;

   // Unlink and destroy the packer link

   if ((PLINK) LstUnlinkElement((PPLE) &parc->plink, ilink) != plink) {
     WriteLog("! Internal error at %s(%lu)\n", __FILE__, __LINE__);
     exit(EXIT_FAILURE);
   } else
     LstDestroyElement((PLE) plink);

   return TRUE;
 }

/*
 * This routine loads the available packer list from Compress.cfg
 */

 BOOL APPENTRY LoadArcSpec(VOID)
 {
   PSZ pszFile = cfg.achArcFile;
   CHAR achLine[1024];
   ULONG iLine = 0;
   PCH pch, pchEnd;
   FILE * pfile;

#ifdef DEBUG
//fprintf(STDAUX, "CheckArcSpec: %s\n\r", pszFile);
#endif

   // Check if we have the compressor configuration file specification

   if (!pszFile || !pszFile[0]) {
     WriteLog("! Missing compressor configuration file specification in Squish.cfg\n");
     exit(EXIT_FAILURE);
   }

   // Open the compressor configuration file

   if ((pfile = fopen(pszFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszFile);
     exit(EXIT_FAILURE);
   } else
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Scanning file: %s\n", pszFile);

   // Scan in the compressor configuration file

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

     // Skip over the keyword and fix its end

     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (!*pchEnd) continue;
     *pchEnd++ = '\0';

     // Check for for the operating system modifier and skip over it

     if (!xstricmp(pch, "DOS") || !xstricmp(pch, "OS2")) {
       pch = pchEnd;
       if (!SkipSpaces(&pch)) continue;
       for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
       if (!*pchEnd) continue;
       *pchEnd++ = '\0';
     }

     // Check if this is an Archiver keyword and skip if not

     if (xstricmp(pch, "Archiver")) continue;

     // Skip over the leading spaces

     pch = pchEnd;
     if (!SkipSpaces(&pch)) continue;

     // Add available archiver name

     if (!AddLsz(&cfg.plszAvailArc, pch, -1, LST_END)) {
       WriteLog("! Insufficient memory (archiver list)\n");
       exit(EXIT_FAILURE);
     } else {
#ifdef DEBUG
//fprintf(STDAUX, "CheckArcSpec: Archiver\t%s\n\r", pch);
#endif
     }
   }

   fclose(pfile);

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszFile);

   return TRUE;
 }

/*
 * This routine fixes packer link addresses in packer list
 */

 VOID APPENTRY CheckArcSpec(VOID)
 {
   PLSZ plsz;

   // Check if we're not set up for packer manipulation and leave,
   // otherwise load the available packer list from Compress.cfg

   if (cfg.plszAllowArc != NULL)
     LoadArcSpec();
   else
     return;

   // Scan through the allowed packer list matching each element
   // agains the available packers list

   for (plsz = cfg.plszAllowArc; plsz != NULL; plsz = plsz->plszNext)
     if (!GetLsz(cfg.plszAvailArc, plsz->ach))
       WriteLog("! Archiver %s is not found in %s\n",
                 plsz->ach, cfg.achArcFile);
 }

/*
 * End of SQAARC.C
 */
