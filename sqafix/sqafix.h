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
 * Header module
 *
 * Created: 06/Jan/92
 * Updated: 12/Jan/00
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

#if defined(__OS2__) && defined(UNIX)
#ifdef __OS2__
#undef __OS2__
#endif
#endif

#ifdef __OS2__
#define INCL_NOCOMMON
#define INCL_NOPM
#define INCL_DOSDATETIME
#define INCL_DOSFILEMGR
#define INCL_DOSMEMMGR
#define INCL_DOSINFOSEG
#define INCL_DOSPROCESS
#ifndef __GNUC__
#include <os2.h>
#endif
#endif

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef UNIX
#include <direct.h>
#endif
// Use Maximus typedef's....
#include <typedefs.h>
// #include <hntypes.h>
#include "dw-lst.h"

 // Make up debugging compile time options

#ifdef DEBUG
#define STDAUX stdaux
#endif

 // Make up MsgApi's style type declarations

#ifdef __OS2__
#define OS_2
#endif

#ifdef __W32__
#define NT
#define pascal
#endif

  // Win32 specific function prototypes

#ifdef __W32__
#include "dw-w32.h"
#endif

  // Disable the MsgApi0 style type declaration

#define MSGAPI_NO_OLD_TYPES

  // Remove the EXPENTRY definition since the Scott's API has one too.
  // Both appear to be identical except for the far call specification,
  // but since we're compiling in large model this is not a problem...

#undef EXPENTRY
#include <msgapi.h>

  // Declare convenient types

#define UMSG UMSGID
#define MSGN ULONG

  // Change readonly section to readwrite since Microsoft Visual C compiler
  // places constant strings in there but we want to modify them.

#if _MSC_VER >= 1000
#pragma comment(linker, "/section:.rdata,rw")
#endif

 // Include all the compiler dependant stuff which is based on the
 // compiler mode identifiers set by the Scott's API header

#include "sqalng.h"

 // Include safe string and memory manipulation declarations

#include "sqastr.h"

 // Make sure we'll not use Ralf Brown's spawno in protected mode versions

#if defined(__OS2__) || defined(__W32__) || defined(__DPMI__) || defined(UNIX)
#ifdef __SPAWNO__
#undef __SPAWNO__
#endif
#endif

 // Include the Ralf Brown's spawno toolkit header

#ifdef __SPAWNO__
#include <spawno.h>
#endif

 // Define the target operating system

#ifdef __OS2__
#define SYSTEM          "OS2"           // OS/2 protected mode
#endif

#ifdef __W32__
#define SYSTEM          "W32"           // Win32 protected mode
#endif

#ifdef __DPMI__
#define SYSTEM          "DPMI"          // DPMI protected mode
#endif

#ifdef UNIX
#define SYSTEM          "Unix/Linux"         // Unix or Linux
#endif

#ifndef SYSTEM
#define SYSTEM          "DOS"           // DOS real mode
#define __DOS__
#endif

 // Some useful defines

 #define mod(x)     ((x) > 0 ? (x) : -(x))      // Return modulo of 'x'

 // Miscellaneous defines

 #define VERSION        "0.99a9/"SYSTEM    // Software revision level/system
 #define SQAFIX_NAME    "SqaFix"           // From/to name
 #define SQAFIX_CFG	"sqafix"	   // UNIX case..
 #define SQAFIX_HOST    SQAFIX_NAME"Host"  // From name to SqaFix
 #define SQAFIX_TEAR    "\r--- "SQAFIX_NAME" v"VERSION"\r"  // Tearline

 // Define program limits

#if   defined __DOS__
 #define DEF_BUFF_SIZE  8*1024u         // Default working buffer size
#elif defined __W32__
 #define DEF_BUFF_SIZE 16*1024u         // Default working buffer size
#elif defined __OS2__
 #define DEF_BUFF_SIZE 16*1024u         // Default working buffer size
#elif defined UNIX
 #define DEF_BUFF_SIZE 16*1024u         // Default working buffer size
#endif

 #define MAX_BUFF_SIZE  0xff00u         // Max working buffer size
 #define MAX_CTRL_LENG  512             // Max ctrl info leng (see API_SDM.H)
 #define MAX_AREA_LENG  128             // Max areatag length
 #define MAX_ADDR_NUMB  32              // Max number of AKA's
 #define MAX_PASS_LENG  16              // Max password length
 #define MAX_ALIAS_NUM  16              // Max number of to name aliases
 #define MAX_IGNORE_NUM 16              // Max number of from name ignore
 #define MAX_AUTO_NODE  16              // Max number of nodes for a new area
 #define MAX_IDLE_NODE  16              // Max number of nodes for idle ignore
 #define MAX_NOTE_NODE  16              // Max number of notify nodes
 #define MAX_NOTE_AREA  16              // Max number of notify areas
 #define MAX_NOTE_FILE  16              // Max number of notify files
 #define MIN_MSG_LINE   32              // Min msg line length
 #define DEF_MSG_LINE   78              // Def msg line length

 // Define default file extensions

#ifndef UNIX
 #define DEF_CFG_EXT    ".CFG"          // Default config file extension
 #define DEF_LOG_EXT    ".LOG"          // Default log file extension
 #define DEF_QUE_EXT    ".QUE"          // Default queue file extension
 #define DEF_USE_EXT    ".USE"          // Default help file extension
 #define DEF_RUL_EXT    ".RUL"          // Default rules file extension
 #define DEF_TMP_EXT    ".$$$"          // Default temporary file extension
 #define DEF_BAK_EXT    ".BAK"          // Default backup file extension
