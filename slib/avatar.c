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

/*# name=Convert AVATAR number into ANSI sequence
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"


/* Convert an AVATAR colour number to ANSI.  This array simply maps         *
 * from AVATAR colour 'n' (at offset avtcvt[n]) to the ANSI colour, which   *
 * is given by the value at that location in the array.                     */

static byte *acol="\x00\x04\x02\x06\x01\x05\x03\x07";

byte * _fast avt2ansi(sword new, sword old, char *ansi)
{
  char *orig=ansi;
  word fore, back;
  word oldfore, oldback;
  word blink, oldblink;

  *ansi='\0';

  
  /* If colours are the same, do nothing */

  if (new==old)
    return orig;


  /* If the high bit is set, blink is enabled */
  
  blink=new & 0x80;
  oldblink=old & 0x80;

  
  /* Now strip off the high bit */
  
  new &= ~0x80;




  /* Find out the foreground and background colours, respectively */
  
  fore=new & 0x0f;
  back=(new >> 4) & 0x07;


  /* If there was no old colour, use impossible colour sequences */

  if (old==-1)
  {
    oldfore=0xff;
    oldback=0xff;
  }
  else
  {
    oldfore=old & 0x0f;
    oldback=(old >> 4) & 0x07;
  }


  /* The ANSI start sequence */
  
  strcpy(ansi, "\x1b[");
  ansi += strlen(ansi);

  if (oldblink && !blink)
  {
    *ansi++='0';

    oldback=0;
    oldfore=7;

    if (fore != oldfore || back != oldback || blink)
      *ansi++=';';
  }

  /* If the foreground colour isn't the same, change it */

  if (fore != oldfore)
  {
    /* If the highlight/dark bit is not the same, output code to change */

    if ((fore >> 3) != (oldfore >> 3))
    {
      *ansi++=(char)('0'+((fore >> 3) ? 1 : 0));

      if ((fore >> 3)==0)
      {
        oldback=0;    /* the "0;" resets the background col to black, */
        oldfore=7;    /* and the foreground to gray.                  */
      }


      /* Add a ';', if necessary */

      if (fore != oldfore || back != oldback || blink)
        *ansi++=';';
    }

    /* Set the foreground colour, but not if we just printed a "0;"         *
     * and are supposed to change the FG to gray, since the "0" already     *
     * does that.                                                           */

    if (fore != oldfore)
    {
      /* Now add the appropriate colour. '3' means fg change,               *
       * next char is colour.                                               */

      *ansi++='3';
      *ansi++=(char)('0'+acol[(word)(fore & 0x07)]);
    }
    

    /* If there will be a background change, insert a separator too */

    if (back != oldback || blink)
      if (ansi[-1] != ';')
        *ansi++=';';
  }

  /* Now handle changes in background colour */

  if (back != oldback)
  {
    *ansi++='4';
    *ansi++=(char)('0'+acol[(word)(back & 0x07)]);
    
    if (blink)
      *ansi++=';';
  }
  
  if (blink)
    *ansi++='5';

  /* Add the trailing 'm' */

  *ansi++='m';
  *ansi='\0';
  
  return orig;
}



#ifdef TEST
main()
{
  char ansi[20];
  sword old=-1;

#define doit(col) printf(#col "=%sTEST\n", avt2ansi(col, old, ansi)); old=col;
  
  doit(2);
  doit(3);
  doit(4);
  doit(5);
  doit(6);
  doit(7);
  doit(8);
  doit(9);
  doit(3);
  doit(15);
  doit(16);
  doit(1);
  doit(16);
  doit(33);
  doit(43);
  doit(33);
  doit(49);
  doit(1);
  doit(1+128);
  doit(2+128);
  doit(2+16+128);
  doit(1+128);
  doit(1);
  doit(1+128);
  doit(41);

  return 0;
}

#endif



/*                             Foregrounds
ÚÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
³ BLACK ³ BLUE  ³ GREEN ³ CYAN  ³ RED   ³MAGENTA³YELLOW ³ WHITE ³ BACK-  ³
³low hi ³low hi ³low hi ³low hi ³low hi ³low hi ³low hi ³low hi ³ GROUND ³
ÃÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄ´
³  0   8³  1   9³  2  10³  3  11³  4  12³  5  13³  6  14³  7  15³BLACK   ³
³ 16  24³ 17  25³ 18  26³ 19  27³ 20  28³ 21  29³ 22  30³ 23  31³BLUE    ³
³ 32  40³ 33  41³ 34  42³ 35  43³ 36  44³ 37  45³ 38  46³ 39  47³GREEN   ³
³ 48  56³ 49  57³ 50  58³ 51  59³ 52  60³ 53  61³ 54  62³ 55  63³CYAN    ³
³ 64  72³ 65  73³ 66  74³ 67  75³ 68  76³ 69  77³ 70  78³ 71  79³RED     ³
³ 80  88³ 81  89³ 82  90³ 83  91³ 84  92³ 85  93³ 86  94³ 87  95³MAGENTA ³
³ 96 104³ 97 105³ 98 106³ 99 107³100 108³101 109³102 110³103 111³YELLOW  ³
³112 120³113 121³114 122³115 123³116 124³117 125³118 126³119 127³WHITE   ³
ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÙ
*/


