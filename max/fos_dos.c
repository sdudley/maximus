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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: fos_dos.c,v 1.3 2004/01/27 21:00:29 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=FOSSIL interface routines (DOS)
*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include "prog.h"
#include "mm.h"
#include "modem.h"

#ifdef __MSDOS__


static int fossil_installed=FALSE;



void fossil_getxy(char *row, char *col)
{
  union REGS r;

  if (fossil_installed)
  {
    r.h.ah=0x12;
#ifdef __386__    
    int386(0x14,&r,&r);
#else
    int86(0x14,&r,&r);
#endif
  }
  else /* BIOS getcurpos function */
  {
    r.h.ah=0x03;
    r.h.bh=0x00;
#ifdef __386__    
    int386(0x10,&r,&r);
#else
    int86(0x10,&r,&r);
#endif
  }

  *row=r.h.dh;
  *col=r.h.dl;
}




int fossil_inf(struct _fossil_info far *finfo)   /* Returns length of structure */
{
  union REGS regs;
  struct SREGS sregs;
  extern int cdecl port;

  if (fossil_installed)
  {
    regs.h.ah = FOSSIL_info;
    
#ifdef __386__    
    regs.w.cx = sizeof(*finfo);
    sregs.es  = FP_SEG(finfo);
    regs.w.di = FP_OFF(finfo);
    regs.w.dx = port;
    int386x(0x14, &regs, &regs, &sregs);
#else
    regs.x.cx = sizeof(*finfo);
    sregs.es  = FP_SEG(finfo);
    regs.x.di = FP_OFF(finfo);
    regs.x.dx = port;
    int86x(0x14, &regs, &regs, &sregs);
#endif
  }
  else
  {
    /* Otherwise, fill info structure using BIOS data area, and some dummy  *
     * info.                                                                */

#ifdef __386__
    regs.w.ax=sizeof(struct _fossil_info); /* Return value */
#else
    regs.x.ax=sizeof(struct _fossil_info); /* Return value */
#endif

    finfo->size=sizeof(struct _fossil_info);
    finfo->majver=5;
    finfo->minver=0;
    finfo->id=(char far *)"None";

    finfo->input_size=finfo->input_free=finfo->output_size=
      finfo->output_free=0x1000;
    
    /* BIOS data area, #cols */
    finfo->width=(char)*(int far *)MK_FP(0x0000,0x044a);
    
#if 1
    finfo->height=(char)_VidGetNumRows();
#else
    /* Length of screen buffer, divided by width of each line, and finally  *
     * divided by two (because two bytes are req'd for one char on screen.  */

    finfo->height=(char)((*(int far *)MK_FP(0x0000,0x044c)/finfo->width) >> 1);
    
    switch (finfo->height)  /* Off-by-one error */
    {
      case 26:
      case 44:
      case 51:
      case 61:
        finfo->height--;
    }
#endif

    finfo->baud=BAUD2400;
  }
  
#ifdef __386__
  return(regs.w.ax);
#else
  return(regs.x.ax);
#endif
}



#ifndef ORACLE

int mdm_deinit(void)          /* Returns 0 if closed, -1 if not open. */
{
  extern int cdecl port;

  CallFossil(FOSSIL_deinit,0);
  fossil_installed=FALSE;
  return 0;
}




int mdm_init(int prt)   /* Returns 0 upon success, 2 on no FOSSIL installed */
{
  union REGS r;

  port=prt;

  r.h.ah=FOSSIL_init;

#ifdef __386__  
  r.w.dx=prt;
  r.w.bx=0;

  int386(0x14,&r,&r);

  if (0x1954 != r.w.ax)
#else
  r.x.dx=prt;
  r.x.bx=0;

  int86(0x14,&r,&r);

  if (0x1954 != r.x.ax)
#endif
    return INIT_nofossil;
  else if (r.h.bl < 0x18 || r.h.bh < 5)
  {
    logit("!Level 5 FOSSIL req'd\n");
    return INIT_nofossil;
  }
  else
  {
    fossil_installed=TRUE;
    return INIT_ok;
  }
}




