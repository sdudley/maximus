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
static char rcs_id[]="$Id: max_rmen.c,v 1.1.1.1 2002/10/01 17:52:02 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Routines to read *.MNU files (Overlaid)
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <stdlib.h>
#include "prog.h"
#include "mm.h"
#include "max_menu.h"
#include "max_msg.h"

/* Count the number of menu options */

static void near CountMenuLines(PAMENU pam, char *mname)
{
  struct _opt *popt, *eopt;
  int num_opt=0;

  /* If we have a canned menu to display, use its length */

  if (*MNU(*pam, m.dspfile) && DoDspFile(menuhelp, pam->m.flag))
    menu_lines=pam->m.menu_length;
  else
  {
    /* Else count the number of valid menu options accordingly */

    for (popt=pam->opt, eopt=popt + pam->m.num_options; popt < eopt; popt++)
      if (popt->type && OptionOkay(pam, popt, TRUE, NULL, &mah, &fah, mname))
        num_opt++;

    if (!pam->m.opt_width)
      pam->m.opt_width=DEFAULT_OPT_WIDTH;

    if (usr.help==NOVICE)
    {
  /*  menu_lines = 3 + (3+num_opt)/4; */
      int opts_per_line = (TermWidth()+1) / pam->m.opt_width;
      if (opts_per_line <= 0)
        opts_per_line=1;
      menu_lines = 3 + (num_opt / opts_per_line)  + !!(num_opt % opts_per_line);
    }
    else menu_lines=2;
  }
}

sword Read_Menu(struct _amenu *menu, char *mname)
{
  long where, end;

  int menufile;
  word menu_items;
  size_t size, hlen;

  char mpath[PATHLEN];

  menu_items=1;

  sprintf(mpath, "%s%s.mnu", menupath, mname);

  if ((menufile=shopen(mpath, O_RDONLY | O_BINARY))==-1)
    return -2;

  if (read(menufile, (char *)&menu->m, sizeof(menu->m)) != sizeof(menu->m))
  {
    logit(cantread, mpath);
    close(menufile);
    quit(2);
  }

  size=sizeof(struct _opt) * menu->m.num_options;

  if ((menu->opt=malloc(size))==NULL)
  {
    logit(mem_none);
    close(menufile);
    return -1;
  }

  if (read(menufile, (char *)menu->opt, size) != (signed)size)
  {
    logit(cantread, mpath);
    close(menufile);
    return -1;
  }
  

  /* Now read in the rest of the file as the variable-length heap */
  
  where=tell(menufile);
  
  lseek(menufile, 0L, SEEK_END);
  end=tell(menufile);

  hlen=(size_t)(end-where);
  
  if ((menu->menuheap=malloc(hlen))==NULL)
  {
    logit(mem_none);
    close(menufile);
    return -1;
  }
  
  lseek(menufile, where, SEEK_SET);

  if (read(menufile, menu->menuheap, hlen) != (signed)hlen)
  {
    logit(cantread, mpath);
    close(menufile);
    return -1;
  }

  close(menufile);

  CountMenuLines(menu, mname);

  return 0;
}





void Initialize_Menu(struct _amenu *menu)
{
  memset(menu,'\0',sizeof(struct _amenu));
}





void Free_Menu(struct _amenu *menu)
{
  if (menu->menuheap)
    free(menu->menuheap);

  if (menu->opt)
    free(menu->opt);

  Initialize_Menu(menu);
}






#if 0
int Header_None(int entry, int silent)
{
  NW(entry);
  NW(silent);

  WhiteN();

  restart_system=FALSE;
  return TRUE;
}
#endif

