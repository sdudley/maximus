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
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include "tui.h"

static char * near DlgButName(HVOPT opt, char *sel_ch, word *sel_ofs);
static char * near DlgStrName(HVOPT opt, char *buf, char *sel_ch, word *sel_ofs);
static void near DlgStrGetLen(HVOPT opt, word *field_len, word *max_len);
static void near DlgRedrawStr(HVOPT opt, word *offset);
static char * near DlgStrIsValue(struct _vopt *opt);
static MenuFunction(DlgRadValue);

extern HVMENU nextmen;            /* From tui.c; next menu to be executed   */
word dlg_ok;                      /* Did the user select "OK" or "Cancel"?  */

static char val_str[]=",%d:";     /* String used to delimit "rolex" value   *
                                   * set options.                           */

static char szValTemp[MAX_NUM];   /* Holds string representation of current *
                                   * value set option.                      */




/* This function is called before a dialog box is created */

word DlgBefore(HVMENU menu)
{
  word n;

  if ((menu->dlgsave=(void **)malloc(sizeof(void *) * menu->num_opt))==NULL)
    return 1;

  dlg_ok=0;

  for (n=0; n < menu->num_opt; n++)
  {
    if (menu->opt[n].display==DlgButShow)
      menu->dlgsave[n] = menu->opt[n].data? *(void **)menu->opt[n].data :NULL;
    else if (menu->opt[n].display==DlgRadShow)
      menu->dlgsave[n] = menu->opt[n].data? *(void **)menu->opt[n].data : NULL;
    else if (menu->opt[n].display==DlgStrShow)
    {
      word max_len;
      
      /* Handle value option strings */

      if (DlgStrIsValue(menu->opt+n))
        menu->dlgsave[n]=(void *)*(char *)menu->opt[n].data;
      else
      {
        DlgStrGetLen(&menu->opt[n], NULL, &max_len);

        /*menu->dlgsave[n]=menu->opt[n].data;*/
      
        /* Allocate a new string to hold the saved copy of the information */

        menu->dlgsave[n]=malloc(max_len);
      
        /* Copy the info into the new buffer */

        if (menu->dlgsave)
          strocpy((char *)menu->dlgsave[n], (char *)menu->opt[n].data);
      }
    }
  }
  
  return 0;
}


/* This function is called just after the user finishes with the dialog     *
 * box.  If dlg_ok==-1, it restores the contents of the dialog box to       *
 * their previous values, before the dialog box was originally called up.   */

word DlgAfter(HVMENU menu)
{
  word n;

  if (!menu->dlgsave)
    return 0;

  for (n=0; n < menu->num_opt; n++)
  {
    if (dlg_ok != 1)
    {
      if (menu->opt[n].display==DlgButShow)
      {
        if (menu->opt[n].data)
          *(void **)menu->opt[n].data = menu->dlgsave[n];
      }
      else if (menu->opt[n].display==DlgRadShow)
      {
        if (menu->opt[n].data)
          *(void **)menu->opt[n].data = menu->dlgsave[n];
      }
      else if (menu->opt[n].display==DlgStrShow)
      {
        if (DlgStrIsValue(menu->opt+n))
          *(char *)menu->opt[n].data=(char)menu->dlgsave[n];
        else if (menu->dlgsave[n])
          strcpy((char *)menu->opt[n].data, (char *)menu->dlgsave[n]);
      }
    }
    
    if (menu->opt[n].display==DlgStrShow && menu->dlgsave[n] &&
        !DlgStrIsValue(menu->opt+n))
    {
      free(menu->dlgsave[n]);
    }
  }

  return 0;
}


/****************************************************************************/
/**                           DIALOG BUTTONS                               **/
/****************************************************************************/

/* Display a dialog button on-screen.  This draws the box for the           *
 * dialog button, and selects a colour depending on whether or not          *
 * the button is currently selected.                                        */

