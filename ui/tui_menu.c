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

/*#define DEBUG_PD*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "tui.h"

#ifdef OS_2
#define INCL_DOS
#include <os2.h>
#endif

HVMENU _TuiRegisterMenu1(HVMENU vmenu, HVKEY keylist);
static word def_menu_attr;

/* Copy for holding current 'nextmen', to make it accessible by extern f()s */
HVMENU nextmen;


/* Flags for the last mouse event */

word lastbut=0, lastcol=-1, lastrow=-1;

/* Was last event caused by a mouse action? */

word last_evt_was_mouse=0;


/* Execute the definable 'after' functions for this menu option */

static void near _TuiOptAfter(HVOPT opt)
{
  word save_appdata;

  if (!opt)
    return;

  /* Save application-defined data before calling fafter */

  save_appdata=opt->appdata;

  /* Implicit 'after' function */

  if (opt->fafter)
    (*opt->fafter)(opt);

  opt->appdata=save_appdata;

  /* User-defined 'after' */
  
  if (opt->uafter)
    (*opt->uafter)(opt);

  if (opt->uafter2)
    (*opt->uafter2)(opt);
}


/* Execute the 'before' functions for this option */

static void near _TuiOptBefore(HVOPT opt)
{
  if (!opt)
    return;

  /* User-defined 'before' option */
  
  if (opt->ubefore)
    (*opt->ubefore)(opt);

  /* Implicit 'before' option */

  if (opt->fbefore)
    (*opt->fbefore)(opt);
}


/* Take a menu option (including tilde) and convert it to a plain string,   *
 * no less than 'min' chars, and figuring out the 'hot character' and       *
 * its offset.                                                              */

char *_TuiMakeOptName(HVOPT opt, char *sel_char, word *sel_col, word minch)
{
  static char optname[MAX_OPTNAME_LEN];
  char *p, *o, *end;

  if (sel_char)
    *sel_char='\0';

  if (sel_col)
    *sel_col=0;

  for (p=opt->name, o=optname; *p; p++)
    if (*p=='~')
    {
      if (sel_char)
        *sel_char=p[1];
      
      if (sel_col)
        *sel_col=(word)(p-opt->name);
    }
    else
    {
      *o++=*p;
    }
  
  /* Now pad the string to make sure that it's at least 'minch' chars */

  end=optname+minch;
  
  while (o < end)
    *o++=' ';
  
  *o='\0';

  return optname;
}


sword _TuiDrawOption(HVOPT opt, char col_norm, char col_hot, word draw_hot)
{
  char *optname, *bigopt;
  char selch;
  word selcol;

  /* If this option has a custom display routine, pass it off to that       *
   * instead of our default one.                                            */

  if (opt->display)
  {
    (*opt->display)(opt);
    return 0;
  }

  
  /* Find out the name of the option, as we're to display it to the user */

  optname=_TuiMakeOptName(opt, &selch, &selcol,
                          (opt->parent->type & MENU_VERT)
                            ? opt->parent->wid
                            : 0);

  if ((bigopt=malloc(strlen(optname)+5))==NULL)
    return -1;

  bigopt[0]=' ';
  strcpy(bigopt+1, optname);
  strcat(bigopt, " ");
  
  WinPutstra(opt->parent->win, opt->cy, opt->cx, col(col_norm), bigopt);
  
  if (draw_hot && selch)
    WinPutch(opt->parent->win, opt->cy, opt->cx+selcol+1, selch, col(col_hot));

  free(bigopt);

  return 0;
}


/* Function to execute when a particular menu item is highlighted on        *
 * the menu.                                                                */

MenuFunction(_TuiMenuOptHighlight)
{
  return (_TuiDrawOption(opt, opt->parent->col_sel,
                         opt->parent->col_selhot, TRUE));
}


/* Function to execute when a particular menu item is unhighlighted on      *
 * the menu.                                                                */

MenuFunction(_TuiMenuOptNormal)
{
  return (_TuiDrawOption(opt, opt->parent->col_item,
                         opt->parent->col_hot, TRUE));
}


/* Add a key to the list of keys which are accepted from the specified      *
 * menu option.                                                             */

sword _TuiMenuAddKey(HVOPT opt, word kb, MenuFunction((*menufn)), HVOPT newopt, HVMENU newmenu, word hotspot, word flag)
{
  HVKEY key;

  /* Allocate memory for the key structure */
  
  if ((key=malloc(sizeof(*key)))==NULL)
    return -1;

  memset(key, '\0', sizeof(*key));
  
  /* Now copy in the appropriate parameters */

  key->key=kb;
  key->menufn=menufn;
  key->newopt=newopt;
  key->newmenu=newmenu;
  key->flag=flag;
  
  /* Create a mouse hotspot for the current option */
  
  if (hotspot)
    key->hot_flag=hotspot;
  
  /* Append this item to the linked list */

  key->next=opt->keys;
  opt->keys=key;
  return 0;
}





