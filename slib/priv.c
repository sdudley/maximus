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

/*# name=Priv-level strings for Maximus
*/

#include "prog.h"
#include "max.h"

#if defined( THIS_IS_NOW_DEFUNCT )

struct __priv _stdc _privs[]={{"Twit",     TWIT},
                              {"Disgrace", DISGRACE},
                              {"Limited",  LIMITED},
                              {"Normal",   NORMAL},
                              {"Worthy",   WORTHY},
                              {"Privil",   PRIVIL},
                              {"Favored",  FAVORED},
                              {"Extra",    EXTRA},
                              {"Clerk",    CLERK},
                              {"AsstSysOp",ASSTSYSOP},
                              {"SysOp",    SYSOP},
                              {"Hidden",   HIDDEN},
                              {NULL,       -999}};

char * _fast Priv_Level(int priv)
{
  int x;

  for (x=0;x < _PRIVS_NUM;x++)
    if (priv==_privs[x].priv)
      return _privs[x].name;

  return "";
}

#endif

