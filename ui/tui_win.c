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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "prog.h"
#include "tui.h"


/* Display a status box and return a handle to it */

VWIN *WinStatus(char *pszText)
{
  VWIN *winStatus;

  winStatus=WinOpen(-1, -1, 3, strlen(pszText)+4, BORDER_DOUBLE,
                    CWHITE | _BLUE, CYELLOW | _BLUE,
                    WIN_CENTRE | WIN_NOCSYNC | WIN_NODRAW);

  WinShadow(winStatus, CGRAY);
  WinPutstr(winStatus, 0, 1, pszText);
  WinSync(winStatus, FALSE);
  return winStatus;
}


/* Print an error message on-screen */

void _stdc WinErr(char *format, ...)
{
  char out[120];
  va_list var_args;
  VWIN *win;

  va_start(var_args, format);
  vsprintf(out, format, var_args);
  va_end(var_args);

  if ((win=WinOpen(0, 0, 3, strlen(out)+4, BORDER_DOUBLE, 
                   col(CWHITE | _RED), col(CYELLOW | _RED),
                   WIN_CENTRE | WIN_NOCSYNC))==NULL)
    return;

  WinPutc(win, ' ');
  WinPuts(win, out);
  WinSync(win, TRUE);
  kgetch();
  MouseFlush();
  WinClose(win);
}



/* Returns the number of "lines" in a static text string */

static word near InfMaxLines(char *txt)
{
  word lines=0;
  
  while (*txt)
    if (*txt++=='\n')
      lines++;
    
  return lines+1;
}



/* Returns the maximum "width" of a static text string */

static word near InfMaxWid(char *txt)
{
  word wid=0, cwid=0;
  
  while (*txt)
    if (*txt++=='\n')
    {
      if (cwid > wid)
        wid=cwid;
      
      cwid=0;
    }
    else cwid++;
    
  return (cwid > wid ? cwid : wid);
}



static char *yn_text=NULL;

/* Display the question that the application requested */

MenuFunction(ShowYNText)
{
  WinGotoXY(opt->parent->win, 1, 0, FALSE);

  WinPuts(opt->parent->win, yn_text);
  return 0;
}


STD_DIALOG(dlg_getyn, -1, -1, BORDER_SINGLE, MENU_NHOT, 40, 16)
  DLG_BUF("1 ; ~Yes ",                    5, 3, &dlg_ok, ShowYNText)
  DLG_BUF("-1; ~No! ",                   10, 3, &dlg_ok, NULL)
END_MENU

/* Ask the user a yes/no question, using a pop-up dialog window */

word _stdc WinGetYN(char *fmt, ...)
{
  char *out;
  va_list var_args;
  HVMENU hAsk;
  HVOPT o;
  word maxwid;
  word maxlines;

  if ((out=malloc(strlen(fmt)+240))==NULL)
    return 0;

  va_start(var_args, fmt);
  vsprintf(out, fmt, var_args);
  va_end(var_args);

  maxwid=InfMaxWid(out)+3;
  maxlines=InfMaxLines(out);

  yn_text=out;

  dlg_getyn->start_x=dlg_getyn->start_y=-1;

  dlg_getyn->sizex=max(maxwid, 10);
  dlg_getyn->sizey=8+maxlines;

  o=dlg_getyn->opt;
  o->cx=dlg_getyn->sizex/2-11;
  o->cy=dlg_getyn->sizey-6;
  o++;
  o->cx=dlg_getyn->sizex/2+1;
  o->cy=dlg_getyn->sizey-6;

  hAsk=TuiRegisterMenu(dlg_getyn);
  TuiExecMenu(hAsk);
  TuiDestroyMenu(hAsk);

  free(out);

  return (dlg_ok==1);
}



static char *inf_text=NULL;

MenuFunction(ShowInfText)
{
  WinGotoXY(opt->parent->win, 1, 0, FALSE);

  WinPuts(opt->parent->win, inf_text);
  return 0;
}

STD_DIALOG(dlg_inf, -1, -1, BORDER_SINGLE, MENU_NHOT, 60, 16)
  DLG_BUF("1 ; ~OK ",                   24,10, &dlg_ok, ShowInfText)
END_MENU




/* Display an informational message to the user, using a pop dialog window */

word WinInfo(char *out)
{
  HVMENU hAsk;
  HVOPT o;
  word maxwid=InfMaxWid(out)+3;
  word maxlines=InfMaxLines(out);

  inf_text=out;

  dlg_inf->start_x=dlg_inf->start_y=-1;

  dlg_inf->sizex=max(maxwid, 10);
  dlg_inf->sizey=8+maxlines;

  o=dlg_inf->opt;

  o->cx=dlg_inf->sizex/2-5;
  o->cy=dlg_inf->sizey-6;

  hAsk=TuiRegisterMenu(dlg_inf);
  TuiExecMenu(hAsk);
  TuiDestroyMenu(hAsk);

  return 0;
}


void WinExit(int erl)
{
  if (wscrn)
    WinCls(wscrn, CGRAY);

  TuiClose();
  
  VidCls(CGRAY);
  VidGotoXY(1, 1, FALSE);
  VidClose();

  exit(erl);
}

void WinFill(VWIN *win, byte ch, byte attr)
{
  word row, col;
  
  for (row=0; row < win->s_height; row++)
    for (col=0; col < win->s_width; col++)
      WinPutch(win, row, col, ch, attr);
}



void WinAssertDir(char *path)
{
  if (!direxist(path))
    if (WinGetYN(" Create directory %s?", path))
      if (make_dir(path)==-1)
        WinErr("Error creating %s", path);
}

    
