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
#include "prog.h"
#include "voc_data.h"
#include "voc_file.h"
#include "play.h"
#include "maxblast.h"

#define INCL_DOSPROCESS
#define INCL_NOPM
#include <os2.h>

unsigned ioaddr=0x220;
unsigned irq=7;
unsigned nocli;
unsigned blastdelay=180;

int main(int argc, char *argv[])
{
  char temp[PATHLEN];
  static blocks *bks;
  VOCH f;

  printf("\nVOCOS2  SoundBlaster .VOC Player for OS/2  Version 1.00\n"
         "Copyright 1991-1992 by Scott J. Dudley.  All rights reserved.\n\n");

  if (argc < 2)
  {
    printf("Usage:\n\n");

    printf("VOCOS2 <voc_file>[.VOC]\n");
    return 1;
  }

  if (ParseBlaster())
    return 1;

  if (SblastInit(ioaddr, irq) != 0)
  {
    printf("Error!  A SoundBlaster is required to use this program!\n");
    return 1;
  }

  if ((f=VocOpen(argv[1]))==NULL)
  {
    strcpy(temp, argv[1]);
    strcat(temp, ".voc");

    if ((f=VocOpen(temp))==NULL)
    {
      printf("Error!  Can't open \"%s\"!\n", argv[1]);
      return 1;
    }
  }

  SblastVocEnable(TRUE);

  if ((bks=malloc(sizeof(*bks)))==NULL)
  {
    printf("Out of memory!\n");
    return 1;
  }

  /* Set our priority to time-critical */

  DosSetPrty(PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);

  while (VocRead(f, bks)==0)
    if (VocProcessBlock(bks) || khit())
      break;

  free(bks);

  if (khit()) /* eat character */
    kgetch();

  SblastVocEnable(FALSE);
  SblastTerm();
  VocClose(f);
  return 0;
}



static int near VocProcessBlock(blocks *b)
{
#ifdef TEXT
  printf("type=%02d  ", b->block_type);
#endif
  
  switch (b->block_type)
  {
    case terminator:
      return TRUE;
      
    case voice_data:
#ifdef TEXT
      printf("sound %5u@%5u %-20s\n",
             b->block_length,
             b->dat.voice_data.sample_rate,
             VocPackType(b->dat.voice_data.packing));
#endif
      play(b);
      break;
      
    case silence:
      play(b);
      break;
      
    case marker:
#ifdef TEXT
      printf("marker=%u\n", (word)b->dat.mark);
#endif
      break;
      
    case text:
#ifdef TEXT
      printf("%s\n", b->dat.text_string);
#endif
      break;
      
    case start_repeat:
#ifdef TEXT
      printf("start repeat %u\n", (word)b->dat.count);
#endif
      break;
      
    case end_repeat:
#ifdef TEXT
      printf("end repeat\n");
#endif
      break;
      
    case voice_continuation:
#ifdef TEXT
      printf("\avoice_continuation (should never happen!)\n");
#endif
      break;
  }
  
  return FALSE;
}

#ifdef TEXT
static char * near VocPackType(pack_types p)
{
  switch (p)
  {
    case unpacked:
      return "unpacked";
      
    case halved:
      return "halved";
      
    case thirds:
      return "thirds";
      
    case quartered:
      return "quartered";
      
    default:
      return "\aunknown";
  }
}
#endif

/* OS2BLASTER=A220 I5 D1 T4 NOCLI */

static int near ParseBlaster(void)
{
#ifdef OS_2
  #define VAR_NAME  "OS2BLASTER"
#else
  #define VAR_NAME  "DOSBLASTER"
#endif
  char *blaster=getenv(VAR_NAME);
  char *p;

  nocli=TRUE;

  if (!blaster)
    return 0;

  strupr(blaster);

  if ((p=strchr(blaster, 'A')) != NULL)
  {
    if (sscanf(p+1, "%x", &ioaddr) != 1)
    {
      printf("Error!  Invalid base address in " VAR_NAME " environment setting!\n");
      return 1;
    }
  }

  if ((p=strchr(blaster, 'X')) != NULL)
  {
    if (sscanf(p+1, "%d", &blastdelay) != 1)
    {
      printf("Error!  Invalid delay in " VAR_NAME " environment setting!\n");
      return 1;
    }
  }

  return 0;
}