MenuFunction(DlgButShow)
{
  VWIN *win=opt->parent->win;

  char *butname;
  char sel_ch;
  byte attr, sel_attr;

  word sel_ofs, high, butlen;

  high=(opt->parent->curopt==opt);
  attr=    col((byte)(high ? BUTTON_HIGH : BUTTON_STD));
  sel_attr=col((byte)(high ? BUTTON_HIGH : BUTTON_SEL));

  butname=DlgButName(opt, &sel_ch, &sel_ofs);
  butlen=strlen(butname);

  WinBox(win, opt->cy, opt->cx, opt->cy+2, opt->cx+butlen+3,
         high ? BORDER_DOUBLE : BORDER_SINGLE, attr);
       
  WinPutch(win,   opt->cy+1,   opt->cx+1,         ' ',    col(attr));
  WinPutstra(win, opt->cy+1,   opt->cx+2,       col(attr), butname);
  
  if (sel_ch)
    WinPutch(win,   opt->cy+1,   opt->cx+2+sel_ofs, sel_ch, col(sel_attr));

  WinPutch(win,   opt->cy+1,   opt->cx+2+butlen,  ' ',    col(attr));

  return 0; 
}


/* Activate a dialog box button.  This stores the specified value in
 * the 'data' field.  Format for the name field is as follows:
 *
 * <val><type>;<name>
 *
 * where <val> is the numeric value to store when the function is selected,
 * <type> is the character 'b' or 'w' (store as a byte or word), and
 * <name> is the ASCII name of the option.
 */

MenuFunction(DlgButAct)
{
  char valbuf[MAX_OPTNAME_LEN];
  word val, len;
  char *s;
  
  for (s=opt->name; *s != ';'; s++)
    ;

  len=(word)(s-opt->name);
  
  memmove(valbuf, opt->name, len);
  valbuf[len]='\0';
  
  val=atoi(valbuf);
  
  if (opt->data)
  {
    if (s[-1]=='b')
      *(byte *)opt->data=(byte)val;
    else *(word *)opt->data=val;
  }

  if (dlg_ok != 0)
  {
    if (opt->parent->parent)
      _TuiCloseOldMenu(opt->parent, opt->parent->parent,
                       opt->parent->parent->curopt);

    nextmen=opt->parent->parent;

    /* If this was the top level, exit the menu structure */

    if (nextmen==NULL)
      return 1;
  }

  return 0;
}

MenuFunction(DlgButOk)
{
  dlg_ok=1;

  if (opt->parent->parent)
    _TuiCloseOldMenu(opt->parent, opt->parent->parent,
                     opt->parent->parent->curopt);

  nextmen=opt->parent->parent;

  /* If this was the top level, exit the menu structure */

  if (nextmen==NULL)
    return 1;

  return 0;
}

MenuFunction(DlgButCan)
{
  dlg_ok=-1;

  if (opt->parent->parent)
    _TuiCloseOldMenu(opt->parent, opt->parent->parent,
                     opt->parent->parent->curopt);

  nextmen=opt->parent->parent;

  /* If this was the top level, exit the menu structure */

  if (nextmen==NULL)
    return 1;

  return 0;
}


/* Handle additional registration needs for the dialog button */

MenuFunction(DlgButReg)
{
  HVOPT prior, next;

  prior=_TuiGetPriorOpt(opt->parent, opt);
  next=_TuiGetNextOpt(opt->parent, opt);

  _TuiMenuAddKey(opt, VKEY_UP,   NULL, prior, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DOWN,  NULL, next, NULL, 0, 0);

  opt->hot_n_rows=3;
  opt->hot_n_cols=(byte)(4+strlen(DlgButName(opt, NULL, NULL)));
  return 0;
}



/* Return the 'name' of a dialog button, plus its quick selection character *
 * and the offset of that character from the start of the name.             */

static char * near DlgButName(HVOPT opt, char *sel_ch, word *sel_ofs)
{
  static char butname[MAX_OPTNAME_LEN];
  char *s, *namestart, *o;
  
  if (sel_ch)
    *sel_ch='\0';
  
  if (sel_ofs)
    *sel_ofs=0;
  
  for (s=opt->name; *s != ';'; s++)
    ;
  
  namestart=++s;

  for (o=butname; *s; s++)
  {
    if (*s=='~')
    {
      if (sel_ch)
        *sel_ch=s[1];
      
      if (sel_ofs)
        *sel_ofs=(word)(s-namestart);
      
      s++;
    }

    *o++=*s;
  }

  *o='\0';
  return butname;
}




/****************************************************************************/
/**                           DIALOG STRINGS                               **/
/****************************************************************************/


