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

#include <string.h>
#include <stdlib.h>
#include "tui.h"



/* Get the x/y dimensions of the listbox */

void GetListDims(HVOPT opt, int *pcxList, int *pcyList)
{
  char *duped, *p;

  if ((duped=strdup(opt->name)) != NULL)
  {
    if ((p=strtok(duped, ",")) != NULL)
    {
      *pcyList=atoi(p);

      if ((p=strtok(NULL, ",")) != NULL)
        *pcxList=atoi(p);
    }

    free(duped);
  }
}


/* Display the listbox */

MenuFunction(DlgLstShow)
{
  PUIHEAD puh=(PUIHEAD)opt->data;
  PUILIST pulThis=puh->top;
  VWIN *win=opt->parent->win;
  int x, y;
  byte attr;
  int cxList, cyList;

  GetListDims(opt, &cxList, &cyList);

  /* Clear out the block of space for our listbox */

  for (y=opt->cy; y < opt->cy + cyList; y++)
  {
    attr=col((byte)((puh->cur==pulThis &&
                      opt->parent->curopt==opt)
                    ? LISTBOX_SELECT_COL : LISTBOX_COL));

    for (x=opt->cx; x < opt->cx + cxList - 1; x++)
    {
      *WinOfs(win, y, x)=(attr << 8) | ' ';
      WinSetDirty(win, y, -1);
    }

    if (pulThis)
    {
      if (pulThis->txt)
      {
        char *p=strdup(pulThis->txt);

        if (p)
        {
          if (strlen(p) > cxList)
            p[cxList-3]='\0';

          WinPutstra(win, y, opt->cx+1, attr, p);

          free(p);
        }
      }

      pulThis=pulThis->next;
    }

    *WinOfs(win, y, opt->cx+cxList-1) = (col(LISTBOX_SCROLL_COL) << 8) |
                            (y==opt->cy ? (byte)'' :
                             y==opt->cy+cyList-1 ? (byte)'' :
                             (byte)'±');
  }

  *WinOfs(win, opt->cy + puh->uiCur*(cyList-2)/puh->uiNumItems + 1,
          opt->cx+cxList-1)=
      (col(LISTBOX_SCROLL_COL) << 8) | (byte)'þ';

  return 0; 
}


/* Activate an item in the listbox.  Execute the user-defined option. */

MenuFunction(DlgLstAct)
{
  PUIHEAD puh=(PUIHEAD)opt->data;

  if (puh->pfnSelect)
    (puh->pfnSelect)(puh->uiCur, puh->cur);

  return 0;
}

MenuFunction(DlgLstUp)
{
  PUIHEAD puh=(PUIHEAD)opt->data;

  if (puh->cur->prior)
  {
    puh->cur=puh->cur->prior;
    puh->uiCur--;

    /* If this would take us above the top of the listbox, scroll           *
     * it up by one.                                                        */

    if (puh->cur==puh->top->prior)
    {
      puh->top=puh->top->prior;
      puh->uiTop--;
    }

    (*opt->display)(opt);
  }

  return 0;
}



MenuFunction(DlgLstDown)
{
  PUIHEAD puh=(PUIHEAD)opt->data;
  PUILIST pui;
  int ofs;
  int cxList, cyList;


  if (puh->cur->next)
  {
    puh->cur = puh->cur->next;
    puh->uiCur++;

    GetListDims(opt, &cxList, &cyList);

    /* Find out how far we are from the top */

    for (ofs=0, pui=puh->cur;
         pui && pui != puh->top;
         pui=pui->prior, ofs++)
      ;

    /* If we went off the bottom of the listbox, scroll it up by one */

    if (ofs >= cyList)
    {
      puh->top=puh->top->next;
      puh->uiTop++;
    }

    (*opt->display)(opt);
  }

  return 0;
}



static void near DlgLstScrollUp(HVOPT opt, PUIHEAD puh)
{
  if (puh->top->prior)
  {
    puh->top = puh->top->prior;
    puh->cur = puh->top;
    puh->uiTop--;
    puh->uiCur = puh->uiTop;
    (*opt->display)(opt);
    tdelay(100);
  }
}


static void near DlgLstScrollDown(HVOPT opt, PUIHEAD puh, int cyList)
{
  if (puh->uiTop+cyList < puh->uiNumItems)
  {
    puh->top = puh->top->next;
    puh->uiTop++;

    puh->uiCur = puh->uiTop;
    puh->cur = puh->top;

    /* Position the current element to the bottom of the list */

    while (puh->uiCur+1 < puh->uiTop + cyList && puh->cur)
    {
      puh->cur = puh->cur->next;
      puh->uiCur++;
    }

    (*opt->display)(opt);
    tdelay(100);
  }
}


