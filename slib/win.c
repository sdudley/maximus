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
#include <dos.h>
#include <stdlib.h>
#include <stdarg.h>
#include "prog.h"
#include "alc.h" /* needed for farmalloc et al */
/*#include "max.h"*/
#include "dv.h"
#include "win.h"

#ifndef __MSDOS__
  #define Vid_Bios FALSE
#endif

extern unsigned int near Vid_Segment;

static VWIN *head=NULL;
static VWIN *tail=NULL;

void (_fast *WinHideFunc)(void)=NULL;
void (_fast *WinShowFunc)(void)=NULL;
VWIN *wscrn=NULL;

static char *boxc[]={"           ",
                     "Ä³ÚÂ¿ÃÅ´ÀÁÙ",
                     "ÍºÉË»ÌÎ¹ÈÊ¼",
                     "ÄºÖÒ·Ç×¶ÓÐ½",
                     "Í³ÕÑ¸ÆØµÔÏ¾"};

static void near _WinUpdateShadows(VWIN *head);
static int near _fast WinUpdateOK(VWIN *current,int Row,int Col);
static void near _fast WinConstructBlitz(VWIN *cur);
static void near _WinSyncCursor(VWIN *win);

#define ValidWin(w)         ((w) && (w)->id==VWIN_ID)


int _fast WinTitle(VWIN *win,char *title,int location)
{
  byte save;

  NW(location);
  
  if (!ValidWin(win))
    return -1;

  win->bdirty=TRUE;

  save=WinGetAttr(win);

  WinSetAttr(win,win->battr);
  WinPutstr(win,-1,1,title);
  WinSetAttr(win,save);

  WinSync(win,FALSE);
  
  return 0;
}

void _fast WinSyncAll(void)
{
  VWIN *w;
  word sflag;
  
  for (w=head; w; w=w->next)
  {
    sflag=w->flag;
    w->flag &= ~WIN_SHADOW;

    WinDirtyAll(w);
    WinSync(w, FALSE);

    w->flag=sflag;
  }

  _WinUpdateShadows(head);
}

void _fast WinToTop(VWIN *win)
{
  VWIN *last;
  VWIN *w;

  if (!ValidWin(win))
    return;

  if (head==win)    /* No need to do anything if already at the top */
    return;

  for (w=head;w->next && w->next != win;w=w->next)
    ;

  if (! w->next)        /* Couldn't find this window?!? */
    return;

  if (tail==win)        /* If we were removed as the last window, then  */
    tail=w;             /* point the tail to the second-last item.      */

  w->next=win->next;    /* Remove specified window from linked list */

  last=head;            /* Save the original top window */
  head=win;             /* Set the new top window to this one */
  win->next=last;       /* Make the original top window the 2nd-top window */

  WinUpdateUnder(head);
}

void _fast WinToBottom(VWIN *win)
{
  VWIN *w;

  if (!ValidWin(win))
    return;

  if (win==tail)
    return;

  if (win==head)    /* If we're at the top, just transfer appropriately. */
  {
    tail->next=win;
    head=win->next;
    win->next=NULL;
    return;
  }
  else
  {
    for (w=head;w->next && w->next != win;w=w->next)
      ;

    if (! w->next)        /* Couldn't find this window?!?  exit. */
      return;

    w->next=win->next;    /* Remove specified window from linked list */

    if (tail)
      tail->next=win;       /* Add us onto the end */
    /* else what the fuck happened? */

    win->next=NULL;       /* And say that nobody is after us */
    tail=win;
  }

  WinUpdateUnder(head);
}


void _fast WinDirtyAll(VWIN *win)
{
  word x;

  if (!ValidWin(win))
    return;

  win->bdirty=TRUE;

  for (x=0; x < win->s_height; x++)
    WinSetDirty(win,x,-1);
}

