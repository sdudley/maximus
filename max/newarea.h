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

#ifndef __NEWAREA_H_DEFINED
#define __NEWAREA_H_DEFINED

#include "option.h"

#ifndef __netaddr_defined
#define __netaddr_defined
typedef struct _netaddr NETADDR;
struct _netaddr
{
  word zone;
  word net;
  word node;
  word point;
};
#endif

/* General structure of MAREA.DAT and FAREA.DAT:

   MAREA_ID or FAREA_ID
   _msgarea
       _ovride ...
       zstr heap
   _msgarea
       _ovride ...
       zstr heap
   eof
*/


/* This key is present at the beginning of an area file.  MAREA_ID is
 * used for the message data file and FAREA_ID is used for the
 * file data file.
 */

#define MAREA_ID      0x1a49023fL
#define FAREA_ID      0x1a01953aL

#define ADATA_START   4L          /* The area data info starts at offset 4 */

typedef struct _ovride
{
  /* Only one of 'opt' or 'name' should be used.  If opt==0, use name.  If  *
   * name==0, use opt.                                                      */

  option opt;           /* Type of menu option to override     ...OR...     */
  byte   name;          /* First letter of command to override              */
  byte   rsvd1;         /* Reserved for future use                          */
  zstr   acs;           /* New ACS required to access option                */
  zstr   menuname;      /* Use this access level on the given menu only     */
} OVERRIDE;


/* Bit masks for the ma.attribs field */

#define MA_PVT      0x0001  /* Private msgs allowed */
#define MA_PUB      0x0002  /* Public msgs allowed */
#define MA_HIBIT    0x0004  /* High bit msgs allowed */

#define MA_NET      0x0008  /* Netmail area */
#define MA_ECHO     0x0010  /* Echomail area */
#define MA_CONF     0x0020  /* Conference area */

#define MA_ANON     0x0040  /* Anonymous messages are OK */
#define MA_NORNK    0x0080  /* Don't use the REALNAME kludge for this area */
#define MA_REAL     0x0100  /* Force  use of       real name for this area */
#define MA_ALIAS    0x0200  /* Force  use of alias      name for this area */
#define MA_AUDIT    0x0400  /* Use auditing (msg tracking) controls in area*/
#define MA_READONLY 0x0800  /* Area is read-only */
#define MA_HIDDN    0x1000  /* Area does not display on normal area list   */
#define MA_ATTACH   0x2000  /* Area allows local file attaches             */
#define MA_DIVBEGIN 0x4000  /* A message area division, not a real area    */
#define MA_DIVEND   0x8000  /* End of the message area division            */

#define MA2_NOMCHK  0x0001  /* Don't do personal mail check in this area   */

#define MA_SHARED   (MA_ECHO | MA_CONF)


/* Bit masks for fa.attribs */

#define FA_SLOW     0x0001  /* Slow-access medium: skip existence checks */
#define FA_STAGED   0x0002  /* Used staged transfer area for downloads */
#define FA_NONEW    0x0004  /* Permanent storage - skip for new file checks */
#define FA_HIDDN    0x0008  /* Area does not display on normal area list */
#define FA_DIVBEGIN 0x4000  /* A file area division, not a real area */
#define FA_DIVEND   0x8000  /* End of file area division */
#define FA_AUTODATE 0x0010  /* Auto-date override */
#define FA_MANDATE  0x0020  /* Manual date override */
#define FA_LISTDATE 0x0040  /* List-date override */
#define FA_FREETIME 0x0100  /* Free download time for all files */
#define FA_FREESIZE 0x0200  /* Free download bytes for all files */
#define FA_NOINDEX  0x0400  /* Don't add this area to maxfiles.idx */

#define FA_CDROM    (FA_SLOW | FA_STAGED | FA_NONEW)
#define FA_FREEALL  (FA_FREETIME | FA_FREESIZE)

