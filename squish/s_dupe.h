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

/* Structure for the *.SQB duplicate database */

typedef struct
{
  #define DUPEHEAD_SIG  0x98761234L
  dword sig;

  word num_dupe;
  word high_dupe;
} DUPEHEAD;

typedef struct
{
  dword crc;                /* CRC of to/from/subj */
  dword date;               /* Date in message header */

  dword msgid_hash;         /* Hash of MSGID address string */
  dword msgid_serial;       /* Hash of MSGID serial number */
  UMSGID umsgid;            /* UMSGID of the message in question */
} DUPEID;

void UndoLastDupe(void);
void DupeFlushBuffer(void);
int IsADupe(struct _cfgarea *ar, XMSG *msg, char *ctrl, dword uid);
dword FindUpdateMessage(HAREA sq, struct _cfgarea *ar, dword msgid_hash, dword msgid_serial, dword **ppmsgid_hash, dword **ppmsgid_serial);
void GetDidMsgid(DUPEID *pid, char *ctrl);