/* Returns non-null if the option is a value-string (combobox) */

static char * near DlgStrIsValue(struct _vopt *opt)
{
  char *p;

  if ((p=strchr(opt->name, '`')) != NULL)
    return p+1;
  else return NULL;
}


/* Find the string associated with display value 'val' in the option 'opt' */

static void near DlgStrGetDisplayVal(struct _vopt *opt, int val, char *out)
{
  char temp[20];
  char *p, *s;

  sprintf(temp, val_str, val);

  if ((p=strstr(opt->name, temp))==NULL)
  {
    *out='\0';
    return;
  }

  s=strchr(++p, ',');

  if (!s)
    s=strchr(p, ';');

  if (s)
  {
    p=strchr(p, ':')+1;

    memmove(out, p, (size_t)(s-p));
    out[(size_t)(s-p)]='\0';
  }
}


/* Rotate this option to the next value */

static void near DlgStrRotateValue(struct _vopt *opt)
{
  char temp[20];
  char *p, *s;

  sprintf(temp, val_str, *(char *)opt->data);

  if ((p=strstr(opt->name, temp)) != NULL)
    p++;
  else p=strchr(opt->name, '`');

  if ((s=strchr(p, ','))==NULL)
    s=strchr(opt->name, ',');

  if (s)
  {
    *(char *)opt->data=(char)atoi(s+1);
    DlgStrShow(opt);
    WinSync(opt->parent->win, TRUE);
  }
}

/* Display the string for the current value menu option */

static void near DlgStrDisplayValue(struct _vopt *opt, VWIN *win, int field_len)
{
  char temp[PATHLEN];

  DlgStrGetDisplayVal(opt, *(char *)opt->data, temp);
  WinPrintf(win, " %-*.*s ", field_len-2, field_len-2, temp);
}


MenuFunction(DlgStrShow)
{
  VWIN *win=opt->parent->win;
  char str[MAX_OPTNAME_LEN];
  byte sel_ch;
  word sel_ofs;
  word high=(opt->parent->curopt==opt);
  word field_len, max_len;
  
  DlgStrName(opt, str, &sel_ch, &sel_ofs);

  WinPutstra(win, opt->cy, opt->cx, 
             col(opt->parent->col_item), str);

  if (sel_ch)
    WinPutch(win, opt->cy, opt->cx+sel_ofs, sel_ch,
             col(opt->parent->col_hot));

         
  DlgStrGetLen(opt, &field_len, &max_len);
  
  WinGotoXY(win, opt->cy, opt->cx+strlen(str), FALSE);
  WinSetAttr(win, col((byte)(high ? STR_SEL : STR_NORMAL)));

  if (DlgStrIsValue(opt))
    DlgStrDisplayValue(opt, win, field_len);
  else
  {
    WinPrintf(win, " %-*.*s ", field_len-2, field_len-2, opt->data);

    if (strlen(opt->data) > field_len /*&& !high*/)
      WinPutch(win, opt->cy, opt->cx+strlen(str)+field_len-1,
               '\xaf', col(high ? STR_SEL : STR_LEFTRIGHT)); /* ae */
  }

  opt->appdata=DLG_NEW;
  return 0; 
}



MenuFunction(DlgStrEdit)
{
  static word offset;
  static HVOPT lastopt=NULL;

  word key, max_len;
  
  char *str=opt->data;

  if (DlgStrIsValue(opt))
  {
    if (opt->parent->laststroke != '\r')
      DlgStrRotateValue(opt);

    return 0;
  }

  DlgStrGetLen(opt, NULL, &max_len);

  if (lastopt != opt || opt->appdata==DLG_NEW)
  {
    opt->appdata=DLG_NEW;

    lastopt=opt;
    offset=0;
  }

  /* Do something based on the last key pressed */

  switch (key=opt->parent->laststroke)
  {
    case VKEY_LEFT:
      if (offset)
        offset--;
      break;
      
    case VKEY_RIGHT:
      if (offset < strlen(str))
        offset++;
      break;
      
    case VKEY_DEL:
      if (offset < strlen(str))
        strocpy(str+offset, str+offset+1);
      break;

    case VKEY_HOME:
      offset=0;
      break;

    case VKEY_END:
      offset=strlen(str);
      break;

    case K_CTRLY:
      str[offset=0]='\0';
      break;
      
    case K_BS:
      if (offset)
      {
        strocpy(str+offset-1, str+offset);
        offset--;
      }
      break;
      
    default:    /* normal character */
      key &= 0xff;
      
      if (key < ' ')
        break;
      
      if (opt->appdata==DLG_NEW)
      {
        str[0]=(byte)key;
        str[offset=1]='\0';
      }
      else if (strlen(str) < max_len-1)
      {
        strocpy(str+offset+1, str+offset);
        str[offset++]=(byte)key;
      }
      break;
  }
  
  DlgRedrawStr(opt, &offset);
  opt->appdata=DLG_EDIT;
  
  return 0;
}