#else
 #define DEF_CFG_EXT    ".cfg"
 #define DEF_LOG_EXT    ".log"          // Default log file extension
 #define DEF_QUE_EXT    ".que"          // Default queue file extension
 #define DEF_USE_EXT    ".use"          // Default help file extension
 #define DEF_RUL_EXT    ".rul"          // Default rules file extension
 #define DEF_TMP_EXT    ".$$$"          // Default temporary file extension
 #define DEF_BAK_EXT    ".bak"          // Default backup file extension
#endif

 // Define area group characters

 #define MIN_GROUP              'A'     // Min valid group id
 #define MAX_GROUP              'Z'     // Max valid group id
 #define DEF_NEWAREA_GROUP      'N'     // Default new area group

 #define NUM_GROUPS (MAX_GROUP - MIN_GROUP + 1) // Max number of groups

 #define ISGROUP(ch) (toupper(ch) >= MIN_GROUP && toupper(ch) <= MAX_GROUP)

 // Define flag to specify config file

#define ACTIVE  TRUE                    // Active links specification
#define PASSIVE FALSE                   // Passive links specification

 // Redefine the exit code constants

#undef  EXIT_SUCCESS
#undef  EXIT_FAILURE

#define EXIT_SUCCESS    cfg.fExitCode   // Successful exit
#define EXIT_FAILURE    255             // Something got wrong exit code
#define EXIT_MAILSENT   0x01            // Mail sent exit code
#define EXIT_PROCAREA   0x02            // Area created/rescanned exit code

  // Define LogMsg() flags

#define LM_CTRL         0x0001          // Log message control info
#define LM_BODY         0x0002          // Log message body

  // Define the assertion macro

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef NDEBUG
#  define ASSERT(p) ((void)0)
#else
#  define ASSERT(p) ((p) ? (void)0 : (void) Assert(     \
                    #p, __FILE__, __LINE__ ) )
#endif

/*
 * Basic objects declarations
 */

 // Date/time structure

 typedef struct _DTTM {         /* dttm */
   SHORT       yr;                      // Year
   SHORT       mo;                      // Month
   SHORT       da;                      // Day
   SHORT       hh;                      // Hour
   SHORT       mm;                      // Minute
   SHORT       ss;                      // Second
 } DTTM, FAR * PDTTM;

 // Date/time constants

#define SECS_IN_DAY     (60l*60l*24l)   // Number of seconds in one day

 // Linked string list element structure

 typedef struct _LSZ {  /* lsz */
   struct _LSZ FAR *plszPrev;           // Pointer to the prev element in list
   struct _LSZ FAR *plszNext;           // Pointer to the next element in list
   ULONG       hash;                    // String hash value
   USHORT      fs;                      // User flags
   USHORT      cch;                     // String length
   CHAR        ach[1];                  // String with trailing null
 } LSZ, FAR * PLSZ;

 // Node list element structure

 typedef struct _NODE { /* node */
   struct _NODE FAR *pnodePrev;         // Pointer to the prev element in list
   struct _NODE FAR *pnodeNext;         // Pointer to the next element in list
   NETADDR      netAddr;                // Node network address
   PSZ          pszSysop;               // Node sysop name if any
   PSZ          pszGroup;               // Node group list (existing)
   PSZ          pszGroupSpec;           // Node group list (specified)
   USHORT       fs;                     // Node flags (NF_)
   USHORT       level;                  // Node area access level
   USHORT       maxFreq;                // Node freq limit or 0 if none
   USHORT       numFreq;                // Node queued freq number
   USHORT       maxRescan;              // Node rescan limit or 0 if none
   USHORT       numRescan;              // Node current rescan number
   CHAR         achPassword[1];         // Node access password
 } NODE, FAR * PNODE;

 // Node flags (node.fs)

 #define NF_KILLSENT            0x0001  // Reply with Kill/Sent attribute
 #define NF_SENDHOLD            0x0002  // Reply with Hold attribute
 #define NF_SENDCRASH           0x0004  // Reply with Crash attribute
 #define NF_KEEPREQS            0x0008  // Keep processed requests
 #define NF_AUTOCREATE          0x0010  // Allow area autocreate
 #define NF_AREACREATE          0x0020  // Allow area %CREATE requests
 #define NF_AREADESTROY         0x0040  // Allow area %DESTROY requests
 #define NF_SENDCREATE          0x0080  // Force area create on linkup
 #define NF_RESCANOK            0x0100  // Allow area %RESCAN requests
 #define NF_VISIBLE             0x0200  // Show not allowed areas
 #define NF_SENDRULES           0x0400  // Send rules when linking areas
 #define NF_FORWARDREQ          0x0800  // Allow linkup requests forwarding
 #define NF_FREQNOPTHRU         0x1000  // No passthru for freq areas
 #define NF_COMPRESSOK          0x2000  // Allow %COMPRESS requests
 #define NF_DONTNOTIFY          0x4000  // Don't send area notifications
 #define NF_DONTREPORT          0x8000  // Don't send remote request reports

 // Uplink list element structure

 typedef struct _UPLINK {       /* uplink */
   struct _UPLINK FAR *puplinkPrev;     // Pointer to the prev element in list
   struct _UPLINK FAR *puplinkNext;     // Pointer to the next element in list
   USHORT       fs;                     // Uplink flags (UF_)
   USHORT       level;                  // Uplink min requesting node level
   NETADDR      netAddr;                // Node address to send afrq from
   PNODE        pnode;                  // Uplink node info
   PSZ          pszName;                // Areafix name or external command
   PLSZ         plszMask;               // List of areatag masks
 } UPLINK, FAR * PUPLINK;

 // Uplink flags (uplink.fs)

 #define UF_AREAFIXPROT         0x0001  // Standard AreaFix protocol
 #define UF_NOPLUSPREFIX        0x0002  // Don't prepend + to areatag
 #define UF_NOMULTIAREATAGS     0x0004  // Don't send multiple areatags in msg
 #define UF_LOWERCASETAG        0x0008  // Send areatags in lower case
 #define UF_FREQNOPTHRU         0x0010  // No passthru for freq areas
 #define UF_EXTERNALAFREQ       0x8000  // External areafix requestor

 // Linked node list element structure

 typedef struct _LINK {         /* link */
   struct _LINK FAR *plinkPrev;         // Pointer to the prev element in list
   struct _LINK FAR *plinkNext;         // Pointer to the next element in list
   NETADDR      netAddr;                // Linked node net address
   USHORT       fs;                     // Linked node flags (LF_)
 } LINK, FAR * PLINK;

 // Linked node flags in (link.fs)

