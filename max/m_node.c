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
static char rcs_id[]="$Id: m_node.c,v 1.1.1.1 2002/10/01 17:52:46 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Display list of nodes and nets
*/

#define MAX_LANG_max_chng

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "max_msg.h"
#include "node.h"

/* Returns TRUE if caller should display a 'press enter' prompt after */

static int near Show_Nodelist(NETADDR *n)
{
  NFIND *nf;
  char nonstop=FALSE;
  int ret=1;

  display_line=display_col=1;
  
  if (prm.nlver==NLVER_7 || prm.nlver==NLVER_FD)
  {
    Puts(not_impl);
    return 1;
  }
  
  if ((nf=NodeFindOpen(n)) != NULL)
  {
    do
    {
      if (nf->found.flag & B_zone)
        Puts(pad_zone);
      else if (nf->found.flag & B_region)
        Puts(pad_region);
      else if (nf->found.flag & B_host)
        Puts(pad_net);
      else Puts(pad_none);

      Printf(node_listing, nf->found.node ? nf->found.node : nf->found.net,
             nf->found.name, nf->found.city);

      if (MoreYnBreak(&nonstop, CYAN))
      {
        ret=0;
        break;
      }
    }
    while (NodeFindNext(nf)==0);
    
    NodeFindClose(nf);
  }

  return ret;
}


int NetList(void)
{
  NETADDR n={ZONE_ALL, NET_ALL, 0, 0};
  
  return Show_Nodelist(&n);
}


int NodeList(word zone, word net)
{
  NETADDR n;
  
  n.zone=zone;
  n.net=net;
  n.node=NODE_ALL;
  n.point=POINT_ALL;
  
  return Show_Nodelist(&n);
}