/* Return the option prior to this one, not counting info strings */

HVOPT _TuiGetPriorOpt(HVMENU menu, HVOPT opt)
{
  HVOPT prior;
  NW(menu);

  prior=(opt >= menu->opt+1 ? opt-1 : menu->opt+menu->num_opt-1);

  do
  {
    while (prior->regist==DlgInfReg && prior > menu->opt)
      prior--;

    if (prior==menu->opt && prior->regist==DlgInfReg)
      prior=menu->opt+menu->num_opt-1;
  }
  while (prior->regist==DlgInfReg);

  /* Now make sure that we start at the beginning of a rad button group */

  while (prior->regist==DlgRadReg && prior-1 >= menu->opt &&
         prior[-1].regist==DlgRadReg)
    prior--;

  return prior;
}


/* Return the option after to this one, not counting info strings */

HVOPT _TuiGetNextOpt(HVMENU menu, HVOPT opt)
{
  HVOPT next;
  NW(menu);

  next=(opt < menu->opt+menu->num_opt-1 ? opt+1 : menu->opt);

  do
  {
    while (next->regist==DlgInfReg && next < menu->opt+menu->num_opt)
      next++;

    if (next >= menu->opt+menu->num_opt)
      next=menu->opt;
  }
  while (next->regist==DlgInfReg);

  return next;
}


/* Add the selection function (either <enter> or a mouse click) to the      *
 * key list for this menu option.                                           */

sword _TuiAddEnterFunc(HVMENU vmenu, HVOPT opt)
{
  sword ret;

  NW(vmenu);

  /* No keys to be added for information strings */

  if (opt->regist==DlgInfReg)
    return 0;
  
  ret=_TuiMenuAddKey(opt, K_ENTER, opt->menufn,
                     ((vmenu->type & MENU_DIALOG) &&
                      opt->regist != DlgButReg && opt->regist != DlgLstReg)
                       ? _TuiGetNextOpt(vmenu, opt)
                       : NULL,
                     opt->menu, HOT_NONE, 0);


  /* If there's a layered menu underneath this one, add in a new key        *
   * record which brings that menu up.                                      */

  if (ret==0 && opt->menu && (vmenu->type & MENU_HORIZ))
    ret=_TuiMenuAddKey(opt, VKEY_DOWN, opt->menufn, NULL, opt->menu, 
                       HOT_NONE, 0);

  return (ret);
}




/* Configure the actions for each menu type.  This is based on the type     *
 * of menu; if it's a vertical (bar) menu, then simply walk through the     *
 * list and set the keys appropriately.  If it's a horizontal, do the       *
 * same.                                                                    */

sword _TuiMenuLinkOpt(HVMENU vmenu, HVOPT opt, HVOPT end_opt, HVOPT last_opt)
{
  word backkey;
  word forekey;
  HVOPT next, prior;
  
  NW(last_opt);
  NW(end_opt);

  /* No keys req'd for info buttons */

  if (opt->regist==DlgInfReg)
    return 0;
  
  switch (vmenu->type & (MENU_VERT|MENU_HORIZ|MENU_DIALOG))
  {
    case MENU_VERT:
      backkey=VKEY_UP;
      forekey=VKEY_DOWN;
      break;
      
    default:
    case MENU_HORIZ:
      backkey=VKEY_LEFT;
      forekey=VKEY_RIGHT;
      break;
      
    case MENU_DIALOG:
      backkey=VKEY_STAB;
      forekey=K_TAB;
      break;
  }

  opt->fbefore=_TuiMenuOptHighlight;
  opt->fafter =_TuiMenuOptNormal;

  prior=_TuiGetPriorOpt(vmenu, opt);
  next=_TuiGetNextOpt(vmenu, opt);

  if (_TuiMenuAddKey(opt, backkey, NULL, prior, NULL, HOT_NONE, 0)==-1)
    return -1;

  if (_TuiMenuAddKey(opt, forekey, NULL, next, NULL, HOT_NONE, 0)==-1)
    return -1;

  return 0;
}


/* Width of option is length of string, minus one if it has a "~" */

static word _TuiOptnameLen(char *name)
{
  return (strlen(name) - (strchr(name, '~') != NULL));
}
    




/* Count number of options on menu, and get width of longest option name */

