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

#error this file no longer used.


/*# name=Nodelist searching and retrieval functions
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "mm.h"

static int near NodeExist(NETADDR *d)
{
  struct _ndi ndi[NUM_NDI];

  int idxfile;
  long offset;
  word current_zone;
  word x, y;

  char temp[PATHLEN];

  current_zone=prm.address[0].zone;

  if (! *PRM(net_info))
    return -1;

  sprintf(temp,idxnode,PRM(net_info));

  if ((idxfile=shopen(temp,O_RDONLY | O_BINARY))==-1)
  {
/*    cant_open(temp);*/
    return -1;
  }

  for (offset=0L;
       (y=read(idxfile, (char *)ndi, sizeof(*ndi) * NUM_NDI)) > sizeof(*ndi);
       offset += (long)y)
  {
    y /= sizeof(struct _ndi);

    for (x=0; x < y; x++)
    {
      if (ndi[x].node==(word)-2)
        current_zone=ndi[x].net;

      /* Convert region/zone addresses */

      if ((sword)ndi[x].node < 0)
        ndi[x].node=0;

      if ((current_zone==d->zone || d->zone==0 || prm.nlver==5) &&
          ndi[x].net==d->net &&
          ndi[x].node==d->node)
      {
        close(idxfile);
        return (int)offset+x;
      }
    }
  }

  close(idxfile);
  return -1;
}



int ReadNode(NETADDR *d,void *nodeptr)
{
  char temp[PATHLEN];

  int nlfile;

  word offset;
  word x;

  unsigned long n6size=0L;

  if ((offset=NodeExist(d))==(unsigned int)-1)
    return FALSE;
  else
  {
    if (prm.nlver==6)
    {
      sprintf(temp, idxnode, PRM(net_info));

      x=(word)(fsize(temp)/(long)sizeof(struct _ndi));

      sprintf(temp, datnode, PRM(net_info));

      n6size=fsize(temp);
      n6size=n6size/(dword)x;
    }
    else sprintf(temp, sysnode, PRM(net_info));

    if ((nlfile=shopen(temp, O_RDONLY | O_BINARY))==-1)
    {
/*    cant_open(temp);*/
      return FALSE;
    }

    lseek(nlfile, ((long)offset)*(prm.nlver==6 ? (long)n6size :
          (long)sizeof(struct _node)), SEEK_SET);

    if (read(nlfile, nodeptr, prm.nlver==6 ? sizeof(struct _newnode) :
         sizeof(struct _node)) <= 0)
    {
      close(nlfile);
      logit(cantread, temp);
      return FALSE;
    }

    close(nlfile);

    return TRUE;
  }
}

