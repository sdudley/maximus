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
 * Main module
 *
 * Created: 06/Jan/92
 * Updated: 24/Dec/99
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 * Update Log:
 *
 * 2003/08/09   rfj       Added copyright and version options
 *                        to command line.
 *
 */

  // Our dpmi library does not support conio so use bios io instead

#ifdef __DPMI__
#define BIOS_CONSOLE
#endif

  // This one controls the dump object code

#define DUMP_CODE

  // Critical error handling is supported only under DOS and DPMI

#if defined(__DOS__) || defined(__DPMI__)
#define CRIT_ERRORHANDLER
#endif

#ifndef UNIX
#include <io.h>
#include <dos.h>
#include <process.h>
#include <conio.h>
#endif
#ifdef BIOS_CONSOLE
#include <bios.h>
#endif
#include "sqafix.h"

  // Include appropriate heap manager declarations

#if   defined (__OS2__)
#elif defined (__W32__)
#elif defined (__DPMI__)
#     include "\dw-text\dpmi\heapmgr.h"
#elif defined (__TURBOC__)
#     include <alloc.h>
#elif defined (__MSC__)
#     include <malloc.h>
#     include <dos.h>
#elif defined (__ZTC__)
#     include <stdlib.h>
#endif

#include "pathdef.h"

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   d e c l a r a t i o n s                                   //
/////////////////////////////////////////////////////////////////////////////

 // Miscellaneous defines

#ifndef UNIX
 #define ISOPTION(ch)  ((ch) == '-' || (ch) == '/')
#else
 #define ISOPTION(ch)  ((ch) == '-')
#endif
 #define ISNEWLINE(ch) ((ch) == '\0' || (ch) == '\r' || (ch) == '\n')

 // Known command codes

 #define CMD_SCAN       0x0001  // Scan netmail for requests
 #define CMD_AUTOCREATE 0x0002  // Scan badmail to autocreate new areas
 #define CMD_LINK       0x0011  // Link node to area or area groups
 #define CMD_UNLINK     0x0012  // Link node from area or area groups
 #define CMD_PASSIVE    0x0013  // Move node to passive list for area or area groups
 #define CMD_ACTIVE     0x0014  // Move node to passive list for area or area groups
 #define CMD_LIST       0x0015  // Report existing available areas
 #define CMD_LINKED     0x0016  // Report linked/passive areas
 #define CMD_UNLINKED   0x0017  // Report unlinked areas
 #define CMD_AVAIL      0x0018  // Report uplink available areas
 #define CMD_NOTIFY     0x0021  // Notify all linked nodes
 #define CMD_REPORT     0x0022  // Notify node of all existing links
 #define CMD_HELP       0x0023  // Send help information
 #define CMD_RULES      0x0024  // Send help information
 #define CMD_CREATE     0x0031  // Create new area
 #define CMD_DESTROY    0x0032  // Destroy existing area
 #define CMD_SYNC       0x0033  // Sync config files echoareas
 #define CMD_MAINT      0x0034  // Perform a maintenace run
 #define CMD_RELINK     0x0035  // Relink existing areas
 #define CMD_REFREQ     0x0036  // Refreq freq areas in queue
 #define CMD_REKILL     0x0037  // Rekill kill areas in queue

#ifdef DUMP_CODE
 #define CMD_DUMPNODE   0x0101  // Dump node control block
 #define CMD_DUMPUPLINK 0x0102  // Dump uplink control block
 #define CMD_DUMPAREA   0x0103  // Dump area control block
#endif

 // Program global variables

 CFG     cfg;                           // Configuration structure
 PCH     pchBuf;                        // Working buffer pointer
 USHORT  cchBuf;                        // Working buffer size
 USHORT  ichBuf;                        // Working buffer used bytes count
 HAREA   hNetMail;                      // Netmail folder handle
 NEWAREA newareaDef;                    // Default new area info
 struct _minf minf;                     // Global msg info

 // Command line request parser table

 static struct {
   PSZ    psz;   USHORT cmd;
 } aCmdReqTab [] = {
   "Scan",       CMD_SCAN,
   "S",          CMD_SCAN,
   "Add",        CMD_LINK,
   "A",          CMD_LINK,
   "Link",       CMD_LINK,
   "Delete",     CMD_UNLINK,
   "Del",        CMD_UNLINK,
   "D",          CMD_UNLINK,
   "Unlink",     CMD_UNLINK,
   "Passive",    CMD_PASSIVE,
   "Active",     CMD_ACTIVE,
   "List",       CMD_LIST,
   "L",          CMD_LIST,
   "Query",      CMD_LINKED,
   "Q",          CMD_LINKED,
   "Linked",     CMD_LINKED,
   "LN",         CMD_LINKED,
   "Unlinked",   CMD_UNLINKED,
   "UL",         CMD_UNLINKED,
   "Notify",     CMD_NOTIFY,
   "N",          CMD_NOTIFY,
   "Report",     CMD_REPORT,
   "R",          CMD_REPORT,
   "Help",       CMD_HELP,
   "H",          CMD_HELP,
   "Rules",      CMD_RULES,
   "Ru",         CMD_RULES,
   "AutoCreate", CMD_AUTOCREATE,
   "AC",         CMD_AUTOCREATE,
   "Create",     CMD_CREATE,
   "Destroy",    CMD_DESTROY,
   "Avail",      CMD_AVAIL,
   "Maintenance",CMD_MAINT,
   "Maint",      CMD_MAINT,
   "ReLink",     CMD_RELINK,
   "ReFReq",     CMD_REFREQ,
   "ReKill",     CMD_REKILL,
   "Sync",       CMD_SYNC,
#ifdef DUMP_CODE
   "DumpNode",   CMD_DUMPNODE,
   "DumpUplink", CMD_DUMPUPLINK,
   "DumpArea",   CMD_DUMPAREA,
#endif
 };

 // Remote request parser table

 static struct {
   PSZ psz;     PFNEXEC pfnExec;        BOOL fGroup;
 } aRemReqTab [] = {
   "%ALL",      ExecLnkNodeGroup,       TRUE,
   "+%ALL",     ExecLnkNodeGroup,       TRUE,
   "-%ALL",     ExecUnlNodeGroup,       TRUE,
   "%PASSIVE",  ExecPasNodeGroup,       TRUE,
   "%ACTIVE",   ExecActNodeGroup,       TRUE,
   "%PAUSE",    ExecPasNodeGroup,       TRUE,
   "%RESUME",   ExecActNodeGroup,       TRUE,
   "%RESCAN",   ExecRescanArea,         FALSE,
   "%RULES",    ExecSendAreaRules,      FALSE,
   "%CREATE",   ExecCreateArea,         FALSE,
   "%DESTROY",  ExecDestroyArea,        FALSE,
   "%COMPRESS", ExecCompress,           FALSE,
   "%LIST",     CreateAreasReport,      FALSE,
   "%QUERY",    CreateLinkedReport,     FALSE,
   "%LINKED",   CreateLinkedReport,     FALSE,
   "%UNLINKED", CreateUnlinkedReport,   FALSE,
   "%AVAIL",    CreateAvailReport,      FALSE,
   "%HELP",     CreateUsageHelp,        FALSE,
 };

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   s u b r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/*
 * This routines are needed because out dpmi library does not support conio :(
 */

 static int SUBENTRY DoCheckKey(void)
 {
#ifdef BIOS_CONSOLE
   return bioskey(1);
#endif
#ifndef UNIX
   return kbhit();
#endif
 }

 static int SUBENTRY DoGetKey(void)
 {
#ifdef BIOS_CONSOLE
   return bioskey(0) & 0x00FF;
#elif defined(UNIX)
   return getchar();
#else
   return getch();
#endif
 }

/*
 * This subroutine is a critical error handler -- just fail the operation
 */

#ifdef CRIT_ERRORHANDLER

#if defined (__MSC__)
 void _far CritErrorHandler(unsigned deverror, unsigned errcode, unsigned far * devhdr)
 {
   _hardresume(_HARDERR_FAIL);
 }

#elif defined (__TURBOC__)
 int CritErrorHandler(int error, int ax, int bp, int si)
 {
   return 3;
 }
#elif
#error "Unknown compiler for critical error handling"
#endif

#endif

/*
 * This subroutine displays logo
 */

 static VOID SUBENTRY DoShowLogo(VOID)
 {
   fprintf(stderr,
"\n"
"Squish Echo Area Manager v"VERSION", "__DATE__", "__TIME__"\n"
"Written by Pete Kvitek of JV Dialogue 1st BBS, 2:5020/6@fidonet\n"
"Copyright (C) 1992-2003 by Pete Kvitek. All rights reserved.\n"
"This program is licensed under the GPL.\n"
"\n"
   );
 }