static word _TuiMenuCountOpts(HVMENU vmenu, word *width)
{
  word opt;
  word thiswid;
  HVOPT thisopt;
  
  *width=0;
  
  /* Count how many options are on this menu */

  for (opt=0; opt < MAX_VOPT && vmenu->opt[opt].name; opt++)
  {
    thisopt=&vmenu->opt[opt];

    /* Find out how wide the option is, and if it's bigger than the old     *
     * witdh, update the old width.                                         */

    thiswid=_TuiOptnameLen(thisopt->name);
    
    if (thiswid > *width)
      *width=thiswid;
  }

  
  return opt;
}


/* Translation table from ASCII to scan codes */

static struct _axlt
{
  char ascii;
  char scan;
} axlt[]=
{
  {'A', K_ALTA},
  {'B', K_ALTB},
  {'C', K_ALTC},
  {'D', K_ALTD},
  {'E', K_ALTE},
  {'F', K_ALTF},
  {'G', K_ALTG},
  {'H', K_ALTH},
  {'I', K_ALTI},
  {'J', K_ALTJ},
  {'K', K_ALTK},
  {'L', K_ALTL},
  {'M', K_ALTM},
  {'N', K_ALTN},
  {'O', K_ALTO},
  {'P', K_ALTP},
  {'Q', K_ALTQ},
  {'R', K_ALTR},
  {'S', K_ALTS},
  {'T', K_ALTT},
  {'U', K_ALTU},
  {'V', K_ALTV},
  {'W', K_ALTW},
  {'X', K_ALTX},
  {'Y', K_ALTY},
  {'Z', K_ALTZ},
  {'1', K_ALT1},
  {'2', K_ALT2},
  {'3', K_ALT3},
  {'4', K_ALT4},
  {'5', K_ALT5},
  {'6', K_ALT6},
  {'7', K_ALT7},
  {'8', K_ALT8},
  {'9', K_ALT9},
  {'0', K_ALT0},
  { 0 , 0},
};

static word near AsciiToScanCode(char sel_char)
{
  word n;
  
  for (n=0; axlt[n].ascii; n++)
    if (axlt[n].ascii==sel_char)
      return ((word)axlt[n].scan << 8);

  return 0;
}


/* Figure out where to place a particular option, relative to this menu */

static void near _TuiPlaceOption(HVMENU menu, HVOPT opt)
{
  if (menu->type & MENU_VERT)
  {
    if (opt->cx==0xffffu)
      opt->cx=menu->cx;
    
    if (opt->cy==0xffffu)
      opt->cy=menu->cy++;
  }
  else if (menu->type & MENU_HORIZ)
  {
    if (opt->cx==0xffffu)
    {
      opt->cx=menu->cx;
      menu->cx += strlen(_TuiMakeOptName(opt, NULL, NULL, 0))+2;
    }
    
    if (opt->cy==0xffffu)
      opt->cy=menu->cy;
  }

  opt->hot_col = (byte)(opt->parent->start_x + opt->cx +
                        (opt->parent->border != BORDER_NONE));

  opt->hot_row = (byte)(opt->parent->start_y + opt->cy +
                        (opt->parent->border != BORDER_NONE));

  if (opt->parent->type & MENU_VERT)
    opt->hot_n_cols=(byte)(opt->parent->wid+2);
  else opt->hot_n_cols=(byte)(strlen(_TuiMakeOptName(opt, NULL, NULL, 0))+2);

  opt->hot_n_rows=1;
}



/* Create the list of hotkeys for each menu, and attach this list to        *
 * each specified menu option, so that the hotkeys will jump to (and        *
 * execute) the appropriate key at the right time.                          */

