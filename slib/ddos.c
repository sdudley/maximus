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

/*# name=DoubleDOS interface routines
*/

#include "prog.h"

#ifdef __MSDOS__

  #ifdef __TURBOC__

  void _fast ddos_timer(unsigned int duration)  /* Pauses for <duration> ms., while */
  {                                       /* giving away timeslice. */
    long tim;

    tim=timerset(duration);

    while (! timeup(tim))
      geninterrupt(0xf4);
  }


  void _fast ddos_priority(int priority)  /* Changes partiton priority.  Uses the */
  {                                 /* PRIORITY_xxxx defines as input.      */
    _AH=0xe9;
    _AL=priority;
    geninterrupt(0x21);
  }


  void _fast ddos_switch()                /* Makes the invisible partition visible */
  {
    _AH=0xe0;
    _AL=0x01;
    geninterrupt(0x21);
  }


  void _fast ddos_suspend()               /* Suspend the invisible partition */
  {
    _AH=0xe0;
    _AL=0x75;

    geninterrupt(0x21);
  }


  void _fast ddos_resume()                /* Unsuspend the invisible partition */
  {
    _AH=0xe0;
    _AL=0x73;

    geninterrupt(0x21);
  }


  void _fast ddos_kill()                  /* Kill the invisible partition */
  {
    _AH=0xe0;
    _AL=0x74;

    geninterrupt(0x21);
  }


  void _fast ddos_clear_vkb()      /* Kill the visible partition's keyboard buffer */
  {
    _AH=0xe1;
    _AL=0x74;

    geninterrupt(0x21);
  }


  void _fast ddos_send(char ch)    /* Insert <ch> in the other partition's keyboard */
  {                          /* buffer. */
    _AH=0xe2;
    _AL=ch;

    geninterrupt(0x21);
  }


  void _fast ddos_addkey(char ch)  /* Add a key to the current partition's buffer. */
  {
    _AH=0xe3;
    _AL=ch;

    geninterrupt(0x21);
  }


  void _fast ddos_key_disable()    /* Disable the visible partition's keyboard */
  {
    _AH=0xe8;
    _AL=0;
    _DX=32768u;

    geninterrupt(0x21);
  }


  void _fast ddos_key_enable()    /* Enable the visible partition's keyboard */
  {
    _AH=0xe8;
    _AL=0;
    _DX=16383;

    geninterrupt(0x21);
  }


  int _fast ddos_herestat(void) /* Gives info. about current partition.  HERESTAT_xxxx */
  {
    char temp;

    _AH=0xe4;
    geninterrupt(0x21);

    temp=_AL;

    switch(temp)
    {
      case 1:
      case 2:
        return temp;

      default:
        return 0;
    }
  }


  int _fast ddos_flipstat(void) /* Gives info. about other partition.  FLIPSTAT_xxxx */
  {
    char temp;

    _AH=0xe5;

    geninterrupt(0x21);

    temp=_AL;

    switch (temp)
    {
      case 1:
      case 2:
        return temp;

      default:
        return 0;
    }
  }


  void _fast ddos_funcs_enable()    /* Enable DoubleDOS-specific keys */
  {
    _AH=0xe0;
    _AL=0;
    _DX=0xffef;     /* 1111 1111  1110 1111 */

    geninterrupt(0x21);
  }


  void _fast ddos_funcs_disable()   /* Disable DoubleDOS-specific keys */
  {
    _AH=0xe8;
    _AL=0;
    _DX=0xfdcc;     /* 1111 1101  1100 1100 */
  }


  #endif /* __TURBOC__ */
#endif /* __MSDOS__ */

