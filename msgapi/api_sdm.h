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

/* $Id: api_sdm.h,v 1.1.1.1 2002/10/01 17:54:21 sdudley Exp $ */

#ifndef __API_SDM_H_DEFINED
#define __API_SFM_H_DEFINED


#define MAX_SDM_CLEN  512   /* Maximum number of bytes which can be used    *
                             * for kludge lines at top of *.MSG-type        *
                             * messages.                                    */


struct _msgh
{
  HAREA ha;
  dword id;      /* Must always equal MSGH_ID */

  dword bytes_written;
  dword cur_pos;

  /* For *.MSG only! */

  sdword clen;
  byte *ctrl;
  dword msg_len;
  dword msgtxt_start;
  word zplen;
  int fd;
};


/*************************************************************************/
/* This junk is unique to *.MSG!       NO APPLICATIONS SHOULD USE THESE! */
/*************************************************************************/

struct _sdmdata
{
  byte base[80];
  
  unsigned *msgnum;   /* has to be of type 'int' for qksort() fn */
  unsigned msgnum_len;
    
  dword hwm;
  word hwm_chgd;
  
  unsigned msgs_open;
};




#endif /* __API_SDM_H_DEFINED */