static sword _TuiCreateMenuKeyList(HVMENU menu, HVKEY otherkeys)
{
  HVKEY keys;
  HVOPT opt, end;
  HVOPT dummyopt;
  char sel_char;
  word did_hot;

  if ((dummyopt=malloc(sizeof(*dummyopt)))==NULL)
    return -1;
  
  /* Create a dummy option record to hold these new key entries.  By        *
   * default, start with the keys which were passed from the higher level   *
   * of the menu, so that any new keys will override the old keys on        *
   * the upper menu, due to the fact that we're inserting entries at the    *
   * front of the linked list.                                              */

  keys=otherkeys;

  /* Loop through all of the menu options, and create a key record for      *
   * each.                                                                  */

  for (opt=menu->opt, end=opt+menu->num_opt; opt < end; opt++)
  {
    /* Initialize the before/after and parent stuff */

    opt->fbefore=opt->fafter=NULL;
    opt->parent=menu;


    /* Calculate the position of this option on-screen */

    _TuiPlaceOption(menu, opt);

    /* Copy this option into the dummy option, so that the other routines   *
     * think that they're adding to its keylist, when they're really adding *
     * to the main keylist.                                                 */

    *dummyopt=*opt;
    dummyopt->keys=keys;
 
    /* Find out what the selection character for this option is */

    if (opt->regist != DlgInfReg)
    {
      _TuiMakeOptName(opt, &sel_char, NULL, 0);
      sel_char=(byte)toupper(sel_char);


      /* This option must not have a hot selection charcater... */

      did_hot=FALSE;

      /* Hotkeys activiated by menu option's normal letter, not <alt>ed */

      if (menu->type & MENU_NHOT)
      {
        _TuiMenuAddKey(dummyopt, sel_char, opt->menufn, opt,
                opt->menu, HOT_NONE, VKF_AFTER);
      }

      /* If we're supposed to hook up hot-keys, do these here too */

      if (sel_char && (menu->type & MENU_HOT))
      {
        word mkey;

        if ((mkey=AsciiToScanCode(sel_char)) != 0)
        {
          if (opt->regist==DlgRadReg || opt->regist==DlgButReg ||
              opt->regist==DlgChkReg /*|| opt->regist==DlgLstReg*/)
          {
            _TuiMenuAddKey(dummyopt, 0xffff,
                           NULL, opt,
                           opt->menu, HOT_PRESS1, VKF_AFTER);

            _TuiMenuAddKey(dummyopt, mkey, opt->menufn, opt,
                           opt->menu, HOT_RELEASE1, VKF_AFTER);
          }
          else
          {
            _TuiMenuAddKey(dummyopt, mkey, NULL /*opt->menufn*/, opt,
                           opt->menu, HOT_PRESS1, VKF_AFTER);

            _TuiMenuAddKey(dummyopt,
                           (menu->type & MENU_HOT_ONLY) ? 0xffff : sel_char,
                           opt->menufn, opt, opt->menu, HOT_RELEASE1,
                           VKF_AFTER);
          }

          did_hot=TRUE;
        }
      }

      if (!did_hot)
      {
        _TuiMenuAddKey(dummyopt, sel_char,
                       opt->menufn, opt, opt->menu,
                       did_hot ? HOT_NONE : HOT_RELEASE1,
                       VKF_AFTER);


        /* gliding lite bar thingy for pulldowns */

        if (!did_hot)
          _TuiMenuAddKey(dummyopt, 0xffffu, NULL, opt, NULL, HOT_PRESS1,
                         VKF_AFTER);
      }
    }

    keys=dummyopt->keys;
  }
  
  if (dummyopt->keys)
    dummyopt->keys->flag |= VKF_MENUKEYS;
  
  for (opt=menu->opt; opt < end; opt++)
    opt->keys=dummyopt->keys;
  
  free(dummyopt);
  
  return 0;
}


/* This function performs the inheritance bit for keys on the lower         *
 * level of the tree, so that higher-level menu functions can be called     *
 * even when inside a deep, nested menu.                                    */
  
static HVKEY _TuiGetSubKeys(HVMENU oldmenu, HVOPT opt)
{
  HVKEY new=NULL, key, add;
  int ky;
  
  /* This loop scans the current option, and figures out which keys we're   *
   * supposed to make the child menu "inherit".  The assumption is that     *
   * we're using pulldowns, such that the left/right (or up/down for        *
   * a reverse pulldown) will move to the next/prior menu.                  */

  for (key=opt->keys; key; key=key->next)
  {
    /* Copy the left/right keys if the master window was a horizontal, or   *
     * the up/down keys if the master was a vertical window.  Also copy     *
     * characters which are extended scan codes only, such as <Alt> keys.   */

    ky=key->key;
    
    if (((oldmenu->type & MENU_HORIZ) && (ky==VKEY_LEFT || ky==VKEY_RIGHT)) ||
        ((oldmenu->type & MENU_VERT)  && (ky==VKEY_UP || ky==VKEY_DOWN)) ||
        (ky & 0xff)==0)
    {
      if ((add=malloc(sizeof(*add)))==NULL)
        return NULL;
      
      *add=*key;

      /* So that pull-down menus stay "down" */

      /* add->menufn=key->newopt->menufn; */
      add->newmenu=key->newopt->menu;

      /* Now append to the linked list */

      add->next=new;
      new=add;
    }
  }

  /* Set the flag at the beginning of this menu list so that these don't    *
   * get freed more than once.                                              */
  
  if (new)
    new->flag |= VKF_MENUKEYS;

  return new;
}
  

/* Create an "escape key" entry for each menu, which simply returns to the  *
 * option that called it.                                                   */

HVKEY _TuiAddEscKey(HVOPT opt)
{
  HVKEY key;
  
  if ((key=malloc(sizeof(*key)))==NULL)
    return NULL;
  
  memset(key, '\0', sizeof(*key));
  
  key->key=K_ESC;
  key->menufn=NULL;
  key->newopt=opt;
  key->newmenu=NULL;

  /* Works anywhere on screen, and escape when right button is pressed */

  key->hot_flag=HOT_PRESS2 | HOT_ANYWHERE;
  
  return key;
}