void _fast WinSetDirty(VWIN *win,int Row,int Col)
{
  if (!ValidWin(win))
    return;

  if (Col==-1)
  {
    win->dleft[Row]=0;
    win->dright[Row]=win->s_width-1;
    return;
  }
  else if (Col==-2)
  {
    if (win->dleft[Row]==-1 || win->dleft[Row] > (sword)win->col)
      win->dleft[Row]=win->col;

    win->dright[Row]=win->s_width-1;
    return;
  }
  
  if (win->dleft[Row]==-1 || win->dleft[Row] > Col)
    win->dleft[Row]=Col;

  if (win->dright[Row]==-1 || win->dright[Row] < Col)
    win->dright[Row]=Col;
}

VWIN * cdecl WinMsg(int border,int attr,int battr,...)
{
  char *s;
  VWIN *win;
  va_list vargs;
  int max;
  int cnt;
  int x;

  va_start(vargs,battr);

  for (max=cnt=0;(s=va_arg(vargs,char *)) != NULL;cnt++)
  {
    if ((x=strlen(s)) > max)
      max=x;
  }

  va_end(vargs);

  win=WinOpen(0, 0, 2+cnt, max+4, border, attr, battr,
              WIN_CENTRE | WIN_NOCSYNC | WIN_NODRAW);

  if (!win)
    return NULL;

  va_start(vargs,battr);

  for (x=0;(s=va_arg(vargs,char *)) != NULL && x < cnt;x++)
    WinCenter(win,x,s);

  va_end(vargs);

  WinSync(win,FALSE);
  return win;
}

void _fast WinCenter(VWIN *win,int Row,char *s)
{
  if (!ValidWin(win))
    return;

  WinPutstr(win,Row,(win->s_width-strlen(s)) >> 1,s);
}

void _fast WinPutstra(VWIN *win,int Row,int Col,int attr,char *s)
{
  byte save;

  if (!ValidWin(win))
    return;

  save=WinGetAttr(win);

  WinSetAttr(win, (byte)attr);
  WinPutstr(win,Row,Col,s);
  WinSetAttr(win,save);
}

void _fast WinPutstr(VWIN *win,int Row,int Col,char *s)
{
  int srow, scol;

  if (!ValidWin(win))
    return;

  scol=win->col;
  srow=win->row;

  WinGotoXY(win,Row,Col,FALSE);

  while (*s)
    WinPutc(win,*s++);

  WinGotoXY(win,srow,scol,FALSE);
}

void _fast WinPuts(VWIN *win,char *s)
{
  while (*s)
    WinPutc(win,*s++);
}

void _fast WinPutsA(VWIN *win,char *s)
{
  while (*s)
    WinPutcA(win,*s++);
}

void _fast WinPutcA(VWIN *win,byte ch)
{
  if (!ValidWin(win))
    return;

  switch (win->avtstate)
  {
    case -1:
      if (ch==22)       /* AVATAR sequence */
        win->avtstate=0;
      else if (ch==25)  /* RLE */
        win->avtstate=25;
      else WinPutc(win,ch);
      break;

    case 0:
      switch(ch)
      {
        case 1:     /* AVATAR attribute change.  Grab 'nother character */
          win->avtstate=1;
          break;
          
        case 2:     /* Turn on BLINK */
          WinSetAttr(win,(byte)(win->attr | _BLINK));
          win->avtstate=-1;
          break;
          
        case 3:     /* Up one row */
          WinGotoXY(win,WinGetRow(win)-1,WinGetCol(win),FALSE);
          win->avtstate=-1;
          break;
          
        case 4:     /* Down one row */
          WinGotoXY(win,WinGetRow(win)+1,WinGetCol(win),FALSE);
          win->avtstate=-1;
          break;
          
        case 5:     /* Left one col */
          WinGotoXY(win,WinGetRow(win),WinGetCol(win)-1,FALSE);
          win->avtstate=-1;
          break;
          
        case 6:     /* Right one col */
          WinGotoXY(win,WinGetRow(win),WinGetCol(win)+1,FALSE);
          win->avtstate=-1;
          break;
          
        case 7:     /* CLEOL */
          WinCleol(win,WinGetRow(win),WinGetCol(win),WinGetAttr(win));
          win->avtstate=-1;
          break;
          
        case 8:     /* Goto */
          win->avtstate=8;
          break;

        case '\x6f':
          win->avtstate='\x6f';
          break;

        default:
          win->avtstate=-1;
      }
      break;        /* case 0 */
      
    case 1:         /* Attribute */

      if (win->lockback)            /* If the background colour is locked. */
      {
        ch &= 0x8f;                 /* Chop off old bkg (mask=10001111) */
        ch |= (win->lockback << 4); /* Add new colour */
      }

      WinSetAttr(win,ch);
      win->avtstate=-1;
      break;
      
    case 8:         /* Goto 1 */
      win->rsvd1=ch;
      win->avtstate=9;
      break;
      
    case 9:         /* Goto 2 */
      WinGotoXY(win,win->rsvd1-1,ch-1,FALSE);
      win->avtstate=-1;
      break;
      
    case 25:          /* RLE 1 */
      win->rsvd1=ch;
      win->avtstate=26;
      break;
      
    case 26:          /* RLE 2 */
      while (ch--)
        WinPutc(win,win->rsvd1);
      
      win->avtstate=-1;
      break;

    case '\x6f':
      win->lockback=ch;
      win->avtstate=-1;
      break;
  }
}

