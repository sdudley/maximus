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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alc.h"
#include "prog.h"
#include "arc_def.h"

void _fast Form_Archiver_Cmd(char *arcname,char *pktname,char *cmd,char *org)
{
  char *p;
  
  if (!arcname || !pktname || !cmd || !org)
  {
    *cmd='\0';
    return;
  }

  strcpy(cmd,org);
  
  for (p=cmd; (p=strchr(cmd,'%')) != NULL; )
  {
    switch (p[1])
    {
      case 'a':
        strocpy(p+strlen(arcname), p+2);
        memmove(p,arcname,strlen(arcname));
        break;
        
      case 'f':
        strocpy(p+strlen(pktname), p+2);
        memmove(p,pktname,strlen(pktname));
        break;
        
      case '%':
        p++;
        strocpy(p, p+1);
        break;
    }
  }
}