/* Register all menu trees which are below this one.  Recursively calls     *
 * the TuiRegister...() procedures.                                         */

sword _TuiRegisterSubordinate(HVMENU oldmenu, HVOPT opt)
{
  HVKEY key, esc;
 
  /* If this menu is to inherit keys from the menu above, do so */

  if (opt->menu->type & MENU_INHERIT)
    key=_TuiGetSubKeys(oldmenu, opt);
  else key=NULL;

  esc=_TuiAddEscKey(opt);
  
  esc->next=key;
  key=esc;
  
  /* Automatically place this menu on the screen, based on the location     *
   * of the parent option.                                                  */

  if (opt->menu->type & MENU_PLACE)
  {
    /* For horizontal menus, use the standard pull-down menu scheme */

    if (oldmenu->type & MENU_HORIZ)
    {
      opt->menu->start_x=oldmenu->start_x + opt->cx +
                         (oldmenu->border != BORDER_NONE);

      opt->menu->start_y=oldmenu->start_y + 1 +
                         (oldmenu->border==BORDER_NONE ? 0 : 1);
    }
    else
    {
      opt->menu->start_x=oldmenu->start_x + 4 /* oldmenu->wid+2 */;
      opt->menu->start_y=oldmenu->start_y + opt->cy + 1;
    }
  }

  /* Link this menu to the old one via its parent */

  opt->menu->parent=oldmenu;
  
  if (_TuiRegisterMenu1(opt->menu, key)==NULL)
    return -1;
  
  return 0;
}


/* Finds out how wide and how tall the specified menu should be */

static void near _TuiGetWindowDim(HVMENU menu, word *cols, word *rows)
{
  if (menu->type & MENU_VERT)
  {
    *rows=menu->num_opt+(menu->border==BORDER_NONE ? 0 : 2);
    *cols=menu->wid+2+(menu->border==BORDER_NONE ? 0 : 2);
  }
  else if (menu->type & MENU_DIALOG)
  {
    *cols=menu->sizex;
    *rows=menu->sizey;
  }
  else /* MENU_HORIZ */
  {
    *rows=1+(menu->border==BORDER_NONE ? 0 : 2);
    *cols=VidNumCols();
  } 
}



/* This figures out where to put this menu on-screen,  It handles           *
 * both vertical and horizontal centering (with start_x/y of -1), and       *
 * adjusting if the menu goes off the screen.                               */

void _TuiFindMenuStartLoc(HVMENU menu)
{
  word rows, cols;
  
  if (menu->start_x==0xffffu)
    menu->start_x=((VidNumCols() - menu->sizex) >> 1);

  if (menu->start_y==0xffffu)
    menu->start_y=((VidNumRows() - menu->sizey) >> 1);

  _TuiGetWindowDim(menu, &cols, &rows);
  
  if ((sword)(menu->start_y + rows) >= VidNumRows())
    menu->start_y = VidNumRows()-rows;

  if ((sword)(menu->start_x + cols) >= VidNumCols())
    menu->start_x = VidNumCols()-cols;

  if ((sword)menu->start_y < 0)
    menu->start_y=0;

  if ((sword)menu->start_x < 0)
    menu->start_x=0;
}
  


/* This function initializes one menu, inheriting the 'keylist' list of     *
 * keys from a parent menu.  This function calls itself (via another        *
 * subroutine) to handle nested menus.                                      */

HVMENU _TuiRegisterMenu1(HVMENU vmenu, HVKEY keylist)
{
  HVOPT opt, last_opt, end_opt;

  
  if ((vmenu->num_opt=_TuiMenuCountOpts(vmenu, &vmenu->wid))==0)
    return NULL;

  if (vmenu->sizey==0xffff)
    vmenu->sizey=((vmenu->type & MENU_HORIZ) ? 1 : vmenu->num_opt) +
                    (vmenu->border != BORDER_NONE ? 2 : 0);

  if (vmenu->sizex==0xffff)
    vmenu->sizex=((vmenu->type & MENU_HORIZ) ? VidNumCols() : vmenu->wid) +
                    (vmenu->border != BORDER_NONE ? 2 : 0);
  
  /* Centre the menu, if necessary */

  _TuiFindMenuStartLoc(vmenu);
  
  /* Start placing menu options after the border */

  vmenu->cx=0;
  vmenu->cy=0;
  
  /* Create the global key list and determine where on screen to place      *
   * each menu option.                                                      */

  if (_TuiCreateMenuKeyList(vmenu, keylist)==-1)
    return NULL;
  
  /* Now loop through all of the options and connect them to each other */

  for (opt=vmenu->opt, last_opt=NULL, end_opt=vmenu->opt+vmenu->num_opt;
       opt < end_opt;
       opt++)
  {
    /* Link this option to the prior and next options */
    
    if (_TuiMenuLinkOpt(vmenu, opt, end_opt, last_opt)==-1 ||
        (opt->menu && _TuiRegisterSubordinate(vmenu, opt)==-1) ||
        _TuiAddEnterFunc(vmenu, opt)==-1)
    {
      TuiDestroyMenu(vmenu);
      return NULL;
    }

    /* Now handle additional registration needs for this option */

    if (opt->regist)
      (*opt->regist)(opt);

    last_opt=opt;
  }

  return vmenu;
}