#define LF_ACTIVE       0x0001          // Active link, passive otherwise
#define LF_UPDATED      0x0002          // Link is updated in config file

 // Area list element descriptor

 typedef struct _AREA {         /* area */
   struct _AREA FAR *pareaPrev;         // Pointer to the prev element in list
   struct _AREA FAR *pareaNext;         // Pointer to the next element in list
   USHORT       fs;                     // Echo area status flags (AF_)
   USHORT       level;                  // Echo area level
   PLINK        plink;                  // Pointer to the first linked node
   PNODE        pnodeAutoCreate;        // Pointer to the autocreate node
   // Pointers into pchBuf
   PCH          pchSqshFlag;            // Points to flags in Squish config
   PCH          pchSqshLink;            // Points to links in Squish config
   PCH          pchSqafLink;            // Points to links in SqaFix config
   PCH          pchSqshTail;            // Points to inline comments in Squish config
   PCH          pchSqafTail;            // Points to inline comments in SqaFix config
   // Allocated psz's
   PSZ          pszDescr;               // Echo area description pointer
   PSZ          pszSqshFlags;           // Echo area Squish flags
   PSZ          pszPath;                // Echo area base path
   PSZ          pszRules;               // Echo area rule file
   // Area tag data
   ULONG        hash;                   // Echo area tag hash value
   CHAR         chGroup;                // Echo area group
   CHAR         achTag[1];              // Echo area tag
 } AREA, FAR * PAREA;

 // Area flags (area.fs)

#define AF_ACTIVECHANGED        0x0001  // Active list has been changed
#define AF_PASSIVECHANGED       0x0002  // Passive list has been changed
#define AF_SQSHFLAGSCHANGED     0x0004  // Squish flags has been changed
#define AF_AUTOCREATEDAREA      0x0010  // Autocreated area
#define AF_AUTOCREATEREPORTED   0x0020  // Autocreated area reported
#define AF_RESCANOK             0x0100  // Allow rescan of this area
#define AF_VISIBLE              0x0200  // Show to non authorized nodes
#define AF_SENDRULES            0x0400  // Send rules when linking up
#define AF_INSQAFIXCFG          0x1000  // Set if area is in SqaFix.Cfg
#define AF_INSQUISHCFG          0x2000  // Set if area is in Squish.Cfg

 // New area info structure

 typedef struct _NEWAREA {      /* newarea */
   struct _NEWAREA FAR *pnewareaPrev;   // Pointer to the prev element in list
   struct _NEWAREA FAR *pnewareaNext;   // Pointer to the next element in list
   USHORT       fs;                     // New area flags NA_
   NETADDR      netAddr;                // Trigger node address
   PSZ          pszArea;                // Trigger area mask
   CHAR         chGroup;                // New area group character
   PSZ          pszFlags;               // New area Squish flags
   CHAR         achPath[MAXPATH];       // New area directory path
   NETADDR      anetNode[MAX_AUTO_NODE];// New area autolink nodes list
   NETADDR      anetNote[MAX_NOTE_NODE];// New area notify nodes list
   PSZ          apszNote[MAX_NOTE_AREA];// New area notify areas list
   PSZ          apszFile[MAX_NOTE_FILE];// New area notify files list
   PLSZ         plszRefuse;             // New area refuse area masks list
 } NEWAREA, FAR * PNEWAREA;

 // New area flags (newarea.fs)

 #define NA_CONVERTMASK         0x000f  // Tag2path conversion -- 8+3 if zero
 #define NA_CONVERTTREE         0x0001  // Tag2path conversion -- TREE
 #define NA_CONVERTTREEDIR      0x0002  // Tag2path conversion -- TREE\@
 #define NA_CONVERTHPFS         0x0003  // Tag2path conversion -- HPFS
 #define NA_CONVERTHPFSDIR      0x0004  // Tag2path conversion -- HPFS\@
 #define NA_CONVERTNTFS         0x0005  // Tag2path conversion -- NTFS
 #define NA_CONVERTNTFSDIR      0x0006  // Tag2path conversion -- NTFS\@
 #define NA_CONVERTCRC          0x0007  // Tag2path conversion -- CRC32
 /* Note: CONVERTLOWER is possible in addition to the above options */
 #define NA_CONVERTLOWER        0x0010  // Tag2path conversion -- Lower case name

 // Deleted area list element descriptor

 typedef struct _DELAREA {      /* delarea */
   struct _DELAREA FAR *pdelareaPrev;   // Pointer to the prev element in list
   struct _DELAREA FAR *pdelareaNext;   // Pointer to the next element in list
   PAREA        parea;                  // Pointer to the deleted AREA (not in list)
   PNODE        pnode;                  // Pointer to the deleted NODE (in list)
 } DELAREA, FAR * PDELAREA;

 // Area description list element descriptor

 typedef struct _AREADESC {     /* areadesc */
   struct _AREADESC FAR *pareadescPrev; // Pointer to the prev element in list
   struct _AREADESC FAR *pareadescNext; // Pointer to the next element in list
   PSZ          pszDescr;               // Echo area description pointer
   ULONG        hash;                   // Echo area tag hash value
   CHAR         achTag[1];              // Echo area tag
 } AREADESC, FAR * PAREADESC;

 // Notify list element structure

 typedef struct _NOTE {         /* note */
   struct _NOTE FAR *pnotePrev;         // Pointer to the prev element in list
   struct _NOTE FAR *pnoteNext;         // Pointer to the next element in list
   BYTE         fs;                     // Notification flags (NF_)
   BYTE         type;                   // Notification type (NT_)
   ULONG        time;                   // Type dependant time
   PSZ          pszDescr;               // Area description
   PUPLINK      puplink;                // Uplink node for NT_FREQ_CREATED
   PNODE        pnode;                  // FReq node for NT_FREQ_CREATED
   CHAR         achTag[1];              // Area tag
 } NOTE, FAR * PNOTE;

 // Notify list element type (note.type)