/*
 * This subroutine displays help
 */

 static VOID SUBENTRY DoShowHelp(VOID)
 {
   time_t tm;

   // Display general help

   fprintf(stdout,
"Usage:    %s <command> [nodeaddr] [options] [areas]\n"
"\n"
"Command:  Scan, AutoCreate, Link, Unlink, Passive, Active, Linked, Unlinked,\n"
"          List, Avail, Help, Rules, Report, Notify, Create, Destroy, Maint,\n"
"          ReLink, ReFreq, ReKill, Sync\n"
"\n"
"Options: -g<...>   - limit the operation scope to these groups\n"
"         -*        - expand the scope to all available node groups\n"
"         -o        - override all the area access restrictions\n"
"         -m        - report by sending a message to the node\n"
"         -n        - suppress force area creation node flags\n"
"         -f        - suppress linkup requests forwarding\n"
"         -v        - verbose mode, show all actions and msgs\n"
#ifdef CRIT_ERRORHANDLER
"         -e        - disable internal critical error handler\n"
#endif
"         -u        - intentionally update both config files\n"
"         -x        - test mode, don't update config files\n"
"         -b<nn>    - working buffer size in kilobytes (1-64k), default is %uk\n"
"         -s<file>  - override Squish config file name, default ext is .CFG\n"
"         -c<file>  - override SqaFix config file name, default ext is .CFG\n"
"         -l<file>  - override SqaFix log file name, default ext is .LOG\n"
"         -q<file>  - override SqaFix que file name, default ext is .QUE\n"
"\n"
"\n",
#if defined(__OS2__)
           "SQAFIXP",
#elif defined(__W32__)
           "SQAFIXW",
#elif defined(__DPMI__)
           "SQAFIXX",
#else
           "SQAFIX",
#endif
           DEF_BUFF_SIZE / 1024
   );

   // Check if displaing on a console and if so, flush keyboard and
   // wait for key pressed not more then 10 secs to prevent accidental
   // system hang in unattended mode. Otherwise just send output to file

   if (isatty(fileno(stdout))) {
     fprintf(stdout, "Press Enter for command summary or any other key to quit help.\n");
#ifndef UNIX

     while (DoCheckKey()) DoGetKey();
     for (tm = time(NULL); !DoCheckKey();) if (tm + 10 < time(NULL)) return;
     if (DoGetKey() != '\r') return;
     if (DoGetKey() != '\n') return;
     fprintf(stdout, "\n");
#else
     getchar();
#endif

   }

   // Display command summary help

   fprintf(stdout,
"\nSqaFix command line commands summary:"
"\n"
"\nScan           - scan and process remote requests in the netmail folder"
"\nAutoCreate     - scan badarea folder and autocreate areas if necessary"
"\nLink,Add       - link the given node to the specified area(s)"
"\nUnlink,Delete  - unlink the given node from the specified area(s)"
"\nPassive        - make the given node passive in the specified area(s)"
"\nActive         - make the given node active in the specified area(s)"
"\nLinked,Query   - list status of linked/passive areas of a given node"
"\nUnlinked       - list the unlinked exsiting areas for a given node"
"\nList           - list status of existing areas available to a given node"
"\nAvail          - list status of uplink areas available to a given node"
"\nHelp           - send remote usage help information to a given node"
"\nRules          - send linked area(s) rules to a given node"
"\nReport         - send list of all existing links to a given node"
"\nNotify         - send status of all areas to all known nodes"
"\nCreate         - create specified area and link the given node"
"\nDestroy        - destroy specified area and notify the given node"
"\nReLink         - resend AreaFix request to uplink for existing area(s)"
"\nReFreq         - resend AreaFix request to uplink for freq queue area(s)"
"\nReKill         - resend AreaFix request to uplink for kill queue area(s)"
"\nMaint          - execute maintenance mode tasks in command line"
"\nSync           - sync Squish and SqaFix EchoArea definitions"
"\n"
   );
 }

/*
 * This subroutine returns available memory under OS/2 protected mode
 */
#ifdef __OS2__
 static ULONG SUBENTRY DoMemAvail(VOID)
 {
   ULONG cbMemAvail;

   if (DosMemAvail(&cbMemAvail)) return FALSE;

   return cbMemAvail;
 }
#endif
/*
 * This subroutine returns available memory under Win32 protected mode
 */
#ifdef __W32__
 static ULONG SUBENTRY DoMemAvail(VOID)
 {
    W32MEMORYSTATUS w32mst;

    W32GlobalMemoryStatus(&w32mst);

    return w32mst.dwAvailPhys;
 }
#endif
/*
 * This subroutine returns available memory under DPMI protected mode
 */
#ifdef __DPMI__
 static ULONG SUBENTRY DoMemAvail(VOID)
 {
   struct {
     ULONG      LargestContiguousBlock;
     ULONG      MaxunloackedPageAllocation;
     ULONG      MaxLoackedPageAllocation;
     ULONG      PagesOfLinearSpace;
     ULONG      TotalUnlockedPages;
     ULONG      TotalFreePages;
     ULONG      TotalPhysicalPages;
     ULONG      FreePagesOfLinearSpace;
     ULONG      SizeOfPagingPartition;
     ULONG      Reserved[3];
   } dpmiMemInfo, * pdpmiMemInfo = &dpmiMemInfo;

ASM     mov     ax, 0500h               // dpmi get memory info
ASM     les     di, pdpmiMemInfo        // memory info struct pointer
ASM     int     31h                     // call dpmi service

   return dpmiMemInfo.LargestContiguousBlock + MemAvail();
 }
#endif
/*
 * This subroutine returns amount of free memory
 */

 ULONG APPENTRY GetFreeMemory(VOID)
 {
   ULONG cb = 0;

   // Get the free memory according to the operating environmetn

#if   defined(__OS2__) || defined(__W32__) || defined(__DPMI__)
   cb = DoMemAvail();
#elif defined(__TURBOC__)
   cb = coreleft();
#elif defined(__MSC__)
   USHORT cSeg;
   _dos_allocmem(-1, &cSeg);
   cb = cSeg * 16l;
#endif

   return cb;
 }