/* "Register" a menu structure.  This fills in all of the holes and         *
 * initializes stuff, which makes it ready for calling by TuiExecMenu().    */
  
HVMENU _fast TuiRegisterMenu(HVMENU vmenu)
{
  vmenu->parent=NULL;
  vmenu->type |= def_menu_attr;

  return (_TuiRegisterMenu1(vmenu, NULL));
}




/* Returns TRUE if the specified menu is on the current menu stack */

static word near _TuiMenuOnStack(HVMENU men, HVMENU menustk)
{
  HVMENU m;

  for (m=menustk; m; m=m->next)
    if (m==men)
      return TRUE;
        
  return FALSE;
}





/* Display a menu on-screen */

static sword near _TuiDrawMenu(HVMENU menu)
{
  HVOPT opt, end;
  
  /* Draw all of the options on this menu */

  for (opt=menu->opt, end=menu->opt+menu->num_opt; opt < end; opt++)
    _TuiMenuOptNormal(opt);

  if (menu->before)
    (*menu->before)(menu);

  return 0;
}

static word _TuiOnThisMenu(HVMENU m, HVOPT opt)
{
  HVOPT o, end;

  for (o=m->opt, end=o+m->num_opt; o < end; o++)
    if (opt==o)
      return TRUE;

  return FALSE;
}


/* obsolete.  the opt->parent member can be used to find this */

/*
static HVMENU _TuiFindMenuOption(HVOPT opt)
{
  HVMENU m;
  HVOPT o, end;

  for (m=menustk; m; m=m->next)
    for (o=m->opt, end=o+m->num_opt; o < end; o++)
      if (opt==o)
        return m;

  return NULL;
}
*/

/* Find the common "parent" of two given menus */
    
static HVMENU _TuiCommonParent(HVMENU m1, HVMENU m2)
{
  HVMENU wm1, wm2;
  
  for (wm1=m1; wm1; wm1=wm1->parent)
    for (wm2=m2; wm2; wm2=wm2->parent)
      if (wm1==wm2)
        return wm1;

  return NULL;
}



static void near _TuiCloseMenu(HVMENU m)
{
  m->type &= ~MENU__DROP;

  /* Execute the 'after' functions for the option on that menu */

  if (m->curopt != m->lastopt)
    _TuiOptAfter(m->lastopt);

  if (m->after)
    (*m->after)(m);

  if (m->win)
    WinClose(m->win);
}


/* Close old menus to get to the current, activated menu in 'nextmen' */

void _TuiCloseOldMenu(HVMENU oldmenu, HVMENU newmenu, HVOPT newopt)
{
  HVMENU compar, m;
  HVOPT lastopt, curopt;

  /* Find the common parent of the two windows, and walk back up to it */

  if ((compar=_TuiCommonParent(oldmenu, newmenu)) != NULL)
  {
    for (m=oldmenu; m && m != compar; m=m->parent)
      m->type |= MENU__DROP;

    compar->lastopt=lastopt=compar->curopt;
    compar->curopt=curopt=(newopt ? newopt : compar->curopt);

    /* Now call the before/after functions to do the transversal */

    if (curopt != lastopt)
    {
      _TuiOptAfter(lastopt);
      _TuiOptBefore(curopt);
    }

    WinSync(compar->win, FALSE);
  }

  if (newopt && _TuiOnThisMenu(newmenu, newopt))
    newmenu->curopt=newopt;
  else newmenu->curopt=newmenu->opt;

  newmenu->lastopt=NULL;
}


/* Execute a particular menu option */