static void near _WinWrapIt(VWIN *win)
{
  if ((sword)win->row >= (sword)win->s_height)
  {
    win->row=win->s_height-1;
    WinScrl(win, SCROLL_up, 0, win->s_height-1, win->attr);
  }
}



void _fast WinPutc(VWIN *win,byte Char)
{
  static sword *dl=NULL, *dr=NULL;
  static VWIN *wold=NULL;
  static long lastctr=-1L;
  static sword lastr=-1;
  static sword lastc=-1;


  if (!ValidWin(win))
    return;

  if (Char < 32)
  {
    if (Char==10)
    {
      win->row++;
      _WinWrapIt(win);
      return;
    }
    else if (Char==13)
    {
      win->col=0;
      return;
    }
    else if (Char==8)
    {
      if (win->col)
        win->col--;

      return;
    }
  }

  /* Scroll the screen down a line, if necessary */

  if ((sword)win->row >= (sword)win->s_height)
    _WinWrapIt(win);

  if (wold != win || lastctr != win->ctr || lastr != (sword)win->row)
  {
    wold=win;
    lastr=win->row;
    lastctr=win->ctr;

    dl=&win->dleft[lastr];
    dr=&win->dright[lastr];
  }

  lastc=win->col;

  if (*dl==-1 || *dl > lastc)
    *dl=lastc;

  if (*dr==-1 || *dr < lastc)
    *dr=lastc;

  *WinOfs(win,win->row,lastc)=(win->attr << 8) | Char;

  if (++win->col < win->s_width)
    return;

  win->col=0;
  win->row++;

  /*_WinWrapIt(win);*/
}


void _fast WinScrl(VWIN *win,int dir,int srow,int erow,int attr)
{
  char far *rptr;
  sword x;
  
  if (!ValidWin(win))
    return;

  if (dir==SCROLL_down)
  {
    rptr=win->rowtable[erow];

    for (x=erow; x > srow; x--)
    {
      win->rowtable[x]=win->rowtable[x-1];
      WinSetDirty(win,x,-1);
    }

    win->rowtable[srow]=rptr;

    WinCleol(win, srow, 0, (byte)attr);
  }
  else
  {
    rptr=win->rowtable[srow];

    for (x=srow; x < erow; x++)
    {
      win->rowtable[x]=win->rowtable[x+1];
      WinSetDirty(win,x,-1);
    }

    win->rowtable[erow]=rptr;

    WinCleol(win, erow, 0, (byte)attr);
  }
}

void _fast WinSetAttr(VWIN *win,byte Attr)
{
  if (!ValidWin(win))
    return;

  win->attr=Attr;
}

void _fast WinCleol(VWIN *win,int Row,int Col,byte Attr)
{
  word colsave;
  word y;

  if (!ValidWin(win))
    return;

  _WinWrapIt(win);

  if ((sword)Row >= (sword)win->s_height)
    Row=win->s_height-1;

  for (y=Col; y < win->s_width; y++)
    *WinOfs(win,Row,y)=(Attr << 8) | ' ';

  /* Set our current column so that WinSetDirty will do it right */

  colsave=win->col;
  win->col=(word)Col;

  WinSetDirty(win,Row,-2);

  /* Restore column */

  win->col=colsave;
}