static void near DlgLstGotoRow(HVOPT opt, PUIHEAD puh, int press)
{
  PUILIST pui;
  extern word lastrow;

  /* Start scanning from the top of the listbox */

  puh->uiCur=puh->uiTop;

  for (pui=puh->top; pui && lastrow--; pui=pui->next)
    puh->uiCur++;

  if (pui && puh->cur != pui)
  {
    puh->cur=pui;
    (*opt->display)(opt);
  }

  /* If it was a button release, activate the option */

  if (!press)
  {
    static long ulLastClick=0L;
    static int uiLastSel;

    /* Allow up to 0.60 sec for a double-click */

    if (timeup(ulLastClick) || puh->uiCur != uiLastSel)
    {
      ulLastClick=timerset(60L);
      uiLastSel=puh->uiCur;
    }
    else (*opt->menufn)(opt);
  }
}




static void near DlgLstHandleScrollbar(HVOPT opt, PUIHEAD puh, int cyList, int press)
{
  extern word lastrow;

  if (press)
  {
    if (lastrow==0)
      DlgLstScrollUp(opt, puh);
    else if (lastrow==cyList-1)
      DlgLstScrollDown(opt, puh, cyList);
    else  /* go to somewhere in the middle */
    {
    }
  }
}



/* Selection of list item using mouse */

int DlgLstMouse(HVOPT opt, int press)
{
  PUIHEAD puh=(PUIHEAD)opt->data;
  int cxList, cyList;
  extern word lastcol, lastrow;

  lastcol -= opt->cx + opt->parent->win->s_col;
  lastrow -= opt->cy + opt->parent->win->s_row;

  GetListDims(opt, &cxList, &cyList);

  if (lastcol==cxList-1)
  {
    DlgLstHandleScrollbar(opt, puh, cyList, press);
    return 0;
  }

  if ((int)lastrow == -1)
    DlgLstScrollUp(opt, puh);
  else if (lastrow == cyList)
    DlgLstScrollDown(opt, puh, cyList);
  else DlgLstGotoRow(opt, puh, press);

  /* Set the current option to this one */

  opt->parent->lastopt=opt;
  opt->parent->curopt=opt;

  return 0;
}

MenuFunction(DlgLstMousePrs)
{
  return DlgLstMouse(opt, TRUE);
}

MenuFunction(DlgLstMouseRel)
{
  return DlgLstMouse(opt, FALSE);
}


/* If this listbox was selected (as opposed to another, non-listbox         *
 * option) by a mouse click, also activate the "mouse press" function       *
 * to highlight the line under the cursor.                                  */

MenuFunction(DlgLstMaybeSelect)
{
  extern word last_evt_was_mouse;

  if (last_evt_was_mouse)
    DlgLstMousePrs(opt);

  return 0;
}

/* Handle additional registration needs for the dialog button */

MenuFunction(DlgLstReg)
{
  PUIHEAD puh=(PUIHEAD)opt->data;
  HVOPT prior, next;
  int cxList, cyList;

  prior=_TuiGetPriorOpt(opt->parent, opt);
  next=_TuiGetNextOpt(opt->parent, opt);

  /* Make sure that the current option is selected on a mouse press */

  opt->ubefore=DlgLstMaybeSelect;

  /* Add custom keys for the up/down action */

  _TuiMenuAddKey(opt, VKEY_UP,    DlgLstUp, NULL, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DOWN,  DlgLstDown, NULL, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DOWN,  DlgLstDown, NULL, NULL, 0, 0);
  _TuiMenuAddKey(opt, 0xffff,     DlgLstMousePrs, opt, opt->menu, HOT_PRESS1, VKF_AFTER);
  _TuiMenuAddKey(opt, 0xffff,     DlgLstMouseRel, opt, opt->menu, HOT_RELEASE1, VKF_AFTER);

  puh->cur=puh->head;
  puh->top=puh->head;

  /* Hotspot region for this control */

  GetListDims(opt, &cxList, &cyList);

  /* Make the row before the first our "hot row" */

  opt->hot_row--;
  opt->hot_n_rows=cyList+2;
  opt->hot_n_cols=cxList;

  return 0;
}


void LstCreateList(PUIHEAD puh, int (*pfnSelect)(int idx, PUILIST pui))
{
  puh->head=puh->tail=NULL;
  puh->uiNumItems=puh->uiTop=puh->uiCur=0;
  puh->pfnSelect=pfnSelect;
}

int LstAddList(PUIHEAD puh, char *txt, void *app_inf)
{
  PUILIST pul;

  if ((pul=malloc(sizeof(*pul)))==NULL)
    return FALSE;

  /* Construct a new node */

  pul->txt=strdup(txt);
  pul->app_inf=app_inf;
  pul->prior=puh->tail;

  if (pul->prior)
    pul->prior->next=pul;

  pul->next=NULL;

  /* Append to list */

  puh->tail=pul;

  if (!puh->head)
    puh->head=puh->tail;

  puh->uiNumItems++;

  return TRUE;
}



void LstDestroyList(PUIHEAD puh)
{
  PUILIST pul, pulNext;

  for (pul=puh->head; pul; pulNext=pul->next, free(pul), pul=pulNext)
    if (pul->txt)
      free(pul->txt);
}