static sword near _TuiExecOption(HVMENU menu, HVKEY key, HVMENU *nxtmen)
{
  sword ret=0;
  HVMENU newmenu, oldnm;

  NW(menu);
  
  /* Handle any specific functions that need to be done */

  if (key->menufn)
  {
    oldnm=nextmen;
    nextmen=*nxtmen;

    ret=(*key->menufn)((key->flag & VKF_AFTER)==0 || !key->newopt
                               ? menu->curopt
                               : key->newopt);

    *nxtmen=nextmen;
    nextmen=oldnm;

    if (ret != 0)
    {
      /* Now call the 'after' function for the old menu option. */
     
      _TuiOptAfter(menu->curopt);

      return ret;
    }
  }
  
  /* Switch to a new menu by default, if this key's actions instructed      *
   * us to do so.                                                           */

  newmenu=NULL;

  if (key->newopt)
  {
    /* Switch the current menu option as appropriate.  If the option        *
     * isn't on the current menu, we have to implicitly switch menus.       */

    if (_TuiOnThisMenu(menu, key->newopt) && !key->newmenu)
    {
      menu->lastopt=menu->curopt;
      menu->curopt=key->newopt;
    }
    else if ((newmenu=key->newopt->parent) != NULL && !key->newmenu)
    {
      /* If the option is on another menu, set that menu's current option   *
       * to the new option.  Don't bother doing this if there is a          *
       * key->newmenu, since we need to update the newmenu pointers below   *
       * anyway.                                                            */

      /*
      newmenu->lastopt=newmenu->curopt;
      newmenu->curopt=key->newopt;
      */
    }
  }

  /* So that pull-down menus can "stick" across options */

  if (key->newmenu)
    newmenu=key->newmenu;

  if (newmenu && newmenu != *nxtmen)
  {
    _TuiCloseOldMenu(*nxtmen, newmenu, key->newopt);

    if (newmenu)
      *nxtmen=newmenu;
  }

  return 0;
}


/* Get a character from the keyboard, and find the HVKEY structure          *
 * that it relates to.                                                      */

static HVKEY near _TuiGetKeyboardOption(HVMENU menu)
{
  HVKEY key;
  word ch;

  if (!khit())
    return NULL;
  
  ch=kgetch();
  
  /* Process IBM scan codes */

  if (ch==0 || ch==0xe0)
    ch = (kgetch() << 8);

  menu->laststroke=ch;
  
  /* Now convert the 'lower byte' to upper case */

  ch = ((ch & 0xff00u) | (toupper(ch & 0xff)));

  /* Now search the current option to find out which key this is for. */
  
  for (key=menu->curopt->keys; key; key=key->next)
    if (ch==key->key)
    {
      last_evt_was_mouse=FALSE;
      return key;
    }

  /* If we couldn't find anything, pretend that it didn't happen.  If       *
   * there's a handler for an unknown keyboard action, call it here.        */

  if (menu->curopt->unknown)
    (*menu->curopt->unknown)(menu->curopt);

  return NULL;
}


/* Check to see if the specified event (press or release) happened within   *
 * one of our hotspots.                                                     */

static HVKEY near _TuiMouseAction(HVMENU menu, word col, word row, word mask)
{
  HVKEY key;

  for (key=menu->curopt->keys; key; key=key->next)
    if (key->hot_flag & mask)
    {
      #ifdef DEBUG_PD
      WinPrintf(wscrn, "Check %s %s at %d %d %d %d\r\n",
                key->newopt->name, key->newmenu->parent->curopt->name,
                key->hot_col, key->hot_n_cols,
                key->hot_row, key->hot_n_rows);

      WinSync(wscrn, FALSE);
      #endif

      if ((key->hot_flag & HOT_ANYWHERE) ||
          ((col >= key->newopt->hot_col && 
            col < key->newopt->hot_col+key->newopt->hot_n_cols) &&
           (row >= key->newopt->hot_row && 
            row < key->newopt->hot_row+key->newopt->hot_n_rows)))
      {
        #ifdef DEBUG_PD
        WinPrintf(wscrn, "Got %s %s (%d)\r\n",
                 key->newopt->name, key->newmenu->parent->curopt->name, mask);
        WinSync(wscrn, FALSE);
        #endif

        return key;
      }
    }
      
  return NULL;
}



/* Handle an option from the mouse.  Take care of changes in status,        *
 * movement, and so forth.                                                  */

static HVKEY near _TuiGetMouseOption(HVMENU menu)
{
  word button, col, row;
  HVKEY key;
  
  if (!has_mouse)
    return NULL;
  
  MouseStatus(&button, &col, &row);

  /* If a button press occurred within this option's hotspot, activate    *
   * it.                                                                  */

  key=NULL;

  /* If cursor moved or button status changed */

  if (col != lastcol || row != lastrow || lastbut != button)
  {
    /* If the right button was depressed, signal that we got it. */

    if ((button & BUT_RIGHT) && (lastbut & BUT_RIGHT)==0)
      key=_TuiMouseAction(menu, col, row, HOT_PRESS2);

    /* Don't check for lastbut here since the left button can be held       *
     * down to generate continuous BUT_LEFT events, and "drag" thigns.      */

    if (button & BUT_LEFT)
      key=_TuiMouseAction(menu, col, row, HOT_PRESS1);
  }


  /* Handle a release too */

  if ((button & BUT_LEFT)==0 && (lastbut & BUT_LEFT))
    key=_TuiMouseAction(menu, col, row, HOT_RELEASE1);

  lastbut=button;
  lastrow=row;
  lastcol=col;
  
  if (key)
    last_evt_was_mouse=TRUE;

  return key;
}