void _fast WinPutch(VWIN *win,int Row,int Col,byte Char,byte Attr)
{
  *WinOfs(win, Row, Col)=((Attr << 8) | Char);
  WinSetDirty(win, Row, Col);
}

void _fast WinVline(VWIN *win, int srow, int scol, int erow, int border, byte attr)
{
  int x;
  
  if (!ValidWin(win))
    return;

  for (x=srow;x <= erow;x++)
    WinPutch(win, x, scol, boxc[border][1], attr);
}

void _fast WinHline(VWIN *win,int srow,int scol,int ecol,int border,byte attr)
{
  int x;

  if (!ValidWin(win))
    return;

  for (x=scol; x <= ecol; x++)
    WinPutch(win, srow, x, boxc[border][0], attr);
}


void _fast WinBox(VWIN *win,int srow,int scol,int erow,int ecol,int border,byte attr)
{
  if (!ValidWin(win))
    return;

  WinHline(win, srow,   scol+1, ecol-1, border, attr);
  WinHline(win, erow,   scol+1, ecol-1, border, attr);
  WinVline(win, srow+1, scol,   erow-1, border, attr);
  WinVline(win, srow+1, ecol,   erow-1, border, attr);
  
  WinPutch(win, srow, scol, boxc[border][2],  attr);
  WinPutch(win, srow, ecol, boxc[border][4],  attr);
  WinPutch(win, erow, scol, boxc[border][8],  attr);
  WinPutch(win, erow, ecol, boxc[border][10], attr);
}


void _fast WinGotoXY(VWIN *win,int row,int col,int do_sync)
{
  sword dec=(win->border != BORDER_NONE);

  if (!ValidWin(win))
    return;

  if ((sword)row < (sword)win->s_height)
    win->row=row;

  if ((sword)col < (sword)win->s_width)
    win->col=col;

  if ((sword)col < 0-dec)
    win->col=0;
  
  if ((sword)row < 0-dec)
    win->row=0;

  if (do_sync)
    _WinSyncCursor(win);  /* WinSync(win, TRUE); */

  #if 0
  VidGotoXY(win->s_col+win->col+1, win->s_row+win->row+1, TRUE);
  #endif
}

int _fast WinWhereX(VWIN *win)
{
  return win->row;
}

int _fast WinWhereY(VWIN *win)
{
  return win->col;
}


void _fast WinCls(VWIN *win,byte attr)
{
  #ifndef OS_2
  word done_now=FALSE;
  #endif

  word x, y;
  word theword, newwid;

  if (!ValidWin(win))
    return;

  /* If using the video bios mode, don't defer the screen clear until a     *
   * WinSync(), since it's too slow...                                      */

#ifdef __MSDOS__
  if (Vid_Bios)
  {
    for (x=0; x < win->s_height; x++)
      if (!win->blitz[x])
        break;

    if (x==win->s_height)
    {
      done_now=TRUE;

      VidScroll(SCROLL_up,0,
                win->attr,
                (char)win->s_col,
                (char)win->s_row,
                (char)(win->s_col+win->s_width-1),
                (char)(win->s_row+win->s_height-1));
    }
  }
#endif

  theword=((attr << 8) | ' ');
  newwid=win->s_width-1;

  for (x=0; x < win->s_height; x++)
  {
    for (y=0; y < win->s_width;)
      *WinOfs(win, x, y++)=theword;

#ifndef OS_2
    if (done_now)
      win->dleft[x]=win->dright[x]=-1;
    else
#endif
    {
      win->dleft[x]=0;
      win->dright[x]=newwid;
    }
  }

  WinGotoXY(win, 0, 0, FALSE);
}


