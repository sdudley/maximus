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

/*# name=Windowing routines (for Maximus and other)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "alc.h"
#include "dv.h"
#include "win.h"

static void near _WinDrawPickList(VPICK *vp);
static void near _WinFixItemTop(VPICK *vp);


VPICK * _fast WinCreatePickList(VWIN *win,int row,int col,int height,int col_item,int col_select,PLIST *picklist,int it_current)
{
  VPICK *vp;
  PLIST *ip;
  int max_len, x;
  
  if ((vp=malloc(sizeof(VPICK)))==NULL)
    return NULL;

  vp->it_current=0;
  
  /* Now find the max width, # of items, and current item */

  for (ip=picklist,max_len=0; ip && ip->name; ip++)
  {
    if (ip->value==it_current)
      vp->it_current=(int)(ip-picklist);

    x=strlen(ip->name)+2;

    if (x > max_len)
      max_len=x;
  }

  vp->win=win;
  vp->row=row;
  vp->col=col;
  vp->height=height;
  vp->width=max_len;
  
  vp->items=picklist;

  vp->it_last=(int)(ip-picklist);
  
  _WinFixItemTop(vp);

  vp->col_item=col_item;
  vp->col_select=col_select;
  
  _WinDrawPickList(vp);
  return vp;
}

int _fast WinPickAction(VPICK *vp,int action)
{
  int cur;

  cur=vp->it_current;

  switch (action)
  {
    case PICK_UP:
      if (vp->it_current)
        vp->it_current--;
      break;

    case PICK_DOWN:
      if (vp->it_current < vp->it_last-1)
        vp->it_current++;
      break;

    case PICK_SELECT:
    case PICK_ABORT:
      return FALSE;
  }

  if (cur != vp->it_current)
  {
    _WinFixItemTop(vp);
    _WinDrawPickList(vp);
  }

  return TRUE;
}



int _fast WinClosePickList(VPICK *vp)
{
  int cur;

  cur=vp->items[vp->it_current].value;
  free(vp);

  return cur;
}


static void near _WinDrawPickList(VPICK *vp)
{
  int len;
  int col;
  int r;
  char *buf;
  char *str;
  
  for (r=0; r < vp->height; r++)
  {
    if (vp->it_top+r == vp->it_current)
      col=vp->col_select;
    else col=vp->col_item;

    if (vp->items)
      str=vp->items[vp->it_top+r].name;
    else str="";

    len=strlen(str);
    
    if ((buf=malloc(min(len,vp->width)+10))==NULL)
      continue;
    
    buf[0]=' ';
    strcpy(buf+1,str);
    
    while (strlen(buf) < (size_t)vp->width)
      strcat(buf," ");
    
    buf[vp->width]='\0';
    
    WinPutstra(vp->win,vp->row+r,vp->col,col,buf);
    
    free(buf);
  }

  WinSync(vp->win,FALSE);
}


static void near _WinFixItemTop(VPICK *vp)
{
  int halfh=vp->height >> 1;

  if (vp->it_current-halfh < 0)
    vp->it_top=0;
  else if (vp->it_current+halfh >= vp->it_last)
    vp->it_top=vp->it_last-vp->height;
  else vp->it_top=vp->it_current-halfh;
}

