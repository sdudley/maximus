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

#pragma off(unreferenced)
static char rcs_id[]="$Id: thunk.c,v 1.2 2003/06/04 23:46:22 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdlib.h>
#include <string.h>
#include "alc.h"
#include "prog.h"
#include "msgapi.h"


/* Thunks for the MsgAPI allocation routines */

/*
#if defined(UNIX)
# define OS2FAR
# define MAPIENTRY
# define farmalloc(a) malloc(a)
# define farfree(a) free(a)
# define farrealloc(a) realloc(a)
#endif
*/

void OS2FAR * MAPIENTRY max_palloc(size_t size)
{ return ((void OS2FAR *)malloc(size)); }

void MAPIENTRY max_pfree(void OS2FAR *ptr)
{ free((void *)ptr); }

void OS2FAR * MAPIENTRY max_repalloc(void OS2FAR *ptr, size_t size)
{ return ((void OS2FAR *)realloc((void *)ptr, size)); }

void far * MAPIENTRY max_farpalloc(size_t size)
{ return ((void far *)farmalloc(size)); }

void MAPIENTRY max_farpfree(void far *ptr)
{ farfree(ptr); }

void far * MAPIENTRY max_farrepalloc(void far *ptr, size_t size)
{ return ((void far *)farrealloc(ptr, size)); }

