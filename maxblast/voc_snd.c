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

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "prog.h"
#include "voc_data.h"
#include "voc_snd.h"
#include "maxblast.h"

#define TURN_LOUDER_LAG 0.2
#define TURN_SOFTER_LAG 0.02

typedef word multiplier_range; /* 4 .. 32 */

#define MULTIPLIER_RANGE_FIRST  4
#define MULTIPLIER_RANGE_LAST   32

#if 0
void halve(voice_blocks *voice_block, bool automatic_gain_control)
{
  sword a,b;
  sword err=0;
  sounds *data;
  
  multiplier_range multiplier=8;

  word volume_adjustment_point=
        (word)voice_block->dat.voice_data.sample_rate*TURN_LOUDER_LAG;

  word too_loud_delta=(word)(TURN_LOUDER_LAG/TURN_SOFTER_LAG);

  word error_level=0;
  word i;
  word j=1;
  
  /* Short-form for addressing voice data */

  data=voice_block->dat.voice_data.data;
  
  
  /* No need to unpack already-unpacked data */
  
  if (voice_block->dat.voice_data.packing != unpacked)
    return;
  
  voice_block->dat.voice_data.packing=halved;
  voice_block->block_length = (voice_block->block_length-1)/2;

  for (i=0; i < voice_block->block_length; i++)
  {
    a=((sword)data[j+1] - (sword)data[j])*multiplier+128+err;
    
    if (a < 0)
    {
      err=a;
      a=0;
      error_level += too_loud_delta;
    }
    else if (a > 255)
    {
      err=a-255;
      a=255;
      error_level += too_loud_delta;
    }
    else
    {
      error_level--;
      err=0;
    }
    
    j++;
    

    b=((sword)data[j+1] - (sword)data[j])*multiplier+128+err;
    
    if (b < 0)
    {
      err=b;
      b=0;
      error_level += too_loud_delta;
    }
    else if (b > 255)
    {
      err=b-255;
      b=255;
      error_level += too_loud_delta;
    }
    else
    {
      error_level--;
      err=0;
    }
    
    j++;
    
    if (automatic_gain_control)
    {
      if (error_level >= volume_adjustment_point)
      {
        if (multiplier > MULTIPLIER_RANGE_FIRST)
          multiplier--;
        
        error_level=0;
      }
      else if (error_level <= -volume_adjustment_point)
      {
        if (multiplier < MULTIPLIER_RANGE_LAST)
          multiplier++;
        
        error_level=0;
      }
    }
    else error_level=0;

    a=(a+7)/16;
    b=(b+7)/16;

    if (a > 15)
      a=15;

    if (b > 15)
      b=15;

    if (a < 8)
      a=15-a;
    else a -= 8;

    if (b < 8)
      b=15-b;
    else b -= 8;

    data[i]=(sound_bytes)(a*16+b);
  }
}
#endif


void play(blocks *block)
{
  char *data;

  if (block->block_type==silence)
  {
    tdelay(block->dat.silence_interval);
    return;
  }
  
  if (block->block_type != voice_data)
    return;


  /* Set up pointer to the voice data */

  data=block->dat.voice_data.data;


/*  printf("sample rate=%ld\n", (long)block->dat.voice_data.sample_rate);*/
#if 0
  {
    int fd;

    if ((fd=open("block", O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE))==-1)
    {
      printf("fde\n");
      exit(1);
    }

    if (write(fd, data, block->block_length) != block->block_length)
    {
      printf("fdw\n");
      exit(1);
    }

    close(fd);
  }
#endif

#if 0
  if (block->dat.voice_data.packing == halved)
  {
    char *p, *s, *e;
    int new_len;

    new_len=block->block_length*2;

    if ((data=malloc(new_len)) == NULL)
    {
      printf("Out of memory for sample block!\n");
      exit(1);
    }


    for (p=block->dat.voice_data.data+1, s=data, e=p+block->block_length/2;
         p < e && s < data+new_len;)
    {
/*      s[0]=s[-1]+(sbyte)((*p & 0x0fu) << 4);
      s[1]=s[ 0]+(sbyte)(*p & 0xf0u);*/
      s[0]=*p;
      s[1]=*p;
/*      s[0]=(*p   & 0xf0);
      s[1]=(*p & 0x0f)+s[0];*/
      p++;
      s += 2;
    }


    block->block_length=new_len;
    block->dat.voice_data.packing=unpacked;
  }
#endif

  if (block->dat.voice_data.packing == unpacked)
  {
    extern unsigned blastdelay;
    extern unsigned nocli;

    SblastVocPlay((char far *)data,
                  block->block_length, 0x220,
                  (int)((long)blastdelay * 11000L / (long)block->dat.voice_data.sample_rate * 11000L / (long)block->dat.voice_data.sample_rate),
                  !nocli, block->dat.voice_data.packing);
  }
  else
  {
    printf("Warning:  Voice data packing type %d not supported!\n",
           block->dat.voice_data.packing);
  }
}


