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

#ifndef __API_BROW_H_DEFINED
#define __API_BROW_H_DEFINED

#define BROWSE_ACUR   0x0001  /* Scan current area only                     */
#define BROWSE_ATAG   0x0002  /* Scan only marked areas                     */
#define BROWSE_AALL   0x0004  /* Scan ALL areas                             */
#define BROWSE_ALNT   0x0080  /* Scan local & netmail areas only            */
#define BROWSE_AWLD   0x4000  /* Scan wildcard areas                        */
#define BROWSE_AGRP   0x8000  /* Scan all areas in current group            */

#define BROWSE_ALL    0x0008  /* Scan all messages in area                  */
#define BROWSE_NEW    0x0010  /* Scan only msgs since lastread              */
#define BROWSE_SEARCH 0x0020  /* Search all msgs in area based on criteria  */
#define BROWSE_FROM   0x0040  /* Search from msg#XX and up.                 */

#define BROWSE_READ   0x0100  /* Show messages in full form                 */
#define BROWSE_LIST   0x0200  /* Display msg headers only                   */
#define BROWSE_QWK    0x0400  /* QWK file format download                   */

#define BROWSE_GETTXT 0x0800  /* We have to read msg text when scanning     */
#define BROWSE_EXACT  0x1000  /* Match search strings EXACTLY               *
                               * (ie. use stricmp(), not stristr().)        */
#define BROWSE_HASH   0x2000  /* Use hash compare for this one only         */

#define BROWSE_AREA (BROWSE_ACUR | BROWSE_ATAG | BROWSE_AALL | BROWSE_AWLD | BROWSE_AGRP | BROWSE_ALNT)
#define BROWSE_TYPE (BROWSE_ALL | BROWSE_NEW | BROWSE_SEARCH | BROWSE_FROM)
#define BROWSE_DISPLAY (BROWSE_READ | BROWSE_LIST | BROWSE_QWK)

/* Flags used for controlling the and/or logic of the search cmd */

#define SF_HAS_ATTR     0x01
#define SF_NOT_ATTR     0x02
#define SF_OR           0x04
#define SF_AND          0x08

/* Where to look for specified search text */

#define WHERE_TO        0x01
#define WHERE_FROM      0x02
#define WHERE_SUBJ      0x04
#define WHERE_BODY      0x08

#define WHERE_ALL (WHERE_TO | WHERE_FROM | WHERE_SUBJ | WHERE_BODY)


#define SCAN_BLOCK_SBREC  512   /* # of msgs to read in from scanfile at    *
                                 * once from SCANFILE.DAT.                  */
#define SCAN_BLOCK_SQUISH 512   /* Same as above, but for xxxxxxxx.SQI      */


typedef struct _search
{
  struct _search *next;         /* Next item in search list */

  long attr;                    /* Attributes to check for in this msg */
  int  flag;                    /* and/or flag */

  char *txt;                    /* Text to find */
  char where;                   /* Where in msg to search */

} SEARCH;


struct _browse;
typedef struct _browse BROWSE;

struct _browse
{
  char *path;               /* Area name */
  word type;                /* MSGTYPE_xxxx */

  word bflag;               /* Browse options */
  dword bdata;              /* Usually current msg number */
  SEARCH *first;            /* List of topics to search for */
  char *nonstop;            /* Nonstop flag for browse output */
  char *menuname;           /* Menu name (for sending files in qwk download) */
  char *wildcard;           /* Current wildcard/group name */
  
  dword msgn;               /* Current message number */
  
  HAREA sq;                 /* Handle for current area */
  HMSG m;                   /* Handle for current message */
  XMSG msg;                 /* Header of current message */
  word matched;             /* Have we matched all options in the search? */
  
  /* Function pointers for various options */

  int (*Begin_Ptr)(BROWSE *b);
  int (*Status_Ptr)(BROWSE *b,char *aname,int colour);
  int (*Idle_Ptr)(BROWSE *b);
  int (*Display_Ptr)(BROWSE *b);
  int (*After_Ptr)(BROWSE *b);
  int (*End_Ptr)(BROWSE *b);
  int (*Match_Ptr)(BROWSE *b);

  int fSilent;            /* Do not display any output (for mb_qwk only) */
};

#endif /* __API_BROW_H_DEFINED */