#ifdef TTYVIDEO
void _stdc fossil_putc(char chr)
{
  if (chr=='\n')
  {
    if (fossil_installed)
      CallFossil(FOSSIL_putc, '\r');
    else fputchar('\r');
  }

  if (fossil_installed)
    CallFossil(FOSSIL_putc, chr);
  else fputchar(chr);
}


void _stdc fossil_puts(char *str)
{
  while (*str)
    fossil_putc(*str++);
}
#endif /* TTYVIDEO */


int mdm_baud(int bod)     /* Returns port status */
{
  return (CallFossil(FOSSIL_baud, steady_baud ? steady_baud : bod));
}



/* Returns number of bytes actually transferred */

int mdm_blockwrite(int max_chars,char *offset)
{
  union REGS r;
  struct SREGS sr;
  extern int cdecl port;
  void far *fptr=offset;

  r.h.ah=FOSSIL_blockwrite;
  sr.es=FP_SEG(fptr);

#ifdef __386__
  r.w.cx=max_chars;
  r.w.di=FP_OFF(fptr);
  r.w.dx=port;

  int386(0x14,&r,&r);

  return(r.w.ax);
#else
  r.x.cx=max_chars;
  r.x.di=FP_OFF(fptr);
  r.x.dx=port;

  int86x(0x14,&r,&r,&sr);

  return(r.x.ax);
#endif
}


/* Returns number of bytes actually transferred */

int mdm_blockread(int max_chars, char *offset)
{
  union REGS r;
  struct SREGS sr;
  extern int cdecl port;
  void far *fptr=offset;

  r.h.ah=FOSSIL_blockread;
  sr.es=FP_SEG(fptr);

#ifdef __386__

  r.w.cx=max_chars;
  r.w.di=FP_OFF(fptr);
  r.w.dx=port;

  int386(0x14,&r,&r);

  return(r.w.ax);
#else
  r.x.cx=max_chars;
  r.x.di=FP_OFF(fptr);
  r.x.dx=port;

  int86x(0x14,&r,&r,&sr);

  return(r.x.ax);
#endif
}




void mdm_break(unsigned int hsecs)
{
  long t1=timerset(hsecs);

  CallFossil(FOSSIL_break,1);

  while (! timeup(t1))
    Giveaway_Slice();

  CallFossil(FOSSIL_break,0);
}



char mdm_dump(char buffer)
{
  if (local)
    return 0;

  switch (buffer)
  {
    case DUMP_INPUT:
      CallFossil(FOSSIL_dumpin, 0);
      break;

    case DUMP_ALL:
      CallFossil(FOSSIL_dumpin, 0);
      /* fall-thru */

    case DUMP_OUTPUT:
      CallFossil(FOSSIL_dumpout, 0);

      if (prm.flags & FLAG_break_clr)
        mdm_break(10);

      Delay(10);
      break;

    default:
      return -1;
  }

  return 0;
}

void mdm_watchdog(int watch) /* No return value */
{
  CallFossil(FOSSIL_watchdog, watch);
}



#endif /* !ORACLE */


sword mdm_getc(void)
{
  return (mdm_avail() ? mdm_ggetcw() : -1);
}



/* The "& 0x00ff" kludge is necessary because X00 is broken, and       *
 * it forgets to zero out the AH register in this call...  O!Comm and  *
 * others do it correctly, though.                                     */

word mdm_ctrlc(word mask)
{
  return (!local && (CallFossil(FOSSIL_ctrlc, mask) & 0x00ff)==0x0001);
}


word out_empty(void)
{
  return (local ? TRUE : (mdm_status() & STATUS_OUTEMPTY));
}

word mdm_avail(void)
{
  return (local ? FALSE : (mdm_status() & STATUS_ANYDATA));
}

word real_carrier(void)
{
  return (mdm_status() & prm.carrier_mask);
}

word carrier(void)
{
  if (no_dcd_check || local)
    return TRUE;

  return real_carrier();
}


#endif /* __MSDOS__ */

