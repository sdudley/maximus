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
static char rcs_id[]="$Id: max_runo.c,v 1.2 2003/06/04 23:46:22 wesgarland Exp $";
#pragma on(unreferenced)

#define MAX_LANG_max_main

#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "mm.h"
#include "max_menu.h"
#include "exec.h"

static int near Exec_Misc(PAMENU pam, struct _opt *thisopt, char *arg,
                          char **result, unsigned *puiFlag);

static option no_ed_ops[]=
{
  display_file, xtern_dos, xtern_run, xtern_erlvl, xtern_chain, 
  xtern_concur, statistics, o_yell, o_cls, userlist, 
  o_version, edit_save, edit_abort, edit_list, edit_edit, edit_insert, 
  edit_delete, edit_continue, edit_to, edit_from, edit_subj, 
  edit_handling, edit_quote, read_diskfile, nothing
};
  
 


/* Run_Option() - The main calling routine which invokes ALL of the menu
 * options, no matter what or where they are.
 */

char * RunOption(struct _amenu *menu, struct _opt *thisopt, int ch, XMSG *msg, unsigned *puiFlag, char *menuname)
{
  char *kp, *arg, *result;
  option *op;
  int type;

  *puiFlag=0;

  /* Initialize some char*'s as short-forms for this option */
  
  kp=menu->menuheap + thisopt->keypoke;
  arg=menu->menuheap + thisopt->arg;
    
  if (msg)  /* If msg is non-NULL (ie. we're in an editor), then we        */
  {         /* can only perform the following options without screwing     */
            /* everything up.  If the SysOp adds an incorrect command,     */
            /* then the user will get a `You can't get there from here'    */
            /* message.                                                    */

    for (op=no_ed_ops; *op != nothing; op++)
      if (thisopt->type==*op)
        break;

    if (*op==nothing)
    {
      Puts(you_dont_have_access);
      return NULL;
    }
  }

  
  /* If someone needs to be keypoked, do so */
  
  if (thisopt->keypoke)
  {
    char *pp=malloc(PATHLEN);

    if (pp)
    {
      if (strchr(kp, '%'))
        Parse_Outside_Cmd(Strip_Underscore(kp), pp);
      else strcpy(pp, Strip_Underscore(kp));

      strcat(pp, linebuf);
      strcpy(linebuf, pp);

      free(pp);
    }
  }

  type=thisopt->type;
  
  /* Now execute the appropriate handler function */

  if (type >= MISC_BLOCK && type < XTERN_BLOCK)
    Exec_Misc(menu, thisopt, arg, &result, puiFlag);
  else if (type >= XTERN_BLOCK && type < MAIN_BLOCK)
    Exec_Xtern(type, thisopt, arg, &result, menuname);
  else if (type >= MAIN_BLOCK && type < MSG_BLOCK)
    Exec_Main(type, &result);
  else if (type >= MSG_BLOCK && type < FILE_BLOCK)
    Exec_Msg(type, &result, ch, arg, menuname);
  else if (type >= FILE_BLOCK && type < CHANGE_BLOCK)
    Exec_File(type, &result, menuname);
  else if (type >= CHANGE_BLOCK && type < EDIT_BLOCK)
    Exec_Change(type, &result);
  else if (type >= EDIT_BLOCK && type < CHAT_BLOCK)
    Exec_Edit(type, &result, msg, puiFlag);
  else if (type >= CHAT_BLOCK && type < END_BLOCK)
    Exec_Chat(type, &result);
  else 
    logit(bad_menu_opt, type);

  return result;
}


static int near Exec_Misc(PAMENU pam, struct _opt *thisopt, char *arg,
                          char **result, unsigned *puiFlag)
{
  NW(pam);
  
  *result=NULL;

  switch (thisopt->type)
  {
    case display_menu:
      if (! (thisopt->flag & OFLAG_NOCLS))
      {
        if (hasRIP())
          Puts(end_rip);
        Puts(CLS);
      }

      *puiFlag |= RO_NEWMENU;
      *result=arg;
      return 0;

    /* Nest a menu */

    case link_menu:
    {
      static int num_link=0;

      if (++num_link >= 8)
      {
        logit(log_max_nest, arg);
        return 0;
      }

      if (! (thisopt->flag & OFLAG_NOCLS))
        Puts(CLS);

      Display_Options(arg, NULL);
      num_link--;
      return 0;
    }

    /* Return from a nested menu */

    case o_return:
      *puiFlag |= RO_QUIT;
      *result="";
      return 0;

    case display_file:
      return (Display_File(0, NULL, arg));

#ifdef MEX
    case mex: /* Execute a MEX file */
      return Mex(arg);
#endif

    case o_press_enter:
      Puts(WHITE);
      Press_ENTER();
      break;

    case clear_stacked:
      *linebuf='\0';
      break;

    case key_poke:
      {
        char *pp=malloc(PATHLEN);

        if (!pp)
          break;

        if (strchr(arg,'%'))
          Parse_Outside_Cmd(Strip_Underscore(arg), pp);
        else strcpy(pp, Strip_Underscore(arg));

        strcat(pp, linebuf);
        strcpy(linebuf, pp);

        free(pp);
      }
      break;

    case o_menupath:
      strcpy(menupath,arg);
      Add_Trailing(menupath, PATH_DELIM);
      break;

    case o_cls:
      Puts(CLS);
      break;

    default:
      logit(bad_menu_opt, thisopt->type);
  }
  
  return 0;
}