/* Display a string in the dialog box */

static void near DlgRedrawStr(HVOPT opt, word *offset)
{
  VWIN *win=opt->parent->win;
  
  char name[MAX_OPTNAME_LEN];
  char *str=opt->data;

  word cx=opt->cx;
  word cy=opt->cy;
  word field_len, max_len;
  word start_pos;


  /* Get length of option text, to find where field starts */
  
  DlgStrName(opt, name, NULL, NULL);
  cx += strlen(name);
  
  /* Find out the maximum length of this field */

  DlgStrGetLen(opt, &field_len, &max_len);

  /*
  field_len:  20

  offset:     19
  start_pos:  1
  */
  
  if (*offset >= field_len-3)
    start_pos=*offset-(field_len-3);
  else start_pos=0;
  
  
  /* Display the 'left' dongle if more is to be shown */

  WinPutch(win, cy, cx, (byte)(start_pos ? '\xae' : ' '), col(STR_LEFTRIGHT));

  WinGotoXY(win, cy, cx+1, FALSE);
  WinSetAttr(win, col(STR_NORMAL));
  
  WinPrintf(win, "%-*.*s", field_len-2, field_len-2,
            str+start_pos);
          
  WinSetAttr(win, col(STR_LEFTRIGHT));

  WinPutc(win, (byte)((strlen(str+start_pos) >= field_len-2) ? '\xaf' : ' '));
 
  WinGotoXY(win, cy, cx+1+(*offset-start_pos), FALSE);
  WinCursorShow(win);
  WinSync(win, TRUE);

  return;
}



/* After leaving a dialog string field, hide the cursor */

MenuFunction(DlgStrAfter)
{
  WinCursorHide(opt->parent->win);
  return 0;
}


/* Handle additional registration needs for the dialog string field */

MenuFunction(DlgStrReg)
{
  HVOPT prior, next;
  char name[MAX_OPTNAME_LEN];
  word field_len;
  
  DlgStrName(opt, name, NULL, NULL);
  DlgStrGetLen(opt, &field_len, NULL);
  
  opt->hot_n_rows=1;
  opt->hot_n_cols=(byte)(strlen(name)+field_len);
  opt->uafter=DlgStrAfter;
  
  prior=_TuiGetPriorOpt(opt->parent, opt);
  next=_TuiGetNextOpt(opt->parent, opt);

  _TuiMenuAddKey(opt, VKEY_UP,   NULL, prior, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DOWN,  NULL, next, NULL, 0, 0);
  return 0;
}


/* Convert the name of a string dialog widget into a printable string */

static char * near DlgStrName(HVOPT opt, char *buf, char *sel_ch, word *sel_ofs)
{
  char *start=strchr(opt->name, ';');
  char *s;
  char *out;
  
  *buf='\0';

  if (sel_ofs)
    *sel_ofs=0;
  
  if (sel_ch)
    *sel_ch='\0';

  if (start==NULL)
    return buf;
  
  for (s=++start, out=buf; *s; s++)
  {
    if (*s=='~')
    {
      if (sel_ofs)
        *sel_ofs=(word)(s-start);
      
      if (sel_ch)
        *sel_ch=s[1];
    }
    else *out++=*s;
  }

  *out='\0';

  return buf;
}


/* Get the field and max string lengths for this string option */

static void near DlgStrGetLen(HVOPT opt, word *field_len, word *max_len)
{
  char *s;
  
  if (field_len)
    *field_len=atoi(opt->name);
  
  s=strchr(opt->name, ':');
  
  if (!s)
    return;
  
  if (max_len)
    *max_len=atoi(s+1);
}