/* Get a character from the keyboard, and process it accordingly */

static sword near _TuiGetOption(HVMENU menu, HVMENU *nextmen)
{
  HVKEY key;

  WinSync(menu->win, FALSE);

  /* Get the first character */

  while ((key=_TuiGetKeyboardOption(menu))==NULL && 
         (key=_TuiGetMouseOption(menu))==NULL)
#ifdef OS_2
    DosSleep(1L);
#else
    ;
#endif

  /* Now execute the option */

  return (_TuiExecOption(menu, key, nextmen));
}







/* Close all of the windows on the current menu stack */

static void near _TuiCloseMenuStack(HVMENU menustk)
{
  HVMENU m;

  /* Now close all of the menus which may be open */

  for (m=menustk; m; m=m->next)
    _TuiCloseMenu(m);

  menustk=NULL;
}






/* Execute a pre-registered menu */

sword _fast TuiExecMenu(HVMENU vmenu)
{
#ifdef PM
  NW(vmenu);
  TuiPmEvent(vmenu);
  return 0;
#else
  HVMENU menustk, m, last;
  HVMENU nextmen;
  HVOPT curopt, lastopt;
  
  sword ret=0;
  
  menustk=NULL;
  nextmen=vmenu;
  vmenu->curopt=vmenu->opt+vmenu->def_opt;
  vmenu->lastopt=NULL;

  MouseFlush();
  
  while (khit())
    kgetch();

  do
  {
    /* Skip any "information" options */

    while (nextmen->curopt->regist==DlgInfReg)
      nextmen->curopt++;

    /* Shortcut for referencing the current option */

    curopt=nextmen->curopt;
    lastopt=nextmen->lastopt;

    /* Now, if any old windows need to be closed, make sure that they       *
     * are closed here and have the __DROP flag removed.                    */

    for (m=menustk, last=NULL; m; m=m->next)
      if (m->type & MENU__DROP)
      {
        _TuiCloseMenu(m);

        if (last)
        {
          last->next=m->next;
          last=m;
        }
        else menustk=m->next;
      }
      else last=m;

    /* If the to-be-displayed menu is not on the stack, draw its window     *
     * and push it on the stack.                                            */
    
    if (! _TuiMenuOnStack(nextmen, menustk))
    {
      word rows, cols;

      /* If an old window was pushed off the top of the stack, since it     *
       * just to make sure that any last screen updates get through.        */

      if (menustk)
        WinSync(menustk->win, FALSE);
      
      /* Push the menu on top of the menu stack */
      
      nextmen->next=menustk;
      menustk=nextmen;

      
      /* Figure out how many columns/rows this window should be */

      _TuiGetWindowDim(nextmen, &cols, &rows);

      
      /* Open the menu for this window */

       if ((nextmen->win=WinOpen(nextmen->start_y, nextmen->start_x,
                                 rows, cols,
                                 nextmen->border,
                                 col(nextmen->col_win),
                                 col(nextmen->col_bor),
                                 WIN_NOCSYNC | WIN_NODRAW))==NULL)
      {
        ret=-1;
        break;
      }

      if (nextmen->type & MENU_SHADOW)
        WinShadow(nextmen->win, CDGRAY);
      
      if (nextmen->title)
        WinTitle(nextmen->win, nextmen->title, TITLE_LEFT);

      if ((ret=_TuiDrawMenu(nextmen)) != 0)
        break;
    }

   
    /* Now call the 'before' and 'after' functions for the new menu option. */
     
    if (curopt != lastopt)
    {
      _TuiOptAfter(lastopt);
      _TuiOptBefore(curopt);
    }

    nextmen->lastopt=lastopt=curopt;
  }
  while ((ret=_TuiGetOption(nextmen, &nextmen))==0);

  _TuiCloseMenuStack(menustk);

  NW(ret);

  return (dlg_ok==1);
#endif
}

void _fast TuiSetMenuType(word attr)
{
  def_menu_attr=attr;
}

sword _fast TuiDestroyMenu(HVMENU vmenu)
{
  NW(vmenu);

  return 0;
}