/////////////////////////////////////////////////////////////////////////////
// I n i t i a l i z a t i o n   a n d   t e r m i n a t i o n             //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine initializes program
 */

 static VOID SUBENTRY DoInitSqaFix(SHORT cArg, PSZ apszArg[])
 {
   SHORT iArg, iArg1st, iCmd;
   CHAR achDrive[MAXDRIVE];
   CHAR achDir[MAXDIR];
   PAREA parea;
   PSZ psz;

   // Display logo screen and check if there is anything specified
   // in the command line and if no, display the help screen and exit

   DoShowLogo();
   if (cArg == 1) {
     DoShowHelp();
     exit(EXIT_FAILURE);
   }

   // Loop through the command table looking for a match

   for (iCmd = 0;
        iCmd < numbof(aCmdReqTab) && xstricmp(aCmdReqTab[iCmd].psz, apszArg[1]);
        iCmd++);

   // Check if we got a match and set the command code

   if (iCmd >= numbof(aCmdReqTab)) {
     WriteLog("! Invalid command: '%s'\n", apszArg[1]);
     exit(EXIT_FAILURE);
   } else
     cfg.cmdCode = aCmdReqTab[iCmd].cmd;

   // Determine the first option index to start with since we
   // don't need node address specification for some commands

   iArg1st = (cfg.cmdCode == CMD_SCAN ||
              cfg.cmdCode == CMD_NOTIFY ||
              cfg.cmdCode == CMD_AUTOCREATE) ? 1 : 2;

   // Process some of the options we need to know in advance and
   // flag those which are not known

   for (iArg = iArg1st; iArg < cArg; iArg++)
     if (ISOPTION(apszArg[iArg][0]))
       switch (tolower(apszArg[iArg][1])) {
         case 'v':      // verbose mode request
                   cfg.fl|= FL_VERBOSEMODE | FL_DISPLAYGENERATEDMSG;
                   break;
         case 'm':      // report sending msg to a node
                   cfg.fl|= FL_REPORTMSG;
                   break;
         case 'n':      // suppress force area creation node flags
                   cfg.fl|= FL_IGNORESENDCREATE;
                   break;
         case 'f':      // suppress request forwarding
                   cfg.fl|= FL_NOREQUESTFORWARDING;
                   break;
         case 'e':      // disable critical error handler
                   cfg.fl|= FL_NOERRORTRAP;
                   break;
         case 'o':      // override group access restrictions
                   cfg.fl|= FL_OVERRIDEGROUP;
                   break;
         case 'x':      // test mode -- keep cfg/msg and que if upper case
                   cfg.fl|= FL_TESTMODE;
                   if (isupper(apszArg[iArg][1])) cfg.fl|= FL_TESTMODEX;
                   break;
         case 'u':      // rewrite all config files
                   cfg.fl|= FL_REWRITECFG;
                   break;
         case 'b':      // working buffer size (in kilobytes)
                   cchBuf = atoi(&apszArg[iArg][2]);
                   break;
         case 'c':      // config file name override ... scan later
                   break;
         case 's':      // squish config file name override ... scan later
                   break;
         case 'l':      // log file name override ... scan later
                   break;
         case 'q':      // que file name override ... scan later
                   break;
         case 'g':      // area groups specification ... scan later
                   break;
         case '*':      // all available area groups... scan later
                   break;
         default :
                   WriteLog("! Invalid option: '%s'\n", apszArg[iArg]);
                   exit(EXIT_FAILURE);
       }

   // Install the critical error handler if required

#ifdef CRIT_ERRORHANDLER
#if defined (__MSC__)
   if (!(cfg.fl & FL_NOERRORTRAP)) _harderr(CritErrorHandler);
#endif

#if defined (__TURBOC__)
   if (!(cfg.fl & FL_NOERRORTRAP)) harderr(CritErrorHandler);
#endif
#endif

   // Allocate memory block for the working buffer

   if (cchBuf == 0)  cchBuf = DEF_BUFF_SIZE; else
   if (cchBuf >= 64) cchBuf = MAX_BUFF_SIZE; else
                     cchBuf*= 1024;

   if ((pchBuf = MemAlloc(cchBuf, 0)) == NULL) {
     WriteLog("! Insuffucient memory (workbuff): %u\n", cchBuf);
     exit(EXIT_FAILURE);
   }

   // Set up the relevant control flags according to the control mode

   if (cfg.cmdCode == CMD_SCAN || cfg.cmdCode == CMD_HELP ||
       cfg.cmdCode == CMD_RULES || cfg.cmdCode == CMD_REPORT ||
       cfg.cmdCode == CMD_NOTIFY)
     cfg.fl|= FL_REPORTMSG;
   else
     if (!(cfg.fl & FL_REPORTMSG))
       cfg.fl|= FL_DISPLAYGENERATEDMSG;

   // Add the primary alias name

   AddApsz(cfg.apszAlias, numbof(cfg.apszAlias), "SqaFix");

   // Process the SQAFSWAP environment variable and
   // initialize the Ralf Brown's spawno library

#ifdef __SPAWNO__
   if ((psz = getenv("SQAFSWAP")) != NULL) {
     cfg.pszSwapPath = AllocString(psz, -1);
     cfg.fsSwapSpawn|= SWAP_DISK;
   }

   init_SPAWNO(cfg.pszSwapPath, cfg.fsSwapSpawn);
#endif

   // Process the SQUISH environment variable if any and set the
   // Squish and SqaFix configuration file names

   if ((psz = getenv("SQUISH")) != NULL) {
     AppendFileExt(cfg.achCfgSqsh, psz, DEF_CFG_EXT, FALSE);
     fnsplit(cfg.achCfgSqsh, achDrive, achDir, NULL, NULL);
     fnmerge(cfg.achCfgSqaf, achDrive, achDir, SQAFIX_CFG, DEF_CFG_EXT);
#ifndef UNIX
     xstrupr(cfg.achCfgSqaf);
#endif
   }

   // Process the file name overrides

   for (iArg = iArg1st; iArg < cArg; iArg++)
     if (ISOPTION(apszArg[iArg][0]))
       switch (tolower(apszArg[iArg][1])) {
         case 'l':      // SqaFix log file name override
                   AppendFileExt(cfg.achLogFile, &apszArg[iArg][2], DEF_LOG_EXT, FALSE);
                   break;
         case 'q':      // SqaFix que file name override
                   AppendFileExt(cfg.achQueFile, &apszArg[iArg][2], DEF_QUE_EXT, FALSE);
                   break;
         case 'c':      // SqaFix config file name override
                   AppendFileExt(cfg.achCfgSqaf, &apszArg[iArg][2], DEF_CFG_EXT, FALSE);
                   break;
         case 's':      // Squish config file name override
                   AppendFileExt(cfg.achCfgSqsh, &apszArg[iArg][2], DEF_CFG_EXT, FALSE);
                   break;
       }

   // If there is no SqaFix config file name override then check if
   // the config file exists in the current directory and if not, look
   // for one in the directory where the executable was started from

   if (!cfg.achCfgSqaf[0]) {
     xstrcpy(cfg.achCfgSqaf, SQAFIX_CFG""DEF_CFG_EXT);
#ifndef UNIX
     if (access(xstrupr(cfg.achCfgSqaf), 0)) {
#else
     if (access(cfg.achCfgSqaf, 0)) {
#endif
       fnsplit(apszArg[0], achDrive, achDir, NULL, NULL);
       fnmerge(cfg.achCfgSqaf, achDrive, achDir, SQAFIX_NAME, DEF_CFG_EXT);
#ifndef UNIX
       if (access(xstrupr(cfg.achCfgSqaf), 0)) {
#else
       if (access(cfg.achCfgSqaf, 0)) {
#endif
         WriteLog("! Can't locate SqaFix config file\n");
         exit(EXIT_FAILURE);
       }
     }
   }

   // Qualify the SqaFix config file path

   if (!BuildFullPath(cfg.achCfgSqaf, cfg.achCfgSqaf)) {
     WriteLog("! Invalid file path: %s\n", cfg.achCfgSqaf);
     exit(EXIT_FAILURE);
   }

   // Scan in the SqaFix and Squish config files

   InitCfgParser();
   ScanCfgFile(PASSIVE);
   ScanCfgFile(ACTIVE);
   QuitCfgParser();

   // Check out the area specifications sync in SqaFix and Squish configs

   CheckAreaSpec(cfg.cmdCode == CMD_SYNC);

   // Reset all the passive and active list changed flags unless we don't
   // need config files be explicitely rewritten and force packer list
   // update otherwise

   if (!(cfg.fl & FL_REWRITECFG)) {
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
       parea->fs&= ~(AF_ACTIVECHANGED | AF_PASSIVECHANGED);
   } else {
     if (cfg.plszAllowArc) cfg.fPackChanged = TRUE;
   }

   // Initialize the message API and check if ok

   minf.req_version = 0;
   minf.def_zone = cfg.anetAddr[0].zone;

   if (MsgOpenApi(&minf) == -1) {
     WriteLog("! MsgAPI initialization failed\n");
     exit(EXIT_FAILURE);
   }

   // Check if we have to access netmail folder and open it if so

   OpenNetMailFolder();
 }

/*
 * This subroutine calculates outstanding freq for all the nodes
 */

 static VOID SUBENTRY DoCountNodeFreq(VOID)
 {
   PNODE pnode;
   PLINK plink;
   PQUE pque;

   // Loop through all the known nodes and queue entries

   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext) {
     for (pque = cfg.pqueFirst; pque != NULL; pque = pque->pqueNext) {

       // Check if this is freq entry and skip if not

       if (pque->type != QE_FREQ) continue;

       // Check if the node this area was requested for match current node
       // and increment its freq counter


       if (pque->plink != NULL && (plink = pque->plink->plinkNext) != NULL)
         if (!xmemcmp(&pnode->netAddr, &plink->netAddr, sizeof(NETADDR)))
           pnode->numFreq++;
     }
#ifdef DEBUG
//if (pnode->numFreq) fprintf(STDAUX, "DoCountNodeFreq: %s\tnumFreq=%u, maxFreq=%u\r\n", FormatNetAddr(&pnode->netAddr), pnode->numFreq, pnode->maxFreq);
#endif
   }
 }

/*
 * This subroutine terminates program
 */

 static VOID SUBENTRY DoQuitSqaFix(VOID)
 {
   // Close the netmail folder if it's open

   if (hNetMail != NULL)
     CloseNetMailFolder();

   // Shut down the message API

   if (MsgCloseApi() == -1)
     WriteLog("! MsgAPI shutdown failed\n");

   // Tell 'em we did not change a thing...

   if (cfg.fl & FL_TESTMODE)
     WriteLog("* TEST MODE -- REQUESTS NOT MARKED, CONFIGS NOT UPDATED%s\n",
               cfg.fl & FL_TESTMODEX ? ", QUEUE NOT SAVED" : "");

   // Release the memory blocks

   if (pchBuf != NULL) MemFree(pchBuf);
 }

/*
 * This subroutine updates the config files if needed
 */

 static BOOL SUBENTRY DoUpdateCfgFiles(VOID)
 {
   PAREA parea;
   USHORT fs;

   // Collect all the areas flags

   for (fs = 0, parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
     fs|= parea->fs;

   // Check if there are changes in the active areas list or packer and
   // update the Squish config file

   if ((cfg.fl & FL_REWRITECFG || fs & AF_ACTIVECHANGED || cfg.fPackChanged) &&
       !UpdateCfgFile(ACTIVE))
     return FALSE;

   // Check if there are changes in the passive areas list and
   // update the SqaFix config file

   if ((cfg.fl & FL_REWRITECFG || fs & AF_PASSIVECHANGED) &&
       !UpdateCfgFile(PASSIVE))
     return FALSE;

   return TRUE;
 }

/////////////////////////////////////////////////////////////////////////////
// M a n u a l   m o d e   p r o c e s s i n g                             //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine is a manual mode effector
 */

 static VOID SUBENTRY DoRunManualMode(SHORT cArg, PSZ apszArg[])
 {
   NETADDR netAddr = cfg.anetAddr[0];
   PNODE pnode = NULL;
   PSZ pszGroup;
   SHORT iArg;

   // Check if there is a node address specified

   if (cArg <= 2) {
     WriteLog("! Node address missing\n");
     exit(EXIT_FAILURE);
   }

   // Scan in the node address and check if ok

   if (ScanNetAddr(&netAddr, apszArg[2]) == NULL) {
     WriteLog("! Invalid node address: '%s'\n", apszArg[2]);
     exit(EXIT_FAILURE);
   }

   // Check if this node is known to the SqaFix

   if ((pnode = GetNodeFromAddr(&netAddr)) == NULL) {
     WriteLog("! Node %s is not known\n", FormatNetAddr(&netAddr));
     exit(EXIT_FAILURE);
   }

   // Increment running in manual mode flag

   cfg.fManualMode++;

   // Check if we need to report by a message and initiate a message write

   if (cfg.fl & FL_REPORTMSG)
     SendLongMsgBeg(GetAddrMatch(&netAddr), SQAFIX_NAME, &netAddr,
                    GetNodeSysop(pnode), GetNodeMsgAttr(pnode),
                    "Manual mode operation report", TRUE);

   // Carry out command

   if (cfg.cmdCode == CMD_LINKED)   CreateLinkedReport(pnode, NULL);    else
   if (cfg.cmdCode == CMD_LIST)     CreateAreasReport(pnode, NULL);     else
   if (cfg.cmdCode == CMD_AVAIL)    CreateAvailReport(pnode, NULL);     else
   if (cfg.cmdCode == CMD_UNLINKED) CreateUnlinkedReport(pnode, NULL);  else
   if (cfg.cmdCode == CMD_HELP)     CreateUsageHelp(pnode, NULL);       else {

     for (iArg = 3; iArg < cArg; iArg++)
       if (!ISOPTION(apszArg[iArg][0])) {
         switch (cfg.cmdCode) {
           case CMD_LINK:    ExecExistAreaMask(pnode, apszArg[iArg], ExecLnkNodeArea);  break;
           case CMD_UNLINK:  ExecExistAreaMask(pnode, apszArg[iArg], ExecUnlNodeArea);  break;
           case CMD_PASSIVE: ExecExistAreaMask(pnode, apszArg[iArg], ExecPasNodeArea);  break;
           case CMD_ACTIVE:  ExecExistAreaMask(pnode, apszArg[iArg], ExecActNodeArea);  break;
           case CMD_RULES:   ExecExistAreaMask(pnode, apszArg[iArg], ExecRulNodeArea);  break;
           case CMD_RELINK:  ExecExistAreaMask(pnode, apszArg[iArg], ExecRLnNodeArea);  break;
           case CMD_REFREQ:  ExecQueueAreaMask(pnode, apszArg[iArg], ExecRFrNodeArea);  break;
           case CMD_REKILL:  ExecQueueAreaMask(pnode, apszArg[iArg], ExecRKlNodeArea);  break;
           case CMD_DESTROY: ExecDestroyArea(pnode, apszArg[iArg]);  break;
           case CMD_CREATE:  ExecCreateArea(pnode,  apszArg[iArg]);  break;
         }
       } else
       if (tolower(apszArg[iArg][1]) == 'g') {
         switch (cfg.cmdCode) {
           case CMD_LINK:    ExecLnkNodeGroup(pnode, &apszArg[iArg][2]); break;
           case CMD_UNLINK:  ExecUnlNodeGroup(pnode, &apszArg[iArg][2]); break;
           case CMD_PASSIVE: ExecPasNodeGroup(pnode, &apszArg[iArg][2]); break;
           case CMD_ACTIVE:  ExecActNodeGroup(pnode, &apszArg[iArg][2]); break;
         }
       } else
       if (tolower(apszArg[iArg][1]) == '*') {
         pszGroup = cfg.fl & FL_OVERRIDEGROUP ? cfg.achAreaGroups : pnode->pszGroup;
         switch (cfg.cmdCode) {
           case CMD_LINK:    ExecLnkNodeGroup(pnode, pszGroup); break;
           case CMD_UNLINK:  ExecUnlNodeGroup(pnode, pszGroup); break;
           case CMD_PASSIVE: ExecPasNodeGroup(pnode, pszGroup); break;
           case CMD_ACTIVE:  ExecActNodeGroup(pnode, pszGroup); break;
         }
       }

     // Report the link changes

     if (!CreateChangesReport(pnode, NULL))
       switch (cfg.cmdCode) {
         case CMD_LINK:    case CMD_UNLINK:
         case CMD_PASSIVE: case CMD_ACTIVE:
              WriteMsg("\nLinks not changed for node %s\n", FormatNetAddr(&netAddr));
              break;
       }
   }

   // Check if we're reporting by a message and finish message writing
   // flushing the message buffer if anything is still there...

   if (cfg.fl & FL_REPORTMSG)
     SendLongMsgEnd();

   // Decrement running in manual mode flag

   cfg.fManualMode--;
 }

/////////////////////////////////////////////////////////////////////////////
// R e m o t e   m o d e   p r o c e s s i n g                             //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine checks for the valid subject line for loop detection
 */

 static BOOL SUBENTRY DoCheckSubj(PSZ psz)
 {
   PCH pch = psz;

   // Check for empty subject line and get past the end of the password

   if (SkipSpaces(&psz))
     for (pch = psz; *pch && !isspace(*pch); pch++);
   else
     return FALSE;

   // Check the password length

   if ((USHORT)(pch - psz) > MAX_PASS_LENG)
     return FALSE;

   // Check out the options if any

   for (; SkipSpaces(&pch); pch+=2) {
     if (pch[0] != '-') return FALSE;
     if (pch[1] && pch[2] && !isspace(pch[2])) return FALSE;
   }

   return TRUE;
 }

/*
 * This subroutine extracts password from the subject line
 */

 static PSZ SUBENTRY DoGetPassword(PSZ psz)
 {
   static CHAR ach[MAX_PASS_LENG + 1];
   SHORT ich;

   SkipSpaces(&psz);
   xmemset(ach, 0, sizeof(ach));
   for (ich = 0; psz[ich] && !isspace(psz[ich]); ich++)
     if (ich < lengof(ach))
       ach[ich] = psz[ich];

   return ach;
 }

/*
 * This subroutine to process AreaFix style requests in the subject line
 */

 static BOOL SUBENTRY DoProcSubjReqs(PNODE pnode, HMSG hmsg, XMSG * pmsg, UMSG umsg)
 {
   PCH pch = pmsg->subj;

   // Check for empty subject line and get past the end of the password

   if (SkipSpaces(&pch))
     while (*pch && !isspace(*pch)) pch++;
   else
     return FALSE;

   // Process all the options if any

   while (SkipSpaces(&pch)) {
     if (*pch != '-')
       WriteMsg("\nInvalid option '%s', ignored\n", pch);
     else
       switch (tolower(pch[1])) {
         case '?': CreateUsageHelp(pnode, NULL);        break;
         case 'l': CreateAreasReport(pnode, NULL);      break;
         case 'a': CreateAvailReport(pnode, NULL);      break;
         case 'q': CreateLinkedReport(pnode, NULL);     break;
         case 'u': CreateUnlinkedReport(pnode, NULL);   break;
         default : WriteMsg("\nUnknown option '%s', ignored\n", pch);
       }

     // Advance to the next option

     pch++; pch++;
   }

   return TRUE;
 }

/*
 * This subroutine returns the remote command operand
 */

 static PSZ SUBENTRY DoGetRemCmdParam(PNODE pnode, CHAR chEnd, PCH pch, BOOL fGroup)
 {
   static CHAR achScope[256];
   SHORT ich;

   // Check if we finished at the new line character so there
   // can not be any scope specification and return defaults

   if (ISNEWLINE(chEnd))
     return fGroup ? pnode->pszGroup : NULL;

   // Skip over blanks and tabs, but not \r and \n and
   // copy the scope specification into the private storage

   xmemset(achScope, 0, sizeof(achScope));
   while (*pch == ' ' || *pch == '\t') pch++;
   for (ich = 0; *pch && !isspace(*pch); pch++, ich++)
     if (ich < sizeof(achScope) - 2)    // additinal null for trailing
       achScope[ich] = toupper(*pch);   // second operand

   // Check if there is anything past the scope specification
   // and copy it past the scope specification string after the trailing null

   while (*pch == ' ' || *pch == '\t') pch++;
   if (!ISNEWLINE(*pch) && ich < sizeof(achScope) - 2) {
     for (ich++; !ISNEWLINE(*pch); ich++, pch++)
       if (ich < sizeof(achScope) - 1)
         achScope[ich] = *pch;
   }

   return ich > 0 ? achScope : fGroup ? pnode->pszGroup : NULL;
 }

/*
 * This subroutine to process requests in the message body
 */

 static BOOL SUBENTRY DoProcBodyReqs(PNODE pnode, HMSG hmsg, XMSG * pmsg, UMSG umsg)
 {
   static CHAR achTear[] = "---";
   static CHAR achOrigin[] = "* Origin:";       // no leading space ok
   ULONG cb = MsgGetTextLen(hmsg);
   USHORT cchText, iCmd;
   PCH pch, pchEnd;
   CHAR ch, chEOL;
   PCH pchText;

   // Check for huge message

   if (cb > 0x0FF00lu) {
Drop:WriteLog("! Msg# %lu is too long to process\n", umsg);
     WriteMsg("\nMemory allocation failure occured while processing your request,"
              "\nSplit message into smaller parts and and try again..."
              "\n");
     return FALSE;
   }

   // Get short length of the message body and check if ok

   if ((cchText = (USHORT) cb) == 0)
     return FALSE;

   // Allocate memory block to hold the entire message body text
   // Note the extra for advance string check

   if ((pchText = MemAlloc(cchText + lengof(achOrigin), 0)) == NULL)
     goto Drop;

   // Read the entire message body text into the memory buffer

   if ((cchText = MsgReadMsg(hmsg, NULL, 0L, cchText, pchText, 0L, NULL)) == (USHORT)-1) {
     WriteLog("! Can't read body of msg #%lu\n", umsg);
     WriteMsg("\nSevere error occured while processing of your request,"
              "\nYou have nothing to do but resubmit it... sorry."
              "\n");
     MemFree(pchText);
     return FALSE;
   } else
     pchText[cchText] = '\0';

   // Scan the message body looking for the commands

   for (pch = pchText; *pch && SkipSpaces(&pch); pch++) {

     // Check for the kludge character. Note: if ctrl info length is
     // more than 512 bytes, MSGAPI returns the rest at the beginning
     // of the message body which causes scan to terminate early...

     if (pch[0] == 0x01) break;

     // Check for the tear line

     if (!xmemcmp(pch, achTear, lengof(achTear))) break;

     // Check for the special case: origin line without tear line.
     // Since origin line starts with *, it's accepted as "link all"
     // request, which is very unpleasant

     if (!xmemcmp(pch, achOrigin, lengof(achOrigin))) break;

     // Scan past the end of the command specification and fix it

     for (pchEnd = pch; *pchEnd && !isspace(*pchEnd); pchEnd++);
     ch = *pchEnd; *pchEnd = '\0';

     // Loop through the command table looking for a match

     for (iCmd = 0;
          iCmd < numbof(aRemReqTab) && xstricmp(aRemReqTab[iCmd].psz, pch);
          iCmd++);

     // Check if we got a match and call the appropriate executor,
     // otherwise check the first character and take the appropriate
     // action

     if (iCmd < numbof(aRemReqTab)) {
       aRemReqTab[iCmd].pfnExec(pnode,
         DoGetRemCmdParam(pnode, ch, pchEnd + 1, aRemReqTab[iCmd].fGroup));
     } else
       switch (*pch) {
         case '>': break;       // comment quote lines
         case '%': WriteMsg("\nInvalid command '%s', ignored\n", pch); break;
         case '-': ExecExistAreaMask(pnode, pch + 1, ExecUnlNodeArea);  break;
         case '+': ExecExistAreaMask(pnode, pch + 1, ExecLnkNodeArea);  break;
         default : ExecExistAreaMask(pnode, pch,     ExecLnkNodeArea);  break;
       }

     // Restore trailing char, advance to the next line and check if
     // we are done with this message

     *pchEnd = ch; pch = pchEnd;
     if (!ch) break;
     while (!ISNEWLINE(*pch)) pch++;
   }

   // Write changes done report

   CreateChangesReport(pnode, NULL);

   // Print out the processed message text fixing the end of the
   // scanned in text so that all those tear and origin lines will
   // not be included

   *pch = '\0';
   if (pch = pchText, SkipSpaces(&pch) && !(*pch == 0x01) &&
      !(pch[0] == '-' && pch[1] == '-' && pch[2] == '-')) {

     // Determine the newline character so that it will be displayed
     // properly on the console device and in log while in verbose mode

     chEOL = (cfg.fl & FL_DISPLAYGENERATEDMSG) ? '\n' : '\r';

     // Write out the original message header

     WriteMsg("\nFollowing is the original message text"
              "\n--------------------------------------"
              "\n");

     // Write out the original message body. Note that we'll do this
     // in chunks to prevent overrunning of the output buffer for very
     // long messages

     for (pch = pchEnd = pchText; (ch = *pchEnd) != '\0'; pchEnd++)
       if (ch == '\r' || ch == '\n') {
         *pchEnd = '\0';
         WriteMsg("%s%c", pch, chEOL);

         // Advance to the next line skipping 0d/0a sequence if needed

         pch = pchEnd + 1;
         if (ch == '\r' && *pch == '\n') {
           pch++; pchEnd++;
         }
       }

     // Write out the rest of the buffer if any

     if (pch < pchEnd) WriteMsg("%s%c", pch, chEOL);
   }

   // Clean up

   MemFree(pchText);

   return TRUE;
 }

/*
 * This subroutine to kill or mark as 'Read' processed request messages
 */

 static BOOL SUBENTRY DoMarkProcMsg(HMSG hmsg, XMSG * pmsg, MSGN * pmsgn,
                                    UMSG umsg, BOOL fKeep)
 {
   // Check if we have to disable processed requests for testing purposes

   if (cfg.fl & FL_TESTMODE) {
     MsgCloseMsg(hmsg);
     return FALSE;
   }

   // Check what should we do with processed requests -- mark as read or kill

   if (fKeep) {

     // Mark as read then close message

     pmsg->attr|= MSGREAD;
     if (MsgWriteMsg(hmsg, FALSE, pmsg, NULL, 0L, 0L, 0L, NULL) == -1)
       WriteLog("! Can't update message #%lu\n", umsg);
     MsgCloseMsg(hmsg);

   } else {

     // Close message and delete it

     MsgCloseMsg(hmsg);
     if (MsgKillMsg(hNetMail, *pmsgn) == -1) {
       WriteLog("! Can't delete message #%lu\n", umsg);
     } else {

       // Log operation

       WriteLog("- Kill msg #%lu\n", umsg);

       // Adjust message index for squish style netmail

       if (cfg.fl & FL_SQUISHNETMAIL) *pmsgn-= 1;
     }
   }

   return TRUE;
 }

/*
 * This subroutine to process a netmail message
 */

 static BOOL SUBENTRY DoProcMsg(HMSG hmsg, XMSG * pmsg, MSGN * pmsgn,
                                UMSG umsg)
 {
   BOOL fKeep = FALSE;
   CHAR achText[256];
   PNODE pnode;

   // Check if 'Read' flag is set and leave if so

   if (pmsg->attr & MSGREAD)
     return FALSE;

   // Check if API does not provide zone and assume the default one

   if (pmsg->dest.zone == 0) pmsg->dest.zone = cfg.anetAddr[0].zone;
   if (pmsg->orig.zone == 0) pmsg->orig.zone = cfg.anetAddr[0].zone;

   // Check if this message is sent from/to the fakenet point and if so,
   // zero out the point info if any since it may get there somehow from
   // the ^aFMPT/^aTOPT kludges

   if (cfg.usPointNet != 0) {
     if (pmsg->orig.net == cfg.usPointNet) pmsg->orig.point = 0;
     if (pmsg->dest.net == cfg.usPointNet) pmsg->dest.point = 0;
   }

   // Check if this message is addressed to one of my aliases

   if (!IsStringInList(cfg.apszAlias, numbof(cfg.apszAlias), pmsg->to))
     return FALSE;

   // Check if this message is addressed to one of my akas

   if (!IsMyAka(&pmsg->dest))
     return FALSE;

   // Log that nice fact that we've got a message ... from now on the
   // message is treaten as one of the SqaFix attantion and thus should
   // be processed and marked somehow.

   WriteLog("- Recv msg #%lu from %s, %s\n",
            umsg, pmsg->from, FormatNetAddr(&pmsg->orig));

   // Log message if necessary

   if (cfg.fl & FL_VERBOSEMODE)
     LogMsg(hNetMail, hmsg, pmsg, umsg, "NetMail", LM_CTRL | LM_BODY);

   // Check if this message is from the name we should never process

   if (IsPrefixInList(cfg.apszIgnore, numbof(cfg.apszIgnore), pmsg->from)) {

     if (cfg.fl & FL_KEEPFAILEDREQ) fKeep = TRUE;

     WriteLog("- Skip msg #%lu from ignored name\n", umsg);

   } else

   // Check if this message has a valid subject line to prevent loops

   if (!DoCheckSubj(pmsg->subj)) {

     if (cfg.fl & FL_KEEPFAILEDREQ) fKeep = TRUE;

     WriteLog("- Skip msg #%lu with subject \"%s\"\n", umsg, pmsg->subj);

   } else

   // Check if this message is from the known node and if not
   // send him a failed request short message

   if ((pnode = GetNodeFromAddr(&pmsg->orig)) == NULL) {

     sprintf(achText, "\rSorry, node %s is not known to SqaFix at this system."
                      "\rContact the SysOp if you think that this is an error..."
                      "\r",
             FormatNetAddr(&pmsg->orig));

     SendMsg(&pmsg->dest, SQAFIX_NAME, &pmsg->orig, pmsg->from,
             GetNodeMsgAttr(pnode), "Request failed -- unknown node",
             achText);

     if (cfg.fl & FL_KEEPFAILEDREQ) fKeep = TRUE;

   } else

   // Check if there is a valid password supplied and if not,
   // respond with a message polite enough to keep them happy

   if (xstricmp(pnode->achPassword, DoGetPassword(pmsg->subj))) {
     sprintf(achText, "\rSorry, password '%s' is invalid for node %s."
                      "\rContact the SysOp if you think that this is an error..."
                      "\r",
             DoGetPassword(pmsg->subj), FormatNetAddr(&pmsg->orig));

     SendMsg(&pmsg->dest, SQAFIX_NAME, &pmsg->orig, pmsg->from,
             GetNodeMsgAttr(pnode), "Request failed -- invalid password",
             achText);

     if ((cfg.fl & FL_KEEPFAILEDREQ) || (pnode->fs & NF_KEEPREQS)) fKeep = TRUE;

   } else {

     // Initialize report message writing unless supressed

     if (!(pnode->fs & NF_DONTREPORT && !xstricmp(pmsg->from, SQAFIX_HOST)))
       SendLongMsgBeg(&pmsg->dest, SQAFIX_NAME, &pmsg->orig, pmsg->from,
                      GetNodeMsgAttr(pnode), "Remote request operation report",
                      TRUE);

     // Process subject line and message body requests unless suppressed

     DoProcSubjReqs(pnode, hmsg, pmsg, umsg);
     DoProcBodyReqs(pnode, hmsg, pmsg, umsg);

     // Finish report message writing

     SendLongMsgEnd();

     fKeep = pnode->fs & NF_KEEPREQS;
   }

   // Mark or kill the processed request message

   DoMarkProcMsg(hmsg, pmsg, pmsgn, umsg, fKeep);

   // Log we're done if in verbose mode

   if (cfg.fl & FL_VERBOSEMODE)
     WriteLog("- Done msg #%lu from %s, %s\n",
              umsg, pmsg->from, FormatNetAddr(&pmsg->orig));

   return TRUE;
 }

/*
 * This subroutine scans netmail folder
 */

 static VOID SUBENTRY DoRunRemoteMode(VOID)
 {
   CHAR achCtrl[MAX_CTRL_LENG];
   HMSG hmsg;
   UMSG umsg;
   MSGN msgn;
   XMSG msg;

   // Inform user of what we're doing

   printf("Scanning mail: %s%s\n", cfg.achNetMail,
//           cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : "\\*.MSG");
           cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   // Scan all the messages in the netmail folder

   for (msgn = 1L; msgn <= MsgHighMsg(hNetMail); msgn++)
     if ((hmsg = MsgOpenMsg(hNetMail, MOPEN_RW, msgn)) != NULL) {

       // Get message uid and check if it is in sent message list

       umsg = MsgMsgnToUid(hNetMail, msgn);
       if (GetSntm(umsg)) {
         MsgCloseMsg(hmsg);
         continue;
       }

       // Read the message header and control info

       if (MsgReadMsg(hmsg, &msg, 0L, 0L, NULL, lengof(achCtrl), achCtrl) == -1) {
         WriteLog("! Can't read msg #%lu\n", umsg);
         MsgCloseMsg(hmsg);
         continue;
       }

       // Check for the MSGID kludge and allocate its data

       achCtrl[lengof(achCtrl)] = '\0';
       if (cfg.pszReqMSGID != NULL) MemFree(cfg.pszReqMSGID);
       cfg.pszReqMSGID = GetMsgKludge("MSGID:", achCtrl, sizeof(achCtrl), FALSE);

       // Process the message

       if (!DoProcMsg(hmsg, &msg, &msgn, umsg)) MsgCloseMsg(hmsg);

       // Free MSGID kludge data if any

       if (cfg.pszReqMSGID != NULL) {
         MemFree(cfg.pszReqMSGID);
         cfg.pszReqMSGID = NULL;
       }
     }

   // Inform user we're done

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s%s\n", cfg.achNetMail,
//             cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : "\\*.MSG");
             cfg.fl & FL_SQUISHNETMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

 }

/////////////////////////////////////////////////////////////////////////////
// N o t i f y   m o d e   p r o c e s s i n g                             //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine notifies all the linked nodes
 */

 static VOID SUBENTRY DoNotifyKnownNodes(VOID)
 {
   PNODE pnode;

   // Loop through all the linked nodes

   for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext) {

     // Check if this node has to be excluded

     if (pnode->fs & NF_DONTNOTIFY) continue;

     // Check if we need to report by a message and initiate a message write

     if (cfg.fl & FL_REPORTMSG)
       SendLongMsgBeg(GetAddrMatch(&pnode->netAddr), SQAFIX_NAME,
                     &pnode->netAddr, GetNodeSysop(pnode),
                     GetNodeMsgAttr(pnode), "Node links notification report",
                     TRUE);

     // Create the linked/available areas report

     CreateAreasReport(pnode, NULL);

     // Check if we're reporting by a message and finish message writing
     // flushing the message buffer if anything is still there...

     if (cfg.fl & FL_REPORTMSG)
       SendLongMsgEnd();
   }
 }

/*
 * This subroutine notifies node of all the existing links
 */

 static VOID SUBENTRY DoNotifyNodeAllLinks(SHORT cArg, PSZ apszArg[])
 {
   NETADDR netAddr = cfg.anetAddr[0];
   PNODE pnode;

   // Check if there is a node address specified

   if (cArg <= 2) {
     WriteLog("! Node address missing\n");
     exit(EXIT_FAILURE);
   }

   // Scan in the node address and check if ok

   if (ScanNetAddr(&netAddr, apszArg[2]) == NULL) {
     WriteLog("! Invalid node address: '%s'\n", apszArg[2]);
     exit(EXIT_FAILURE);
   }

   // Check if we need to report by a message and initiate a message write

   if (cfg.fl & FL_REPORTMSG) {
     pnode = GetNodeFromAddr(&netAddr);
     SendLongMsgBeg(GetAddrMatch(&netAddr), SQAFIX_NAME, &netAddr,
                    GetNodeSysop(pnode), GetNodeMsgAttr(pnode) | MSGKILL,
                    "Existing links notification report", TRUE);
   }

   // Create all echo links report

   CreateAllLinksReport(cfg.anetAddr);

   // Check if we're reporting by a message and finish message writing
   // flushing the message buffer if anything is still there...

   if (cfg.fl & FL_REPORTMSG)
     SendLongMsgEnd();
 }

/////////////////////////////////////////////////////////////////////////////
// A u t o c r e a t e   m o d e   p r o c e s s i n g                     //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine fakes autocreate new area process
 */

 static BOOL SUBENTRY DoFakeAutoCreate(SHORT cArg, PSZ apszArg[])
 {
   NETADDR netAddr = cfg.anetAddr[0];
   SHORT iArg, copt;
   PSZ pszArea;

   // Scan through the command line

   for (iArg = 2, copt = 0; iArg < cArg; iArg++) {

     // Check if this is an option and skip it

     if (ISOPTION(apszArg[iArg][0]))
       continue;

     // Check if this is the command line item which is not an option and
     // scan in the node address and check if ok

     if (copt == 0) {
       if (ScanNetAddr(&netAddr, apszArg[iArg]) == NULL) {
         WriteLog("! Invalid node address: '%s'\n", apszArg[iArg]);
         exit(EXIT_FAILURE);
       } else
         copt++;
     } else

     // Check if this is the command line item which is not an option and
     // save the area tag specification

     if (copt == 1) {
       pszArea = apszArg[iArg]; 
#ifndef UNIX
       xstrupr(pszArea);
#endif
       if (GetAreaFromTag(pszArea)) {
         WriteLog("! Area %s already exists\n", pszArea);
         exit(EXIT_FAILURE);
       } else
         copt++;
     } else
       break;
   }

   // Check if it's ok to fake autocreation

   switch (copt) {
     case 0: // Run normal autocreation
             return FALSE;
     case 1: WriteLog("! No area specified for fake autocreation\n");
             return TRUE;
     case 2: WriteLog("* FAKE AutoCreation of %s by %s\n", pszArea, FormatNetAddr(&netAddr));
             CreateNewArea(pszArea, &netAddr);
             return TRUE;
   }

   return FALSE;
 }

/*
 * This subroutine automatically creates new areas
 */

 static VOID SUBENTRY DoAutoCreateNewAreas(SHORT cArg, PSZ apszArg[])
 {
   USHORT iNewArea = 0;
   NETADDR netAddr;
   HAREA hBadMail;
   PSZ pszArea;
   BOOL fCode;
   UMSG umsg;
   MSGN msgn;
   HMSG hmsg;
   XMSG msg;

   // Check if we're in test mode and fake autocreation

   if (cfg.fl & FL_TESTMODE)
     if (DoFakeAutoCreate(cArg, apszArg)) {
       CreateNewAreasReport();
       return;
     }

   // Open the bad mail folder

   hBadMail = OpenBadMailFolder();

   // Inform user of what we're doing

   printf("Scanning mail: %s%s\n", cfg.achBadMail,
//           cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : "\\*.MSG");
           cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   // Read all the message headers and control info and check if ok

   for (msgn = 1L; msgn <= MsgHighMsg(hBadMail); msgn++)
     if ((hmsg = MsgOpenMsg(hBadMail, MOPEN_READ, msgn)) != NULL) {
       umsg = MsgMsgnToUid(hBadMail, msgn); fCode = FALSE;

       // Read message header and control info and check if ok

       if (MsgReadMsg(hmsg, &msg, 0L, 0L, NULL, cchBuf, pchBuf) == -1) {
         WriteLog("! Can't read msg #%lu\n", umsg);
       } else {

         // Get the area tag this message should be tossed to and
         // check if this area does not exist yet

         if ((pszArea = GetMsgKludge("AREA:", pchBuf, xstrlen(pchBuf), FALSE)) != NULL)        {
           if (GetAreaFromTag(pszArea) == NULL)
             if (GetEchoMsgPath(hmsg, &msg, umsg, &netAddr)) {
               if (cfg.fl & FL_VERBOSEMODE)
                 LogMsg(hBadMail, hmsg, &msg, umsg, "BadMail", LM_CTRL);
               fCode = CreateNewArea(pszArea, &netAddr);
               if (fCode == TRUE) iNewArea++;
             }
           MemFree(pszArea);
         }
       }

       // Close the processed message

       MsgCloseMsg(hmsg);

       // Check if we need to kill this message

       if (fCode == (BOOL)-1 && !(cfg.fl & FL_TESTMODE)) {

         // Kill the message and check if ok

         if (MsgKillMsg(hBadMail, msgn) == -1) {
           WriteLog("! Can't delete message #%lu in BadMail\n", umsg);
         } else {
           WriteLog("- Kill msg #%lu in BadMail\n", umsg);
           if (cfg.fl & FL_SQUISHBADMAIL) msgn-= 1;
         }
       }
     }

   // Close bad mail folder

   CloseBadMailFolder(hBadMail);

   // Inform user we're done

   if (cfg.fl & FL_VERBOSEMODE)
     printf("Finished scan: %s%s\n", cfg.achBadMail,
//             cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : "\\*.MSG");
             cfg.fl & FL_SQUISHBADMAIL ? ".SQ?" : PATH_DELIMS "*.MSG");

   // Create new areas creation report

   CreateNewAreasReport();
 }

/////////////////////////////////////////////////////////////////////////////
// M a i n t e n a n c e   m o d e   p r o c e s s i n g                   //
/////////////////////////////////////////////////////////////////////////////

/*
 * This subroutine runs a maintenance mode
 */

 static BOOL SUBENTRY DoSetDayNumLimit(PAREA parea, HAREA harea)
 {
   USHORT iflg;
   PCH pch;

   static struct {
     PSZ psz; ULONG ul;
   } aflg[] = {
     "-$m",     0,
     "-$s",     0,
     "-$d",     0,
   };

   // Check if this is a squish area

   if (!IsSquishArea(parea->pszSqshFlags)) return FALSE;

   // Scan in all the interesting squish flags

   for (iflg = 0; iflg < numbof(aflg); iflg++)
     if ((pch = xstrstr(parea->pszSqshFlags, aflg[iflg].psz)) != NULL)
       aflg[iflg].ul = atol(pch + xstrlen(aflg[iflg].psz));
     else
       aflg[iflg].ul = 0;

   // Set max msg values to for the Squish base

   SquishSetMaxMsg(harea, aflg[0].ul, aflg[1].ul, aflg[2].ul);

   return TRUE;
 }

/*
 * This subroutine executes a maintenance mode tasks
 */

 static VOID SUBENTRY DoExecMaint(PAREA parea, SHORT cArg, PSZ apszArg[])
 {
   PSZ pszType = IsSquishArea(parea->pszSqshFlags) ? "Squish" : "Msg";
   PSZ pszPThru = IsPassThruArea(parea->pszSqshFlags) ? "PassThru" : "NotPassThru";
   CHAR achCmd[MAXPATH], achArea[MAX_AREA_LENG + 4];
   SHORT iArg, code;
#ifdef UNIX
    char* tmp = NULL;
#endif

   // Scan through all executors specified in the command line

   for (iArg = 2; iArg < cArg; iArg++) {

     // Check if this is an option and skip it

     if (ISOPTION(apszArg[iArg][0])) continue;

     // Check if there are special command line characters and enclose
     // string in double quotas so that they will not screw up command line

     if (IsSpecCmdChars(parea->achTag))
       sprintf(achArea, "\"%.128s\"", parea->achTag);
     else
       sprintf(achArea, "%.128s", parea->achTag);

     // Get the operating system command shell name and
     // spawn external program

     GetCommandShellName(achCmd);
#ifndef UNIX
     code = spawnlp(P_WAIT, achCmd, achCmd, "/C", apszArg[iArg],
                    achArea, parea->pszPath, pszType, pszPThru,
                    NULL);
#else
    tmp = (char*) malloc(strlen(achCmd) + 3 + strlen(apszArg[iArg]));
    sprintf(tmp, "%s %s %s", achCmd, "/C", apszArg[iArg]);
    code = system(tmp);
#endif
     // Check if spawn failed

     if (code == (SHORT)-1) {
       WriteLog("$ Exec %s failed\n", apszArg[iArg]);
     }
   }
 }

/*
 * This subroutine runs a maintenance mode
 */

 static VOID SUBENTRY DoRunMaintenance(SHORT cArg, PSZ apszArg[])
 {
   PAREA parea;
   HAREA harea;

   // Loop through all the known areas

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {

     // Check if this area is in the deleted list ... this should not
     // happen since we're not processing requests during a maintenance
     // run and idle passthru areas are checked before terminating

     if (GetDelAreaFromTag(parea->achTag)) continue;

     // Inform user if requested

     if (cfg.fl & FL_VERBOSEMODE)
       printf("Checking area: %s\n", parea->achTag);

     // Check if this is a passthru area and if not, open the echo mail
     // folder and set the squish message base limits to the values in
     // Squish.Cfg

     if (!IsPassThruArea(parea->pszSqshFlags))
       if ((harea = OpenEchoMailFolder(parea)) != NULL) {
         DoSetDayNumLimit(parea, harea);
         CloseEchoMailFolder(parea, harea);
       }

     // Execute the maintenance mode tasks

     DoExecMaint(parea, cArg, apszArg);
   }
 }

/*
 * This subroutine syncs cfg files
 */

 static VOID SUBENTRY DoSyncCfgFiles(SHORT cArg, PSZ apszArg[])
 {
   PAREA parea;

   // Scan through all the defined areas

   for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext) {
     if (!(parea->fs & AF_INSQAFIXCFG)) {
       parea->fs|= AF_AUTOCREATEDAREA | AF_PASSIVECHANGED;
       parea->pszDescr = GetAreaDescrFromTag(parea->achTag);
     }
     if (!(parea->fs & AF_INSQUISHCFG)) {
       parea->fs|= AF_AUTOCREATEDAREA | AF_ACTIVECHANGED;
       SyncSqafArea(parea);
     }
   }
 }

/////////////////////////////////////////////////////////////////////////////
// D u m p   m o d e   p r o c e s s i n g                                 //
/////////////////////////////////////////////////////////////////////////////
#ifdef DUMP_CODE
/*
 * This subroutine dumps given node info
 */

 static VOID SUBENTRY DoDumpNodeInfo(PNODE pnode)
 {
   static struct {
     USHORT fs;      PSZ psz;
   } afs[] = {
     NF_KILLSENT,    "K",
     NF_SENDHOLD,    "H",
     NF_SENDCRASH,   "C",
     NF_KEEPREQS,    "P",
     NF_AUTOCREATE,  "A",
     NF_FORWARDREQ,  "O",
     NF_FREQNOPTHRU, "N",
     NF_VISIBLE,     "V",
     NF_SENDRULES,   "I",
     NF_RESCANOK,    "R",
     NF_COMPRESSOK,  "S",
     NF_SENDCREATE,  "F",
     NF_AREACREATE,  "!C",
     NF_AREADESTROY, "!D",
     NF_DONTNOTIFY,  "Y",
     NF_DONTREPORT,  "Z",
   };

   USHORT ifs;

   // Dump node info

   WriteLog("* Dump node\n");
   WriteLog("-  netAddr: %s\n", FormatNetAddr(&pnode->netAddr));
   WriteLog("-  pszSysop: '%s'\n", pnode->pszSysop);
   WriteLog("-  pszGroup, pszGroupSpec: '%s', '%s'\n", pnode->pszGroup, pnode->pszGroupSpec);
   WriteLog("-  numFreq, maxFreq: %u, %u\n", pnode->numFreq, pnode->maxFreq);
   WriteLog("-  numRescan, maxRescan: %u, %u\n", pnode->numRescan, pnode->maxRescan);

   // Dump node flags

   WriteLog("-  flags:");
   for (ifs = 0; ifs < numbof(afs); ifs++)
     WriteLog(" %c%s", afs[ifs].fs & pnode->fs ? '+' : '-', afs[ifs].psz);
   WriteLog("\n");

   // More info

   WriteLog("-  level: %u\n", pnode->level);
   WriteLog("-  achPassword: '%s'\n", pnode->achPassword);

 }

/*
 * This subroutine dumps node info
 */

 static VOID SUBENTRY DoDumpNode(SHORT cArg, PSZ apszArg[])
 {
   NETADDR netAddr = cfg.anetAddr[0];
   SHORT iArg, cnode;
   PNODE pnode;

   // Scan through the command line

   for (iArg = 2, cnode = 0; iArg < cArg; iArg++) {

     // Check if this is an option and skip it, otherwize
     // increment node count to prevent all nodes dump

     if (ISOPTION(apszArg[iArg][0]))
       continue;
     else
       cnode++;

     // Scan in the node address and check if ok

     if (ScanNetAddr(&netAddr, apszArg[iArg]) == NULL) {
       WriteLog("! Invalid node address: '%s'\n", apszArg[iArg]);
       continue;
     }

     // Check if this node is known to the SqaFix

     if ((pnode = GetNodeFromAddr(&netAddr)) == NULL) {
       WriteLog("! Node %s is not known\n", FormatNetAddr(&netAddr));
       continue;
     }

     // Dump the node info

     DoDumpNodeInfo(pnode);
   }

   // Check if no nodes were specified on the command line and
   // dump all existing nodes

   if (!cnode)
     for (pnode = cfg.pnodeFirst; pnode != NULL; pnode = pnode->pnodeNext)
       DoDumpNodeInfo(pnode);
 }

/*
 * This subroutine dumps given uplink info
 */

 static VOID SUBENTRY DoDumpUplinkInfo(PUPLINK puplink)
 {
   static struct {
     USHORT fs;      PSZ psz;
   } afs[] = {
     UF_AREAFIXPROT,     "A",
     UF_NOPLUSPREFIX,    "+",
     UF_NOMULTIAREATAGS, "*",
     UF_LOWERCASETAG,    "C",
     UF_FREQNOPTHRU,     "N",
   };

   USHORT ifs;
   PLSZ plsz;

   // Dump uplink info

   WriteLog("* Dump uplink\n");
   WriteLog("-  netAddr: %s\n", FormatNetAddr(&puplink->pnode->netAddr));
   WriteLog("-  pszName: '%s'\n", puplink->pszName);
   WriteLog("-  level: %u\n", puplink->level);
   WriteLog("-  From netAddr: %s\n", FormatNetAddr(puplink->netAddr.zone ? &puplink->netAddr : &cfg.anetAddr[0]));

   // Dump uplink flags

   WriteLog("-  flags:");
   for (ifs = 0; ifs < numbof(afs); ifs++)
     WriteLog(" %c%s", afs[ifs].fs & puplink->fs ? '+' : '-', afs[ifs].psz);
   WriteLog("\n");

   // Dump uplink area masks

   WriteLog("-  masks:\n");
   for (plsz = puplink->plszMask; plsz != NULL; plsz = plsz->plszNext)
     WriteLog("-   '%s'\n", plsz->ach);
 }

/*
 * This subroutine dumps uplink info
 */

 static VOID SUBENTRY DoDumpUplink(SHORT cArg, PSZ apszArg[])
 {
   NETADDR netAddr = cfg.anetAddr[0];
   SHORT iArg, cuplink;
   PUPLINK puplink;

   // Scan through the command line

   for (iArg = 2, cuplink = 0; iArg < cArg; iArg++) {

     // Check if this is an option and skip it, otherwize
     // increment uplink count to prevent all uplinks dump

     if (ISOPTION(apszArg[iArg][0]))
       continue;
     else
       cuplink++;

     // Scan in the uplink address and check if ok

     if (ScanNetAddr(&netAddr, apszArg[iArg]) == NULL) {
       WriteLog("! Invalid uplink address: '%s'\n", apszArg[iArg]);
       continue;
     }

     // Check if this uplink is known to the SqaFix

     if ((puplink = GetUplinkFromAddr(&netAddr)) == NULL) {
       WriteLog("! Uplink %s is not known\n", FormatNetAddr(&netAddr));
       continue;
     }

     // Dump the uplink info

     DoDumpUplinkInfo(puplink);
   }

   // Check if no uplinks were specified on the command line and
   // dump all existing uplinks

   if (!cuplink)
     for (puplink = cfg.puplinkFirst; puplink != NULL; puplink = puplink->puplinkNext)
       DoDumpUplinkInfo(puplink);
 }

/*
 * This subroutine dumps given area info
 */

 static VOID SUBENTRY DoDumpAreaInfo(PAREA parea)
 {
   static struct {
     USHORT fs;      PSZ psz;
   } afs[] = {
     AF_RESCANOK,    "R",
     AF_VISIBLE,     "V",
     AF_SENDRULES,   "I",
   };

   USHORT ifs;
   PLINK plink;

   // Dump area info

   WriteLog("* Dump area\n");
   WriteLog("-  achTag: '%s'\n", parea->achTag);
   WriteLog("-  chGroup: '%c'\n", parea->chGroup);
   WriteLog("-  pszDescr: '%s'\n", parea->pszDescr);
   WriteLog("-  pszSqshFlags: '%s'\n", parea->pszSqshFlags);
   WriteLog("-  pszPath: '%s'\n", parea->pszPath);
   WriteLog("-  pszRules: '%s'\n", parea->pszRules);

   // Dump area flags

   WriteLog("-  flags:");
   for (ifs = 0; ifs < numbof(afs); ifs++)
     WriteLog(" %c%s", afs[ifs].fs & parea->fs ? '+' : '-', afs[ifs].psz);
   WriteLog("\n");

   // More info

   WriteLog("-  level: %u\n", parea->level);

   // Dump area active links

   WriteLog("-  Active links:");
   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext)
     if (plink->fs & LF_ACTIVE)
       WriteLog(" %s", FormatNetAddr(&plink->netAddr));
   WriteLog("\n");

   // Dump area passive links

   WriteLog("-  Passive links:");
   for (plink = parea->plink; plink != NULL; plink = plink->plinkNext)
     if (!(plink->fs & LF_ACTIVE))
       WriteLog(" %s", FormatNetAddr(&plink->netAddr));
   WriteLog("\n");

 }

/*
 * This subroutine dumps area control blocks
 */

 static VOID SUBENTRY DoDumpArea(SHORT cArg, PSZ apszArg[])
 {
   SHORT iArg, carea;
   PAREA parea;

   // Scan through the command line

   for (iArg = 2, carea = 0; iArg < cArg; iArg++) {

     // Check if this is an option and skip it, otherwize
     // increment area count to prevent all areas dump

     if (ISOPTION(apszArg[iArg][0]))
       continue;
     else
       carea++;

     // Check if this area is known to the SqaFix

     if ((parea = GetAreaFromTag(apszArg[iArg])) == NULL) {
       WriteLog("! Area %s is not known\n", apszArg[iArg]);
       continue;
     }

     // Dump the area info

     DoDumpAreaInfo(parea);
   }

   // Check if no areas were specified on the command line and
   // dump all existing areas

   if (!carea)
     for (parea = cfg.pareaFirst; parea != NULL; parea = parea->pareaNext)
       DoDumpAreaInfo(parea);
 }
#endif
/////////////////////////////////////////////////////////////////////////////
// M a i n ...                                                             //
/////////////////////////////////////////////////////////////////////////////

/*
 * Main...
 */

 int main(int argc, char * argv[])
 {
   ULONG cb, cbStart = GetFreeMemory();
   PVOID p;

#ifdef DEBUG
fprintf(STDAUX, "\n\rSqaFix: DEBUGGING SESSION\r\n");
#endif

   // Partially process the command line, scan in the config
   // files and initialize all things

   DoInitSqaFix(argc, argv);

   // Load and process the queue

   LoadQueFile(cfg.achQueFile);

   // Calculate outstanding freqs

   DoCountNodeFreq();

   // Get available memory after loading config files

   cb = GetFreeMemory();

   // Allocate and free some memory to see if we can do this

   if ((p = MemAlloc(16*1024, 0)) == NULL) {
     WriteLog("! Not enough memory to operate: %luKb\n", cb / 1024);
     exit(EXIT_FAILURE);
   } else {
     MemFree(p);
     if (cfg.fl & FL_VERBOSEMODE)
       fprintf(stdout, "Memory usage : available %luKb, at startup: %luKb, used %luKb\n",
                        cb / 1024, cbStart / 1024, (cbStart - cb) / 1024);
   }

   // This is only for debugging

#ifdef DEBUG
fprintf(STDAUX, "%u bytes in CFG\r\n", sizeof(cfg));
fprintf(STDAUX, "%u area(s)\r\n", LstQueryElementCount((PLE) cfg.pareaFirst));
fprintf(STDAUX, "%u description(s)\r\n", LstQueryElementCount((PLE) cfg.pareadescFirst));
fprintf(STDAUX, "%u node(s)\r\n", LstQueryElementCount((PLE) cfg.pnodeFirst));
fprintf(STDAUX, "%u uplink(s)\r\n", LstQueryElementCount((PLE) cfg.puplinkFirst));
#endif

   // Run manual or remote more or console only commands

   switch (cfg.cmdCode) {
     case CMD_SCAN:             DoRunRemoteMode();                      break;
     case CMD_NOTIFY:           DoNotifyKnownNodes();                   break;
     case CMD_REPORT:           DoNotifyNodeAllLinks(argc, argv);       break;
     case CMD_AUTOCREATE:       DoAutoCreateNewAreas(argc, argv);       break;
     case CMD_MAINT:            DoRunMaintenance(argc, argv);           break;
     case CMD_SYNC:             DoSyncCfgFiles(argc, argv);             break;
#ifdef DUMP_CODE
     case CMD_DUMPNODE:         DoDumpNode(argc, argv);                 break;
     case CMD_DUMPUPLINK:       DoDumpUplink(argc, argv);               break;
     case CMD_DUMPAREA:         DoDumpArea(argc, argv);                 break;
#endif
     default:                   DoRunManualMode(argc, argv);            break;
   }

   // Send out all the areafix requests if any

   SendAreaFixReqs();

   // Send out all the files if any

   SendFiles();

   // Create notifications

   CreateNotes();

   // Save the queue file.

   SaveQueFile(cfg.achQueFile);

   // Update the config files if changed

   DoUpdateCfgFiles();

   // Update the arealist file

   SaveAreaListFile(cfg.pszAreaListSaveFile);

   // Terminate all things and clean up

   DoQuitSqaFix();

   exit(EXIT_SUCCESS);
 }

/*
 * End of SQAFIX.C
 */