static void near WinDrawShadow(VWIN *win, unsigned check)
{
  int maxrow=win->s_row+win->s_height+!!(win->border);
  int maxcol=win->s_col+win->s_width+!!(win->border);
  word row, col;

  for (row=maxrow, col=win->s_col+1+!(win->border); (sword)col <= maxcol; col++)
    if (!check || WinUpdateOK(win, row, col))
      VidPutch(row, col, (byte)(VidGetch(row, col) & 0xff), win->shadattr);

  for (row=win->s_row+!(win->border), col=maxcol; (sword)row <= maxrow; row++)
    if (!check || WinUpdateOK(win, row, col))
      VidPutch(row, col, (byte)(VidGetch(row, col) & 0xff), win->shadattr);
}


void _fast WinSync(VWIN *win,int sync_cursor)
{
  sword x,y;
  sword chattr;
  sword max;
  word this_row;
  word this_col;
  word flag;
  sword left, right;
  word hidden=FALSE;

  if (!ValidWin(win))
    return;

  if (win->bdirty && win->border)
  {
    max=win->s_height+1;

    /* Update rows as being dirty */
    for (x=-1;x < max;x++)
    {
      win->dleft[x]=-1;
      win->dright[x]=win->s_width;
    }

    flag=TRUE;
    x=-1;
  }
  else
  {
    x=0;
    max=win->s_height;
    flag=FALSE;
  }

  for (;x < max;x++)
  {
    /* If the row has been updated... */

    if (flag || win->dleft[x] != -1)
    {
      /* Hide the mouse cursor, if necessary */

      if (!hidden)
      {
        hidden=TRUE;

        if (WinHideFunc)
          (*WinHideFunc)();
      }

      this_row=win->s_row+x;

      if (!Vid_Bios && /*x >= 0 &&*/ x < (sword)win->s_height+(win->border != BORDER_NONE) && win->blitz[x])
      {
        _WinBlitz(win->s_col+win->dleft[x],
                  win->dright[x]-win->dleft[x]+1,
                  win->rowtable[x],
                  win->dleft[x],
                  this_row);
      }
      else
      {
        y=left=win->dleft[x];
        right=win->dright[x];
        this_col=win->s_col+left;

        /* If we can do a fast update */

        if (x >= 0 && x < (sword)win->s_height && win->blitz[x])
        {
          while (y <= right)
          {
            chattr=*WinOfs(win,x,y++);

            VidPutch(this_row, this_col++,
                     (byte)(chattr & '\xff'), (byte)(chattr >> 8));
          }
        }
        else
        {
          while (y <= right)
          {
            if (WinUpdateOK(win,this_row,this_col))
            {
              chattr=*WinOfs(win,x,y);
              VidPutch(this_row,this_col, (byte)(chattr & '\xff'),
                       (byte)(chattr >> 8));
            }

            y++;
            this_col++;
          }
        }
      }

      win->dleft[x]=win->dright[x]=-1;
    }
  }

  win->bdirty=FALSE;


  /* And synchronize the cursor position */

  if (sync_cursor && (win->flag & WFLAG_NOCUR)==0)
    _WinSyncCursor(win);


  /* If we actually changed anything, update the virtual screen buf too */

  if (hidden)
  {
    #ifdef __MSDOS__
    VidSyncDV();
    #endif

    /* Update the shadow, if necessary */

    if (win->flag & WIN_SHADOW)
      WinDrawShadow(win, FALSE);

    /* Now restore the mouse cursor, if necessary */

    if (WinShowFunc)
      (*WinShowFunc)();
  }
}



static void near _WinSyncCursor(VWIN *win)
{
  if (WinUpdateOK(win, win->s_row+win->row, win->s_col+win->col))
    VidGotoXY(win->s_col+win->col+1, win->s_row+win->row+1, TRUE);
  else VidHideCursor();
}


/* Place the cursor in the appropriate window */

static void _fast near _WinPlaceCursor(void)
{
  VWIN *w;
  
  /* Hide the cursor to start with */

  VidHideCursor();
    
  /* Now place it in the first non-hidden window that we come to */

  for (w=head; w; w=w->next)
    if ((w->flag & WFLAG_NOCUR)==0)
    {
      _WinSyncCursor(w); /*WinSync(w, TRUE);*/
      break;
    }
}