typedef struct _msgarea
{
  word cbArea;          /* Length of THIS INDIVIDUAL RECORD                0*/
  word num_override;    /* Number of overrides following this record       2*/
  word cbHeap;          /* Length of the zstr heap following the overrides 4*/
  word division;        /* Reserved for future use                         6*/
  zstr name;            /* String format of area's name.                   8*/
  zstr acs;             /* Access control string for this area            10*/
  zstr path;            /* Path to messages (but for MA_DIVBEGIN only,    12*
                         * used instead as name of custom .bbs file).       */
  zstr echo_tag;        /* The 'tag' of the area, for use in ECHOTOSS.LOG 14*/
  zstr descript;        /* The DIR.BBS-like description for msg section   16*/
  zstr origin;          /* The ORIGIN line for this area                  18*/
  zstr menuname;        /* Custom menu name                               20*/
  zstr menureplace;     /* Replace this menu name with menuname from above22*/

  word attribs;         /* Attributes for this area                       24*/

  NETADDR primary;      /* Use as primary address for this area           26*/
  NETADDR seenby;       /* Use as address in seen-bys                     34*/
  word attribs_2;       /* More attributes                                42*/
  word type;            /* Message base type.  MSGTYPE_SDM = *.MSG.       44*
                         * MSGTYPE_SQUISH = SquishMail.  (Constants are     *
                         * in MSGAPI.H)                                     */
  word killbyage;       /* Make sure msgs are less than X days old        46*/
  word killbynum;       /* Make sure there are less than X msgs           48*/
  word killskip;        /* Exempt the first X msgs from this processing   50*/
  zstr barricade;       /* Barricade file                                 52*/
  zstr barricademenu;   /* Apply barricade priv while using this menu     54*/
  sdword cbPrior;       /* Seek offset from start of this area to get back56*
                         * to prior area.                                   */
  zstr attachpath;      /* Reserved for future use                        58*/
  dword rsvd4;                                                          /*60*/
} MAREA;                                                                /*64*/


typedef struct _filearea
{
  word cbArea;          /* Length of THIS INDIVIDUAL RECORD                0*/
  word num_override;    /* Number of overrides following this record       2*/
  word cbHeap;          /* Length of the zstr heap following the overrides 4*/
  word division;        /* Reserved for future use                         6*/

  zstr acs;             /* Access control string for this area             8*/
  zstr name;            /* String format of area's name.                  10*/
  zstr downpath;        /* Path for downloads.                            12*/
  zstr uppath;          /* Path for uploads                               14*/
  zstr filesbbs;        /* Path to FILES.BBS-like catalog for this area   16*
                         * (For FA_DIVBEGIN only, also used as name of      *
                         * custom .bbs display file.)                       */
  zstr descript;        /* The DIR.BBS-like description for file section  18*/
  zstr menuname;        /* Custom menu for this file area                 20*/
  zstr menureplace;     /* Replace this menu name with menuname from above22*/
  zstr barricade;       /* Barricade file                                 24*/
  zstr barricademenu;   /* Barricade file                                 26*/
  sdword cbPrior;       /* Offset to prior area                           28*/
  word attribs;         /* Attributes for this file area                  32*/
  sword date_style;     /* Date style override for this area              34*/
  byte rsvd1[28];       /* Reserved by Maximus for future use             36*/
} FAREA;                                                                /*64*/


typedef struct _barinfo
{
  int use_barpriv;            /* Use the privs in this structure? */
  word priv;                  /* New priv level to use while in this area */
  dword keys;                 /* New keys to use while in this area */
} BARINFO;

/* In-memory handles for accessing message and file areas */

typedef struct _mahandle
{
  OVERRIDE *pov;
  char *heap;
  MAREA ma;
  int heap_size;

  BARINFO bi;
} MAH, *PMAH;

typedef struct _fahandle
{
  OVERRIDE *pov;
  char *heap;
  FAREA fa;
  int heap_size;

  BARINFO bi;
} FAH, *PFAH;


/* Macros for accessing the zstr values of the MAH/FAH structures. */

#define MAS(ah, var) ((ah).heap + (ah).ma.var)
#define PMAS(ah, var) ((ah)->heap + (ah)->ma.var)

#define FAS(ah, var) ((ah).heap + (ah).fa.var)
#define PFAS(ah, var) ((ah)->heap + (ah)->fa.var)


/* Index structure for message/file areas */

typedef struct _mfidx
{
  char name[16];        /* First 15 chars of area name                      */
  dword name_hash;      /* SquishHash of full area name                     */
  dword ofs;            /* Offset within data file                          */
} MFIDX;

#endif /* __NEWAREA_H_DEFINED */


