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
#define MAX_NOPROTO

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"

static char *colon=":";
static char *slash="/";

void _fast ParseNNN(char *netnode, NETADDR *pn, word all)
{
  ParseNN(netnode, &pn->zone, &pn->net, &pn->node, &pn->point, all);
}

void _fast Parse_NetNode(char *netnode,word *zone,word *net,word *node,word *point)
{
  ParseNN(netnode,zone,net,node,point,FALSE);
}

void _fast ParseNN(char *netnode,word *zone,word *net,word *node,word *point,word all)
{
  char *p;

  p=netnode;
  
  if (all && point)
    *point=POINT_ALL;

  if (all && toupper(*netnode)=='W')  /* World */
  {
    if (zone)
      *zone=ZONE_ALL;

    if (net)
      *net=NET_ALL;

    if (node)
      *node=NODE_ALL;

    return;
  }

  /* If we have a zone (and the caller wants the zone to be passed back).. */

  if (strchr(netnode,':'))
  {
    if (zone)
    {
      if (all && toupper(*p)=='A')  /* All */
        *zone=ZONE_ALL;
      else *zone=atoi(p);
    }

    p=firstchar(p,colon,2);
  }

  /* If we have a net number... */

  if (p && *p)
  {
    if (strchr(netnode,'/'))
    {
      if (net)
      {
        if (all && toupper(*p)=='A')  /* All */
          *net=NET_ALL;
        else *net=atoi(p);
      }

      p=firstchar(p,slash,2);
    }
    else if (all && toupper(*p)=='A')
    {
      /* If it's in the form "1:All" or "All" */

      if (strchr(netnode,':')==NULL && zone)
        *zone=ZONE_ALL;

      *net=NET_ALL;
      *node=NODE_ALL;
      p += 3;
    }
  }

  /* If we got a node number... */

  if (p && *p && node && *netnode != '.')
  {
    if (all && toupper(*p)=='A')  /* All */
    {
      *node=NODE_ALL;

      /* 1:249/All implies 1:249/All.All too... */

      if (point && all)
        *point=POINT_ALL;
    }
    else *node=atoi(p);
  }

  if (p)
    while (*p && isdigit(*p))
      p++;

  /* And finally check for a point number... */

  if (p && *p=='.')
  {
    p++;

    if (point)
    {
      if (!p && *netnode=='.')
        p=netnode+1;

      if (p && *p)
      {
        *point=atoi(p);

        if (all && toupper(*p)=='A')  /* All */
          *point=POINT_ALL;
      }
      else *point=0;
    }
  }
}