VWIN * _fast WinOpen(int row,int col,int height,int width,int border,int attr,int battr,int flag)
{
  static long int winctr=1L;
  VWIN *win;
  VWIN *w;
  int x,y;

  if (flag & WIN_CENTRE)
  {
    row=((VidNumRows()-height) >> 1);
    col=(VidNumCols()-width) >> 1;
  }

  if ((win=malloc(sizeof(VWIN)))==NULL)
    return NULL;

  memset(win,'\x00',sizeof(VWIN));

  if ((win->rowtable=win->orig_rt=
                       (byte far **)malloc(height*sizeof(char far *)))==NULL)
  {
    free(win);
    return NULL;
  }
  
  if ((win->buf=farmalloc((height+1)*(width+2)*sizeof(int)))==NULL)
  {
    free(win->orig_rt);
    free(win);
    return NULL;
  }

  if ((win->dleft=win->orig_dl=malloc((height+2)*sizeof(sword)))==NULL ||
       (win->dright=win->orig_dr=malloc((height+2)*sizeof(sword)))==NULL ||
       (win->blitz=win->orig_blitz=malloc((height+2)*sizeof(sword)))==NULL)
  {
    if (win->dleft)
    {
      free(win->dleft);
      
      if (win->dright)
        free(win->dright);
    }
    
    farfree(win->buf);
    free(win->orig_rt);
    free(win);
    return NULL;
  }

  memset(win->dleft,(signed char)-1,height+2);
  memset(win->dright,(signed char)-1,height+2);
  memset(win->blitz,'\x01',height+2);

  win->id=VWIN_ID;
  win->ctr=winctr++;
  win->s_row=win->b_row=row;
  win->s_col=win->b_col=col;
  win->s_width=win->b_width=(word)width;
  win->s_height=win->b_height=(word)height;
  win->attr=(byte)attr;
  win->battr=(byte)battr;
  win->border=border;
  win->avtstate=-1;
  win->lockback=0;

  for (x=0;x < height;x++)
    win->rowtable[x]=(char far *)((char far *)win->buf+(width*x*2));


  if (flag & WIN_INHERIT) /* Inherit underlying text (transparent) */
  {
    for (x=0;x < height;x++)
    {
      for (y=0; y < width; y++)
        *WinOfs(win,x,y)=VidGetch((byte)(row+x), (byte)(col+y));

      WinSetDirty(win,x,-1);
    }
  }
  else
  {
    WinCls(win, (byte)attr);
  }

  win->bdirty=TRUE;

  if (win->border)
  {
    WinBox(win, 0, 0, (byte)(win->s_height-1), (byte)(win->s_width-1),
           border, (byte)battr);
  
    win->s_col++;
    win->s_row++;
    win->s_width -= 2;
    win->s_height -= 2;

    win->dleft++;
    win->dright++;
    win->blitz++;

    for (x=0;x < height;x++)
      win->rowtable[x] += 2;

    win->rowtable++;  /* Bump past original border */

  }

  if (head==NULL)
    tail=win;

  win->next=head;
  head=win;

  WinConstructBlitz(win);
  
  if ((flag & WIN_NODRAW)==0)
    WinSync(win, (flag & WIN_NOCSYNC) ? FALSE : TRUE);

  if (flag & WIN_NOCSYNC)
  {
    win->flag |= WFLAG_NOCUR;
    _WinPlaceCursor();
  }

  /* Reconstruct the blitz table for all underlying windows */

  for (w=win->next; w; w=w->next)
    WinConstructBlitz(w);

  return win;
}

void _fast WinCursorHide(VWIN *win)
{
  win->flag |= WFLAG_NOCUR;
  _WinPlaceCursor();
}

void _fast WinCursorShow(VWIN *win)
{
  win->flag &= ~WFLAG_NOCUR;
  _WinPlaceCursor();
}

