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

#define SFLAG_ECHO      0x0001
#define SFLAG_CONF      0x0002
#define SFLAG_NET       0x0004
#define SFLAG_LOCAL     0x0008
#define SFLAG_ALL       (SFLAG_ECHO | SFLAG_CONF | SFLAG_NET | SFLAG_LOCAL)
#define SFLAG_FORCE     0x0010
#define SFLAG_NODEL     0x0020
#define SFLAG_QUIET     0x0040

#define SFLAG_DEFAULT   (SFLAG_ALL | 0)

#define MAX_SCANAREA    512

typedef struct
{
  int num_msg;
  int high_msg;
} SBHDR;

typedef struct
{
  word msgnum;      /* This message number */
  word attr;        /* Message attribute */
  char to[36];      /* Message "To:" field */
} SBREC;

