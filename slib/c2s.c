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

/*# name=Colour-number to string conversion routine
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"

void _fast colour_to_string(int colour,char *s)
{
  static char *colours[]={"black",
                          "blue",
                          "green",
                          "cyan",
                          "red",
                          "magenta",
                          "brown",
                          "gray",
                          "darkgray",
                          "lightblue",
                          "lightgreen",
                          "lightcyan",
                          "lightred",
                          "lightmagenta",
                          "yellow",
                          "white"};

  colour &= 0x7f;

  if (colour < 16)
    sprintf(s,"%s",colours[colour]);
  else sprintf(s,"%s on %s",colours[colour % 16],colours[colour/16]);
}


