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

/*# name=Direct-video routines for Maximus
*/

#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include "prog.h"
#include "dv.h"

#ifdef __MSDOS__
#if defined(__TURBOC__) && !defined(__TOPAZ__)

/* TC invokes its own stupid window routines if we use the normal startup  *
 * code.  My hacked version includes an external var called VidHook,       *
 * so if this isn't linked with the proper startup code, it will give      *
 * an error message.  If you're not using TC, this serves no purpose, so   *
 * feel free to ignore it.                                                 */

extern byte VidHook;

void thisfunc(void);
void thatfunc(void);

void thisfunc(void)
{
  VidHook=1;
  thatfunc();
}

void thatfunc(void)
{
  thisfunc();
}

#endif

/* 'RowTable' must be the FIRST DECLARATION IN THIS FILE!!! */

int * near RowTable=NULL;         /* Table of row offsets                  */
int *      First_RowTable=NULL;   /* Copy of above                         */

char far * near Vid_Display;      /* Base segment for video IO             */

int near Vid_StatusPort;          /* Port to check for retrace             */
int near Vid_HaveSnow;            /* If we should check for snow or not    */
int near Vid_MonoCard;            /* MDA?                                  */
int near Vid_NumRows;             /* Number of rows, total                 */
int near Vid_NumCols;             /* Number of columns, total              */
int near Vid_Row;                 /* Current row                           */
int near Vid_Col;                 /* Current column                        */
unsigned int near Vid_Segment=0xb000; /* Segment the video buffer is in        */
int near Vid_TotChars;            /* Total # of chars on screen            */

char near Vid_Page,               /* Current video page number (CGA only)  */
     near Vid_Attribute,          /* Current attribute                     */
     near Vid_Bios=FALSE;         /* If we're to use BIOS calls            */

char near DesqView;               /* If we're running under DV             */
char near Vid_Open;               /* If we've been init'ed or not          */




word _fast VidOpen(int has_snow,int desqview,int dec_rows)
{
  int x,y;
  
  if (Vid_Open)
    return 0;

  Vid_Open=TRUE;

  Vid_MonoCard=(VidGetMode()==7);
  Vid_NumCols=NUM_COLS;
  Vid_NumRows=_VidGetNumRows();
  Vid_TotChars=Vid_NumCols*Vid_NumRows;
  Vid_Segment=VidGetBuffer(Vid_MonoCard);

  DesqView=(char)desqview;

  if (desqview)
    Start_Shadow();

#ifdef __FLAT__
  Vid_Display=(char *)((unsigned long)VidGetBuffer(Vid_MonoCard) << 4);
#else
  Vid_Display=MK_FP(VidGetBuffer(Vid_MonoCard),0);
#endif

  Vid_StatusPort=(Vid_MonoCard ? 0x3BA : 0x3DA);
  Vid_HaveSnow=has_snow;

  if (VidGetBPage() != 0)
    VidSetPage(0);

  Vid_Page=0;

  VidSetAttr(7);

  _VidGetXYB(&Vid_Col,&Vid_Row);

  if ((First_RowTable=RowTable=(int *)malloc(Vid_NumRows*sizeof(int)))==NULL)
  {
    Vid_Open=FALSE;
    return -1;
  }

  for (x=0,y=PAGE_OFS;x < Vid_NumRows;x++)
  {
    RowTable[x]=y;
    y += Vid_NumCols*2;
  }


  /* Status-line? */

  if (dec_rows)
  {
    Vid_NumRows--;

    if (Vid_Row >= Vid_NumRows)
    {
      VidScroll(SCROLL_up, 1, Vid_Attribute, 0, 0,
                (char)(Vid_NumCols-1), (char)Vid_NumRows);

      Vid_Row=Vid_NumRows-1;
    }
  }
  
  return 1;
}



int _fast VidClose(void)
{
  if (!Vid_Open)
    return -1;

  Vid_Open=FALSE;

  Vid_Open=FALSE;
  VidSync();

  if (DesqView)
    End_Shadow();

  if (First_RowTable)
    free(First_RowTable);

  First_RowTable=NULL;

  return 0;
}


void _fast VidBios(int use_bios)
{
  Vid_Bios=(char)use_bios;
}

void _fast VidHideCursor(void)
{
  Vid_Col=0;
  Vid_Row=Vid_NumCols;
  VidSyncCur();
}

void pascal VidSyncDVWithForce(int fForce)
{
  VidSyncDV();
}


#endif /* __MSDOS__ */

