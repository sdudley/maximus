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

/* $Id: trackcom.h,v 1.1 2002/10/01 17:49:31 sdudley Exp $ */

// Common definitions for track.h and trackc.h

#ifndef __TRACKCOM_H_DEFINED
#define __TRACKCOM_H_DEFINED

#ifndef __MSGAPI_H_DEFINED
  typedef unsigned long UMSGID;
#endif

#ifndef __MAX_U_H_DEFINED
  #define MAX_ALEN 64
#endif

#include "stamp.h"

// Types used for "status of tracked message"

#define TS_NEW        0
#define TS_OPEN       1
#define TS_WORKING    2
#define TS_CLOSED     3

typedef unsigned short TRK_STATUS;

// Types used for "priority of tracked message"

#define TP_NOTIFY     0
#define TP_LOW        1
#define TP_NORMAL     2
#define TP_URGENT     3
#define TP_CRIT       4

typedef unsigned short TRK_PRIORITY;

// Types used for storing ordinal area/owner number information

typedef char TRK_OWNER[5];

// Maximum length of message tracking ID

#define MAX_TRACK_LEN 20


// Max length of an owner's name (ASCII format, not the alias)

#define TRK_ASCII_OWNER_SIZE 36

// Type used to hold location of a message in the physical message
// areas.

typedef struct
{
  char szArea[MAX_ALEN];        /* Link to the message area cross-ref base */
  UMSGID uid;                   /* Message number of this msg */
} TRK_LOCATION;


// Key used in the main message tracking database
//
// !!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!
//  You must also change afDbFields in
//  track.cc if you change this declaration!
// !!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!

typedef struct
{
  // Format of a tracking number:

  // date  time  area
  // ++++++------++++
  // 9307011412420000

  char szTrackID[MAX_TRACK_LEN];        // Tracking number for this message
  TRK_OWNER to;                         // Owner of this message
  TRK_LOCATION tl;                      // Location of msg (area, msg#)

  ////////////////////////////////////////////////////////////////

  TRK_STATUS ts;                        // Status of the tracked message
  TRK_PRIORITY tp;                      // Priority of message
  SCOMBO scDateWritten;                 // Date the msg was written

  // Chained data records contain message trail auditing in an ASCII
  // format.

} TRK_MSG_NDX;


// Key used in the "message owner" database

typedef struct
{
  TRK_OWNER to;                         // This owner key
  ////////////////////////////////////////////////////////////////
  char szOwner[TRK_ASCII_OWNER_SIZE];   // ASCII user who is owner of msg
} TRK_OWNER_NDX;


// Key used in the "area owner" database

typedef struct
{
  char szArea[MAX_ALEN];                // Name of this msg area
  /////////////////////////////////////////////////////////////////////
  TRK_OWNER to;                         // Default owner of this area
} TRK_AREA_NDX;


#endif // __TRACKCOM_H_DEFINED