void _fast WinClose(VWIN * win)
{
  VWIN *w;

  if (ValidWin(win))
  {
    /* Free up used memory */

    free(win->orig_blitz);
    free(win->orig_dl);
    free(win->orig_dr);
    farfree(win->buf);

    free(win->orig_rt);

    /* Now walk the list of windows to remove the current node */

    if (win==head)    /* That was easy, since we're at the top! */
      head=win->next;
    else
    {
      w=head;                /* Start at the beginning */

      /* Loop until we find one which points to this */
      
      while (w->next && w->next != win) 
        w=w->next;

      /* If we found another window, then point the current window to it. */

      if (w->next)
        w->next=win->next;
    }


    /* Now update the tail pointer */

    if (tail==win)
    {
      for (w=head; w && w->next; w=w->next)
        ;

      tail=w;
    }
    
    /* Update everything under the window he just closed */

    WinUpdateUnder(win);

    /* Update the first window too */

    if (head)
    {
/*      WinSync(head, FALSE);*/
      _WinPlaceCursor();
    }

    win->id=0L;
    free(win);
  }
}


/* The window 'w' was just removed from the screen, and this function       *
 * redisplays the other windows as necessary, to restore the screen to      *
 * its original state.                                                      */

void _fast WinUpdateUnder(VWIN *w)
{
  VWIN *h;
  word b_col, b_width;
  word b_row, b_height;
  word sflag;
  
  if (!ValidWin(w))
    return;

  b_col=w->b_col;
  b_width=w->b_width+!!(w->flag & WIN_SHADOW);
  b_row=w->b_row;
  b_height=w->b_height+!!(w->flag & WIN_SHADOW);

  for (h=head; h; h=h->next)
  {
    /* Reconstruct the blitz table for all underlying windows */

    WinConstructBlitz(h);

    if (h->b_col+h->b_width >= b_col && h->b_col <= b_col+b_width)
    {
      if (h->b_row+h->b_height >= b_row && h->b_row <= b_row+b_height)
      {
        sword row, endrow;
        sword left, right;

        if (b_row < h->b_row)
          row=0;
        else row = b_row - h->b_row;

        endrow=b_row + b_height - h->b_row;
        endrow=min((word)endrow, h->b_height);

        if (b_col < h->b_col)
          left=0;
        else left=b_col - h->b_col;

        right=b_col + b_width - h->b_col;
        right=min((word)right, h->b_width-1);

        if (w->border)
          h->bdirty=TRUE;

        for (; row <= endrow; row++)
        {
          sword *dl=&h->dleft[row];
          sword *dr=&h->dright[row];

          if (*dl != -1 || *dr != -1)
          {
            *dl=0;
            *dr=h->b_width;
          }
          else
          {
            *dl=left;
            *dr=right;
          }
        }

        /* We draw shadows later, so make sure that we don't try it now! */

        sflag=h->flag;
        h->flag &= ~WIN_SHADOW;

/*        WinDirtyAll(h); */ /*SJD Fri  07-24-1992  18:56:40 */
        WinSync(h, FALSE);

        h->flag=sflag;
      }
    }
  }

  /* Now update shadows, if necessary */

  _WinUpdateShadows(head);
}


/* Do the shadow updating */

static void near _WinUpdateShadows(VWIN *head)
{
  VWIN *h;
  unsigned did_shadow;

  did_shadow=FALSE;

  for (h=head; h; h=h->next)
    if (h->flag & WIN_SHADOW)
    {
      if (did_shadow==FALSE && WinHideFunc)
        (*WinHideFunc)();

      WinDrawShadow(h, TRUE);

      did_shadow=TRUE;
    }

  if (did_shadow)
  {
    if (WinShowFunc)
      (*WinShowFunc)();

  #ifdef __MSDOS__
    VidSyncDV();
  #endif
  }
}





/* Walk the list of windows.  If we find any windows higher up in the list  *
 * which occupy the point we want to write to, then return FALSE, since we  *
 * don't want to overwrite that portion of the screen.  However, if we      *
 * maange to get to 'h==current' (the next window on the list is the        *
 * current window), then it shows that we're the "highest" window using     *
 * this position on the list, so it's okay to write to that screen location.*
 * We should never get to 'h==NULL', but just in case, there are a few      *
 * safeguards here.                                                         */

static int near _fast WinUpdateOK(VWIN *current,int Row,int Col)
{
  VWIN *h;
  word row, col;

  row=Row;
  col=Col;

  if (!ValidWin(current))
    return FALSE;

  for (h=head; h && h != current; h=h->next)
  {
    if (row < h->b_row || row >= h->b_row+h->b_height)
      continue;
    else if (col < h->b_col || col >= h->b_col+h->b_width)
      continue;
    else return FALSE;
  }

  return TRUE;
}



