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

/*# name=Run-time constants and static structures
*/

#ifndef __MAX_CON_H_DEFINED
#define __MAX_CON_H_DEFINED

#define NUM_COMPRESS_TYPE 12

#ifdef MAX_INITIALIZE

  char * cdecl compression_type[]=
  {
    " Unknown", /* 0 */
    " Stored ", /* 1 */
    " Stored ", /* 2 */
    "  RLE   ", /* 3 */
    "Squeezed", /* 4 */
    "crunched", /* 5 */
    "crunched", /* 6 */
    "crunched", /* 7 */
    "Crunched", /* 8 */
    "Squashed", /* 9 */
    " Crushed", /* 10 */
    " Distill"  /* 11 */
  };
#else
  extern char * cdecl compression_type[];
#endif


#define YES       cYes
#define NO        cNo
#define M_NONSTOP cNonStop

#endif /* __MAX_CON_H_DEFINED */
