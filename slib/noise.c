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
#include <string.h>
#include <dos.h>
#include "prog.h"

#ifdef __MSDOS__

  #if defined(__MSC__) || defined(__WATCOMC__)
    #include "conio.h"
    #define getvect(int)            _dos_getvect(int)
    #define setvect(int, func)      _dos_setvect(int, func)
    #define inportb(port)           inp(port)
    #define outportb(port, byte)    outp(port, byte)
  #endif

  #define TIMER_PORT   0x61
  #define COMMAND_REG 0x43
  #define CHANNEL_2   0x42


  /* Routine for queueing a note under the DESQview multitasker */

  void _fast dv_noise(int freq,int duration)
  {
    union REGS r;
    int ms_18;

    if (duration==0)
      ms_18=0;
    else
    {
      ms_18 = (duration/(1000/18));

      if (ms_18==0)
        ms_18=1;
    }


    #ifdef __386__
      r.w.ax=0x1019;
      r.w.bx=freq;
      r.w.cx=ms_18;

      int386(0x15,&r,&r);
    #else
      r.x.ax=0x1019;
      r.x.bx=freq;
      r.x.cx=ms_18;

      int86(0x15,&r,&r);
    #endif
  }
#endif


#if defined(OS_2)

  #define INCL_NOPM
  #include <os2.h>

  void _fast noise(int freq,int duration)
  {
    DosBeep(freq,duration);
  }

#elif defined(__MSDOS__)

  void _fast noise(int freq,int duration)
  {
    long div;

    if (freq)
    {
      outportb(TIMER_PORT,inportb(TIMER_PORT) | 0x03);
      outportb(COMMAND_REG,0xb6);
      outportb(CHANNEL_2,(unsigned)((div=1190000L/freq) & 0xff));
      outportb(CHANNEL_2,(unsigned)(div >> 8));
    }
    else outportb(TIMER_PORT,inportb(TIMER_PORT) & 0xfc);

    tdelay(duration);
  }

#elif defined(NT)

  #include "pwin.h"

  void _fast noise(int freq,int duration)
  {
    Beep((DWORD)freq, (DWORD)duration);
  }

#elif defined(UNIX)
void noise(int freq /*hz*/, int duration /*ms*/)
{
  /* hack by wes to make noise. Maybe nice noise will come some day */

  /* assumes the device is already set up for for 8khz raw 8-bit PCM */

  char oneSecond[8000 / 8];
  int  i, b;
  FILE *file;

  if (freq > 8000)
    freq = 7999;

  /* e.g. - 440 hz -- write 440 equally spaced ones into the 8000 bit second */
  for (i = 0; i < sizeof(oneSecond); i++)
  {
    for (b = 0; b < 8; b++)
    {
      if (i || b)
        if (freq % ((i * 8) + b) == 0)
	  oneSecond[i] |= 1 << b;
    }
  }

  file = fopen("/dev/audio", "a");
  if (!file)
    return;

  for (i = 0; i < duration / 1000; i++)
    fwrite(oneSecond, sizeof(oneSecond), 1, file);

  fwrite(oneSecond, duration % sizeof(oneSecond), 1, file);
  fclose(file);  
}
#else
  #error Unknown OS
#endif

