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


int far pascal SblastInit(unsigned ioaddr, unsigned irq);
void far pascal SblastTerm(void);

void far pascal SblastVocEnable(unsigned status);
int far pascal SblastVocPlay(char far *block, unsigned len, unsigned port, unsigned delay, unsigned docli, unsigned packtype);

void far pascal SblastFMOutAll(unsigned reg, unsigned dat);
void far pascal SblastFMOutLeft(unsigned reg, unsigned dat);
void far pascal SblastFMOutRight(unsigned reg, unsigned dat);

void far pascal SblastFMNoteAll(unsigned octave, unsigned note);
void far pascal SblastFMNoteLeft(unsigned octave, unsigned note);
void far pascal SblastFMNoteRight(unsigned octave, unsigned note);

