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
 * Config file management routines
 *
 * Created: 06/Jan/92
 * Updated: 13/Jan/00
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

 #include "sqafix.h"

 #include "pathdef.h"

 // Comment the following line to use linear search of config keywords

 #define USE_BSEARCH

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   d e c l a r a t i o n s                                   //
/////////////////////////////////////////////////////////////////////////////

 // Config keyword parser table element structure

 typedef struct {
   PSZ psz;                             // Keyword string
   ULONG hash;                          // Keyword hash value
   VOID (SUBENTRY * pfn)(PSZ psz);      // Parser func pointer
 } TAB, FAR * PTAB;

 // Keywords common for Squish and SqaFix config files

 #define SQXXNEWAREAPLACE       ";NewAreaPlace"
 #define SQXXECHOAREA           "EchoArea"
 #define SQXXINCLUDE            "Include"

 // Parcer functions common for both Squish and SqaFix configs

 static VOID SUBENTRY DoSqxxInclude(PSZ psz);

 // SqaFix config keyword parser function prototypes

 static VOID SUBENTRY DoSqafAddress(PSZ psz);
 static VOID SUBENTRY DoSqafAlias(PSZ psz);
 static VOID SUBENTRY DoSqafIgnoreMsgsFrom(PSZ psz);
 static VOID SUBENTRY DoSqafIgnoreKeyword(PSZ psz);
 static VOID SUBENTRY DoSqafMailPath(PSZ psz);
 static VOID SUBENTRY DoSqafSqshCfg(PSZ psz);
 static VOID SUBENTRY DoSqafQueueFile(PSZ psz);
 static VOID SUBENTRY DoSqafSwapPath(PSZ psz);
 static VOID SUBENTRY DoSqafSwapEMS(PSZ psz);
 static VOID SUBENTRY DoSqafSwapXMS(PSZ psz);
 static VOID SUBENTRY DoSqafRescan(PSZ psz);
 static VOID SUBENTRY DoSqafRescanPThru(PSZ psz);
 static VOID SUBENTRY DoSqafAllowPackerReq(PSZ psz);
 static VOID SUBENTRY DoSqafLogFile(PSZ psz);
 static VOID SUBENTRY DoSqafDetailedLog(PSZ psz);
 static VOID SUBENTRY DoSqafHelpExt(PSZ psz);
 static VOID SUBENTRY DoSqafSortAreaTag(PSZ psz);
 static VOID SUBENTRY DoSqafSortAreaGroup(PSZ psz);
 static VOID SUBENTRY DoSqafKeepFailed(PSZ psz);
 static VOID SUBENTRY DoSqafMaxMsgLineLeng(PSZ psz);
 static VOID SUBENTRY DoSqafMaxMsgPartLeng(PSZ psz);
 static VOID SUBENTRY DoSqafUseMSGID(PSZ psz);
 static VOID SUBENTRY DoSqafUseREPLY(PSZ psz);
 static VOID SUBENTRY DoSqafForceINTL(PSZ psz);
 static VOID SUBENTRY DoSqafAddOrigin(PSZ psz);
 static VOID SUBENTRY DoSqafAddTearLine(PSZ psz);
 static VOID SUBENTRY DoSqafUseFullAddr(PSZ psz);
 static VOID SUBENTRY DoSqafPresDestrAreas(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaPath(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaNodes(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaNotify(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaFlags(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaGroup(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaRefuse(PSZ psz);
 static VOID SUBENTRY DoSqafNewAreaPlace(PSZ psz);
 static VOID SUBENTRY DoSqafAreaDescrFileNA(PSZ psz);
 static VOID SUBENTRY DoSqafAreaDescrFileDZ(PSZ psz);
 static VOID SUBENTRY DoSqafAreaDescrSaveFile(PSZ psz);
 static VOID SUBENTRY DoSqafAreaListSaveFile(PSZ psz);
 static VOID SUBENTRY DoSqafDefAreaDescr(PSZ psz);
 static VOID SUBENTRY DoSqafDefAreaFlags(PSZ psz);
 static VOID SUBENTRY DoSqafEchoArea(PSZ psz);
 static VOID SUBENTRY DoSqafPasswordList(PSZ psz);
 static VOID SUBENTRY DoSqafNode(PSZ psz);
 static VOID SUBENTRY DoSqafDefNodeGroups(PSZ psz);
 static VOID SUBENTRY DoSqafDefNodeFlags(PSZ psz);
 static VOID SUBENTRY DoSqafUpLink(PSZ psz);
 static VOID SUBENTRY DoSqafFReqRefuse(PSZ psz);
 static VOID SUBENTRY DoSqafFReqTimeout(PSZ psz);
 static VOID SUBENTRY DoSqafFReqNotify(PSZ psz);
 static VOID SUBENTRY DoSqafFReqRetryAll(PSZ psz);
 static VOID SUBENTRY DoSqafFReqKeepAreas(PSZ psz);
 static VOID SUBENTRY DoSqafIdleKeepAreas(PSZ psz);
 static VOID SUBENTRY DoSqafIdleTimeout(PSZ psz);
 static VOID SUBENTRY DoSqafIdleNotify(PSZ psz);
 static VOID SUBENTRY DoSqafIdleIgnoreNodes(PSZ psz);
 static VOID SUBENTRY DoSqafIdleIgnorePassive(PSZ psz);

 // SqaFix config keyword parser table

 TAB atabSqaf[] = {
    SQXXECHOAREA,               0, DoSqafEchoArea, // checked first to speedup
    SQXXINCLUDE,                0, DoSqxxInclude,
   "Address",                   0, DoSqafAddress,
   "Alias",                     0, DoSqafAlias,
   "IgnoreMsgsFrom",            0, DoSqafIgnoreMsgsFrom,
   "IgnoreKeyword",             0, DoSqafIgnoreKeyword,
   "NetMail",                   0, DoSqafMailPath,
   "SquishCfg",                 0, DoSqafSqshCfg,
   "QueueFile",                 0, DoSqafQueueFile,
   "SwapPath",                  0, DoSqafSwapPath,
   "SwapEMS",                   0, DoSqafSwapEMS,
   "SwapXMS",                   0, DoSqafSwapXMS,
   "RescanCommand",             0, DoSqafRescan,
   "RescanPassthru",            0, DoSqafRescanPThru,
   "AllowPackerRequest",        0, DoSqafAllowPackerReq,
   "LogFile",                   0, DoSqafLogFile,
   "DetailedLog",               0, DoSqafDetailedLog,
   "HelpFileExt",               0, DoSqafHelpExt,
   "SortAreaTag",               0, DoSqafSortAreaTag,
   "SortAreaGroup",             0, DoSqafSortAreaGroup,
   "KeepFailedRequests",        0, DoSqafKeepFailed,
   "MaxMsgLineLength",          0, DoSqafMaxMsgLineLeng,
   "MaxMsgPartLength",          0, DoSqafMaxMsgPartLeng,
   "UseMSGID",                  0, DoSqafUseMSGID,
   "UseREPLY",                  0, DoSqafUseREPLY,
   "ForceINTL",                 0, DoSqafForceINTL,
   "AddTearLine",               0, DoSqafAddTearLine,
   "AddOrigin",                 0, DoSqafAddOrigin,
   "UseFullAddr",               0, DoSqafUseFullAddr,
   "PreserveDestroyedAreas",    0, DoSqafPresDestrAreas,
   "PreserveDeletedAreas",      0, DoSqafPresDestrAreas,        // Old keyword
   "NewAreaPath",               0, DoSqafNewAreaPath,
   "NewAreaNodes",               0, DoSqafNewAreaNodes,
   "NewAreaNotify",             0, DoSqafNewAreaNotify,
   "NewAreaFlags",              0, DoSqafNewAreaFlags,
   "NewAreaGroup",              0, DoSqafNewAreaGroup,
   "NewAreaRefuse",             0, DoSqafNewAreaRefuse,
    SQXXNEWAREAPLACE,           0, DoSqafNewAreaPlace,
   "AreaDescrFileNA",           0, DoSqafAreaDescrFileNA,
   "AreaDescrFileDZ",           0, DoSqafAreaDescrFileDZ,
   "AreaDescrSaveFile",         0, DoSqafAreaDescrSaveFile,
   "AreaListSaveFile",          0, DoSqafAreaListSaveFile,
   "DefaultAreaDescr",          0, DoSqafDefAreaDescr,
   "DefaultAreaFlags",          0, DoSqafDefAreaFlags,
   "DefaultNodeGroups",         0, DoSqafDefNodeGroups,
   "DefaultNodeFlags",          0, DoSqafDefNodeFlags,
   "PasswordList",              0, DoSqafPasswordList,
   "Node",                      0, DoSqafNode,
   "UpLink",                    0, DoSqafUpLink,
   "ForwardRequestRefuse",      0, DoSqafFReqRefuse,
   "ForwardRequestTimeout",     0, DoSqafFReqTimeout,
   "ForwardRequestNotify",      0, DoSqafFReqNotify,
   "ForwardRequestRetryAll",    0, DoSqafFReqRetryAll,
   "ForwardRequestKeepAreas",   0, DoSqafFReqKeepAreas,
   "KeepIdlePassthru",          0, DoSqafIdleKeepAreas,         // Old keyword
   "IdlePassthruKeepAreas",     0, DoSqafIdleKeepAreas,
   "IdlePassthruTimeout",       0, DoSqafIdleTimeout,
   "IdlePassthruNotify",        0, DoSqafIdleNotify,
   "IdlePassthruIgnoreNodes",   0, DoSqafIdleIgnoreNodes,
   "IdlePassthruIgnorePassive", 0, DoSqafIdleIgnorePassive,
 };

 // Squish config keyword parser function prototypes

 static VOID SUBENTRY DoSqshAddress(PSZ psz);
 static VOID SUBENTRY DoSqshPointNet(PSZ psz);
 static VOID SUBENTRY DoSqshOrigin(PSZ psz);
 static VOID SUBENTRY DoSqshCompress(PSZ psz);
 static VOID SUBENTRY DoSqshPack(PSZ psz);
 static VOID SUBENTRY DoSqshDefaultPacker(PSZ psz);
 static VOID SUBENTRY DoSqshAreasBbs(PSZ psz);
 static VOID SUBENTRY DoSqshNetArea(PSZ psz);
 static VOID SUBENTRY DoSqshBadArea(PSZ psz);
 static VOID SUBENTRY DoSqshDupeArea(PSZ psz);
 static VOID SUBENTRY DoSqshEchoArea(PSZ psz);
 static VOID SUBENTRY DoSqshNewAreaPlace(PSZ psz);

 // Squish config reused keywords

 #define SQSHPACK       "Pack"
 #define SQSHDEFPACKER  "DefaultPacker"

 // Squish config keyword parser table

 TAB atabSqsh[] = {
    SQXXECHOAREA,               0, DoSqshEchoArea, // checked first to speedup
    SQXXINCLUDE,                0, DoSqxxInclude,
   "Address",                   0, DoSqshAddress,
   "PointNet",                  0, DoSqshPointNet,
   "Origin",                    0, DoSqshOrigin,
   "Compress",                  0, DoSqshCompress,
    SQSHPACK,                   0, DoSqshPack,
    SQSHDEFPACKER,              0, DoSqshDefaultPacker,
   "AreasBbs",                  0, DoSqshAreasBbs,
   "NetArea",                   0, DoSqshNetArea,
   "BadArea",                   0, DoSqshBadArea,
   "DupeArea",                  0, DoSqshDupeArea,
    SQXXNEWAREAPLACE,           0, DoSqshNewAreaPlace,
 };

 // Must have keyword definitions

 #define CF_DEFADDR     0x0001  // Address keyword defined
 #define CF_DEFMAIL     0x0002  // NetMail keyword defined
 #define CF_DEFSQSH     0x0004  // SquishCfg keyword defined
 #define CF_DEFQUEF     0x0008  // SqafixQue keyword defined
 #define CF_SQSHECHO    0x0010  // Squish EchoArea keyword defined
 #define CF_SQAFECHO    0x0100  // Sqafix EchoArea keyword defined
 #define CF_SQAFNODE    0x0200  // Sqafix Node keyword defined
 #define CF_SQAFUPLN    0x0400  // Sqafix UpLink keyword defined

 static USHORT fsCfgScan;       // Config file flags (CF_)
 static ULONG iLine;            // Config file line counter

 static PTAB ptabSqafNewAreaPlace;      // SqaFix NewAreaPlace keytab entry
 static PTAB ptabSqshNewAreaPlace;      // Squish NewAreaPlace keytab entry

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   s u b r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine displays an error message
 */

 static VOID DoLineError(PSZ pszFormat, ...)
 {
   va_list argptr;
   CHAR achText[1024];

   va_start(argptr, pszFormat);
   vsprintf(achText, pszFormat, argptr);
   WriteLog("! Line %lu: %s", iLine, achText);
   va_end(argptr);
 }

/*
 * This subroutine compares two key tab's hash values for qsort()
 */
#ifdef USE_BSEARCH

 static int DoCompTab2Tab(const void * ptab1, const void * ptab2)
 {
   LONG diff = (LONG)(((PTAB)ptab1)->hash) - (LONG)(((PTAB)ptab2)->hash);
   if (diff < 0 ) return -1;
   if (diff > 0 ) return +1;
   return 0;
 }
#endif

/*
 * This subroutine compares hash to key tab hash for bsearch()
 */
#ifdef USE_BSEARCH

 static int DoCompHash2Tab(const void * phash, const void * ptab)
 {
   LONG diff = *((PLONG)phash) - (LONG)(((PTAB)ptab)->hash);
   if (diff < 0 ) return -1;
   if (diff > 0 ) return +1;
   return 0;
 }
#endif

/*
 * This subroutine sets key hash values in the given key table
 */

 static VOID SUBENTRY DoMakeKeyHash(TAB atab[], USHORT ctab)
 {
   USHORT itab;
   PTAB ptab;
#ifdef USE_BSEARCH
   ULONG hash;
#endif

   // Calculate the keyword hash values

   for (itab = ctab, ptab = atab; itab--; ptab++)
     ptab->hash = CalcHash(ptab->psz);

#ifdef USE_BSEARCH

   // Sort the table by the hash values

   qsort(atab, ctab, sizeof(TAB), DoCompTab2Tab);

   // Scan the sorted hash table matching the adjasent hash values
   // to make sure they are unique

   for (itab = ctab, ptab = atab, hash = -1; itab--; ptab++)
     if (hash == ptab->hash) {
       WriteLog("! Hash clash for %s and %s (0x%08lX)\n",
               (ptab - 1)->psz, ptab->psz, hash);
       exit(EXIT_FAILURE);
     } else {
//     WriteLog("+ Hash 0x%08lX %s\t[%09Fp]\n", ptab->hash, ptab->psz, ptab->pfn);
       hash = ptab->hash;
     }
#endif
 }

/*
 * This subroutine searches for the specified key in given key table
 */

 static PTAB SUBENTRY DoSearchKey(PTAB ptab, USHORT ctab, PSZ pszKey)
 {
   ULONG hash = CalcHash(pszKey);

#ifdef USE_BSEARCH

   // Perform a binary search for the given hash value

   ptab = bsearch(&hash, ptab, ctab, sizeof(TAB), DoCompHash2Tab);

   // Check if we got a hash match and verify it

   if (ptab != NULL && !xstricmp(ptab->psz, pszKey))
     return ptab;

#else

   // Perform a linear search for the given key

   for (; ctab--; ptab++)
     if (ptab->hash == hash && !xstricmp(ptab->psz, pszKey))
       return ptab;

#endif

   return NULL;
 }

/////////////////////////////////////////////////////////////////////////////
// S q a F i x   c o n f i g   p a r s e r   s u b r o u t i n e s         //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine to process the 'Address' keyword
 */

 static VOID SUBENTRY DoSqafAddress(PSZ psz)
 {
   NETADDR netAddr = {0,0,0,0};
   SHORT iAddr;
   PCH pch;

   // Skip to the first address slot available if any

   for (iAddr = 0; iAddr < numbof(cfg.anetAddr)
        && cfg.anetAddr[iAddr].zone != 0; iAddr++);

   // Scan in all the specified aka's

   for (; SkipSpaces(&psz); iAddr++)
     if (iAddr >= numbof(cfg.anetAddr))  {
       DoLineError("Too many addresses: '%s'\n", psz);
       exit(EXIT_FAILURE);
     } else
       if ((pch = ScanNetAddr(&netAddr, psz)) == NULL) {
         DoLineError("Invalid address: '%s'\n", psz);
         exit(EXIT_FAILURE);
       } else {
         xmemcpy(&cfg.anetAddr[iAddr], &netAddr, sizeof(NETADDR));
         psz = pch;
       }

   // Set the default address for new areas info address evaluation

   newareaDef.netAddr = cfg.anetAddr[0];

   // Note that we've got an address

   fsCfgScan|= CF_DEFADDR;
 }

/*
 * This subroutine to process the 'Alias' keyword
 */

 static VOID SUBENTRY DoSqafAlias(PSZ psz)
 {
   // Add this new alias to the alias strings list

   AddApsz(cfg.apszAlias, numbof(cfg.apszAlias), psz);
 }

/*
 * This subroutine to process the 'IgnoreMsgsFrom' keyword
 */

 static VOID SUBENTRY DoSqafIgnoreMsgsFrom(PSZ psz)
 {
   // Add this new name to the list of names to ignore

   AddApsz(cfg.apszIgnore, numbof(cfg.apszIgnore), psz);
 }

/*
 * This subroutine to process the 'IgnoreKeyword' keyword
 */

 static VOID SUBENTRY DoSqafIgnoreKeyword(PSZ psz)
 {
   USHORT cch;
   PCH pch;

   // Fix up the end of the specified keyword

   for (pch = psz; *pch && !isspace(*pch); pch++);
   *pch = '\0'; cch = xstrlen(psz);

   // Create new ignore keyword list element and check if ok

   if (cch > 0 && !AddLsz(&cfg.plszIgnoreKeyFirst, psz, cch, LST_END)) {
     DoLineError("Insufficient memory (ignore keyword)\n");
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'NetMail' keyword
 */

 static VOID SUBENTRY DoSqafMailPath(PSZ psz)
 {
   // Check if already defined

   if (fsCfgScan & CF_DEFMAIL) {
     DoLineError("'NetMail' keyword is already defined\n");
     exit(EXIT_FAILURE);
   } else
     fsCfgScan|= CF_DEFMAIL;

   // Check if the netmail path is a SQ! database

   if (*psz == '$') {
     cfg.fl|= FL_SQUISHNETMAIL;
     psz++;
   }

   // Copy the netmail path specification and make it uppercase

   xstrncpy(cfg.achNetMail, psz, lengof(cfg.achNetMail));
#ifndef UNIX
   xstrupr(cfg.achNetMail);
#endif

   // Check if we got something and make it fully qualified file name

   if (!cfg.achNetMail[0] ||
       !BuildFullPath(cfg.achNetMail, cfg.achNetMail)) {
     DoLineError("Invalid netmail path: %s\n", cfg.achNetMail);
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'SquishCfg' keyword
 */

 static VOID SUBENTRY DoSqafSqshCfg(PSZ psz)
 {
   // Check if already defined

   if (fsCfgScan & CF_DEFSQSH) {
     DoLineError("'SquishCfg' keyword is already defined\n");
     exit(EXIT_FAILURE);
   } else
     fsCfgScan|= CF_DEFSQSH;

   // Copy the Squish config file specification if it is not
   // overriden previously and append the default extension

   if (!cfg.achCfgSqsh[0]) xstrncpy(cfg.achCfgSqsh, psz, lengof(cfg.achCfgSqsh));
   AppendFileExt(cfg.achCfgSqsh, cfg.achCfgSqsh, DEF_CFG_EXT, FALSE);

   // Check if we got something and make it fully qualified file name

   if (!cfg.achCfgSqsh[0] || !BuildFullPath(cfg.achCfgSqsh, cfg.achCfgSqsh)) {
     DoLineError("Invalid Squish config path: %s\n", cfg.achCfgSqsh);
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'QueueFile' keyword
 */

 static VOID SUBENTRY DoSqafQueueFile(PSZ psz)
 {
   // Check if we have seen this keyword before

   if (fsCfgScan & CF_DEFQUEF) {
     DoLineError("'QueueFile' keyword is already defined\n");
     exit(EXIT_FAILURE);
   } else
     fsCfgScan|= CF_DEFQUEF;

   // Copy the SqaFix queue file specification if it is not
   // overriden previously and append the default extension

   if (!cfg.achQueFile[0]) xstrncpy(cfg.achQueFile, psz, lengof(cfg.achQueFile));
   AppendFileExt(cfg.achQueFile, cfg.achQueFile, DEF_QUE_EXT, FALSE);

   // Check if we got something and make it fully qualified file name

   if (!cfg.achQueFile[0] || !BuildFullPath(cfg.achQueFile, cfg.achQueFile)) {
     DoLineError("Invalid queue file path: %s\n", cfg.achQueFile);
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'SwapPath' keyword
 */

 static VOID SUBENTRY DoSqafSwapPath(PSZ psz)
 {
#ifdef __SPAWNO__

   // Check if we have the swap path overridden by the environment variable

   if (cfg.pszSwapPath != NULL) return;

   // Store the swap path and check if ok

   if ((cfg.pszSwapPath = AllocString(psz, -1)) == NULL) {
     DoLineError("Insufficient memory (swap path)\n");
     exit(EXIT_FAILURE);
   }

   // Set the 'dos swap allowed' flag

   cfg.fsSwapSpawn|= SWAP_DISK;

#endif
 }

/*
 * This subroutine to process the 'SwapEMS' keyword
 */

 static VOID SUBENTRY DoSqafSwapEMS(PSZ psz)
 {
#ifdef __SPAWNO__
   cfg.fsSwapSpawn|= SWAP_EMS;
#endif
 }

/*
 * This subroutine to process the 'SwapXMS' keyword
 */

 static VOID SUBENTRY DoSqafSwapXMS(PSZ psz)
 {
#ifdef __SPAWNO__
   cfg.fsSwapSpawn|= SWAP_XMS;
#endif
 }

/*
 * This subroutine to process the 'RescanCommand' keyword
 */

 static VOID SUBENTRY DoSqafRescan(PSZ psz)
 {
   // Check if we have seen this line before

   if (cfg.pszRescanCommand != NULL) {
     DoLineError("Duplicate rescan command definition\n");
     exit(EXIT_FAILURE);
   }

   // Store the rescan command and check if ok

   if ((cfg.pszRescanCommand = AllocString(psz, -1)) == NULL) {
     DoLineError("Insufficient memory (rescan command)\n");
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'RescanPassthru' keyword
 */

 static VOID SUBENTRY DoSqafRescanPThru(PSZ psz)
 {
   cfg.fl|= FL_RESCANPTHRUAREAS;
 }

/*
 * This subroutine to process the 'AllowPackerRequest' keyword
 */

 static VOID SUBENTRY DoSqafAllowPackerReq(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;

   // Scan through all the packer name specifications

   pch = psz;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Add the allowed packer name string list

     if (!AddLsz(&cfg.plszAllowArc, pch, -1, LST_END)) {
       DoLineError("Insufficient memory (allowed packer)\n");
       exit(EXIT_FAILURE);
     }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'LogFile' keyword
 */

 static VOID SUBENTRY DoSqafLogFile(PSZ psz)
 {
   // Copy the log file specification if it was not
   // overriden previously and append the default extension

   if (!cfg.achLogFile[0]) xstrncpy(cfg.achLogFile, psz, lengof(cfg.achLogFile));
   AppendFileExt(cfg.achLogFile, cfg.achLogFile, DEF_LOG_EXT, FALSE);

   // Check if we got something and make it fully qualified file name
   // Note that it's important to have fully qualified name since log
   // may be opened while the current drive or dir is not the same as
   // the startup one in which case we'll have it in the incorrect
   // location for relative specification in config

   if (!cfg.achLogFile[0] || !BuildFullPath(cfg.achLogFile, cfg.achLogFile)) {
     DoLineError("Invalid log file path: %s\n", cfg.achLogFile);
     exit(EXIT_FAILURE);
   }

   // The rest is done in the WriteLog(); routine
 }

/*
 * This subroutine to process the 'DetailedLog' keyword
 */

 static VOID SUBENTRY DoSqafDetailedLog(PSZ psz)
 {
   cfg.fl|= FL_DETAILEDLOG;
 }

/*
 * This subroutine to process the 'HelpFileExt' keyword
 */

 static VOID SUBENTRY DoSqafHelpExt(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;

   // Scan through all the help file extension specifications

   pch = psz;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Add the help file extension string list

     if (!AddLsz(&cfg.plszHelpExtFirst, pch, -1, LST_END)) {
       DoLineError("Insufficient memory (help file ext)\n");
       exit(EXIT_FAILURE);
     }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'SortAreaTag' keyword
 */

 static VOID SUBENTRY DoSqafSortAreaTag(PSZ psz)
 {
   cfg.fl|= FL_SORTAREATAG;
 }

/*
 * This subroutine to process the 'SortAreaGroup' keyword
 */

 static VOID SUBENTRY DoSqafSortAreaGroup(PSZ psz)
 {
   cfg.fl|= FL_SORTAREAGROUP;
 }

/*
 * This subroutine to process the 'KeepFailedRequests' keyword
 */

 static VOID SUBENTRY DoSqafKeepFailed(PSZ psz)
 {
   cfg.fl|= FL_KEEPFAILEDREQ;
 }

/*
 * This subroutine to process the 'UseMSGID' keyword
 */

 static VOID SUBENTRY DoSqafUseMSGID(PSZ psz)
 {
   cfg.fl|= FL_USEMSGIDKLUDGE;
 }

/*
 * This subroutine to process the 'UseREPLY' keyword
 */

 static VOID SUBENTRY DoSqafUseREPLY(PSZ psz)
 {
   cfg.fl|= FL_USEREPLYKLUDGE;
 }

/*
 * This subroutine to process the 'ForceINTL' keyword
 */

 static VOID SUBENTRY DoSqafForceINTL(PSZ psz)
 {
   cfg.fl|= FL_FORCEINTLKLUDGE;
 }

/*
 * This subroutine to process the 'AddOriginLine' keyword
 */

 static VOID SUBENTRY DoSqafAddOrigin(PSZ psz)
 {
   cfg.fl|= FL_ADDORIGINLINE;
 }

/*
 * This subroutine to process the 'AddTearLine' keyword
 */

 static VOID SUBENTRY DoSqafAddTearLine(PSZ psz)
 {
   cfg.fl|= FL_ADDTEARLINE;
 }

/*
 * This subroutine to process the 'UseFullAddr' keyword
 */

 static VOID SUBENTRY DoSqafUseFullAddr(PSZ psz)
 {
   cfg.fl|= FL_USEFULLNETADDR;
 }

/*
 * This subroutine to process the 'MaxMsgLineLength' keyword
 */

 static VOID SUBENTRY DoSqafMaxMsgLineLeng(PSZ psz)
 {
   cfg.cchMaxMsgLine = atoi(psz);

   // Validate the message line length specification

   if (cfg.cchMaxMsgLine < MIN_MSG_LINE)
     cfg.cchMaxMsgLine = DEF_MSG_LINE;
 }

/*
 * This subroutine to process the 'MaxMsgPartLength' keyword
 */

 static VOID SUBENTRY DoSqafMaxMsgPartLeng(PSZ psz)
 {
   cfg.cchMaxMsgPart = atoi(psz);

   // Validate the message part length specification

   if (cfg.cchMaxMsgPart < 1024 || cfg.cchMaxMsgPart > cchBuf)
     cfg.cchMaxMsgPart = cchBuf;
 }

/*
 * This subroutine to process the 'PreserveDestroyedAreas' keyword
 */

 static VOID SUBENTRY DoSqafPresDestrAreas(PSZ psz)
 {
   cfg.fl|= FL_PRESERVEDESTRAREAS;
 }

/*
 * This subroutine returns a new area info pointer
 */

 PNEWAREA GetNewAreaInfo(PSZ * ppsz)
 {
   NETADDR netAddr = newareaDef.netAddr;
   PSZ psz, pszArea = NULL;
   PNEWAREA pnewarea;
   USHORT cch;
   PCH pch;

   // Check if there is trigger node or area mask specification and if not,
   // return default new area info pointer

   if (SkipSpaces(ppsz) != '=')
     return &newareaDef;
   else
     psz = *ppsz + 1;

   // Skip over node address or area mask specification and fix its end

   for (pch = psz, cch = 0; *pch && !isspace(*pch); pch++, cch++);
   if (!*pch) {
     *ppsz = pch;
   } else {
     *pch = '\0'; *ppsz = pch + 1; SkipSpaces(ppsz);
   }

   // Scan in the trigger node address specification and check if ok

   if ((pch = ScanNetAddr(&netAddr, psz)) != NULL) {

     // Scan through the existing new area info blocks looking for the
     // trigger node match

     for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
          pnewarea = pnewarea->pnewareaNext)
       if (pnewarea->pszArea == NULL &&
          !CompNetAddr(&netAddr, &pnewarea->netAddr))
         return pnewarea;

   } else {
     xmemset(&netAddr, 0, sizeof(NETADDR));

     // Scan through the existing new area info blocks looking for the
     // trigger areamask match

     for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
          pnewarea = pnewarea->pnewareaNext)
       if (pnewarea->pszArea != NULL &&
          !xstricmp(psz, pnewarea->pszArea))
         return pnewarea;

     // No match found, so allocate new areatag mask

     if ((pszArea = AllocString(psz, -1)) == NULL) {
       DoLineError("Insufficient memory (new area mask)\n");
       exit(EXIT_FAILURE);
     }
   }

   // Create the new area info block and check if ok

   if ((pnewarea = (PNEWAREA) LstCreateElement(sizeof(NEWAREA))) == NULL) {
     DoLineError("Insufficient memory (new area info)\n");
     exit(EXIT_FAILURE);
   } else {
     LstLinkElement((PPLE) &cfg.pnewareaFirst, (PLE) pnewarea, LST_END);
     xmemcpy(&pnewarea->netAddr, &netAddr, sizeof(NETADDR));
     pnewarea->pszArea = pszArea;
   }

   return pnewarea;
 }

/*
 * This subroutine to process the 'NewAreaPath' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaPath(PSZ psz)
 {
   PNEWAREA pnewarea;
   PCH pch, pchEnd;

   // Get the new area info structure

   pnewarea = GetNewAreaInfo(&psz);

   // Scan to the end of the new area path specification
   // and process the area tag to path conversion method if any

   for (pchEnd = psz; *pchEnd && !isspace(*pchEnd); pchEnd++);
   if ((pch = pchEnd) != '\0' && SkipSpaces(&pch)) {
     *pchEnd = '\0';
     if (!xstricmp(pch, "CRC")) {
       pnewarea->fs|= NA_CONVERTCRC;
     } else
     if (!xstricmp(pch, "TREE")) {
       pnewarea->fs|= NA_CONVERTTREE;
     } else
     if (!xstricmp(pch, "TREEDIR")) {
       pnewarea->fs|= NA_CONVERTTREEDIR;
     } else
     if (!xstricmp(pch, "HPFS")) {
#if defined(__OS2__) || defined(UNIX)
       pnewarea->fs|= NA_CONVERTHPFS;
#else
       DoLineError("Warning: HPFS conversion ignored in this version\n");
#endif
     } else
     if (!xstricmp(pch, "HPFSDIR")) {
#if defined(__OS2__) || defined(UNIX)
       pnewarea->fs|= NA_CONVERTHPFSDIR;
#else
       DoLineError("Warning: HPFSDIR conversion ignored in this version\n");
#endif
     } else
     if (!xstricmp(pch, "NTFS")) {
#if defined(__W32__) || defined(UNIX)
       pnewarea->fs|= NA_CONVERTNTFS;
#else
       DoLineError("Warning: NTFS conversion ignored in this version\n");
#endif
     } else
     if (!xstricmp(pch, "NTFSDIR")) {
#if defined(__W32__) || defined(UNIX)
       pnewarea->fs|= NA_CONVERTNTFSDIR;
#else
       DoLineError("Warning: NTFSDIR conversion ignored in this version\n");
#endif
     } else
     if (!xstricmp(pch, "LOWERCASE")) {
#if defined(__W32__) || defined(UNIX) || defined(__OS2__)
       pnewarea->fs|= NA_CONVERTLOWER;
# ifdef DEBUG
       DoLineError("DEBUG: LOWERCASE convertion option found in first position!\n");
# endif
#else
       DoLineError("Warning: LOWERCASE conversion ignored in this version\n");
#endif
     } else
       DoLineError("Warning: Unknown conversion\n");
   }

   // Copy the path specification and make it uppercase

   xstrncpy(pnewarea->achPath, psz, lengof(pnewarea->achPath));
#ifndef UNIX
   xstrupr(pnewarea->achPath);
#endif

   // Check if we got something and make it fully qualified file name
   // NOTE: Starting with 0.98a1 we don't insist an having the correct
   // path specified in NewAreaPath anymore

#if 0
   if (!pnewarea->achPath[0] ||
       !BuildFullPath(pnewarea->achPath, pnewarea->achPath)) {
     DoLineError("Invalid path: %s\n", pnewarea->achPath);
     exit(EXIT_FAILURE);
   }
#endif

   // Make sure that there is a trailing back slash

//   if (*(xstrchr(pnewarea->achPath, 0) - 1) != '\\')
   if (*(xstrchr(pnewarea->achPath, 0) - 1) != PATH_DELIM)
//     xstrcat(pnewarea->achPath, "\\");
     xstrcat(pnewarea->achPath, PATH_DELIMS);
 }

/*
 * This subroutine to process the 'NewAreaNodes' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaNodes(PSZ psz)
 {
   PNEWAREA pnewarea;
   NETADDR netAddr;
   SHORT iAddr;
   PCH pch;

   // Get the new area info structure

   pnewarea = GetNewAreaInfo(&psz);
   netAddr = newareaDef.netAddr;

   // Skip over the used address slots if any

   for (iAddr = 0; iAddr < numbof(pnewarea->anetNode)
        && pnewarea->anetNode[iAddr].zone != 0; iAddr++);

   // Scan in all the specified addresses

   for (; SkipSpaces(&psz); iAddr++)
     if (iAddr >= numbof(pnewarea->anetNode))  {
       DoLineError("Too many addresses: '%.128s'\n", psz);
       exit(EXIT_FAILURE);
     } else
       if ((pch = ScanNetAddr(&netAddr, psz)) == NULL) {
         DoLineError("Invalid address: '%.128s'\n", psz);
         exit(EXIT_FAILURE);
       } else {
         xmemcpy(&pnewarea->anetNode[iAddr], &netAddr, sizeof(NETADDR));
         psz = pch;
       }
 }

/*
 * This subroutine to process the 'NewAreaNotify' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaNotify(PSZ psz)
 {
   SHORT iAddr, iArea, iFile;
   PNEWAREA pnewarea;
   NETADDR netAddr;
   PCH pch;

   // Get the new area info structure

   pnewarea = GetNewAreaInfo(&psz);
   netAddr = newareaDef.netAddr;

   // Scan to the first available addr/area/file notify slots

   for (iAddr = 0; iAddr < numbof(pnewarea->anetNote)
        && pnewarea->anetNote[iAddr].zone != 0; iAddr++);

   for (iArea = 0; iArea < numbof(pnewarea->apszNote)
        && pnewarea->apszNote[iArea] != NULL; iArea++);

   for (iFile = 0; iFile < numbof(pnewarea->apszFile)
        && pnewarea->apszFile[iFile] != NULL; iFile++);

   // Scan in all the addr/area/file specified

   while (SkipSpaces(&psz))

     // Check if this is a file since addr can have @

     if (*psz == '@') {
       for (pch = ++psz; *pch && !isspace(*pch); pch++);
       if (pch == psz) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (iFile < numbof(pnewarea->apszFile))  {
         if ((pnewarea->apszFile[iFile] =
              AllocString(psz, (SHORT)(pch - psz))) == NULL) {
           DoLineError("Insufficient memory (notify file list)\n");
           exit(EXIT_FAILURE);
         } else {
#ifndef UNIX
           xstrupr(pnewarea->apszFile[iFile++]);
#endif
           psz = pch;
         }
       } else {
         DoLineError("Too many notify files: '%.128s'\n", psz);
         exit(EXIT_FAILURE);
       }
     } else

     // Check if this is an address

     if ((pch = ScanNetAddr(&netAddr, psz)) != NULL) {
       if (iAddr < numbof(pnewarea->anetNote))  {
         xmemcpy(&pnewarea->anetNote[iAddr++], &netAddr, sizeof(NETADDR));
         psz = pch;
       } else {
         DoLineError("Too many notify addresses: '%.128s'\n", psz);
         exit(EXIT_FAILURE);
       }
     } else {

       // So this must be an echo area

       for (pch = psz; *pch && !isspace(*pch); pch++);
       if (iArea < numbof(pnewarea->apszNote))  {
         if ((pnewarea->apszNote[iArea] =
              AllocString(psz, (SHORT)(pch - psz))) == NULL) {
           DoLineError("Insufficient memory (notify area list)\n");
           exit(EXIT_FAILURE);
         } else {
#ifndef UNIX
           xstrupr(pnewarea->apszNote[iArea++]);
#endif
           psz = pch;
         }
       } else {
         DoLineError("Too many notify areas: '%.128s'\n", psz);
         exit(EXIT_FAILURE);
       }
     }
 }

/*
 * This subroutine to process the 'NewAreaFlags' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaFlags(PSZ psz)
 {
   PNEWAREA pnewarea;
   SHORT cch;

   // Get the new area info structure

   pnewarea = GetNewAreaInfo(&psz);

   // Check if there is something and allocate string

   if ((cch = xstrlen(psz)) > 0)
     if ((pnewarea->pszFlags = AllocString(psz, cch)) == NULL) {
       DoLineError("Insufficient memory (new area flags)\n");
       exit(EXIT_FAILURE);
     }
 }

/*
 * This subroutine to process the 'NewAreaGroup' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaGroup(PSZ psz)
 {
   PNEWAREA pnewarea;

   // Get the new area info structure

   pnewarea = GetNewAreaInfo(&psz);

   // Check if there is a group specification

   if (!SkipSpaces(&psz)) {
     DoLineError("New area group missing\n");
     exit(EXIT_FAILURE);
   }

   // Validate and set the new area group specification

   if (!ISGROUP(*psz)) {
     DoLineError("Invalid new area group: '%c'\n", *psz);
     exit(EXIT_FAILURE);
   } else
     pnewarea->chGroup = toupper(*psz);
 }

/*
 * This subroutine to process the 'NewAreaRefuse' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaRefuse(PSZ psz)
 {
   PNEWAREA pnewarea;
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;

   // Get the new area info structure

   pnewarea = GetNewAreaInfo(&psz);

   // Scan through all the area masks specifications

   pch = psz;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if this is a reference file and process it, otherwise
     // add the area mask string list

     if (*pch == '@') {
       if (!pch[1] || isspace(pch[1])) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (!LoadAreaMaskFileNA(pch + 1, &pnewarea->plszRefuse)) {
         DoLineError("Can't load areamask file: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }
     } else
#ifndef UNIX
       if (!AddLsz(&pnewarea->plszRefuse, xstrupr(pch), -1, LST_END)) {
#else
       if (!AddLsz(&pnewarea->plszRefuse, pch, -1, LST_END)) {
#endif
         DoLineError("Insufficient memory (newarea refuse areamask)\n");
         exit(EXIT_FAILURE);
       }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'NewAreaPlace' keyword
 */

 static VOID SUBENTRY DoSqafNewAreaPlace(PSZ psz)
 {
   cfg.fl|= FL_SQAFNEWAREAPLACE;
 }

/*
 * This subroutine to process the 'AreaDescrFileNA' keyword
 */

 static VOID SUBENTRY DoSqafAreaDescrFileNA(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH pch;

   // Scan through all the area desription file specifications

   while (!fDone && SkipSpaces(&psz)) {
     for (pch = psz; *pch && !isspace(*pch); pch++);
     if (*pch) *pch = '\0';
     else fDone = TRUE;
     LoadAreaDescrFileNA(psz);
     psz = ++pch;
   }
 }

/*
 * This subroutine to process the 'AreaDescrFileDZ' keyword
 */

 static VOID SUBENTRY DoSqafAreaDescrFileDZ(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH pch;

   // Scan through all the area desription file specifications

   while (!fDone && SkipSpaces(&psz)) {
     for (pch = psz; *pch && !isspace(*pch); pch++);
     if (*pch) *pch = '\0';
     else fDone = TRUE;
     LoadAreaDescrFileDZ(psz);
     psz = ++pch;
   }
 }

/*
 * This subroutine to process the 'AreaDescrSaveFile' keyword
 */

 static VOID SUBENTRY DoSqafAreaDescrSaveFile(PSZ psz)
 {
   PCH pch;

   // Scan in the area description save file name

   if (SkipSpaces(&psz)) {
     for (pch = psz; *pch && !isspace(*pch); pch++);
     if (*pch) *pch = '\0';
     SaveAreaDescrFileNA(psz);
   }
 }

/*
 * This subroutine to process the 'AreaListSaveFile' keyword
 */

 static VOID SUBENTRY DoSqafAreaListSaveFile(PSZ psz)
 {
   USHORT cch;
   PCH pch;

   // Check if we have seen this keyword before

   if (cfg.pszAreaListSaveFile != NULL) {
     DoLineError("'AreaListSaveFile' keyword is already defined\n");
     exit(EXIT_FAILURE);
   }

   // Scan in the area list save file name

   if (SkipSpaces(&psz)) {
     for (pch = psz, cch = 0; *pch && !isspace(*pch); pch++, cch++);
     if (*pch) *pch = '\0';
   }

   // Check if we got something and if so, allocate the memory
   // to hold the arealist file name and set it into the config

   if (cch > 0)
     if ((cfg.pszAreaListSaveFile = AllocString(psz, cch)) == NULL) {
       DoLineError("Insufficient memory (area list file name)\n");
       exit(EXIT_FAILURE);
     }
 }

/*
 * This subroutine to process the 'DefaultAreaDescr' keyword
 */

 static VOID SUBENTRY DoSqafDefAreaDescr(PSZ psz)
 {
   SHORT cch;
   PCH pch;

   // Check if we have seen this keyword before

   if (cfg.pszDefAreaDescr != NULL) {
     DoLineError("'DefaultAreaDescr' keyword is already defined\n");
     exit(EXIT_FAILURE);
   }

   // Check if there is a leading double quota and skip over it,
   // otherwise start with the first non space character

   if (*psz == '"') psz++;

   // Scan 'till the trailing double quota or the end of the line
   // calculating the number of characters in the area description

   for (pch = psz, cch = 0; *pch && *pch != '"'; pch++, cch++);

   // Check if we got something and if so, allocate the memory
   // to hold the default area descriptor and set it into the config

   if (cch > 0)
     if ((cfg.pszDefAreaDescr = AllocString(psz, cch)) == NULL) {
       DoLineError("Insufficient memory (def area descr)\n");
       exit(EXIT_FAILURE);
     }
 }

/*
 * This subroutine to process the echo area flag
 */

 static BOOL SUBENTRY DoSqafAreaFlag(PCH pch, PAREA parea)
 {
   USHORT fs = 0, cch = 0;
   PUSHORT pfs, plevel;
   PSZ * ppszRules;

   // Check if we're setting particular area flags or the default area flags

   if (parea != NULL) {
     pfs = &parea->fs;
     plevel = &parea->level;
     ppszRules = &parea->pszRules;
   } else {
     pfs = &cfg.fsDefAreaFlags;
     plevel = &cfg.usDefAreaLevel;
     ppszRules = &cfg.pszDefAreaRules;
   }

   // Scan in the area flags specification

   switch (tolower(pch[1])) {
     case 'r': // allow rescan
               fs = AF_RESCANOK;
               break;
     case 'v': // visible to non authorized
               fs = AF_VISIBLE;
               break;
     case 'i': // echo area rules
               fs = AF_SENDRULES;
               for (cch = 0; pch[2 + cch] && !isspace(pch[2 + cch]); cch++);
               if (cch > 0 && (*ppszRules = AllocString(pch + 2, cch)) == NULL) {
                 DoLineError("Insufficient memory (rule file)\n");
                 exit(EXIT_FAILURE);
               }
               break;
     case 'l': // area access level
               for (cch = 0; isdigit(pch[2 + cch]); cch++);
               *plevel = atoi(pch + 2);
               break;
     default : return FALSE;
   }

   // Set or reset the scanned flag

   if (fs) {
     if (*pch == '+')
       *pfs|= fs;
     else
       *pfs&= ~fs;
   }

   // Return number of scanned in characters

   return 2 + cch;
 }

/*
 * This subroutine to process the 'DefaultAreaFlags' keyword
 */

 static VOID SUBENTRY DoSqafDefAreaFlags(PSZ psz)
 {
   USHORT cch;

   // Scan through all the area flags

   while (SkipSpaces(&psz))
     if (*psz != '+' && *psz != '-') {
       DoLineError("Invalid area flag: '%s'\n", psz);
       exit(EXIT_FAILURE);
     } else
       if ((cch = DoSqafAreaFlag(psz, NULL)) == 0) {
         DoLineError("Unknown area flag: '%s'\n", psz);
         exit(EXIT_FAILURE);
       } else {
         psz+= cch;
       }
 }

/*
 * This subroutine to process the 'EchoArea' keyword
 */

 static VOID SUBENTRY DoSqafEchoArea(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   PSZ pszArea, pszDescr;
   BOOL fMulti = FALSE;
   PCH pch, pchEnd;
   CHAR chGroup;
   PAREA parea;
   USHORT cch;

   // Check if we have seen this keyword before

   if (!(fsCfgScan & CF_SQAFECHO)) {
     fsCfgScan|= CF_SQAFECHO;
     if (cfg.fl & FL_VERBOSEMODE)
       printf("SqaFix config: scanning echo area definitions...\n");
   }

   // Check if there is an echo tag specification

   if (!*psz) {
     DoLineError("Echo area tag missing\n");
     exit(EXIT_FAILURE);
   }

   // Get past the end of the area tag and check if we got something

   for (pch = psz; !isspace(*pch); pch++)
     if (*pch == '\0') {
       DoLineError("Echo area group missing\n");
       exit(EXIT_FAILURE);
     }

   // Fix the area tag trailing null and check if we already
   // have seen the area definition with the same tag and if no,
   // create the new area descriptor

   *pch++ = '\0';
   if ((parea = GetAreaFromTag(psz)) != NULL) fMulti = TRUE;
   pszArea = psz;

   // Skip over the leading spaces and check if there is a valid area group

   chGroup = toupper(SkipSpaces(&pch));
   if (!ISGROUP(chGroup)) {
     DoLineError("Invalid echo area group: '%.128s'\n", pch);
     exit(EXIT_FAILURE);
   } else pch++;

   // Allocate area with this tag and group

   if (!fMulti) {
     parea = AddArea(pszArea, chGroup);
     parea->fs|= AF_INSQAFIXCFG;
   }

   // Store the new group id in the known group list if it's not yet there

   if (!fMulti) {
     if (!xstrchr(cfg.achAreaGroups, chGroup))
       cfg.achAreaGroups[xstrlen(cfg.achAreaGroups)] = chGroup;
   }

   // Skip over the area group and it's trailing spaces if any and
   // check if there is a valid area decription text specification

   if (SkipSpaces(&pch) == '"') {

     // Scan to the end of the echo area description text

     for (pchEnd = ++pch, cch = 0; *pchEnd && *pchEnd != '"'; pchEnd++, cch++);
     if (!*pchEnd) {
       DoLineError("Invalid echo area description: '%.128s'\n", pch);
       exit(EXIT_FAILURE);
     } else pchEnd++;

     // Check if we got something and if so, allocate the memory
     // to hold the area description and set it into the config.

     if (!fMulti && cch > 0) {
       if ((parea->pszDescr = AllocString(pch, cch)) == NULL) {
         DoLineError("Insufficient memory (area descr)\n");
         exit(EXIT_FAILURE);
       }
     }

     pch = pchEnd;
   }

   // Check if there is no description and try to locate it in
   // area description list

   if (!fMulti) {
     if (!parea->pszDescr || !parea->pszDescr[0] ||
          parea->pszDescr == cfg.pszDefAreaDescr)
       if ((pszDescr = GetAreaDescrFromTag(parea->achTag)) != NULL)
         parea->pszDescr = pszDescr;
   }

   // Scan in all the area flags if any

   if (!fMulti) {
     while (SkipSpaces(&pch) && (cch = DoSqafAreaFlag(pch, parea)) > 0)
       pch+= cch;
   } else {
     while (SkipSpaces(&pch) && (*pch == '+' || *pch == '-'))
       while (*pch && !isspace(*pch)) pch++;
   }

   // Save the links definition beginning pointer for future update

   if (!fMulti) parea->pchSqafLink = pch;

   // Scan it all the passive links if any. Check if there is an inline
   // comment and store its pointer

   while (SkipSpaces(&pch))
     if ((pchEnd = ScanNetAddr(&netAddr, pch)) == NULL ||
          netAddr.zone == 0 || netAddr.net == 0) {
       if (*pch == ';' || *pch == '%') {
         if (!fMulti) parea->pchSqafTail = pch;
         break;
       }
       DoLineError("Invalid address: '%.128s'\n", pch);
       exit(EXIT_FAILURE);
     } else {
       AddAreaLink(parea, &netAddr, PASSIVE, NULL);
       pch = pchEnd;
     }
 }

/*
 * This subroutine to process the 'DefaultNodeGroups' keyword
 */

 static VOID SUBENTRY DoSqafDefNodeGroups(PSZ psz)
 {
   CHAR ach[NUM_GROUPS + 1];
   USHORT cch;
   PCH pch;

   // Check if we have seen this keyword before

   if (cfg.pszDefNodeGroups != NULL) {
     DoLineError("'DefaultNodeGroups' keyword is already defined\n");
     exit(EXIT_FAILURE);
   }

   // Scan in the default node groups

   for (pch = psz, cch = 0; *pch && !isspace(*pch); pch++)
     if (!ISGROUP(*pch)) {
       DoLineError("Invalid group character: '%c'\n", *pch);
       exit(EXIT_FAILURE);
     } else
     if (xmemchr(ach, tolower(*pch), cch) == NULL &&
         xmemchr(ach, toupper(*pch), cch) == NULL)
       ach[cch++] = *pch;

   // Set the default groups specification string

   if ((cfg.pszDefNodeGroups = AllocString(ach, cch)) == NULL) {
     DoLineError("Insufficient memory (def node groups)\n");
     exit(EXIT_FAILURE);
   }

#ifdef DEBUG
//fprintf(STDAUX, "DoSqafDefNodeGroups: %s\r\n", cfg.pszDefNodeGroups);
#endif
 }

/*
 * This subroutine to process the node flag
 */

 static BOOL SUBENTRY DoSqafNodeFlag(PCH pch, PNODE pnode)
 {
   PUSHORT pfs, plevel, pmaxFreq, pmaxRescan;
   USHORT fs = 0, cch = 2;

   // Check if we're setting particular node flags or the default node flags

   if (pnode != NULL) {
     pfs = &pnode->fs;
     plevel = &pnode->level;
     pmaxFreq = &pnode->maxFreq;
     pmaxRescan = &pnode->maxRescan;
   } else {
     pfs = &cfg.fsDefNodeFlags;
     plevel = &cfg.usDefNodeLevel;
     pmaxFreq = &cfg.usDefNodeMaxFreq;
     pmaxRescan = &cfg.usDefNodeMaxRescan;
   }

   // Scan in the node flags specification

   switch (tolower(pch[1])) {
     case 'k': // reply kill/sent
               fs = NF_KILLSENT;
               break;
     case 'h': // reply hold
               fs = NF_SENDHOLD;
               break;
     case 'c': // reply crash
               fs = NF_SENDCRASH;
               break;
     case 'p': // keep processed requests
               fs = NF_KEEPREQS;
               break;
     case 'v': // show not allowed areas
               fs = NF_VISIBLE;
               break;
     case 'i': // send rules when linking up
               fs = NF_SENDRULES;
               break;
     case 'a': // allow areas autocreate
               fs = NF_AUTOCREATE;
               break;
     case 'f': // force area create
               fs = NF_SENDCREATE;
               break;
     case '!': // allow remote operations
               cch++;
               switch (tolower(pch[2])) {
                 case 'c': // allow %CREATE requests
                           fs = NF_AREACREATE;
                           break;
                 case 'd': // allow %DESTROY requests
                           fs = NF_AREADESTROY;
                           break;
                 default : return FALSE;
               }
               break;
     case 's': // allow %COMPRESS requests
               fs = NF_COMPRESSOK;
               break;
     case 'r': // allow %RESCAN requests
               fs = NF_RESCANOK;
               while (isdigit(pch[cch])) cch++;
               *pmaxRescan = atoi(pch + 2);
               break;
     case 'o': // allow requests forwarding and limit
               fs = NF_FORWARDREQ;
               while (isdigit(pch[cch])) cch++;
               *pmaxFreq = atoi(pch + 2);
               break;
     case 'n': // no passthru for freq areas
               fs = NF_FREQNOPTHRU;
               break;
     case 'y': // don't send area notifications
               fs = NF_DONTNOTIFY;
               break;
     case 'z': // don't send remote request reports
               fs = NF_DONTREPORT;
               break;
     case 'l': // node access level
               while (isdigit(pch[cch])) cch++;
               *plevel = atoi(pch + 2);
               break;
     default : return FALSE;
   }

   // Set or reset the scanned flag

   if (fs) {
     if (*pch == '+')
       *pfs|= fs;
     else
       *pfs&= ~fs;
   }

   // Return number of scanned in characters

   return cch;
 }

/*
 * This subroutine to process the 'DefaultNodeFlags' keyword
 */

 static VOID SUBENTRY DoSqafDefNodeFlags(PSZ psz)
 {
   USHORT cch;

   // Scan through all the node flags

   while (SkipSpaces(&psz))
     if (*psz != '+' && *psz != '-') {
       DoLineError("Invalid node flag: '%s'\n", psz);
       exit(EXIT_FAILURE);
     } else
       if ((cch = DoSqafNodeFlag(psz, NULL)) == 0) {
         DoLineError("Unknown node flag: '%s'\n", psz);
         exit(EXIT_FAILURE);
       } else {
         psz+= cch;
       }
 }

/*
 * This subroutine to process the 'PasswordList' keyword
 */

 static VOID SUBENTRY DoSqafPasswordList(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH pch;

   // Scan through all the password list file specifications

   while (!fDone && SkipSpaces(&psz)) {
     for (pch = psz; *pch && !isspace(*pch); pch++);
     if (*pch) *pch = '\0';
     else fDone = TRUE;
     LoadPwlFile(psz);
     psz = ++pch;
   }
 }

/*
 * This subroutine to process the 'Node' keyword
 */

 static VOID SUBENTRY DoSqafNode(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   CHAR ach[NUM_GROUPS + 1];
   USHORT cch, cchPassword;
   PSZ pszPassword;
   PCH pch, pchEnd;
   PNODE pnode;
   PPWL ppwl;

   // Check if we have seen this keyword before

   if (!(fsCfgScan & CF_SQAFNODE)) {
     fsCfgScan|= CF_SQAFNODE;
     if (cfg.fl & FL_VERBOSEMODE)
       printf("SqaFix config: scanning node definitions...\n");
   }

   // Check if we have an address and scan in the node address

   if (!*psz) {
     DoLineError("Node address missing\n");
     exit(EXIT_FAILURE);
   } else
     if ((pch = ScanNetAddr(&netAddr, psz)) == NULL) {
       DoLineError("Invalid address: '%s'\n", psz);
       exit(EXIT_FAILURE);
     }

   // Check if we have seen the node with the same address

   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext)
     if (!xmemcmp(&pnode->netAddr, &netAddr, sizeof(NETADDR))) {
       DoLineError("Node %s is already defined\n", FormatNetAddr(&netAddr));
       exit(EXIT_FAILURE);
     }

   // Check if there is a password and scan it in

   if (!SkipSpaces(&pch)) {
     DoLineError("Password missing for node %s\n", FormatNetAddr(&netAddr));
     exit(EXIT_FAILURE);
   } else {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if ((cch = (USHORT)(pchEnd - pch)) > MAX_PASS_LENG) {
       DoLineError("Password is too long for node %s\n", FormatNetAddr(&netAddr));
       exit(EXIT_FAILURE);
     } else {
       pszPassword = pch;
       cchPassword = cch;
     }
   }

   // Check if this password has to be taken from the password list

   if (cchPassword == 1 && *pszPassword == '*') {
     if ((ppwl = GetNodePwl(&netAddr)) == NULL) {
       DoLineError("Password for node %s is not found in password list\n", FormatNetAddr(&netAddr));
       exit(EXIT_FAILURE);
     } else {
       pszPassword = ppwl->achPassword;
       cchPassword = xstrlen(pszPassword);
     }
   }

   // Create the new node list element and link it in

   if ((pnode = (PNODE) LstCreateElement(sizeof(NODE) + cchPassword)) == NULL) {
     DoLineError("Insufficient memory (node list)\n");
     exit(EXIT_FAILURE);
   } else
     LstLinkElement((PPLE) &cfg.pnodeFirst, (PLE) pnode, LST_END);

   // Set the node address and password

   xmemcpy(&pnode->netAddr, &netAddr, sizeof(NETADDR));
   xmemcpy(pnode->achPassword, pszPassword, cchPassword);

   // Set the node default flags and level

   pnode->fs = cfg.fsDefNodeFlags;
   pnode->level = cfg.usDefNodeLevel;
   pnode->maxFreq = cfg.usDefNodeMaxFreq;
   pnode->maxRescan = cfg.usDefNodeMaxRescan;

   // Check if there is a group specification and scan it in

   pch = pchEnd;
   if (SkipSpaces(&pch) && (ISGROUP(*pch) || *pch == '*')) {
     for (pchEnd = pch, cch = 0; *pchEnd && !isspace(*pchEnd); pchEnd++)
       if (!(ISGROUP(*pchEnd) || *pchEnd == '*')) {
         DoLineError("Invalid group character: '%c'\n", *pchEnd);
         exit(EXIT_FAILURE);
       } else
       if (xmemchr(ach, tolower(*pchEnd), cch) == NULL &&
           xmemchr(ach, toupper(*pchEnd), cch) == NULL)
         ach[cch++] = *pchEnd;

     // Allocate the group list and check if ok

     if ((pnode->pszGroupSpec = AllocString(ach, cch)) == NULL) {
       DoLineError("Insufficient memory (node group spec)\n");
       exit(EXIT_FAILURE);
     }
   } else
     pchEnd = pch;

   // Scan in all the node flags if any

   pch = pchEnd;
   while (SkipSpaces(&pch) && (*pch == '+' || *pch == '-'))
     if ((cch = DoSqafNodeFlag(pch, pnode)) > 0) {
       pch+= cch;
     } else {
       DoLineError("Unknown node flag: '%s'\n", pch);
       exit(EXIT_FAILURE);
     }

   // Check if there is an optional sysop name specified and
   // scan it in if so

   if (*pch == '"') {
     for (pchEnd = ++pch; *pchEnd && *pchEnd != '"'; pchEnd++);
     if ((cch = (SHORT)(pchEnd - pch)) > 0)
       pnode->pszSysop = AllocString(pch, cch);
     if (*pchEnd == '"') pchEnd++;
   } else
     pchEnd = pch;

   // Check for extra characters

   if (SkipSpaces(&pchEnd) && *pchEnd != ';') {
     DoLineError("Extra characters: '%s'\n", pchEnd);
     exit(EXIT_FAILURE);
   }

   // Check if the +A flag is set for a point address

   if (pnode->netAddr.point != 0 && (pnode->fs & NF_AUTOCREATE)) {
     DoLineError("Autocreate flag can't be set for a point address %s\n", FormatNetAddr(&pnode->netAddr));
     exit(EXIT_FAILURE);
   }

#ifdef DEBUG
//fprintf(STDAUX, "DoSqafNode: %s\t%s\t%s\t0x%04X\t%s\r\n", FormatNetAddr(&pnode->netAddr), pnode->achPassword, pnode->pszGroupSpec, pnode->fs, pnode->pszSysop);
#endif
 }

/*
 * This subroutine to process the uplink flag
 */

 static BOOL SUBENTRY DoSqafUpLinkFlag(PCH pch, PUPLINK puplink)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   USHORT fs, cch = 2;
   PCH pchEnd;

   // Scan in the uplink flags specification

   switch (tolower(pch[1])) {
     case 'a':  // standard areafix
               fs = UF_AREAFIXPROT;
               break;
     case '+':  // don't prepend +
               fs = UF_NOPLUSPREFIX;
               break;
     case '*':  // don't send multiple areatags in one msg
               fs = UF_NOMULTIAREATAGS;
               break;
     case 'c':  // lowercase areatags
               fs = UF_LOWERCASETAG;
               break;
     case 'n': // no passthru for freq areas
               fs = UF_FREQNOPTHRU;
               break;
     case 'l':  // requesting node access level
               for (cch = 0; isdigit(pch[2 + cch]); cch++);
               puplink->level = atoi(pch + 2);
               return 2 + cch;
     case 'p': // address to send areafix requests from
               if ((pchEnd = ScanNetAddr(&netAddr, pch + 2)) == NULL) {
                 DoLineError("Invalid address: '%s'\n", pch);
                 exit(EXIT_FAILURE);
               }
               puplink->netAddr = netAddr;
               return (USHORT)(pchEnd - pch);
     default : return FALSE;
   }

   // Set the scanned flag (we never reset them here since there are no
   // default uplink flags

   puplink->fs|= fs;

   // Return number of scanned in characters

   return cch;
 }

/*
 * This subroutine to process the 'UpLink' keyword
 */

 static VOID SUBENTRY DoSqafUpLink(PSZ psz)
 {
   static CHAR achAreaFix[] = "AreaFix";
   NETADDR netAddr = cfg.anetAddr[0];
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;
   PUPLINK puplink;
   PNODE pnode;
   USHORT cch;

   // Check if we have seen this keyword before

   if (!(fsCfgScan & CF_SQAFUPLN)) {
     fsCfgScan|= CF_SQAFUPLN;
     if (cfg.fl & FL_VERBOSEMODE)
       printf("SqaFix config: scanning uplink definitions...\n");
   }

   // Check if we have an address and scan in the uplink address

   if (!*psz) {
     DoLineError("Uplink address missing\n");
     exit(EXIT_FAILURE);
   } else
     if ((pch = ScanNetAddr(&netAddr, psz)) == NULL) {
       DoLineError("Invalid uplink address: '%s'\n", psz);
       exit(EXIT_FAILURE);
     }

   // Locate the uplink's node descriptor

   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext)
     if (!xmemcmp(&pnode->netAddr, &netAddr, sizeof(NETADDR)))
       break;

   // Check if the uplink's node descriptor exists

   if (pnode == NULL) {
     DoLineError("Uplink node %s is not defined\n", FormatNetAddr(&netAddr));
     exit(EXIT_FAILURE);
   }

   // Make sure the uplink node is allowed autocreation
   // Note: removed in 99a8 (13/Aug/98)
/*
   pnode->fs|= NF_AUTOCREATE;
*/
   // Create the new uplink list element and link it in

   if ((puplink = (PUPLINK) LstCreateElement(sizeof(UPLINK))) == NULL) {
     DoLineError("Insufficient memory (uplink list)\n");
     exit(EXIT_FAILURE);
   } else
     LstLinkElement((PPLE) &cfg.puplinkFirst, (PLE) puplink, LST_END);

   // Set the uplink node pointer

   puplink->pnode = pnode;

   // Scan in all the node flags if any

   while (SkipSpaces(&pch) && (*pch == '+' || *pch == '-'))
     if ((cch = DoSqafUpLinkFlag(pch, puplink)) > 0) {
       pch+= cch;
     } else {
       DoLineError("Unknown uplink flag: '%s'\n", pch);
       exit(EXIT_FAILURE);
     }

   // Scan in the areafix name or external command specification and
   // check if ok

   for (pchEnd = pch, cch = 0; *pchEnd && !isspace(*pchEnd); pchEnd++, cch++);

   if (!cch)  {
     DoLineError("Invalid uplink areafix type: '%s'\n", psz);
     exit(EXIT_FAILURE);
   }

   // Check if areafix name is areafix and set the standard areafix flag

   if (cch == lengof(achAreaFix) &&
      !xmemicmp(pch, achAreaFix, lengof(achAreaFix))) {
     puplink->fs|= UF_AREAFIXPROT;
   } else

   // Check if this is external command specification and if so, remove
   // the leading @ and set the appropriate flag

   if (*pch == '@') {
     pch++; cch--;
     if (!cch || isspace(*pch)) {
       DoLineError("Missing file name after @\n");
       exit(EXIT_FAILURE);
     }
     puplink->fs|= UF_EXTERNALAFREQ;
   }

   // Allocate areafix name or external command string and check if ok

   if ((puplink->pszName = AllocString(pch, cch)) == NULL) {
     DoLineError("Insufficient memory (uplink areafix name)\n");
     exit(EXIT_FAILURE);
   }

   // Scan through all the area masks specifications

   pch = pchEnd;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if this is a reference file and process it, otherwise
     // add the area mask string list

     if (*pch == '@') {
       if (!pch[1] || isspace(pch[1])) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (!LoadAreaMaskFileNA(pch + 1, &puplink->plszMask)) {
         DoLineError("Can't load uplink areamask file: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }
     } else
#ifndef UNIX
       if (!AddLsz(&puplink->plszMask, xstrupr(pch), -1, LST_END)) {
#else
       if (!AddLsz(&puplink->plszMask, pch, -1, LST_END)) {
#endif
         DoLineError("Insufficient memory (uplink areamask)\n");
         exit(EXIT_FAILURE);
       }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'ForwardRequestRefuse' keyword
 */

 static VOID SUBENTRY DoSqafFReqRefuse(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;

   // Scan through all the area masks specifications

   pch = psz;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if this is a reference file and process it, otherwise
     // add the area mask string list

     if (*pch == '@') {
       if (!pch[1] || isspace(pch[1])) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (!LoadAreaMaskFileNA(pch + 1, &cfg.plszFReqRefuseAreaFirst)) {
         DoLineError("Can't load areamask file: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }
     } else
#ifndef UNIX
       if (!AddLsz(&cfg.plszFReqRefuseAreaFirst, xstrupr(pch), -1, LST_END)) {
#else
       if (!AddLsz(&cfg.plszFReqRefuseAreaFirst, pch, -1, LST_END)) {       
#endif
         DoLineError("Insufficient memory (freq refuse areamask)\n");
         exit(EXIT_FAILURE);
       }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'ForwardRequestTimeout' keyword
 */

 static VOID SUBENTRY DoSqafFReqTimeout(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;
   USHORT ipar, ich;

   // Scan through all the days specifications

   pch = psz;
   for (ipar = 1; !fDone && SkipSpaces(&pch); ipar++) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if it's all digits

     for (ich = 0; pch + ich < pchEnd; ich++)
       if (!isdigit(pch[ich])) {
         DoLineError("Invalid numeric value: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }

     // Set in the parameter

     switch (ipar) {
       case 1 : cfg.dayFReqTimeout = atoi(pch); break;
       case 2 : cfg.dayFReqWarning = atoi(pch);
                // fall through
       default: return;
     }

     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'ForwardRequestNotify' keyword
 */

 static VOID SUBENTRY DoSqafFReqNotify(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   SHORT iAddr, iArea, iFile;
   PCH pch;

   // Scan to the first available addr/area/file notify slots

   for (iAddr = 0; iAddr < numbof(cfg.anetFReqNote)
        && cfg.anetFReqNote[iAddr].zone != 0; iAddr++);

   for (iArea = 0; iArea < numbof(cfg.apszFReqNote)
        && cfg.apszFReqNote[iArea] != NULL; iArea++);

   for (iFile = 0; iFile < numbof(cfg.apszFReqFile)
        && cfg.apszFReqFile[iFile] != NULL; iFile++);

   // Scan in all the addr/area/file specified

   while (SkipSpaces(&psz))

     // Check if this is a file specification since addr can have @

     if (*psz == '@') {
       for (pch = ++psz; *pch && !isspace(*pch); pch++);
       if (pch == psz) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (iFile < numbof(cfg.apszFReqFile))  {
         if ((cfg.apszFReqFile[iFile] =
              AllocString(psz, (SHORT)(pch - psz))) == NULL) {
           DoLineError("Insufficient memory (notify area list)\n");
           exit(EXIT_FAILURE);
         } else {
#ifndef UNIX
           xstrupr(cfg.apszFReqFile[iFile++]);
#endif
           psz = pch;
         }
       } else {
         DoLineError("Too many notify files: '%s'\n", psz);
         exit(EXIT_FAILURE);
       }
     } else

     // Now check if this is an address

     if ((pch = ScanNetAddr(&netAddr, psz)) != NULL) {
       if (iAddr < numbof(cfg.anetFReqNote))  {
         xmemcpy(&cfg.anetFReqNote[iAddr++], &netAddr, sizeof(NETADDR));
         psz = pch;
       } else {
         DoLineError("Too many notify addresses: '%s'\n", psz);
         exit(EXIT_FAILURE);
       }
     } else {

       // Otherwise this is an echo area

       for (pch = psz; *pch && !isspace(*pch); pch++);
       if (iArea < numbof(cfg.apszFReqNote))  {
         if ((cfg.apszFReqNote[iArea] =
              AllocString(psz, (SHORT)(pch - psz))) == NULL) {
           DoLineError("Insufficient memory (notify area list)\n");
           exit(EXIT_FAILURE);
         } else {
#ifndef UNIX
           xstrupr(cfg.apszFReqNote[iArea++]);
#endif
           psz = pch;
         }
       } else {
         DoLineError("Too many notify areas: '%s'\n", psz);
         exit(EXIT_FAILURE);
       }
     }
 }

/*
 * This subroutine to process the 'ForwardRequestRetryAll' keyword
 */

 static VOID SUBENTRY DoSqafFReqRetryAll(PSZ psz)
 {
   cfg.fl|= FL_FREQTRYALLUPLINKS;
 }

/*
 * This subroutine to process the 'ForwardRequestKeepAreas' keyword
 */

 static VOID SUBENTRY DoSqafFReqKeepAreas(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;

   // Scan through all the area masks specifications

   pch = psz;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if this is a reference file and process it, otherwise
     // add the area mask string list

     if (*pch == '@') {
       if (!pch[1] || isspace(pch[1])) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (!LoadAreaMaskFileNA(pch + 1, &cfg.plszFReqKeepAreaFirst)) {
         DoLineError("Can't load areamask file: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }
     } else
#ifndef UNIX
       if (!AddLsz(&cfg.plszFReqKeepAreaFirst, xstrupr(pch), -1, LST_END)) {
#else
       if (!AddLsz(&cfg.plszFReqKeepAreaFirst, pch, -1, LST_END)) {       
#endif       
         DoLineError("Insufficient memory (freq keeparea areamask)\n");
         exit(EXIT_FAILURE);
       }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'KeepIdlePassthru' keyword
 */

 static VOID SUBENTRY DoSqafIdleKeepAreas(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;

   // Scan through all the area masks specifications

   pch = psz;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if this is a reference file and process it, otherwise
     // add the area mask string list

     if (*pch == '@') {
       if (!pch[1] || isspace(pch[1])) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (!LoadAreaMaskFileNA(pch + 1, &cfg.plszIdleKeepAreaFirst)) {
         DoLineError("Can't load areamask file: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }
     } else
#ifndef UNIX
       if (!AddLsz(&cfg.plszIdleKeepAreaFirst, xstrupr(pch), -1, LST_END)) {
#else       
       if (!AddLsz(&cfg.plszIdleKeepAreaFirst, pch, -1, LST_END)) {
#endif
         DoLineError("Insufficient memory (keep idle areamask)\n");
         exit(EXIT_FAILURE);
       }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'IdlePassthruTimeout' keyword
 */

 static VOID SUBENTRY DoSqafIdleTimeout(PSZ psz)
 {
   BOOL fDone = FALSE;
   PCH  pch, pchEnd;
   USHORT ipar, ich;

   // Scan through all the days specifications

   pch = psz;
   for (ipar = 1; !fDone && SkipSpaces(&pch); ipar++) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Check if it's all digits

     for (ich = 0; pch + ich < pchEnd; ich++)
       if (!isdigit(pch[ich])) {
         DoLineError("Invalid numeric value: '%s'\n", pch);
         exit(EXIT_FAILURE);
       }

     // Set in the parameter

     switch (ipar) {
       case 1 : cfg.dayIdleTimeout = atoi(pch); break;
       case 2 : cfg.dayIdleWarning = atoi(pch); break;
       case 3 : cfg.dayIdleIgnore = atoi(pch);
                // fall through
       default: return;
     }

     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'IdlePassthruNotify' keyword
 */

 static VOID SUBENTRY DoSqafIdleNotify(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   SHORT iAddr, iArea, iFile;
   PCH pch;

   // Scan to the first available addr/area/file notify slots

   for (iAddr = 0; iAddr < numbof(cfg.anetIdleNote)
        && cfg.anetIdleNote[iAddr].zone != 0; iAddr++);

   for (iArea = 0; iArea < numbof(cfg.apszIdleNote)
        && cfg.apszIdleNote[iArea] != NULL; iArea++);

   for (iFile = 0; iFile < numbof(cfg.apszIdleFile)
        && cfg.apszIdleFile[iFile] != NULL; iFile++);

   // Scan in all the addr/area/file specified

   while (SkipSpaces(&psz))

     // Check if this is a file specification since addr can have @

     if (*psz == '@') {
       for (pch = ++psz; *pch && !isspace(*pch); pch++);
       if (pch == psz) {
         DoLineError("Missing file name after @\n");
         exit(EXIT_FAILURE);
       }
       if (iFile < numbof(cfg.apszIdleFile))  {
         if ((cfg.apszIdleFile[iFile] =
              AllocString(psz, (SHORT)(pch - psz))) == NULL) {
           DoLineError("Insufficient memory (notify area list)\n");
           exit(EXIT_FAILURE);
         } else {
#ifndef UNIX
           xstrupr(cfg.apszIdleFile[iFile++]);
#endif	   
           psz = pch;
         }
       } else {
         DoLineError("Too many notify areas: '%s'\n", psz);
         exit(EXIT_FAILURE);
       }
     } else

     // Now check if this is an address

     if ((pch = ScanNetAddr(&netAddr, psz)) != NULL) {
       if (iAddr < numbof(cfg.anetIdleNote))  {
         xmemcpy(&cfg.anetIdleNote[iAddr++], &netAddr, sizeof(NETADDR));
         psz = pch;
       } else {
         DoLineError("Too many notify addresses: '%s'\n", psz);
         exit(EXIT_FAILURE);
       }
     } else {

       // Otherwise this is an echo area

       for (pch = psz; *pch && !isspace(*pch); pch++);
       if (iArea < numbof(cfg.apszIdleNote))  {
         if ((cfg.apszIdleNote[iArea] =
              AllocString(psz, (SHORT)(pch - psz))) == NULL) {
           DoLineError("Insufficient memory (notify area list)\n");
           exit(EXIT_FAILURE);
         } else {
#ifndef UNIX
           xstrupr(cfg.apszIdleNote[iArea++]);
#endif	   
           psz = pch;
         }
       } else {
         DoLineError("Too many notify areas: '%s'\n", psz);
         exit(EXIT_FAILURE);
       }
     }
 }

/*
 * This subroutine to process the 'IdlePassthruIgnoreNodes' keyword
 */


 static VOID SUBENTRY DoSqafIdleIgnoreNodes(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   SHORT iAddr;
   PCH pch;

   // Skip to the first address slot available if any

   for (iAddr = 0; iAddr < numbof(cfg.anetIdleNode)
        && cfg.anetIdleNode[iAddr].zone != 0; iAddr++);

   // Scan in all the specified aka's

   for (; SkipSpaces(&psz); iAddr++)
     if (iAddr >= numbof(cfg.anetIdleNode))  {
       DoLineError("Too many addresses: '%s'\n", psz);
       exit(EXIT_FAILURE);
     } else
       if ((pch = ScanNetAddr(&netAddr, psz)) == NULL) {
         DoLineError("Invalid address: '%s'\n", psz);
         exit(EXIT_FAILURE);
       } else {
         xmemcpy(&cfg.anetIdleNode[iAddr], &netAddr, sizeof(NETADDR));
         psz = pch;
       }
 }

/*
 * This subroutine to process the 'IdlePassthruIgnorePassive' keyword
 */


 static VOID SUBENTRY DoSqafIdleIgnorePassive(PSZ psz)
 {
   cfg.fl|= FL_IDLEIGNOREPASSIVE;
 }

/*
 * This subroutine checks if the specified keyword is in the ignore list
 */

 static BOOL SUBENTRY DoIsIgnoreKeyword(PSZ pszKeyword)
 {
   return GetLsz(cfg.plszIgnoreKeyFirst, pszKeyword) ? TRUE : FALSE;
 }

/*
 * This subroutine to process the SqaFix config file line
 */

 static BOOL SUBENTRY DoScanSqafLine(PSZ pszKey, USHORT cch, PSZ psz)
 {
   PTAB ptab;

   // Check if this is a comment line which can contain ;NewAreaPlace key
   // and process it now since there can be lots of comments like this

   if (pszKey[0] == ';')
     if (cch == lengof(SQXXNEWAREAPLACE) &&
        !xstricmp(pszKey, SQXXNEWAREAPLACE)) {
       ptabSqafNewAreaPlace->pfn(psz);
       return TRUE;
     } else
       return FALSE;

   // Search for the keyword and if found, process the line

   if ((ptab = DoSearchKey(atabSqaf, numbof(atabSqaf), pszKey)) != NULL) {
     ptab->pfn(psz);
     return TRUE;
   }

   // Check if this is not an ignored line and report

   if (!DoIsIgnoreKeyword(pszKey))
     DoLineError("Warning: unknown keyword: '%s'\n", pszKey);

   return FALSE;
 }

/////////////////////////////////////////////////////////////////////////////
// S q u i s h   c o n f i g   p a r s e r   s u b r o u t i n e s         //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine to process the 'Address' keyword
 */

 static VOID SUBENTRY DoSqshAddress(PSZ psz)
 {
   static BOOL fFirstTurn = TRUE;
   NETADDR netAddr = {0,0,0,0};

   // Process only the primary address

   if (fFirstTurn) {
     fFirstTurn = FALSE;

     // Skip spaces and scan in the primary address

     SkipSpaces(&psz);
     if (ScanNetAddr(&netAddr, psz) == NULL ||
         netAddr.zone == 0 || netAddr.net == 0) {
       DoLineError("Invalid address: '%s'\n", psz);
       exit(EXIT_FAILURE);
     }

     // Check if it is equal to the one specified in the SqaFix.Cfg

     if (xmemcmp(&cfg.anetAddr[0], &netAddr, sizeof(NETADDR))) {
       DoLineError("Different primary address specified for Squish and SqaFix\n");
       exit(EXIT_FAILURE);
     }
   }
 }

/*
 * This subroutine to process the 'PointNet' keyword
 */

 static VOID SUBENTRY DoSqshPointNet(PSZ psz)
 {
   cfg.usPointNet = atoi(psz);
 }

/*
 * This subroutine to process the 'Origin' keyword
 */

 static VOID SUBENTRY DoSqshOrigin(PSZ psz)
 {
   PCH pchEnd;
   USHORT cchOrigin;

   // Skip over the leading blanks and check for the missing origin line

   if (!SkipSpaces(&psz)) {
     DoLineError("Missing origin line: '%s'\n", psz);
     exit(EXIT_FAILURE);
   }

   // Get past the end of the origin line specification and claculate its length

   pchEnd = xstrchr(psz, 0);
   while (--pchEnd > psz && isspace(*pchEnd));
   cchOrigin = (USHORT)(pchEnd - psz);

   if (cchOrigin == 0 || cchOrigin >= sizeof(cfg.achOrigin)) {
     DoLineError("Invalid origin line: '%s'\n", psz);
     exit(EXIT_FAILURE);
   }

   // Copy the origin line

   xstrcpy(cfg.achOrigin, psz);
 }

/*
 * This subroutine to process the 'Compress' keyword
 */

 static VOID SUBENTRY DoSqshCompress(PSZ psz)
 {
   // Copy the Squish compress file specification if it is not
   // overriden previously and append the default extension

   if (!cfg.achArcFile[0]) xstrncpy(cfg.achArcFile, psz, lengof(cfg.achArcFile));
   AppendFileExt(cfg.achArcFile, cfg.achArcFile, DEF_CFG_EXT, FALSE);

   // Check if we got something and make it fully qualified file name

   if (!cfg.achArcFile[0] || !BuildFullPath(cfg.achArcFile, cfg.achArcFile)) {
     DoLineError("Invalid compress file path: %s\n", cfg.achArcFile);
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'Pack' keyword
 */

 static VOID SUBENTRY DoSqshPack(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   PCH pchEnd, pch = psz;
   PARC parc, parcDupe;
   BOOL fDone = FALSE;

   // Scan past the end of the packer name and fix its end

   for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);

   // Check if ok and fix its end. Note that we want to have this
   // packer entry even if it has no associated nodes

   if (pchEnd == pch)
     return;
   else
     if (*pchEnd) *pchEnd++ = '\0';

   // Add the new packer list element

   if ((parc = AddArc(pch, LST_END)) == NULL) {
     WriteLog("! Can't add packer entry\n");
     exit(EXIT_FAILURE);
   }

   // Scan through all the packer node specifications

   pch = pchEnd;
   while (!fDone && SkipSpaces(&pch)) {
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     if (*pchEnd) *pchEnd = '\0';
     else fDone = TRUE;

     // Scan in the node network address mask

     if ((psz = ScanNetAddrMask(&netAddr, pch)) == NULL || psz != pchEnd) {
       DoLineError("Invalid address mask: '%s'\n", pch);
       exit(EXIT_FAILURE);
     }

     // Check if we already have this node in the packer list and
     // delete it if so then force the packer list update. Note that
     // we don't do this for address masks

     if (!IsAddrMask(&netAddr))
       while ((parcDupe = GetArc(&netAddr, NULL)) != NULL) {
         WriteLog("! Duplicate packer entry %s %s removed\n",
                   parcDupe->achArc, FormatNetAddr(&netAddr));
         DelArcLink(parcDupe, &netAddr);
         cfg.fPackChanged = TRUE;
       }

     // Add address mask to the packer node list

     if (!AddArcLink(parc, &netAddr)) {
       DoLineError("Insufficient memory (packer node list)\n");
       exit(EXIT_FAILURE);
     }
     pch = ++pchEnd;
   }
 }

/*
 * This subroutine to process the 'DefaultPacker' keyword
 */

 static VOID SUBENTRY DoSqshDefaultPacker(PSZ psz)
 {
   PCH pchEnd, pch = psz;

   // Scan past the end of the packer name and fix its end

   for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);

   // Check if ok and fix its end

   if (pchEnd == pch)
     return;
   else
     *pchEnd = '\0';

   // Allocate the default packer entry

   if ((cfg.pszDefPacker = AllocString(pch, -1)) == NULL) {
     DoLineError("Insufficient memory (default packer)\n");
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'AreasBbs' keyword
 */

 static VOID SUBENTRY DoSqshAreasBbs(PSZ psz)
 {
   DoLineError("Squish AreasBBS mode can't be used with SqaFix\n");
   exit(EXIT_FAILURE);
 }

/*
 * This subroutine to process the 'NetArea' keyword
 */

 static VOID SUBENTRY DoSqshNetArea(PSZ psz)
 {
   PCH pch;

   // Fix up the end of the NetArea tag

   for (pch = psz; *pch && !isspace(*pch); pch++);
   while (isspace(*pch)) *pch++ = '\0';

   // Add the NetArea tag to the refuse create list

   if (!AddLsz(&cfg.plszRefuseCreate, psz, -1, LST_END)) {
     DoLineError("Insufficient memory (refuse create NetArea)\n");
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'BadArea' keyword
 */

 static VOID SUBENTRY DoSqshBadArea(PSZ psz)
 {
   PCH pch, pchEnd;

   // Fix up the end of the bad mail tag

   for (pch = psz; *pch && !isspace(*pch); pch++);
   while (isspace(*pch)) *pch++ = '\0';

   // Add the BadArea tag to the refuse create list

   if (!AddLsz(&cfg.plszRefuseCreate, psz, -1, LST_END)) {
     DoLineError("Insufficient memory (refuse create BadArea)\n");
     exit(EXIT_FAILURE);
   }

   // Check if there is a path specification

   if (!SkipSpaces(&pch)) {
     DoLineError("Warning -- bad messages path missing\n");
     return;
   }

   // Skip over the area path specification and store it

   for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
   *pchEnd++ = '\0';
   xstrncpy(cfg.achBadMail, pch, lengof(cfg.achBadMail));
#ifndef UNIX
   xstrupr(cfg.achBadMail);
#endif   
   pch = pchEnd;

   // Scan in all the area options if any and set
   // the bad mail is Squish base flag

   if (IsSquishArea(pch)) cfg.fl|= FL_SQUISHBADMAIL;
 }

/*
 * This subroutine to process the 'DupeArea' keyword
 */

 static VOID SUBENTRY DoSqshDupeArea(PSZ psz)
 {
   PCH pch;

   // Fix up the end of the DupeArea tag

   for (pch = psz; *pch && !isspace(*pch); pch++);
   while (isspace(*pch)) *pch++ = '\0';

   // Add the DupeArea tag to the refuse create list

   if (!AddLsz(&cfg.plszRefuseCreate, psz, -1, LST_END)) {
     DoLineError("Insufficient memory (refuse create DupeArea)\n");
     exit(EXIT_FAILURE);
   }
 }

/*
 * This subroutine to process the 'NewAreaPlace' keyword
 */

 static VOID SUBENTRY DoSqshNewAreaPlace(PSZ psz)
 {
   cfg.fl|= FL_SQSHNEWAREAPLACE;
 }

/*
 * This subroutine scans in the squish flag address
 */

 static VOID SUBENTRY DoScanSqshFlagAddr(NETADDR * pnetAddr, PSZ psz)
 {
   CHAR ch = tolower(*psz);

   // Check if this is an address flag

   if (ch == 'p' || ch == 'x' || ch == '+') {
     if (ScanNetAddr(pnetAddr, psz + 1) == NULL) {
       DoLineError("Invalid address: '%s'\n", psz - 1);
       exit(EXIT_FAILURE);
     }
   }
 }

/*
 * This subroutine to process the 'EchoArea' keyword
 */

 static VOID SUBENTRY DoSqshEchoArea(PSZ psz)
 {
   NETADDR netAddr = cfg.anetAddr[0];
   BOOL fMulti = FALSE;
   PNEWAREA pnewarea;
   PCH pch, pchEnd;
   CHAR chGroup;
   PAREA parea;
   PLINK plink;

   // Check if we have seen this keyword before

   if (!(fsCfgScan & CF_SQSHECHO)) {
     fsCfgScan|= CF_SQSHECHO;
     if (cfg.fl & FL_VERBOSEMODE)
       printf("Squish config: scanning echo area definitions...\n");
   }

   // Fix up the end of the echo mail tag

   for (pch = psz; *pch && !isspace(*pch); pch++);
   while (isspace(*pch)) *pch++ = '\0';

   // Get the area list element for the this tag and if it is not
   // in SqaFix.Cfg allocate it now with the default group.
   // Note: this is used for SqaFix Sync run...

   if ((parea = GetAreaFromTag(psz)) == NULL) {

     // Locate new area info specific for the area tag

     for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
          pnewarea = pnewarea->pnewareaNext)
       if (GrepSearch(psz, pnewarea->pszArea, FALSE))
         break;

     // Check if not found and assume the default new area info

     if (pnewarea == NULL) pnewarea = &newareaDef;

     // Determine the area group. Note that new area defaults are not yet set

     chGroup = pnewarea->chGroup ? pnewarea->chGroup : newareaDef.chGroup;
     if (!chGroup) chGroup = DEF_NEWAREA_GROUP;

     // Allocate an area with the default group

     parea = AddArea(psz, chGroup);
   }

   // Check if we have seen this area and set flag area is known to Squish

   if (parea->fs & AF_INSQUISHCFG) {
     fMulti = TRUE;
   } else
     parea->fs|= AF_INSQUISHCFG;

   // Scan in the area path specification

   for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
   if (!fMulti) {
     if ((parea->pszPath = AllocString(pch, (SHORT) (pchEnd - pch))) == NULL) {
       DoLineError("Insufficient memory (filename)\n");
       exit(EXIT_FAILURE);
     } 
#ifndef UNIX
       else
       xstrupr(parea->pszPath);
#endif       
   }

   // Scan in all the area flags skipping the leading spaces

   if (*pchEnd && SkipSpaces(&pchEnd) == '-') {

     pch = pchEnd;
     while (*pchEnd && SkipSpaces(&pchEnd) == '-') {
       DoScanSqshFlagAddr(&netAddr, pchEnd + 1);
       while (*pchEnd && !isspace(*pchEnd)) pchEnd++;
       while (isspace(*pchEnd)) pchEnd++;
     }

     if (!fMulti) {
       parea->pchSqshFlag = pch;
       if ((parea->pszSqshFlags = AllocString(pch, (SHORT) (pchEnd - pch))) == NULL) {
         DoLineError("Insufficient memory (areaflags)\n");
         exit(EXIT_FAILURE);
       }
     }
   }

   // Save the inline pointers for future update

   pch = pchEnd;
   if (!fMulti) {
     parea->pchSqshLink = pch;
     if (!parea->pchSqshFlag) parea->pchSqshFlag = parea->pchSqshLink;
   }

   // Scan in all the active links if any. Check if the link is already
   // stored as passive and reset it to the active if so. Also, check if
   // there is an inline comment and store its pointer

   while (SkipSpaces(&pch))
     if ((pchEnd = ScanNetAddr(&netAddr, pch)) == NULL ||
          netAddr.zone == 0 || netAddr.net == 0) {
       if (*pch == ';' || *pch == '%') {
         if (!fMulti) parea->pchSqshTail = pch;
         break;
       }
       DoLineError("Invalid address: '%.128s'\n", pch);
       exit(EXIT_FAILURE);
     } else {
       if ((plink = GetAreaLink(parea, &netAddr)) == NULL)
         AddAreaLink(parea, &netAddr, ACTIVE, NULL);
       else
         if (!(plink->fs & LF_ACTIVE)) {
           plink->fs|= LF_ACTIVE;
           parea->fs|= AF_ACTIVECHANGED | AF_PASSIVECHANGED;
           DoLineError("Warning -- %s is both active and passive for area '%s'\n",
                        FormatNetAddr(&netAddr), parea->achTag);
         }
       pch = pchEnd;
     }
 }

/*
 * This subroutine to process the Squish config file line
 */

 static BOOL SUBENTRY DoScanSqshLine(PSZ pszKey, USHORT cch, PSZ psz)
 {
   PTAB ptab;

   // Check if this is a comment line which can contain ;NewAreaPlace key
   // and process it now since there can be lots of comments like this

   if (pszKey[0] == ';')
     if (cch == lengof(SQXXNEWAREAPLACE) &&
        !xstricmp(pszKey, SQXXNEWAREAPLACE)) {
       ptabSqshNewAreaPlace->pfn(psz);
       return TRUE;
     } else
       return FALSE;

   // Search for the keyword and if found, process the line

   if ((ptab = DoSearchKey(atabSqsh, numbof(atabSqsh), pszKey)) != NULL) {
     ptab->pfn(psz);
     return TRUE;
   }

   return FALSE;
 }

/////////////////////////////////////////////////////////////////////////////
// C o m m o n   c o n f i g   p a r s e r   s u b r o u t i n e s         //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine to process the 'Include' keyword
 */

 static VOID SUBENTRY DoSqxxInclude(PSZ psz)
 {
   // It would be easy to scan included files in both config files, but
   // updating them would be quite messy... so there is no support for now.
 }

/*
 * This subroutine matches the link and config type
 */

 static BOOL SUBENTRY DoMatchLinkType(PLINK plink, BOOL fActive)
 {
   // Check if this is active or passive link

   if (plink->fs & LF_ACTIVE) {
     if (fActive) return TRUE;
   } else {
     if (!fActive) return TRUE;
   }

   return FALSE;
 }

/*
 * This subroutine updates list of linked nodes in the 'EchoArea' config line
 */

 static VOID SUBENTRY DoUpdateAreaLinks(PAREA parea, BOOL fActive, PCH pchEnd)
 {
   NETADDR netAddr = {0, 0, 0, 0};      // Always start list with full addr
   PSZ psz, pszTail;
   PLINK plink;
   PCH pch;

   // Get the approprite pointer to the inline comments in the config line
   // and if it's there, allocate the comment string and check if ok

   if ((psz = fActive ? parea->pchSqshTail : parea->pchSqafTail) == NULL)
     pszTail = NULL;
   else
     if ((pszTail = AllocString(psz, -1)) == NULL) {
       WriteLog("! Insufficient memory (inline comment)\n");
       exit(EXIT_FAILURE);
     }

   // Get the approprite pointer to the list beginning in the config line and
   // check if this is active EchoArea and Squish flags has been changed and
   // write out the updated Squish flags

   if (fActive && (parea->fs & AF_SQSHFLAGSCHANGED) && parea->pchSqshFlag) {

     pch = parea->pchSqshFlag;
     pch+= sprintf(pch, "%s\t", parea->pszSqshFlags);

   } else {

     // Otherwise get back to past the last non space character and
     // append a tab

     pch = fActive ? parea->pchSqshLink : parea->pchSqafLink;
     while (isspace(*(pch - 1))) pch--;
     *pch++ = '\t'; *pch = '\0';
   }

   // Fix the current end of string and append all the links for this area

   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext)
     if (DoMatchLinkType(plink, fActive)) {

       // Make up the link address and check if we have enough
       // space left in the buffer to store it

       psz = MakeNetAddrList(&plink->netAddr, &netAddr);
       if (pch + xstrlen(psz) < pchEnd) {
         xstrcat(pch, psz);
       } else {
         WriteLog("! The linked node list is too long\n");
         exit(EXIT_FAILURE);
       }
     }

   // Check if we have the inline comment we need to append and if not,
   // just append the newline character to terminate the links list and exit

   if (pszTail == NULL) {
     xstrcat(pch, "\n");
   } else {

     // Now check to see if there is enough room in the buffer to accomodate
     // the inline comments text and if so, append it to the end of the line,
     // then free the tail buffer

     if (pch + 1 + xstrlen(psz) < pchEnd) {
       xstrcat(pch, "\t"); xstrcat(pch, pszTail);
       MemFree(pszTail);
     } else {
       WriteLog("! The inline comment text is too long for area '%s'\n", parea->achTag);
       exit(EXIT_FAILURE);
     }
   }
 }

/*
 * This subroutine updates the 'EchoArea' config line
 */

 static BOOL SUBENTRY DoUpdateEchoArea(PSZ psz, BOOL fActive, PCH pchEnd)
 {
   time_t tm;
   CHAR achText[128], achDateTime[80];
   PDELAREA pdelarea;
   USHORT cchText;
   PAREA parea;
   CHAR ch;
   PCH pch;

   // Skip over the leading spaces and fix the end of the area tag
   // while preserving the character there

   SkipSpaces(&psz);
   for (pch = psz; *pch && !isspace(*pch); pch++);
   ch = *pch; *pch = '\0';

   // Get the area descriptor and check if it's in the list

   if ((parea = GetAreaFromTag(psz)) != NULL) {
     *pch = ch;
   } else {

     // Check if this area is in the deleted area list

     if ((pdelarea = GetDelAreaFromTag(psz)) == NULL) {

       // Absolutely unknown area -- this should never happen in reality...

       sprintf(achText, "; Unknown\n; ");

       // Log this weird fact...

       WriteLog("! %s %s (unknown)\n", fActive ? "Sqsh" : "Sqaf", psz);

     } else {

       // Log deleted area

       WriteLog("- %s %s (destroyed)\n", fActive ? "Sqsh" : "Sqaf", psz);

       // Check if we don't need to preserve deleted areas info

       if (!(cfg.fl & FL_PRESERVEDESTRAREAS))
         return FALSE;

       // Compose the deleted area comment info

       time(&tm); xstrcpy(achDateTime, asctime(localtime(&tm)));
       if (xstrchr(achDateTime, '\n')) *xstrchr(achDateTime, '\n') = '\0';
       if (pdelarea->pnode != NULL)
         cchText = sprintf(achText, "; Destroyed by %s, %s\n; ",
                           FormatNetAddr(&pdelarea->pnode->netAddr),
                           achDateTime);
       else
         cchText = sprintf(achText, "; Destroyed idle passthru area, %s\n; ",
                           achDateTime);
     }

     // Move in the comment info

     *pch = ch;
     xmemmove(pchBuf + cchText, pchBuf, xstrlen(pchBuf) + 1);
     xmemcpy(pchBuf, achText, cchText);
     return TRUE;
   }

   // Check if this line should be updated

   if (fActive) {
     if (!(parea->fs & AF_ACTIVECHANGED)) return TRUE;
   } else {
     if (!(parea->fs & AF_PASSIVECHANGED)) return TRUE;
   }

   // Log echoarea line changes

   WriteLog("- %s %s %s\n", fActive ? "Sqsh" : "Sqaf",
             parea->achTag,
             parea->fs & AF_SQSHFLAGSCHANGED
            ? "(flags and links updated)"
            : "(links updated)");

   // Update linked nodes list to match a passive node list for this area

   DoUpdateAreaLinks(parea, fActive, pchEnd);

   return TRUE;
 }

/*
 * This subroutine writes out all the new area definitions
 */

 static BOOL SUBENTRY DoWriteNewAreas(FILE * pfileTmp, BOOL fActive)
 {
   USHORT cch, cchLine, cchMax = 0;
   CHAR achLine[1024];
   PAREA parea;

   // Loop through all areas getting max area tag length

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
     if ((cch = xstrlen(parea->achTag)) > cchMax) cchMax = cch;

   // Loop through all areas processing the newly created ones

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
     if (parea->fs & AF_AUTOCREATEDAREA) {

       // Calculate length of the padding string

       cch = cchMax - xstrlen(parea->achTag);

       // Compose the echo area definition string

       if (fActive) {

         // Check if this area is already in Squish config

         if (parea->fs & AF_INSQUISHCFG) continue;

         // Format the new Squish EchoArea line

         cchLine = sprintf(achLine, "EchoArea %s%*s %s\t%s\t",
                           parea->achTag, cch, "", parea->pszPath,
                           parea->pszSqshFlags ? parea->pszSqshFlags : "");

       } else {

         // Check if this area is already in SqaFix config

         if (parea->fs & AF_INSQAFIXCFG) continue;

         // Format the new SqaFix EchoArea line

         if (parea->pszDescr != NULL && parea->pszDescr[0])
           cchLine = sprintf(achLine, "EchoArea %s%*s %c\t\"%s\"\t",
                             parea->achTag, cch, "",
                             parea->chGroup ? parea->chGroup : '?',
                             parea->pszDescr);
         else
           cchLine = sprintf(achLine, "EchoArea %s%*s %c\t",
                             parea->achTag, cch, "",
                             parea->chGroup ? parea->chGroup : '?');

         // Check if this area level is different from the default and
         // set it's specification for the new area

         if (parea->level != cfg.usDefAreaLevel)
           cchLine+= sprintf(achLine + cchLine, "-l%u\t", parea->level);
       }

       // Log echoarea line changes

       WriteLog("- %s %s (new %sarea)\n",
                 fActive ? "Sqsh" : "Sqaf", parea->achTag,
                 IsPassThruArea(parea->pszSqshFlags) ? "passthru " : "");

       // Make up the links beginning pointer and create linked nodes list

       parea->pchSqshLink = parea->pchSqafLink = achLine + cchLine;
       DoUpdateAreaLinks(parea, fActive, achLine + sizeof(achLine));

       // Write out the new echo area definition and check if ok

       if (fputs(achLine, pfileTmp) == EOF)
         return FALSE;
     }

   return TRUE;
 }

/*
 * This subroutine writes out packer definitions
 */

 static BOOL SUBENTRY DoWritePackers(FILE * pfileTmp)
 {
   CHAR achLine[128];
   USHORT cchLine;
   PLINK plink;
   PARC parc;

   // Scan all the packer list elements

   for (parc = cfg.parcFirst; parc != NULL; parc = parc->parcNext) {

     // Make up the packer definition

     cchLine = sprintf(achLine, SQSHPACK" %s ", parc->achArc);

     // Scan all the packer node specifications

     for (plink = parc->plink; plink != NULL; plink = plink->plinkNext) {

       // Check for the buffer overrun and split the line if so

       if (cchLine > lengof(achLine) - lengof("65535:65535/65535.65535  ")) {
         xstrcat(achLine, "\n");
         if (fputs(achLine, pfileTmp) == EOF)
           return FALSE;
         else
           cchLine = sprintf(achLine, SQSHPACK" %s ", parc->achArc);
       }

       // Format the packer link address or address mask

       if (plink->netAddr.zone == (USHORT)-1) {
         cchLine+= sprintf(achLine + cchLine, " All");
         continue;
       } else
         cchLine+= sprintf(achLine + cchLine, " %u:", plink->netAddr.zone);

       if (plink->netAddr.net == (USHORT)-1) {
         cchLine+= sprintf(achLine + cchLine, "All");
         continue;
       } else
         cchLine+= sprintf(achLine + cchLine, "%u/", plink->netAddr.net);

       if (plink->netAddr.node == (USHORT)-1) {
         cchLine+= sprintf(achLine + cchLine, "All");
         continue;
       } else
         cchLine+= sprintf(achLine + cchLine, "%u", plink->netAddr.node);

       if (plink->netAddr.point == 0) {
         continue;
       } else
       if (plink->netAddr.point == (USHORT)-1) {
         cchLine+= sprintf(achLine + cchLine, ".All");
         continue;
       } else
         cchLine+= sprintf(achLine + cchLine, ".%u", plink->netAddr.point);
     }

     // Write out the packer definition and check if ok

     xstrcat(achLine, "\n");
     if (fputs(achLine, pfileTmp) == EOF)
       return FALSE;
   }

   // Log packer list changes

   WriteLog("- Sqsh packer list updated\n");

   return TRUE;
 }

/*
 * This subroutine checks the given area list
 */

 static VOID SUBENTRY DoCheckAreaList(PSZ apsz[], USHORT cpsz, PSZ pszText)
 {
   USHORT ipsz;

   for (ipsz = 0; ipsz < cpsz && apsz[ipsz] != NULL; ipsz++)
     if (GetAreaFromTag(apsz[ipsz]) == NULL)
       WriteLog("! Area %s is not known %s\r\n", apsz[ipsz], pszText);
 }

/*
 * This subroutine fixes the new area specifications
 */

 static VOID SUBENTRY DoCheckNewAreaSpec(VOID)
 {
   PNEWAREA pnewarea;

   // Check if there is no default new area character and fix it

   if (!newareaDef.chGroup) newareaDef.chGroup = DEF_NEWAREA_GROUP;

   // Check the default new area notify areas

   DoCheckAreaList(newareaDef.apszNote, numbof(newareaDef.apszNote),
                  "(NewAreaNotify)");

   // Scan through the additinal new area info blocks filling up
   // the missing info with default one

   for (pnewarea = newareaDef.pnewareaNext; pnewarea != NULL;
        pnewarea = pnewarea->pnewareaNext) {

     // Check new area group character

     if (!pnewarea->chGroup) pnewarea->chGroup = newareaDef.chGroup;

     // Check new area directory path and flags

     if (!pnewarea->achPath[0]) {
       xstrcpy(pnewarea->achPath, newareaDef.achPath);
       pnewarea->fs&= ~NA_CONVERTMASK;
       pnewarea->fs|= (newareaDef.fs & NA_CONVERTMASK);
     }

     // Check the new area notify areas

     DoCheckAreaList(pnewarea->apszNote, numbof(pnewarea->apszNote),
                    "(NewAreaNotify)");
   }
 }

/*
 * This subroutine adds new area group to the group array
 */

 static VOID SUBENTRY DoAddGroup(CHAR achGroup[], CHAR chGroup)
 {
   // Check if this group is not known and is not in array and append it

   if (CheckGroup(cfg.achAreaGroups, chGroup, NULL) &&
      !CheckGroup(achGroup, chGroup, NULL))
     achGroup[xstrlen(achGroup)] = chGroup;
 }

/*
 * This subroutine fixes the node area group specifications
 */

 static VOID SUBENTRY DoCheckNodeGroupSpec(VOID)
 {
   CHAR achGroup[NUM_GROUPS + 1];
   BOOL fDefGroups;
   PNODE pnode;
   PCH pch;

   // Loop through all the nodes descriptors

   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext) {

     // Reset things for this node

     xmemset(achGroup, 0, sizeof(achGroup)); fDefGroups = FALSE;

     // Check if this node does not have groups specified and
     // add the default groups if so

     if (pnode->pszGroupSpec == NULL) {
       fDefGroups = TRUE;
     } else {
       for (pch = pnode->pszGroupSpec; *pch; pch++) {
         if (*pch == '*') fDefGroups = TRUE;
         else DoAddGroup(achGroup, *pch);
       }
     }

     // Check if we need to add default groups and do it

     if (fDefGroups && cfg.pszDefNodeGroups)
       for (pch = cfg.pszDefNodeGroups; *pch; pch++)
         DoAddGroup(achGroup, *pch);

     // Allocate the existing group list and check if ok

     if ((pnode->pszGroup = AllocString(achGroup, -1)) == NULL) {
       WriteLog("Insufficient memory (node groups)\n");
       exit(EXIT_FAILURE);
     }

#ifdef DEBUG
//fprintf(STDAUX, "DoCheckNodeGroupSpec: %s\t%s\t%s\r\n", FormatNetAddr(&pnode->netAddr), pnode->pszGroupSpec, pnode->pszGroup);
#endif
   }
 }

/////////////////////////////////////////////////////////////////////////////
// P u b l i c   r o u t i n e s                                           //
/////////////////////////////////////////////////////////////////////////////

/*
 * This routine inits the config file parser
 */

 VOID APPENTRY InitCfgParser(VOID)
 {
   // Point newarea list pointer to default newarea descriptor

   cfg.pnewareaFirst = &newareaDef;

   // Make up both key tables

   DoMakeKeyHash(atabSqaf, numbof(atabSqaf));
   DoMakeKeyHash(atabSqsh, numbof(atabSqsh));

   // Get direct pointers to most oftenly used keywords

   ptabSqafNewAreaPlace = DoSearchKey(atabSqaf, numbof(atabSqaf), SQXXNEWAREAPLACE);
   ptabSqshNewAreaPlace = DoSearchKey(atabSqsh, numbof(atabSqsh), SQXXNEWAREAPLACE);
 }

/*
 * This routine scans Squish or SqaFix configuration file
 */

 VOID APPENTRY ScanCfgFile(BOOL fActive)
 {
#ifdef DEBUG
   clock_t clk;
#endif
   USHORT cch, cchMaxLine = min(cchBuf, 0x7fffu) - 1;
   BOOL fFail = FALSE;
   PCH pch, pchEnd;
   PSZ pszCfgFile;
   FILE * pfile;

   // Get the appropriate config file name

   pszCfgFile = fActive ? cfg.achCfgSqsh : cfg.achCfgSqaf;

   // Open the configuration file

   if ((pfile = fopen(pszCfgFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszCfgFile);
     exit(EXIT_FAILURE);
   } else {
     printf("Scanning file: %s\n", pszCfgFile);
#ifdef DEBUG
     clk = clock();
#endif
   }

   // Scan in the configuration file

   pchBuf[cchMaxLine] = '\0';
   iLine = 0;

   loop {

     iLine++;

     // Read the subsequent line from the config file and check if ok

     if (fgets(pchBuf, cchMaxLine, pfile) == NULL)
       if (!feof(pfile)) {
         WriteLog("$ Can't read file: %s\n", pszCfgFile);
         exit(EXIT_FAILURE);
       } else
         break;

     // Check if we got the complete line and remove the trailing \n

     if (*(pch = xstrchr(pchBuf, 0) - 1) == '\n')
       *pch = '\0';
     else
       if (!feof(pfile)) {
         DoLineError("Line is too long: '%s'\n", pszCfgFile);
         exit(EXIT_FAILURE);
       }

     // Remove all the trailing spaces if any

     for (--pch; pch >= pchBuf && isspace(*pch); --pch) *pch = '\0';

     // Skip over leading spaces and check if this is an empty or
     // empty line % comment line

     for (pch = pchBuf; isspace(*pch); pch++);
     if (*pch && *pch != '%') {

       // Fix up the end of the keyword if any

       for (pchEnd = pch, cch = 0; *pchEnd && !isspace(*pchEnd);
            pchEnd++, cch++);
       while (isspace(*pchEnd)) *pchEnd++ = '\0';

       // Call the appropriate line scanner

       if (fActive)
         DoScanSqshLine(pch, cch, pchEnd);
       else
         DoScanSqafLine(pch, cch, pchEnd);
     }
   }

   fclose(pfile);

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s\n", pszCfgFile);

#ifdef DEBUG
fprintf(STDAUX, "ScanCfgFile: %s\t%lu tics\r\n", pszCfgFile, clock() - clk);
#endif

   // Make sure we have everything what we need and echo area
   // definitions in Squish and Sqafix config files match

   if (!fActive) {
     if (!(fsCfgScan & CF_DEFADDR)) {
       WriteLog("! Missing 'Address' keyword\n");
       fFail = TRUE;
     }
     if (!(fsCfgScan & CF_DEFMAIL)) {
       WriteLog("! Missing 'NetMail' keyword\n");
       fFail = TRUE;
     }
     if (!(fsCfgScan & CF_DEFSQSH)) {
       WriteLog("! Missing 'SquishCfg' keyword\n");
       fFail = TRUE;
     }
     if (!(fsCfgScan & CF_DEFQUEF)) {
       WriteLog("! Missing 'QueueFile' keyword\n");
       fFail = TRUE;
     }
   }

   // Abort execution if error detected

   if (fFail) exit(EXIT_FAILURE);
 }

/*
 * This routine quits the config file parser
 */

 VOID APPENTRY QuitCfgParser(VOID)
 {
#ifdef DEBUG
//fprintf(STDAUX, "QuitCfgParser: known groups %s\r\n", achAreaGroups);
#endif

   // Set message line and part length defaults if not specified

   if (!cfg.cchMaxMsgLine) cfg.cchMaxMsgLine = DEF_MSG_LINE;
   if (!cfg.cchMaxMsgPart) cfg.cchMaxMsgPart = cchBuf - 1;

   // Check all the relevant info

   DoCheckNewAreaSpec();
   DoCheckNodeGroupSpec();

   // Check notification areas specification

   DoCheckAreaList(cfg.apszFReqNote, numbof(cfg.apszFReqNote),
                   "(ForwardRequestNotify)");
   DoCheckAreaList(cfg.apszIdleNote, numbof(cfg.apszIdleNote),
                   "(IdlePassthruNotify)");

   // Check packer specifications

   CheckArcSpec();
 }

/*
 * This routine checks the area specification sync in SqaFix and Squish cfgs
 */

 VOID APPENTRY CheckAreaSpec(BOOL fSync)
 {
   USHORT fs = 0;
   PAREA parea;

   // Scan through all the defined areas

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
     if (!(parea->fs & AF_INSQAFIXCFG)) {
       if (!fSync) WriteLog("! Area %s is not defined in %s\n",
                             parea->achTag, cfg.achCfgSqaf);
       fs|= AF_INSQAFIXCFG;
     }
     if (!(parea->fs & AF_INSQUISHCFG)) {
       if (!fSync) WriteLog("! Area %s is not defined in %s\n",
                             parea->achTag, cfg.achCfgSqsh);
       fs|= AF_INSQUISHCFG;
     }
   }

   // Check if areas are not in sync and we're not running Sync command

   if (fs && !fSync) exit(EXIT_FAILURE);
 }

/*
 * This routine updates the configuration file
 */

 BOOL APPENTRY UpdateCfgFile(BOOL fActive)
 {
   USHORT cchMaxLine = min(cchBuf, 0x7fffu) - 1;
   BOOL fWriteLineBack, fWriteNewAreasFirst = TRUE;
   BOOL fEchoArea, fWriteNewAreas, fNewAreasWritten = FALSE;
   BOOL fPack, fSkipPack = FALSE;
   CHAR achTmpFile[MAXPATH];
   CHAR achBakFile[MAXPATH];
   CHAR achDrive[MAXDRIVE];
   CHAR achDir[MAXDIR];
   CHAR achFile[MAXFILE];
   PSZ  pszCfgFile;
   FILE * pfileCfg;
   FILE * pfileTmp;
   PCH  pch, pchEnd;
   INT ch;

   // Check if there was an appropriate new area placement specification and
   // if so disable the logic which puts them before the first EchoArea line

   if (fActive) {
     if (cfg.fl & FL_SQSHNEWAREAPLACE) fWriteNewAreasFirst = FALSE;
   } else {
     if (cfg.fl & FL_SQAFNEWAREAPLACE) fWriteNewAreasFirst = FALSE;
   }

   // Get the appropriate config file name

   pszCfgFile = fActive ? cfg.achCfgSqsh : cfg.achCfgSqaf;

   // Build the temporary and backup file names

   fnsplit(pszCfgFile, achDrive, achDir, achFile, NULL);
   fnmerge(achTmpFile, achDrive, achDir, achFile, DEF_TMP_EXT);
   fnmerge(achBakFile, achDrive, achDir, achFile, DEF_BAK_EXT);

   // Open the config file and check if ok

   if ((pfileCfg = fopen(pszCfgFile, "rt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", pszCfgFile);
     return FALSE;
   } else
     WriteLog("* Updating file: %s\n", pszCfgFile);

   // Open the temporary config file and check if ok

   if ((pfileTmp = fopen(achTmpFile, "wt")) == NULL) {
     WriteLog("$ Can't open file: %s\n", achTmpFile);
     fclose(pfileCfg);
     return FALSE;
   }

   // Write the temporary configuration file

   WriteLog("- Make %s\n", achTmpFile);

   pchBuf[cchBuf - 1] = '\0';

   loop {

     // Read the line from the config file and check if ok,
     // assume that this line would be written back

     fWriteLineBack = TRUE;
     if (fgets(pchBuf, cchMaxLine, pfileCfg) == NULL) {
       if (!feof(pfileCfg)) {
         WriteLog("$ Can't read file: %s\n", pszCfgFile);
         fclose(pfileCfg); fclose(pfileTmp);
         return FALSE;
       } else {

         // We're about to close output file, so check to see if we still
         // need to write out new areas past the end of file and do it.
         // Note: this happens if there are no EchoArea or ;NewAreaPlace

         if (!fNewAreasWritten) {
           if (!DoWriteNewAreas(pfileTmp, fActive)) {
FailTmp:     WriteLog("$ Can't write file: %s\n", achTmpFile);
             fclose(pfileCfg); fclose(pfileTmp);
             return FALSE;
           }
         }

         // Also check to see if we still need to update packers list.
         // This happens if there are no Pacl statemnts yet

         if (fActive && cfg.fPackChanged) {
           if (!DoWritePackers(pfileTmp)) {
             goto FailTmp;
           }
         }

         break;
       }
     }

     // Skip over leading spaces and fix up the end of the keyword if any

     for (pch = pchBuf; isspace(*pch); pch++);
     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     ch = *pchEnd; *pchEnd = '\0';

     // Reset all the relevant flags and check if this is commented out
     // statement

     fEchoArea = fWriteNewAreas = fPack = FALSE;

     // Check if this is EchoArea or NewAreaPlace keyword and set
     // the appropriate flags

     if (!xstricmp(pch, SQXXECHOAREA)) {
       fEchoArea = TRUE;
     } else
     if (!fNewAreasWritten && !xstricmp(pch, SQXXNEWAREAPLACE)) {
       fWriteNewAreas = TRUE;
     } else
     if (fActive && !xstricmp(pch, SQSHPACK)) {
       fPack = TRUE;
     }

     // Restore the past-the-keyword character

     *pchEnd = ch;

     // Check if this line has the 'EchoArea' keyword and update
     // the area definition. If this is a first area defined, then
     // write out all the newly creates areas definitions before it

     if (fEchoArea) {
       if (fWriteNewAreasFirst && !fNewAreasWritten)
         if (DoWriteNewAreas(pfileTmp, fActive)) {
           fNewAreasWritten = TRUE;
         } else
           goto FailTmp;

       fWriteLineBack = DoUpdateEchoArea(pchEnd + 1, fActive, pchBuf + cchBuf);
     } else

     // Check if this is a Squish Pack statement and write out all the
     // packer declarations

     if (fPack) {
       if (cfg.fPackChanged) {
         if (!DoWritePackers(pfileTmp)) goto FailTmp;
         cfg.fPackChanged = FALSE; fSkipPack = TRUE;
         fWriteLineBack = FALSE;
       } else {
         fWriteLineBack = !fSkipPack;
       }
     }

     // Write out the line and append the newline if it's not there.
     // Note: this is important if ;NewAreaPlace is at the very end
     // of the file without a newline. Also note that MS C 6.00 bug:
     // fgets returns 0 instead of last character written

     if (fWriteLineBack) {
       if (fputs(pchBuf, pfileTmp) == EOF) goto FailTmp;
       if ((pch = xstrchr(pchBuf, 0)) != NULL && (pch > pchBuf) &&
          *(pch - 1) != '\n') {
         if (fputc('\n', pfileTmp) == EOF) goto FailTmp;
       }
     }

     // Check if we need to write new areas past this line and do it

     if (fWriteNewAreas && !fNewAreasWritten) {
       if (DoWriteNewAreas(pfileTmp, fActive)) {
         fNewAreasWritten = TRUE;
       } else
         goto FailTmp;
     }
   }

   // Close the both files

   fclose(pfileCfg); fclose(pfileTmp);

   // Delete *.BAK, rename *.CFG to *.BAK, rename *.$$$ to *.CFG

   if (!(cfg.fl & FL_TESTMODE)) {
     if (!DelFile(achBakFile)) return FALSE;
     if (!RenFile(pszCfgFile, achBakFile)) return FALSE;
     if (!RenFile(achTmpFile, pszCfgFile)) return FALSE;
   }

   return TRUE;
 }

/*
 * End of SQACFG.C
 */
