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

#ifndef __ARCDEF_H_DEFINED
#define __ARCDEF_H_DEFINED

#ifndef _MAX_H_DEFINED
  #include "max.h"
#endif



#define MAX_ARI     32

/* SEEN-BY node.  Also used for linked lists of nodes in other places */

struct _sblist
{
  struct _sblist *next;

  word zone;
  word net;
  word node;
  word point;
};


struct _arcinfo
{
  struct _arcinfo *next;
  
  char *arcname;
  char *extension;
  char *add;
  char *extract;
  char *view;

  long id_ofs;
  char *id;
  
  struct _sblist *nodes;
};


struct _arcinfo * _fast Parse_Arc_Control_File(char *afname);
void _fast Form_Archiver_Cmd(char *arcname,char *pktname,char *cmd,char *org);
#endif /* __ARCDEF_H_DEFINED */

