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

/***************************************************************************
 *  mcroclck.c -- Microsecond resolution clock routine.                    *
 *  Written by David L. Fox.                                               *
 *  Implements in C the timer chip tweaking described by                   *
 *  Byron Sheppard, _Byte_, Jan 1987, p 157-164.                           *
 *  Replaces standard clock() from the library.                            *
 *  The definition of CLK_TCK in time.h may have to                        *
 *  be changed to 1000000L.                                                *
 *  Does not correctly handle intervals spanning                           *
 *  midnight or intervals greater than about 6 hrs.                        *
 ***************************************************************************/

#define _EXT_CLOCK_FUNC

#include <stdio.h>
#include <time.h>
#include "compiler.h"
 
/* Interrupt handling and i/o ports are compiler dependent.  */
/* The following set of preprocessor directives selects the  */
/* correct include files and macros for various compilers.   */

#if defined(__ZTC__)
  #include    <dos.h>
  #include    <int.h>
  #define     inportb     inp
  #define     outportb    outp
#elif defined(__TURBOC__)
  #include    <dos.h>
  #define     int_off     disable
  #define     int_on      enable
#elif defined(__MSC__) || defined(__WATCOMC__)
  #include    <dos.h>
  #include    <conio.h>
  #undef      inportb
  #undef      outportb
  #define     inportb(p)  (unsigned char)inp(p)
  #define     outportb    outp
  #define     int_off     _disable
  #define     int_on      _enable
#else
  #error Unknown compiler
#endif

#include "prog.h"

/* Constants */
#define CONTVAL   0x34   /* == 00110100 Control byte for 8253 timer. */
                    /* Sets timer 0 to 2-byte read/write, mode 2, binary.
*/#define T0DATA    0x40    /* Timer 0 data port address. */
#define TMODE     0x43    /* Timer mode port address. */
#define BIOS_DS   0x40    /* BIOS data segment. */
#define B_TIKP    0x6c    /* Address of BIOS (18.2/s) tick count. */
#define SCALE    10000    /* Scale factor for timer ticks. */


/* The following values assume 18.2 BIOS ticks per second resulting from
 the 8253 being clocked at 1.19 MHz. */
#define us_BTIK  54925L    /* Micro sec per BIOS clock tick. */
#define f_BTIK    4595    /* Fractional part of micro sec per BIOS tick. */
#define us_TTIK   8381    /* Micro sec per timer tick * SCALE. (4/4.77 MHz)
*/ 
clock_t microclock(void)
{
  unsigned char msb, lsb;
  unsigned int tim_ticks;
  static int init = 0;
  unsigned long count, us_tmp;
  static unsigned long init_count;
 
  if (0 == init)
  {
    init = 1;     /* This is the first call, have to set up timer. */
    int_off();
    outportb(TMODE, CONTVAL);   /* Write new control byte to timer. */
    outportb(T0DATA, 0);        /* Initial count = 0 = 65636. */
    outportb(T0DATA, 0);
    init_count = *(unsigned long int far *)MK_FP(BIOS_DS, B_TIKP);
    int_on();
    return 0;                   /* First call returns zero. */
  }

  int_off();    /* Don't want an interrupt while getting time. */
  outportb(TMODE, 0);               /* Latch count. */
  lsb = inportb(T0DATA);            /* Read count. */
  msb = inportb(T0DATA);
  /* Get BIOS tick count (read BIOS ram directly for speed and
      to avoid turning on interrupts). */
  count =  *(unsigned long far *)MK_FP(BIOS_DS, B_TIKP) - init_count;
  int_on();                   /* Interrupts back on. */
  tim_ticks = (unsigned)-1 - ((msb << 8) | lsb);
  us_tmp = count*us_BTIK;
  return us_tmp + ((long)tim_ticks*us_TTIK + us_tmp%SCALE)/SCALE;
}

