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

/*
  EBNF structure for SQUISH.STT, replacing the old SQUISH.STA:

    file:       { block }*

    block:      _thdr blockdata

    blockdata:  _tpkt
             |  _tarea { tnode }*
             |  _tdupe
             |  _tstamp
*/


#ifndef __SSTAT2_H_DEFINED
#define __SSTAT2_H_DEFINED

#include <time.h>




/* Tag header */

struct _thdr
{
  word type;    /* Record type.  See TYPE_XXXX below */
  word len;     /* Length of the following record */
};


#define TYPE_PKT    0x0001    /* _tpkt follows */
#define TYPE_AREA   0x0002    /* _tarea follows */
#define TYPE_DUPE   0x0003    /* _tdupe follows */
#define TYPE_BEGIN  0x0004    /* _tstamp follows.  Beginning of execution */
#define TYPE_BTOSS  0x0005    /* 0-length, marks toss from bad_msgs */
#define TYPE_END    0x0006    /* _tstamp follows.  End of execution */

/* If thdr.type has a value other than the above, the record should         *
 * be ignored!  Simply skip ahead by thdr.len bytes and read the next       *
 * thdr structure.                                                          */


/* Maximum length of an area tag written to SQUISH.STT */

#define AH_TAGLEN   30


/* Structure written when a packet is received */

struct _tpkt
{
  NETADDR orig;           /* Origination address of packet */
  char pktname[14];       /* Packet filename */
  dword size;             /* Size of packet */
  SCOMBO proc_time;       /* Time at which packet was processed */
};

/* Structure written once for every area received in every packet.          *
 * An array of _tnode structures always follows this, as defined by         *
 * the n_nodes field in this structure.                                     */

struct _tarea
{
  char tag[AH_TAGLEN];
  dword in_msgs;
  dword in_bytes;

  word n_nodes;           /* Number of _tnode structs following this one */
  word taflag;
};

#define TAFLAG_PASSTHRU   0x01 /* This echo is a passthru area */


/* Record for each node exported to, for each area.  Zero or more _tnode    *
 * structures follow each _tarea structure.                                 */

struct _tnode
{
  NETADDR node;
  dword out_msgs;
  dword out_bytes;
};


/* Duplicate messages received in the specified echo.  Comes at the end     *
 * of each packet, after any _tnode and _tarea structures, but before       *
 * the next _tpkt.                                                          */

struct _tdupe
{
  char tag[AH_TAGLEN];
  word n_dupes;
};


/* Timestamp - for both TYPE_BEGIN and TYPE_END */

struct _tstamp
{
  time_t date;      /* as returned by time(NULL) */
};

#endif /* !__SSTAT2_H_DEFINED */

