/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/***************************************************************************/
/*                                                                         */
/*            DW-text: Double linked list manager                          */
/*                     Header file                                         */
/*                                                                         */
/*            Created: 20/Aug/90                                           */
/*            Updated: 20/Apr/97                                           */
/*                                                                         */
/*            Copyright (C) 1990-1997 JV DIALOGUE                          */
/*            Written by Pete I. Kvitek                                    */
/*                                                                         */
/***************************************************************************/

#ifndef DW_LST_DEFS
#define DW_LST_DEFS

#include "dw-def.h"

/***************************************************************************
* List manager types and constants
*/

  // List element structure

  typedef struct _LE {          /* le */
    struct _LE FAR * plePrev;   // Pointer to the prev element in list
    struct _LE FAR * pleNext;   // Pointer to the next element in list
  } LE, FAR * PLE;
  typedef PLE FAR * PPLE;

  // List manager constants

#define LST_ERROR      ((USHORT)-1u)
#define LST_END        ((USHORT)-1u)

/***************************************************************************
* List manager function prototypes
*/

  PLE    APIENTRY LstCreateElement(USHORT cb);
  VOID   APIENTRY LstDestroyElement(PLE ple);
  USHORT APIENTRY LstLinkElement(PPLE pple1st, PLE ple, USHORT ile);
  PLE    APIENTRY LstUnlinkElement(PPLE pple1st, USHORT ile);
  PLE    APIENTRY LstElementFromIndex(PLE ple1st, USHORT ile);
  USHORT APIENTRY LstIndexFromElement(PLE ple1st, PLE ple);
  USHORT APIENTRY LstQueryElementCount(PLE ple1st);

#endif /* DW_LST_DEFS */

/***************************************************************************
* End of DW-LST.H                                                          *
****************************************************************************/
