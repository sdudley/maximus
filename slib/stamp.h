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

#ifndef __STAMP_H_DEFINED
#define __STAMP_H_DEFINED

#include "typedefs.h"

struct _stamp   /* DOS-style datestamp */
{
  struct
  {
    /* IBM Cset/2 is allergic to "unsigned short" when declaring bitfields! */

#ifdef __IBMC__
    unsigned int da : 5;
    unsigned int mo : 4;
    unsigned int yr : 7;
#else
/*lint -e46 */
    word da : 5;
    word mo : 4;
    word yr : 7;
/*lint -restore */
#endif
  } date;

  struct
  {
#ifdef __IBMC__
    unsigned int ss : 5;
    unsigned int mm : 6;
    unsigned int hh : 5;
#else
/*lint -e46 */
    word ss : 5;
    word mm : 6;
    word hh : 5;
/*lint -restore */
#endif
  } time;
};


struct _dos_st
{
  word date;
  word time;
};

/* Union so we can access stamp as "int" or by individual components */

union stamp_combo   
{
  dword ldate;
  struct _stamp msg_st;
  struct _dos_st dos_st;
};

typedef union stamp_combo SCOMBO;

#endif /* __STAMP_H_DEFINED */

