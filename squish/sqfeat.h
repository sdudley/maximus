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

#ifndef __SQFEAT_H_DEFINED
#define __SQFEAT_H_DEFINED

#ifdef OS_2 /* DLL feature library */

#ifndef PATHLEN
  #define PATHLEN 120
#endif

#ifndef CFGLEN
  #define CFGLEN 4096
#endif

#ifndef MAX_TAGLEN
  #define MAX_TAGLEN 128
#endif

/* API entry type */

#ifdef __FLAT__
  #define FEATENTRY pascal
  #define FEATRET   USHORT
  #define FEATFAR
#else
  #define FEATENTRY pascal far _loadds
  #define FEATRET   USHORT
  #define FEATFAR   far
#endif


/* Structure for the "init" call */

struct _feat_init
{
  USHORT struct_len;                                /* Length of struct */
  char szConfigName[PATHLEN];                       /* Config items */
  void (cdecl far *pfnLogMsg)(char far *line);      /* Write to Squish log. *
                                                     * SEE BELOW!           */
  ULONG ulFlag;                                     /* Flags filled in by   *
                                                     * DLL.                 */
};

/* NOTE!  The format of the string passed to the LogMsg function must be
 * as follows:
 *
 * <char><description>
 *
 * where <char> is the priority character to appear at the beginning of
 * the line, and <descrption> is the log message itself.  For example,
 * the following function call:
 *
 * (*pfi->LogMsg)("!Invalid address declared");
 *
 * might result in this log entry:
 *
 * ! 01 Jan 00:11:22 SQSH Invalid address declared!
 *
 * Note that log messages will only be written to the screen until the
 * config file has been completely parsed.  (In other words, log messages
 * in FeatureInit or FeatureConfig will not be written to the log file.)
 * Since the LogMsg pointer is only passed once, a private copy of the
 * function pointer MUST be saved as a static variable in your .DLL,
 * since the _feat_init pointer will become invalid after FeatureInit
 * returns.
 */

/* Structure for the "config line" call */

struct _feat_config
{
  USHORT struct_len;
  char szConfigLine[CFGLEN];
  char FEATFAR * FEATFAR * ppszArgs;
};

/* Structure for the "process netmail message" call */

struct _feat_netmsg
{
  USHORT struct_len;            /* Length of this structure                 */
  ULONG ulAction;               /* Filled in by the feature: Requested      *
                                 * action.  See FACT_XXX, below.            */
  char FEATFAR *pszAreaTag;     /* Tag for the current netmail area         */
  HAREA ha;                     /* MsgAPI handle for current area           */
  HMSG hmsg;                    /* MsgAPI handle for current message        */
  ULONG ulMsgNum;               /* Message number of this message           */
  PXMSG pMsg;                   /* Pointer to this message's header info    */
  char FEATFAR *pszCtrl;        /* Pointer to this message's control info   */
  char FEATFAR *pszMsgTxt;      /* Pointer to this message's body info      */
  NETADDR us;                   /* Our primary address                      */
};


/* Bitflags for the ulAction field, above.  Any number of these may be      *
 * combined using the bitwise OR ("|") operator.  Note that FACT_KILL       *
 * will be processed after the message has been packed and/or passed to     *
 * any of the other handlers.  If the feature wants the message to be       *
 * killed immediately, use FACT_KILL|FACT_SKIP|FACT_HIDE.  Note that        *
 * FACT_RWMSG cannot be used in conjunction with FACT_KILL.                 */

#define FACT_NONE     0x00000000  /* No action */
#define FACT_KILL     0x00000001  /* Delete this message */
#define FACT_SKIP     0x00000002  /* Leave this message alone (do not pack) */
#define FACT_HIDE     0x00000004  /* Do not pass this message to other      *
                                   * features.                              */
#define FACT_RWMSG    0x00000008  /* Rewrite message.  The header was       *
                                   * updated and needs to be written back   *
                                   * to disk.                               */

/* Structure for tossing a message */

struct _feat_toss
{
  USHORT struct_len;
  ULONG ulTossAction;           /* Filled in by the feature: Requested      *
                                 * action.  See FTACT_XXX, below.           */

  char szArea[MAX_TAGLEN];      /* Area tag for this message.  Changing     *
                                 * this tag instructs Squish to toss the    *
                                 * message to a different area.  The        *
                                 * FTACT_AREA flag must be used for this to *
                                 * take effect!                             */
  char szPktName[PATHLEN];      /* Name of current packet file              */
  PXMSG pMsg;                   /* Message header                           */
  char *pszCtrl;                /* Message control info                     */
  char *pszMsgTxt;              /* Message body                             */
};


#define FTACT_NONE    0x00000000
#define FTACT_KILL    0x00000001  /* Kill message without tossing it        */
#define FTACT_AREA    0x00000002  /* Toss to the new area tag specified in  *
                                   * szArea.                                */
#define FTACT_HIDE    0x00000004  /* Do not pass this to any of the other   *
                                   * features.                              */
#define FTACT_NSCN    0x00000008  /* Do not scan this message to anyone     *
                                   * else (if doing a one-pass toss/scan).  */


/* Structure for scanning a message */

struct _feat_scan
{
  USHORT struct_len;
  ULONG ulScanAction;           /* Filled in by the feature: Requested      *
                                 * action.  See FSACT_XXX, below.           */

  char szArea[MAX_TAGLEN];      /* Area tag for this message.  This field   *
                                 * cannot be changed.                       */

  PXMSG pMsg;                   /* Message header                           */
  char *pszCtrl;                /* Message control info                     */
  char *pszMsgTxt;              /* Message body                             */
};



/* Termination structure */

struct _feat_term
{
  USHORT struct_len;
};


/* Linked list of features */

struct _feature
{
  char *pszDLLName;       /* Name of .DLL */
  HMODULE hmod;           /* Module handle for .DLL */
  ULONG ulFlag;           /* Flags for this feature - see FFLAG_XXX, below*/

  char *pszConfigName;    /* Name for DLL-specific items in config file */

  FEATRET (FEATENTRY *pfnInit)(struct _feat_init FEATFAR *pfi);
  FEATRET (FEATENTRY *pfnConfig)(struct _feat_config FEATFAR *pfc);
  FEATRET (FEATENTRY *pfnNetMsg)(struct _feat_netmsg FEATFAR *pfn);
  FEATRET (FEATENTRY *pfnTossMsg)(struct _feat_toss FEATFAR *pft);
  FEATRET (FEATENTRY *pfnScanMsg)(struct _feat_scan FEATFAR *pft);
  FEATRET (FEATENTRY *pfnTerm)(struct _feat_term FEATFAR *pft);

  struct _feature *pfNext;
};


/* For the netmail packer feature: */

#define FFLAG_NETSENT   0x00000001 /* Only call FeatureNetMsg for messages  *
                                    * which do NOT have the MSGSENT bit     *
                                    * set.                                  */

#define FFLAG_NETTOUS   0x00000002 /* Only call FeatureNetMsg for messages  *
                                    * which are to one of our addresses.    */

#define FFLAG_NETRECD   0x00000004 /* Only call FeatureNetMsg for messages  *
                                    * which do not have MSGREAD set.        */

#define FFLAG_NETNOTTOUS 0x00000008 /* Only call FeatureNetMsg for messages  *
                                    * which are NOT to one of our addresses. */

#endif /* OS_2 */

#endif /* __SQFEAT_H_DEFINED */