/****************************************************************************/
/**                           RADIO BUTTONS                                **/
/****************************************************************************/

/* Display a dialog button on-screen.  This draws the box for the           *
 * dialog button, and selects a colour depending on whether or not          *
 * the button is currently selected.                                        */

MenuFunction(DlgRadShow)
{
  VWIN *win=opt->parent->win;
  char but_txt[10];

  char *butname;
  char sel_ch;
  byte attr, sel_attr;

  word sel_ofs, high;

  high=(opt->parent->curopt==opt);
  attr=    col((byte)(high ? RAD_HIGH : RAD_STD));
  sel_attr=col((byte)(high ? RAD_HIGH : RAD_SEL));

  butname=DlgButName(opt, &sel_ch, &sel_ofs);
/*butlen=strlen(butname);*/

  sprintf(but_txt, "(%c)", *(sword *)opt->data==DlgRadValue(opt) ? 'þ' : ' ');
  
  WinPutstra(win, opt->cy,   opt->cx, col(attr), but_txt);

  WinPutstra(win, opt->cy,   opt->cx+3,   col(attr), butname);
  
  if (sel_ch)
    WinPutch(win,   opt->cy,   opt->cx+3+sel_ofs, sel_ch, col(sel_attr));
  
  return 0;
}


/* Returns the 'value' of a radio button option */

static MenuFunction(DlgRadValue)
{
  char valbuf[MAX_OPTNAME_LEN];
  word len;
  char *s;
  
  for (s=opt->name; *s != ';'; s++)
    ;
  
  len=(word)(s-opt->name);
  
  memmove(valbuf, opt->name, len);
  valbuf[len]='\0';
  
  return (atoi(valbuf));
}



/* Selecting a radio button */

MenuFunction(DlgRadAct)
{
  word val=DlgRadValue(opt);
  HVMENU par;
  HVOPT o, oend;

  /* If this option has a data field, set it appropriately */
  
  if (opt->data)
    *(word *)opt->data=val;

  /* Now redraw all of the options in this group, to reflect the new        *
   * position of the radio button.                                          */

  for (par=opt->parent, o=par->opt, oend=o+par->num_opt; o < oend; o++)
    if (o->data==opt->data && o->display)
      (*o->display)(o);

  return 0;
}


/* Handle additional registration needs for the radio button */

MenuFunction(DlgRadReg)
{
  HVOPT first, last, prior, next;
  HVOPT before, after;

  opt->hot_n_rows=1;
  opt->hot_n_cols=(byte)(3+strlen(DlgButName(opt, NULL, NULL)));

  /* Now find the 1st and last options in this button group */

  for (first=opt; first[-1].data==opt->data; first--)
    ;

  for (last=opt; last[1].data==opt->data; last++)
    ;

  /* Find the first valid option before this group of buttons */

  before=_TuiGetPriorOpt(opt->parent, first);

  /* Find the first valid option after this group of buttons */

  for (after=opt; 
       after[1].regist==DlgRadReg && after[1].data==opt->data;
       after++)
    ;

  after=_TuiGetNextOpt(opt->parent, after);

  next=(opt+1 <= last ? opt+1 : first);
  prior=(opt-1 >= first ? opt-1 : last);

  /* Make this option selectable by the space bar */

  _TuiMenuAddKey(opt, ' ', DlgRadAct, NULL, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_LEFT, NULL, prior, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_UP,   NULL, prior, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_RIGHT, NULL, next, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DOWN,  NULL, next, NULL, 0, 0);
  _TuiMenuAddKey(opt, K_ENTER,   NULL /*DlgRadAct*/ /* it's not SAA, but this is less confusing for DOS users */, last+1, NULL, 0, 0);
  _TuiMenuAddKey(opt, K_TAB,     NULL, after, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_STAB, NULL, before, NULL, 0, 0);
  return 0;
}



/****************************************************************************/
/**                              CHECKBOXES                                **/
/****************************************************************************/

/* Returns TRUE or FALSE depending on whether or not the specified          *
 * checkbutton is set.                                                      */