#define NT_FREQ                   0x10  // Freq notification
#define NT_FREQ_EXPWARN (NT_FREQ |0x01) // Freq expiration warning
#define NT_FREQ_EXPNOTE (NT_FREQ |0x02) // Freq expiration notification
#define NT_FREQ_CREATED (NT_FREQ |0x03) // Freq creation notification

#define NT_IDLE                   0x20  // Idle pthru notification
#define NT_IDLE_EXPWARN (NT_IDLE |0x01) // Idle pthru expiration warning
#define NT_IDLE_EXPNOTE (NT_IDLE |0x02) // Idle pthru expiration notification

 // Notify list element flags (note.fs)

#define NF_REPORTED               0x01  // Notification reported

 // AreaFix request list element descriptor

 typedef struct _AFRQ {         /* afrq */
   struct _AFRQ FAR *pafrqPrev;         // Pointer to the prev element in list
   struct _AFRQ FAR *pafrqNext;         // Pointer to the next element in list
   PUPLINK      puplink;                // Pointer to the uplink
   PLSZ         plszArea;               // List of areas to link or unlink
 } AFRQ, FAR * PAFRQ;

 // AreaFix request list element area flags (afrq.plszArea->fs)

#define AFRQ_LINK       0x0001          // Link area
#define AFRQ_UNLINK     0x0002          // Unlink area

 // SendFile list element descriptor

 typedef struct _SNDF {         /* sndf */
   struct _SNDF FAR *psndfPrev;         // Pointer to the prev element in list
   struct _SNDF FAR *psndfNext;         // Pointer to the next element in list
   PNODE        pnode;                  // Target node
   PSZ          pszSubj;                // Subject line
   CHAR         achFile[1];             // File to send
 } SNDF, FAR * PSNDF;

 // Done list element descriptor

 typedef struct _DONE {         /* done */
   struct _DONE FAR *pdonePrev;         // Pointer to the prev element in list
   struct _DONE FAR *pdoneNext;         // Pointer to the next element in list
   PAREA        parea;                  // Pointer to the AREA in SqaFix list
   BOOL         fWhat;                  // Action id (DN_)
 } DONE, FAR * PDONE;

 // Done list action flags (done.fWhat)

 #define DN_LINKED              0x0001  // Linked node to the area
 #define DN_UNLINKED            0x0002  // Unlinked node from the area
 #define DN_PASSIVE             0x0004  // Passivated node for the area
 #define DN_ACTIVE              0x0008  // Activated node for the area

 // Queue list element structure

 typedef struct _QUE {  /* que */
   struct _QUE FAR *pquePrev;           // Pointer to the prev element in list
   struct _QUE FAR *pqueNext;           // Pointer to the next element in list
   BYTE         type;                   // Que element types (QE_)
   BYTE         fs;                     // Que element flags (QF_)
   ULONG        time1;                  // Queueing time in secs
   ULONG        time2;                  // Expiration time in secs
   PLINK        plink;                  // List of node links
   ULONG        hash;                   // Echo area tag hash value
   CHAR         achTag[1];              // Echo area tag
 } QUE, FAR * PQUE;

 // Queue list element types (que.type)

#define QE_FREQ         1               // Forwarded request
#define QE_IDLE         2               // Idle passthru area
#define QE_KILL         3               // Killed passthru area

 // Queue list element flags (que.fs)