static void near _fast WinConstructBlitz(VWIN *cur)
{
  VWIN *h;
  word row, max, sub;


  if (!ValidWin(cur))
    return;

  sub=(cur->border != BORDER_NONE);

  for (row=cur->s_row-sub, max=cur->s_row+cur->s_height+sub; row < max; row++)
  {
    /* Scan all of the windows.  Assume that each row can be blitzed        *
     * at the beginning, unless the search through the linked list          *
     * of windows shows that another window is on top of that row           *
     * somewhere, in which case we make a note that the row                 *
     * can't be blitzed.                                                    */

    /* Assume it's OK to blitz */
    cur->blitz[(sword)row-(sword)cur->s_row]=TRUE;

    /* Now scan each window in the linked list... */

    for (h=head; h && h != cur; h=h->next)
    {
      /* If it conflicts with the current window... */

      if (! (row < h->b_row || row >= h->b_row+h->b_height))
      {
        /* Then mark it as such, and get outta' the loop. */

        cur->blitz[(sword)row-(sword)cur->s_row]=FALSE;
        break;
      }
    }
  }
}


#ifdef NEVER

/* Copy a string to and from the far heap */

static void near _fast fmemmove(char far *to,char far *from,int n)
{
  while (n--)
    *to++=*from++;
}

void _fast WinScrl(VWIN *win,int dir,int srow,int scol,int erow,int ecol,int attr)
{
  int x;
  
  if (!ValidWin(win))
    return;

  if (dir==SCROLL_down)
  {
    for (x=erow;x > srow;x--)
    {
      fmemmove((char far *)WinOfs(win,x,scol),
               (char far *)WinOfs(win,x-1,scol),
               (ecol-scol+1) << 1);

      WinSetDirty(win,x,-1);
    }

    WinCleol(win,srow,0,attr);
  }
  else
  {
    for (x=srow;x < erow;x++)
    {
      fmemmove((char far *)WinOfs(win,x,scol),
               (char far *)WinOfs(win,x+1,scol),
               (ecol-scol+1) << 1);

      WinSetDirty(win,x,-1);
    }

    WinCleol(win, erow, 0, (byte)attr);
  }
}

#endif





static int save_col, save_row;
static sword save_scrn=FALSE;
static sword close_vid=FALSE;

/* Initialize the windowing API */

void _fast WinApiOpen(int save)
{
  save_scrn=save;
  
  /* Init the video package, if we haven't done so alredy */

  close_vid=VidOpen(FALSE, TRUE, FALSE);

  
  /* Save current screen coordinates */

  if (save)
    VidGetXY(&save_col, &save_row);

  #if defined(__WATCOMC__) && defined(__NEARDATA__)

  /* Expand the data segment to 64K when running in medium/small memory     *
   * models, since doing a farmallo() with WATCOM will fix the size         *
   * of the data segment at whatever it is currently at.                    */

  _heapgrow();

  #endif

  if (save)
    wscrn=WinOpen(0, 0, VidNumRows(), VidNumCols(), 0, 
                  CGRAY, CGRAY, WIN_INHERIT);
}

void _fast WinApiClose(void)
{
  if (wscrn && save_scrn)
    WinClose(wscrn);

  if (save_scrn)
    VidGotoXY(save_col, save_row, TRUE);

  if (close_vid)
    VidClose();

  wscrn=NULL;
  save_scrn=FALSE;
}


void _fast WinShadow(VWIN *w, byte attr)
{
  if (attr==SHADOW_NONE)
    w->flag &= ~WIN_SHADOW;
  else
  {
    w->flag |= WIN_SHADOW;
    w->shadattr=attr;
  }

  w->bdirty=TRUE;
}


void _stdc WinPrintf(VWIN *win, char *format, ...)
{
  char out[120];
  va_list var_args;

  va_start(var_args, format);
  vsprintf(out, format, var_args);
  va_end(var_args);

  WinPuts(win, out);
}