static unsigned near _DlgGetChkSetting(struct _vopt *opt)
{
  char *s;
  unsigned ret;

  for (s=opt->name; *s != ';'; s++)
    ;

  if (isdigit(s[-1]) || (s[-1] >= 'a' && s[-1] <= 'f'))
  {
    unsigned ofs, mask;

    sscanf(s-1, "%x", &ofs);
    mask = 1 << (ofs-1);

    ret=!!(*(word *)opt->data & mask);
  }
  else if (s[-1]=='B')
    ret=!!*(byte *)opt->data;
  else ret=!!*(word *)opt->data;

  return ret;
}

/* Sets a checkbox setting to a particular value */

static void near _DlgSetChkSetting(struct _vopt *opt, unsigned val)
{
  char *s;

  for (s=opt->name; *s != ';'; s++)
    ;

  if (isdigit(s[-1]) || (s[-1] >= 'a' && s[-1] <= 'f'))
  {
    unsigned ofs, mask;

    sscanf(s-1, "%x", &ofs);
    mask = 1 << (ofs-1);

    if (val)
      *(word *)opt->data |= mask;
    else *(word *)opt->data &= ~mask;
  }
  else if (s[-1]=='B')
    *(byte *)opt->data=(byte)val;
  else *(word *)opt->data=(word)val;
}

MenuFunction(DlgChkShow)
{
  VWIN *win=opt->parent->win;
  char but_txt[10];

  char *butname;
  char sel_ch;
  byte attr, sel_attr;

  word sel_ofs, high;

  high=(opt->parent->curopt==opt);
  attr=    col((byte)(high ? CHK_HIGH : CHK_STD));
  sel_attr=col((byte)(high ? CHK_HIGH : CHK_SEL));

  butname=DlgButName(opt, &sel_ch, &sel_ofs);
/*butlen=strlen(butname);*/

  sprintf(but_txt, "[%c]", _DlgGetChkSetting(opt) ? 'X' : ' ');
  
  WinPutstra(win, opt->cy,   opt->cx,     col(attr), but_txt);
  WinPutstra(win, opt->cy,   opt->cx+3,   col(attr), butname);
  
  if (sel_ch)
    WinPutch(win,   opt->cy,   opt->cx+3+sel_ofs, sel_ch, col(sel_attr));
  
  return 0;
}



/* Selecting a checkbox */

MenuFunction(DlgChkAct)
{
  /* If this option has a data field, set it appropriately */
  
  if (opt->data)
    _DlgSetChkSetting(opt, !_DlgGetChkSetting(opt));


  /* Redraw this checkbox */

  (*opt->display)(opt);

  return 0;
}


/* Handle additional registration needs for the checkbox */

MenuFunction(DlgChkReg)
{
  HVOPT prior, next;

  opt->hot_n_rows=1;
  opt->hot_n_cols=(byte)(3+strlen(DlgButName(opt, NULL, NULL)));

  prior=_TuiGetPriorOpt(opt->parent, opt);
  next=_TuiGetNextOpt(opt->parent, opt);

  _TuiMenuAddKey(opt, VKEY_UP,   NULL, prior, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DOWN,  NULL, next, NULL, 0, 0);

  /* Make this option selectable by the space bar */

  _TuiMenuAddKey(opt, ' ', DlgChkAct, NULL, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_DEL, DlgChkAct, NULL, NULL, 0, 0);
  _TuiMenuAddKey(opt, VKEY_INS, DlgChkAct, NULL, NULL, 0, 0);

  _TuiMenuAddKey(opt, K_ENTER,   /*DlgChkAct*/ NULL /* it's not saa, but it's less confusing for DOS users */,
                 _TuiGetNextOpt(opt->parent, opt), NULL, 0, 0);
  return 0;
}




/****************************************************************************/
/**                         INFORMATION STRINGS                            **/
/****************************************************************************/

/* Registration for information strings -- simply clear the hotspot for     *
 * this option so that it can't be selected.                                */

MenuFunction(DlgInfReg)
{
  opt->hot_n_rows=0;
  opt->hot_n_cols=0;
  return 0;
}


/****************************************************************************/
/**                                NUMBERS                                 **/
/****************************************************************************/

static char szNumTemp[MAX_NUM];

