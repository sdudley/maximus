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

#ifndef __M_FOR_H_DEFINED
#define __M_FOR_H_DEFINED

/* This file is #included by M_FOR.C and M_SAVE.C... */

struct _fwdp
{
  HAREA tosq;
  
  HMSG fh;

  MAH toar;
  XMSG fmsg;
  XMSG tmsg;
  
  byte toname[MAX_ALEN];
  
  long msgnum;
  
  char bomb;
  char kill;
  
  UMSGID original;
};



#endif /* __M_FOR_H_DEFINED*/

