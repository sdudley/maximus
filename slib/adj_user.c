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

#define NOVARS
#define NOVER

#include <string.h>
#include "prog.h"
#include "max.h"


/* Stuff the binary info back into right place for Max 1.02 compatibility */

void _fast Adjust_User_Record(struct _usr *user)
{
  strncpy(user->rsvd45, user->msg, 10);
  user->rsvd45[9]=0;

  strncpy(user->rsvd45, user->files, 10);
  user->rsvd45[19]=0;

  user->struct_len=sizeof(struct _usr)/20;
}