static void near DlgIntWrap1(HVOPT opt)
{
  if (!opt->appdatap)
  {
    opt->appdatap=(void *)opt->data;

    opt->data=szNumTemp;

    if (opt->appflag & AF_BYTE)
      sprintf(szNumTemp, "%u", *(byte *)opt->appdatap);
    else if (opt->appflag & AF_LONG)
      sprintf(szNumTemp, "%lu", *(long *)opt->appdatap);
    else sprintf(szNumTemp, "%u", *(word *)opt->appdatap);
  }
}

static void near DlgIntWrap2(HVOPT opt)
{
  if (opt->appdatap)
  {
    if (opt->appflag & AF_BYTE)
      *(byte *)opt->appdatap=atoi(opt->data);
    else if (opt->appflag & AF_LONG)
      *(long *)opt->appdatap=atol(opt->data);
    else *(word *)opt->appdatap=atoi(opt->data);

    opt->data=opt->appdatap;
    opt->appdatap=NULL;
  }
}


MenuFunction(DlgIntReg)
{
  sword ret;

  DlgIntWrap1(opt);
  ret=DlgStrReg(opt);
  DlgIntWrap2(opt);

  return ret;
}

MenuFunction(DlgIntEdit)
{
  sword ret;

  DlgIntWrap1(opt);
  ret=DlgStrEdit(opt);
  /* no DlgIntWrap2 because we want the user to finish editing before we    *
   * convert it back!                                                       */

  return ret;
}

MenuFunction(DlgIntShow)
{
  sword ret;

  /* In case the DlgIntEdit needed to be finished up */

  DlgIntWrap2(opt);

  DlgIntWrap1(opt);
  ret=DlgStrShow(opt);
  DlgIntWrap2(opt);

  return ret;
}

/****************************************************************************/
/**                                Value Sets                              **/
/****************************************************************************/

/* This is the function invoked by <esc> - it simply returns 1 so that      *
 * the menu handler will exit the current menu.                             */

static MenuFunction(TuiValKeyRet1)
{
  NW(opt);
  dlg_ok=-1;
  return 1;
}

/* Handler for the value list.  This is executed when a value-list option   *
 * is selected.  This sets the word value for this value-list to the        *
 * value for the option that the user selected.                             */

MenuFunction(TuiValHandlerWord)
{
  dlg_ok=1;
  *(word *)opt->data=((struct _val_list *)opt->appdatap)->value;
  return 1;
}

MenuFunction(TuiValHandlerByte)
{
  dlg_ok=1;
  *(byte *)opt->data=(byte)((struct _val_list *)opt->appdatap)->value;
  return 1;
}



/* Return the name of an option in a value list, sans tilde */

static byte * near _TuiValName(struct _val_list *l)
{
  static byte valname[MAX_NUM];
  byte *p=l->name;
  byte *s=valname;

  for (p=l->name, s=valname; *p; p++)
    if (*p != '~')
      *s++=*p;

  *s='\0';

  return valname;
}


/* Create a new value-list menu based on the val_list items passed to us */

static HVMENU near _TuiValCreateMenu(struct _val_list *val_list, void *data, unsigned do_byte)
{
  HVMENU v;
  HVOPT opt;
  struct _val_list *l;

  if ((v=malloc(sizeof(struct _vmenu)))==NULL)
    return NULL;

  (void)memset(v, '\0', sizeof(struct _vmenu));
  v->start_x=-1;
  v->start_y=-1;
  v->col_win=CWHITE | _BLUE;
  v->col_bor=CYELLOW | _BLUE;
  v->col_item=CWHITE | _BLUE;
  v->col_sel=CBLACK | _WHITE;
  v->col_hot=CYELLOW | _BLUE;
  v->col_selhot=CBLACK | _WHITE;
  v->border=BORDER_DOUBLE;
  v->type=MENU_HOT | MENU_VERT | MENU_SHADOW;
  v->sizex=0xffffu;
  v->sizey=0xffffu;

  /* Now loop through all of the items and add them to the menu */

  for (l=val_list, opt=v->opt; l->name; l++, opt++)
  {
    opt->name=l->name;
    opt->menufn=do_byte ? TuiValHandlerByte : TuiValHandlerWord;
    opt->cx=0xffffu;
    opt->cy=0xffffu;
    opt->data=data;
    opt->appdatap=(void *)l;

    /* Set the default for this menu, if the options match */

    if (do_byte)
    {
      if (*(byte *)opt->data==(byte)l->value)
        v->def_opt=opt - v->opt;
    }
    else
    {
      if (*(word *)opt->data==l->value)
        v->def_opt=opt - v->opt;
    }
  }

  return v;
}


