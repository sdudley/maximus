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

#ifndef __NODE_H_DEFINED
#define __NODE_H_DEFINED

typedef NETADDR *NETADDRP;

typedef struct
{
  NETADDR find;
  struct _maxnode found;
  
  struct _v56
  {
    int dfd, ifd;
    long pos;
    word zone, net, node, point;
    
    word recsize, idxcnt, idxcur;

    struct _ndi *idxbuf;
  } v56;
  
} NFIND;

NFIND * NodeFindOpen(NETADDR *find);
int NodeFindNext(NFIND *nf);
void NodeFindClose(NFIND *nf);
int V7FindNode(NETADDRP opus_addr, struct _newnode *node, char *net_info);
int V7FindName(char *name, NETADDRP faddr, struct _newnode *node, char *net_info);
int FDFindName(char *find, NETADDR *n, char *path);


#endif /* __NODE_H_DEFINED */