#define QF_WARNINGDONE  0x01            // Warning has been created
#define QF_SEENKILLAREA 0x02            // Seen kill area in BadMail

 // Archiver list element structure

 typedef struct _ARC {  /* arc */
   struct _ARC FAR *parcPrev;           // Pointer to the prev element in list
   struct _ARC FAR *parcNext;           // Pointer to the next element in list
   USHORT       fs;                     // Archiver flags (AF_)
   PLINK        plink;                  // Archiver nodes list
   CHAR         achArc[1];              // Archiver name
 } ARC, FAR * PARC;

 // Password list element structure

 typedef struct _PWL {  /* pwl */
   struct _PWL FAR *ppwlPrev;           // Pointer to the prev element in list
   struct _PWL FAR *ppwlNext;           // Pointer to the next element in list
   NETADDR      netAddr;                // Node address
   CHAR         achPassword[1];         // Node access password
 } PWL, FAR * PPWL;

 // Sent message list element structure

 typedef struct _SNTM {         /* psntm */
   struct _SNTM  FAR *psntmPrev;        // Pointer to the prev element in list
   struct _SNTM  FAR *psntmNext;        // Pointer to the next element in list
   USHORT       imsg;                   // Number of msg uids in this block
   UMSG         aumsg[16];              // Sent message uids array
 } SNTM, FAR * PSNTM;

 // SqaFix configuration structure

 typedef struct _CFG {  /* cfg */
   ULONG        fl;                     // Program flags (FL_)
   NETADDR      anetAddr[MAX_ADDR_NUMB];// My net address and AKA's
   CHAR         achNetMail[MAXPATH];    // Net mail folder path
   CHAR         achBadMail[MAXPATH];    // Bad mail folder path
   CHAR         achCfgSqaf[MAXPATH];    // SqaFix config file path
   CHAR         achCfgSqsh[MAXPATH];    // Squish config file path
   CHAR         achLogFile[MAXPATH];    // SqaFix log file path
   CHAR         achQueFile[MAXPATH];    // SqaFix que file path
   CHAR         achArcFile[MAXPATH];    // Squish arc file path
   CHAR         achOrigin[60];          // Squish origin line
#ifdef __SPAWNO__
   USHORT       fsSwapSpawn;            // Spawno init flags
   PSZ          pszSwapPath;            // Spawno swapping directory
#endif
   USHORT       cchMaxMsgLine;          // Report line max length
   USHORT       cchMaxMsgPart;          // Report part max length
   USHORT       usPointNet;             // Fakenet net number
   USHORT       fsDefAreaFlags;         // Default echo area flags (AF_)
   USHORT       fsDefNodeFlags;         // Default node flags (NF_)
   USHORT       usDefAreaLevel;         // Default echo area level
   USHORT       usDefNodeLevel;         // Default node level
   USHORT       usDefNodeMaxFreq;       // Default node freq limit
   USHORT       usDefNodeMaxRescan;     // Default node rescan limit
   USHORT       dayFReqTimeout;         // Days to keep forward request
   USHORT       dayFReqWarning;         // Days to warn forward request before
   USHORT       dayIdleTimeout;         // Days to keep idle passthru
   USHORT       dayIdleWarning;         // Days to warn idle passthru before
   USHORT       dayIdleIgnore;          // Days to keep idle passthru after
   PSZ          pszDefAreaRules;        // Default echo area rules
   PSZ          pszDefAreaDescr;        // Default echo area description
   PSZ          pszDefNodeGroups;       // Default node groups
   PSZ          pszDefPacker;           // Default packer name
   PSZ          pszRescanCommand;       // Rescan command string
   PSZ          pszAreaListSaveFile;    // Arealist save file
   NETADDR anetFReqNote[MAX_NOTE_NODE]; // Forwarded request notify nodes list
   PSZ     apszFReqNote[MAX_NOTE_AREA]; // Forwarded request notify areas list
   PSZ     apszFReqFile[MAX_NOTE_FILE]; // Forwarded request notify files list
   NETADDR anetIdleNote[MAX_NOTE_NODE]; // Idle passthru notify nodes list
   PSZ     apszIdleNote[MAX_NOTE_AREA]; // Idle passthru notify areas list
   PSZ     apszIdleFile[MAX_NOTE_FILE]; // Idle passthru notify files list
   NETADDR anetIdleNode[MAX_IDLE_NODE]; // Idle passthru ignore nodes list
   PSZ     apszAlias[MAX_ALIAS_NUM];    // To name alias list
   PSZ     apszIgnore[MAX_IGNORE_NUM];  // From name ignore list
   // Linked lists headers
   PAREA     pareaFirst;                // Areas list
   PNODE     pnodeFirst;                // Nodes list
   PUPLINK   puplinkFirst;              // Uplinks list
   PNEWAREA  pnewareaFirst;             // New area info list
   PAREADESC pareadescFirst;            // Area descriptions list
   PARC      parcFirst;                 // Packer list
   PLSZ      plszFReqRefuseAreaFirst;   // Refuse forward areas list
   PLSZ      plszFReqKeepAreaFirst;     // Keep forwarded areas list
   PLSZ      plszIdleKeepAreaFirst;     // Keep idle passthru areas list
   PLSZ      plszHelpExtFirst;          // Help file extension list
   PLSZ      plszAllowArc;              // Allowed packers list
   PLSZ      plszAvailArc;              // Available packers list
   PLSZ      plszIgnoreKeyFirst;        // Ignore config keyword list
   PLSZ      plszRefuseCreate;          // Create area refuse masks list
   // Miscellaneous runtime data
   USHORT cmdCode;                      // Requested command code
   BOOL  fExitCode;                     // Exit code to return to DOS
   BOOL  fManualMode;                   // TRUE if processing manual mode request
   BOOL  fPackChanged;                  // TRUE if packer list changed
   BOOL  fExecAreaMask;                 // TRUE if executing area mask
   CHAR  achAreaGroups[NUM_GROUPS + 1]; // Known groups identifier list
   PSZ   pszReqMSGID;                   // Remote request MSGID kludge data
   PQUE  pqueFirst;                     // Queue elements list
   PPWL  ppwlFirst;                     // Password list
   PNOTE pnoteFirst;                    // Notification list
   PAFRQ pafrqFirst;                    // AreaFix request list
   PSNDF psndfFirst;                    // Send file list
   PDONE pdoneFirst;                    // Done actions list
   PSNTM psntmFirst;                    // Sent messages list
   PDELAREA pdelareaFirst;              // Deleted area list
 } CFG, FAR * PCFG;

 // Program flags (cfg.fl)

 #define FL_REPORTMSG           0x00000001ul    // Send report to a node
 #define FL_SQUISHNETMAIL       0x00000002ul    // NetMail is in .SQ? base
 #define FL_SQUISHBADMAIL       0x00000004ul    // BadMail is in .SQ? base
 #define FL_VERBOSEMODE         0x00000008ul    // Verbose mode request
 #define FL_DETAILEDLOG         0x00000010ul    // Produce detailed log
 #define FL_NOERRORTRAP         0x00000020ul    // No critical error handler
 #define FL_USEREPLYKLUDGE      0x00000040ul    // Use ^aREPLY kludge
 #define FL_REWRITECFG          0x00000080ul    // Rewrite all config files
 #define FL_KEEPFAILEDREQ       0x00000100ul    // Keep failed requests
 #define FL_OVERRIDEGROUP       0x00000200ul    // Override group restrictions
 #define FL_USEMSGIDKLUDGE      0x00000400ul    // Use ^aMSGID kludge
 #define FL_FORCEINTLKLUDGE     0x00000800ul    // Force ^aINTL kludge
 #define FL_USEFULLNETADDR      0x00001000ul    // Use full network addresses
 #define FL_ADDTEARLINE         0x00002000ul    // Add tearline to netmail msgs
 #define FL_ADDORIGINLINE       0x00004000ul    // Add origin to netmail msgs
 #define FL_PRESERVEDESTRAREAS  0x00008000ul    // Preserve destroyed areas info
 #define FL_IGNORESENDCREATE    0x00010000ul    // Ignore force area creation flags
 #define FL_SQAFNEWAREAPLACE    0x00020000ul    // New area pos set in SqaFix.Cfg
 #define FL_SQSHNEWAREAPLACE    0x00040000ul    // New area pos set in Squish.Cfg
 #define FL_SORTAREATAG         0x00080000ul    // Sort reports by area tag
 #define FL_SORTAREAGROUP       0x00100000ul    // Sort reports by area group
 #define FL_RESCANPTHRUAREAS    0x00200000ul    // Rescan passthru areas
 #define FL_NOREQUESTFORWARDING 0x00400000ul    // Suppress request forwading
 #define FL_FREQTRYALLUPLINKS   0x00800000ul    // Try all available uplinks
 #define FL_IDLEIGNOREPASSIVE   0x01000000ul    // Ignore idle pthru passive links

 #define FL_TESTMODE            0x20000000ul    // Test mode -- keep cfg/msg
 #define FL_TESTMODEX           0x40000000ul    // Test mode -- keep queue
 #define FL_DISPLAYGENERATEDMSG 0x80000000ul    // Display generated message