/* Wrapper for the value-list class.  This takes the value list, finds the  *
 * current value, dumps it into szValTemp, and then munges the entry so     *
 * that it looks like a string field.  We then pass it off to the string    *
 * handler for displaying to the user.                                      */

static void near DlgValWrap1(HVOPT opt)
{
  struct _val_list *l;

  /* Do the wrapper bit only if we haven't already done so */

  if (!opt->appdatap)
  {
    /* Save a copy of the data pointer in appdatap, and point the           *
     * 'data' field to our static string which holds the textual value      *
     * of this item.                                                        */

    opt->appdatap=(void *)opt->data;
    opt->data=szValTemp;

    /* Default to the first item in the value list in case it's not found */

    strcpy(szValTemp, _TuiValName(opt->valdatap));

    /* However, if we find a matching value for our data item in the value  *
     * list, set the textual representation to that item.                   */

    for (l=opt->valdatap; l->name; l++)
      if (((opt->appflag & AF_BYTE) &&    (byte)l->value==*(byte *)opt->appdatap) ||
          ((opt->appflag & AF_BYTE)==0 &&       l->value==*(word *)opt->appdatap))
      {
        strcpy(szValTemp, _TuiValName(l));
      }
  }
}

/* This function undoes the mangling done in DlgValWrap1 by putting         *
 * everything back to normal and getting rid of the static string garbage.  */

static void near DlgValWrap2(HVOPT opt)
{
  if (opt->appdatap)
  {
    opt->data=opt->appdatap;
    opt->appdatap=NULL;
  }
}


/* Alternate registration for the baud rate option */

MenuFunction(DlgValReg)
{
  sword ret;

  /* Make the space key activate the menu option */

  _TuiMenuAddKey(opt, ' ', DlgValAct, NULL, NULL, 0, 0);

  /* Do the wrapper, call the string registration function, and then deinit */

  DlgValWrap1(opt);
  ret=DlgStrReg(opt);
  DlgValWrap2(opt);
  return ret;
}


MenuFunction(DlgValAct)
{
  HVKEY key;
  char temp[PATHLEN];
  HVMENU h;
  int okay;

  /* Create the value-list menu */

  h=_TuiValCreateMenu(opt->valdatap, opt->data, !!(opt->appflag & AF_BYTE));

  /* Find out how long the current menu option text is (this is on the      *
   * parent dialog, not the value menu itself.)                             */

  DlgStrName(opt, temp, NULL, NULL);

  /* Position the X coordinate so it is just to the right of the            *
   * current option location.                                               */

  h->start_x=opt->cx + opt->parent->start_x +
                (opt->parent->border != BORDER_NONE) + strlen(temp) + 1;

  /* Do the same with the Y coordinate, but put it on the same row */

  h->start_y=opt->cy + opt->parent->start_y +
                (opt->parent->border != BORDER_NONE) -
                1 - h->def_opt;

  /* Make sure it doesn't go off the top of the screen */

  if ((sword)h->start_y < 0)
    h->start_y=0;

  /* Adjust menu in case it goes off other parts of screen */

  _TuiFindMenuStartLoc(h);


  /* Now register the priv menu.  Fix the <esc> key so that it can be used  *
   * to leave the menu, even though the menu system thinks that the val     *
   * menu is a top-level menu.                                              */

  h->parent=NULL;
  key=_TuiAddEscKey(NULL);
  key->menufn=TuiValKeyRet1;
  _TuiRegisterMenu1(h, key);

  okay=TuiExecMenu(h);
  TuiDestroyMenu(h);
  free(h);

  if (okay)
    DlgValShow(opt);    /* Redisplay this option */

  return 0;
}
/*#endif*/

/* Display a priv level on-screen */

MenuFunction(DlgValShow)
{
  sword ret;

  DlgValWrap1(opt);
  ret=DlgStrShow(opt);
  DlgValWrap2(opt);

  return ret;
}


