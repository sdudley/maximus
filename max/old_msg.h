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

#ifndef __OLD_MSG_H_DEFINED
#define __OLD_MSG_H_DEFINED

/*--------------------------------------------------------------------------*/
/* Message header                                                           */
/*--------------------------------------------------------------------------*/
struct _omsg
   {
      byte from[36];
      byte to[36];
      byte subj[72];
      byte date[20];       /* Obsolete/unused ASCII date information        */
/**/  word times;          /* FIDO<tm>: Number of times read                */
      sword dest;          /* Destination node                              */
      sword orig;          /* Origination node number                       */
/**/  word cost;           /* Unit cost charged to send the message         */

      sword orig_net;      /* Origination network number                    */
      sword dest_net;      /* Destination network number                    */

                           /* A TIMESTAMP is a 32-bit integer in the Unix   */
                           /* flavor (ie. the number of seconds since       */
                           /* January 1, 1970).  Timestamps in messages are */
                           /* always Greenwich Mean Time, never local time. */

      struct _stamp date_written;   /* When user wrote the msg              */
      struct _stamp date_arrived;   /* When msg arrived on-line             */

      word reply;          /* Current msg is a reply to this msg number     */
      word attr;           /* Attribute (behavior) of the message           */
      word up;             /* Next message in the thread                    */
   } __attribute__((packed, aligned(2)));

#endif

