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

/*# name=Maximus prototype declarations
*/

#ifndef __PROTO_DEF_
#define __PROTO_DEF_

#ifdef MAX_INCL_PROTO  /* If we need to include prototypes */

#ifdef __WATCOMC__
#ifndef __cplusplus
#pragma aux quit aborts;
#endif
#endif

#include "typedefs.h"
#include "msgapi.h"

#ifndef _BINK_H_DEFINED
#include "protod.h"
#endif

cpp_begin()
  #ifdef OS_2
    void IoPause(void);
    void IoResume(void);
    #define DdosInstalled()  FALSE
  #else
    #define IoPause()
    #define IoResume()
    int pascal far DdosInstalled(void);
  #endif

  #ifdef __TURBOC__
    int cdecl utime(char *name, struct utimbuf *times);
  #endif
cpp_end()

#endif /* MAX_INCL_PROTO */

#endif /* !_BINK_H_DEFINED */

