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

#ifdef OS_2

struct _sbinfo
{
  int use_sb;
  HMODULE hmod;
  USHORT usOldPrty;

  void (far _fast *pfnOldNoise)(int, int);

#ifdef __FLAT__ /* 32-bit OS/2 */
  short int (__far16 pascal *SblastInit)(unsigned short ioaddr, unsigned short irq);
  void (__far16 pascal *SblastTerm)(void);
  void (__far16 pascal *SblastFMOutAll)(unsigned short reg, unsigned short dat);
  void (__far16 pascal *SblastFMNoteAll)(unsigned short octave, unsigned short note);
#else
  int (far pascal *SblastInit)(unsigned ioaddr, unsigned irq);
  void (far pascal *SblastTerm)(void);
  void (far pascal *SblastFMOutAll)(unsigned reg, unsigned dat);
  void (far pascal *SblastFMNoteAll)(unsigned octave, unsigned note);
#endif
};

extern void (_fast *noisefunc)(int, int);

static struct _sbinfo *spsb;

#endif

static void * near YellSblastInit(void);
static void near YellSblastTerm(void *);