/*
 * Function prototypes
 */

 // SQAFIX.C -- Main module

 ULONG APPENTRY GetFreeMemory(VOID);

 // SQACFG.C -- Config file management routines

 VOID APPENTRY InitCfgParser(VOID);
 VOID APPENTRY ScanCfgFile(BOOL fActive);
 VOID APPENTRY QuitCfgParser(VOID);
 VOID APPENTRY CheckAreaSpec(BOOL fSync);
 BOOL APPENTRY UpdateCfgFile(BOOL fActive);

 // SQAEXE.C -- Request executor routines

 typedef BOOL (APPENTRY * PFNEXEC)(PNODE, PSZ);

 VOID APPENTRY ExecExistAreaMask(PNODE pnode, PSZ pszAreaMask, PFNEXEC pfnExec);
 VOID APPENTRY ExecQueueAreaMask(PNODE pnode, PSZ pszAreaMask, PFNEXEC pfnExec);

 BOOL APPENTRY ExecLnkNodeArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecUnlNodeArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecPasNodeArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecActNodeArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecRulNodeArea(PNODE pnode, PSZ pszArea);

 BOOL APPENTRY ExecLnkNodeGroup(PNODE pnode, PSZ pszGroup);
 BOOL APPENTRY ExecUnlNodeGroup(PNODE pnode, PSZ pszGroup);
 BOOL APPENTRY ExecPasNodeGroup(PNODE pnode, PSZ pszGroup);
 BOOL APPENTRY ExecActNodeGroup(PNODE pnode, PSZ pszGroup);

 BOOL APPENTRY ExecRescanArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecSendAreaRules(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecCreateArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecDestroyArea(PNODE pnode, PSZ pszArea);

 BOOL APPENTRY ExecRLnNodeArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecRFrNodeArea(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY ExecRKlNodeArea(PNODE pnode, PSZ pszArea);

 BOOL APPENTRY ExecCompress(PNODE pnode, PSZ pszArc);

 BOOL APPENTRY CreateChangesReport(PNODE pnode, PSZ pszNull);
 BOOL APPENTRY CreateLinkedReport(PNODE pnode, PSZ pszNull);
 BOOL APPENTRY CreateUnlinkedReport(PNODE pnode, PSZ pszNull);
 BOOL APPENTRY CreateAllLinksReport(NETADDR * pnetAddr);
 BOOL APPENTRY CreateAreasReport(PNODE pnode, PSZ pszNull);
 BOOL APPENTRY CreateAvailReport(PNODE pnode, PSZ pszNull);
 BOOL APPENTRY CreateUsageHelp(PNODE pnode, PSZ pszExt);
 BOOL APPENTRY CreateRulesInfo(PNODE pnode, PSZ pszArea);
 BOOL APPENTRY CreateNewArea(PSZ pszArea, NETADDR * pnetAddr);
 BOOL APPENTRY CreateNewAreasReport(VOID);
 BOOL APPENTRY CreateNotes(VOID);

 BOOL APPENTRY SyncSqafArea(PAREA parea);

 // SQAMSG.C -- Message management routines

 HAREA APPENTRY OpenNetMailFolder(VOID);
 BOOL APPENTRY CloseNetMailFolder(VOID);
 HAREA APPENTRY OpenBadMailFolder(VOID);
 BOOL APPENTRY CloseBadMailFolder(HAREA hBadMail);
 HAREA APPENTRY OpenEchoMailFolder(PAREA parea);
 BOOL APPENTRY CloseEchoMailFolder(PAREA parea, HAREA hEchoMail);

 BOOL APPENTRY SendLongMsgBeg(NETADDR * pnetFromAddr, PSZ pszFrom,
                              NETADDR * pnetToAddr, PSZ pszTo,
                              USHORT attr, PSZ pszSubj, BOOL fFrame);
 BOOL APPENTRY SendLongMsgPut(PSZ pszText);
 BOOL APPENTRY SendLongMsgEnd(VOID);

 BOOL APPENTRY SendMsg(NETADDR * pnetFromAddr, PSZ pszFrom,
                       NETADDR * pnetToAddr, PSZ pszTo,
                       USHORT attr, PSZ pszSubj, PSZ pszText);

 BOOL APPENTRY PostMsg(PAREA parea,
                       NETADDR * pnetFromAddr, PSZ pszFrom,
                       NETADDR * pnetToAddr, PSZ pszTo,
                       USHORT attr, PSZ pszSubj, PSZ pszText);

//BOOL APPENTRY SendNotify(NETADDR anetAddr[], USHORT cnetAddr,
//                         PSZ pszTitle, PSZ pszText);
//BOOL APPENTRY PostNotify(PSZ apszArea[], USHORT cpszArea,
//                         PSZ pszTitle, PSZ pszText);

 PSZ APPENTRY GetMsgKludge(PSZ pszKludge, PCH pch, USHORT cch, BOOL fLast);
 BOOL APPENTRY GetEchoMsgPath(HMSG hmsg, XMSG * pmsg, UMSG umsg,
                              NETADDR * pnetAddr);

 BOOL APPENTRY LogMsg(HAREA harea, HMSG hmsg, XMSG * pmsg, UMSG umsg,
                      PSZ pszArea, USHORT fs);

 // SQAARC.C -- Packer management routines

 PARC APPENTRY AddArc(PSZ pszArc, USHORT ipos);
 PARC APPENTRY GetArc(NETADDR * pnetAddr, PLINK * pplink);
 BOOL APPENTRY CheckArcAddrMask(PARC parc);
 PLINK APPENTRY AddArcLink(PARC parc, NETADDR * pnetAddr);
 PLINK APPENTRY GetArcLink(PARC parc, NETADDR * pnetAddr);
 BOOL APPENTRY DelArcLink(PARC parc, NETADDR * pnetAddr);
 BOOL APPENTRY LoadArcSpec(VOID);
 VOID APPENTRY CheckArcSpec(VOID);

 // SQAUTI.C -- Miscellaneous utility routines

 ULONG APPENTRY CalcHash(PSZ psz);
 CHAR APPENTRY SkipSpaces(PCH * ppch);
 BOOL APPENTRY IsSpecCmdChars(PSZ psz);
 BOOL APPENTRY IsSquishArea(PSZ pszFlags);
 BOOL APPENTRY IsPassThruArea(PSZ pszFlags);
 BOOL APPENTRY IsIdlePassThruArea(PAREA parea);
 BOOL APPENTRY CheckGroup(PSZ pszGroups, CHAR chGroup, BOOL * pfReadOnly);
 PSZ APPENTRY AllocString(PCH pch, SHORT cch);
 BOOL APPENTRY AddApsz(PSZ apsz[], USHORT cpsz, PSZ psz);
 BOOL APPENTRY IsStringInList(PSZ apsz[], USHORT cpsz, PSZ psz);
 BOOL APPENTRY IsPrefixInList(PSZ apsz[], USHORT cpsz, PSZ psz);
 BOOL APPENTRY IsMatchInList(PSZ apsz[], USHORT cpsz, PSZ psz);
 PLSZ APPENTRY AddLsz(PLSZ * pplsz, PCH pch, SHORT cch, SHORT ipos);
 PLSZ APPENTRY GetLsz(PLSZ plsz, PSZ psz);
 VOID APPENTRY DelLszList(PLSZ * pplsz);
 NETADDR * APPENTRY GetAddrMatch(NETADDR * pnetAddr);

 USHORT APPENTRY GetNodeMsgAttr(PNODE pnode);
 PSZ APPENTRY GetNodeSysop(PNODE pnode);
 PNODE APPENTRY GetNodeFromAddr(NETADDR * pnetAddr);
 PUPLINK APPENTRY GetUplinkFromAddr(NETADDR * pnetAddr);
 PUPLINK APPENTRY GetUplinkFromNode(PNODE pnode);
 PUPLINK APPENTRY GetIdleAreaUplink(PAREA parea);
 PCH APPENTRY ScanNetAddr(NETADDR * pnetAddr, PSZ psz);
 PCH APPENTRY ScanNetAddrMask(NETADDR * pnetAddr, PSZ psz);
 PSZ APPENTRY FormatNetAddr(NETADDR * pnetAddr);
 SHORT APPENTRY CompNetAddr(NETADDR * pnetAddr1, NETADDR * pnetAddr2);
 PSZ APPENTRY MakeNetAddrList(NETADDR * pnetAddr, NETADDR * pnetAddrPrev);
 BOOL APPENTRY IsAddrInList(NETADDR anetAddr[], USHORT cnetAddr, NETADDR * pnetAddr);
 BOOL APPENTRY IsAddrMask(NETADDR * pnetAddr);
 BOOL APPENTRY IsMyAka(NETADDR * pnetAddr);

 PAREA APPENTRY AddArea(PSZ pszArea, CHAR chGroup);
 PAREA APPENTRY GetAreaFromTag(PSZ pszArea);
 PAREA APPENTRY GetAreaFromTagAlt(PAREA pareaFirstAlt, PSZ pszArea);
 PAREA APPENTRY GetAreaFromPath(PSZ pszPath);
 BOOL APPENTRY GetAreaOrigAddr(PAREA parea, NETADDR * pnetAddr);
 BOOL APPENTRY AddAreaLink(PAREA parea, NETADDR * pnetAddr, BOOL fActive, PSZ pszLog);
 BOOL APPENTRY DelAreaLink(PAREA parea, NETADDR * pnetAddr);
 PLINK APPENTRY GetAreaLink(PAREA parea, NETADDR * pnetAddr);
 BOOL APPENTRY SetAreaLink(PAREA parea, PLINK plink, BOOL fActive);
 VOID APPENTRY AddToDone(PAREA parea, BOOL fWhat);
 PDELAREA APPENTRY DelArea(PAREA parea);
 BOOL APPENTRY AutoDelArea(PAREA parea);
 BOOL APPENTRY DelAreaMsgBase(PAREA parea);
 PDELAREA APPENTRY GetDelAreaFromTag(PSZ pszArea);

 VOID APPENTRY AppendFileExt(PSZ pszDst, PSZ pszSrc, PSZ pszExt, BOOL fAlways);
 BOOL APPENTRY BuildFullPath(PSZ pszDest, PSZ pszSrc);

 USHORT GetTextLeng(va_list argptr, PSZ pszFormat, ...);
 VOID APPENTRY WriteMsgLine(SHORT cch, CHAR ch, BOOL fNewLine);
 USHORT WriteMsg(PSZ pszFormat, ...);
 VOID WriteLog(PSZ pszFormat, ...);
 VOID Assert(PSZ pszText, PSZ pszFile, USHORT iLine);

 PAREADESC APPENTRY AddAreaDescr(PSZ pszAreaTag, PSZ pszDescr);
 PSZ APPENTRY GetAreaDescrFromTag(PSZ pszAreaTag);
 BOOL APPENTRY LoadAreaDescrFileNA(PSZ pszFile);
 BOOL APPENTRY SaveAreaDescrFileNA(PSZ pszFile);
 BOOL APPENTRY LoadAreaDescrFileDZ(PSZ pszFile);
 BOOL APPENTRY SaveAreaListFile(PSZ pszFile);
 BOOL APPENTRY LoadAreaMaskFileNA(PSZ pszFile, PLSZ * pplszFirst);

 BOOL APPENTRY GrepSearch(PSZ pszText, PSZ pszPattern, BOOL fCase);
 BOOL APPENTRY IsWildGrep(PSZ pszPattern);

 time_t APPENTRY DttmToSecs(PDTTM pdttm);
 VOID APPENTRY SecsToDttm(time_t secs, PDTTM pdttm);
 USHORT APPENTRY ScanDttm(PCH pch, PDTTM pdttm);
 PSZ APPENTRY FormatDttm(PDTTM pdttm);
 PSZ APPENTRY FormatSecs(time_t secs);

 BOOL APPENTRY AddQueNodeLink(PQUE pque, NETADDR * pnetAddr);
 BOOL APPENTRY DelQueNodeLink(PQUE pque, NETADDR * pnetAddr);
 VOID APPENTRY DelQueNodeLinks(PQUE pque);
 PLINK APPENTRY GetQueNodeLink(PQUE pque, NETADDR * pnetAddr);
 PQUE APPENTRY AddQueEntry(PSZ pszArea, USHORT type, time_t time1, time_t time2);
 PQUE APPENTRY GetQueEntry(PSZ pszArea, USHORT type);
 BOOL APPENTRY DelQueEntry(PSZ pszArea);

 BOOL APPENTRY CheckIdlePassThruAreas(VOID);
 BOOL APPENTRY CheckQueEntry(PQUE pque);

 BOOL APPENTRY LoadQueFile(PSZ pszFile);
 BOOL APPENTRY SaveQueFile(PSZ pszFile);

 PUPLINK APPENTRY GetFreqUplink(PNODE pnode, PSZ pszArea);
 PUPLINK APPENTRY GetNextFreqUplink(PQUE pque, PNODE * ppnode);

 PAFRQ APPENTRY GetAfrqForUplink(PUPLINK puplink);
 PAFRQ APPENTRY AddAfrq(PUPLINK puplink, PSZ pszArea, USHORT fs);
 VOID APPENTRY DelAfrqList(VOID);
 VOID APPENTRY SendAreaFixReqs(VOID);

 PSNDF APPENTRY AddSndf(PNODE pnode, PSZ pszFile, PSZ pszSubj);
 VOID APPENTRY DelSndfList(VOID);
 VOID APPENTRY SendFiles(VOID);

 BOOL APPENTRY AddSntm(UMSG umsg);
 BOOL APPENTRY GetSntm(UMSG umsg);

 PNOTE APPENTRY AddNote(PSZ pszArea, USHORT type);
 VOID APPENTRY DelNoteList(VOID);
//USHORT APPENTRY CalcNotes(USHORT type);

 BOOL APPENTRY LoadPwlFile(PSZ pszFile);
 BOOL APPENTRY AddNodePwl(NETADDR * pnetAddr, PSZ pszPassword);
 PPWL APPENTRY GetNodePwl(NETADDR * pnetAddr);

 VOID APPENTRY GetCommandShellName(PSZ pszCmd);
 BOOL APPENTRY DelFile(PSZ pszFile);
 BOOL APPENTRY RenFile(PSZ pszFileOld, PSZ pszFileNew);
 VOID APPENTRY MakeTmpFile(PSZ pszFile, PSZ pszBase);

/*
 * Program global variables
 */

 extern CFG     cfg;                    // Configuration structure
 extern PCH     pchBuf;                 // Working buffer pointer
 extern USHORT  cchBuf;                 // Working buffer size
 extern USHORT  ichBuf;                 // Working buffer used bytes count
 extern HAREA   hNetMail;               // Netmail folder handle
 extern NEWAREA newareaDef;             // Default new area info
 extern struct _minf minf;              // Global msg info

/*
 * End of SQAFIX.H
 */
