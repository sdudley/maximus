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
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "voc_data.h"
#include "voc_file.h"


static block_lengths block_length;
#define MAX_SOUND_LENGTH MAX_BLOCK_LENGTH



VOCH VocOpen(char *name)
{
  VOCH handle;
  headers header;
  
  if ((handle=malloc(sizeof(*handle)))==NULL)
    return NULL;
  
  memset(handle, '\0', sizeof(*handle));

  if ((handle->file_handle=Bopen(name, BO_RDONLY | BO_BINARY, 32767, 0))==NULL)
  {
    free(handle);
    return NULL;
  }
  
  handle->is_input=TRUE;
  
  if (Bread(handle->file_handle, (char *)&header, sizeof(headers))==-1 ||
      0x12-header.version_major != header.id_code_major ||
      0x33-header.version_minor != header.id_code_minor)
  {
    Bclose(handle->file_handle);
    free(handle);
    return NULL;
  }
  
  if (header.offset_of_data_block != sizeof(headers))
  {
    Bseek(handle->file_handle,
          (long)header.offset_of_data_block-sizeof(headers),
          SEEK_CUR);
  }
  
  handle->voice_info.voice_to_continue=FALSE;

  return handle;
}



int VocRead(VOCH handle, blocks *block)
{
  block_types this_type;
  sample_rates sample_rate;
  One_Byte_Naturals null_byte;
  blklens blklen;
  
  if (! handle->is_input)
  {
    printf("VOC is not for input!\n");
    return -1;
  }
  
  if (handle->terminated)
  {
    block->block_type=terminator;
    block->block_length=0;
    return 0;
  }
  
  if (handle->voice_info.voice_to_continue &&
      handle->voice_info.remaining_length > 0)
  {
    return (_VocReadGetSound(handle, block));
  }
  
  if (Bread(handle->file_handle, (char *)&this_type, 1)==-1)
  {
    printf("Can't read type!\n");
    return -1;
  }
  
  switch (this_type)
  {
    case terminator:
      handle->terminated=TRUE;
      block->block_type=terminator;
      break;
      
    case voice_data:
      blklen=_VocReadThisLength(handle)-2;
      sample_rate=_VocReadThisRate(handle);
    
      handle->voice_info.voice_to_continue=TRUE;
      handle->voice_info.sample_rate=sample_rate;
      handle->voice_info.packing=_VocReadThisPack(handle);
      handle->voice_info.remaining_length=blklen;
      
      return (_VocReadGetSound(handle, block));
      
    case voice_continuation:
      if (! handle->voice_info.voice_to_continue)
      {
        printf("No continuation!\n");
        return -1;
      }
      
      handle->voice_info.remaining_length=_VocReadThisLength(handle);
      break;
      
    case silence:
      if (_VocReadThisLength(handle) != 3)
      {
        printf("No length!\n");
        return -1;
      }
      
      block->block_type=silence;
      block->block_length=0;
      block->dat.silence_interval=((duration)_VocReadThis2(handle)+1.0)/
                                                   _VocReadThisRate(handle);
      break;
      
    case marker:
      if (_VocReadThisLength(handle) != 2)
      {
        printf("No length!\n");
        return -1;
      }
      
      block->block_type=marker;
      block->block_length=0;
      block->dat.mark=(markers)_VocReadThis2(handle);
      break;
      
    case text:
      block_length=(block_lengths)_VocReadThisLength(handle)-1;
      block->block_type=text;
      block->block_length=block_length;

      memset(&block->dat.text_string, '\0', MAX_BLOCK_LENGTH);
      
      if (Bread(handle->file_handle, (char *)&block->dat.text_string,
               block_length) == -1)
      {
        printf("Can't read text\n");
        return -1;
      }
      
      if (Bread(handle->file_handle, &null_byte, 1) == -1 ||
          null_byte != 0)
      {
        printf("Can't read nul byte\n");
        return -1;
      }
      
    case start_repeat:
      if (_VocReadThisLength(handle) != 2)
      {
        printf("Can't read length\n");
        return -1;
      }
      
      block->block_type=start_repeat;
      block->block_length=0;
      block->dat.count=(repeat_counts)_VocReadThis2(handle);
      break;
      
    case end_repeat:
      if (_VocReadThisLength(handle) != 0)
        return 0;
      
      block->block_type=end_repeat;
      block->block_length=0;
      break;
  }
  
  return 0;
}



blklens _VocReadThisLength(VOCH handle)
{
  blklens blklen;
  
  blklen=0; /* zero out the fourth byte, assuming intel format */
  
  if (Bread(handle->file_handle, (char *)&blklen, 3) == -1)
  {
    printf("Error reading length!\n");
    return -1;
  }
  
  return blklen;
}

sample_rates _VocReadThisRate(VOCH handle)
{
  typedef byte srs; /* 0 .. 255 */
  srs sr;
  
  if (Bread(handle->file_handle, (char *)&sr, sizeof(sr)) == -1)
  {
    printf("Error reading rate!\n");
    return -1;
  }
  
  return (sample_rates)(1000000L/(long)(256L-(long)sr));
}



pack_types _VocReadThisPack(VOCH handle)
{
  typedef byte packs; /* 0 .. 255 */
  packs pack;
  
  if (Bread(handle->file_handle, &pack, sizeof(pack)) == -1)
  {
    printf("Error reading pack!\n");
    return -1;
  }
  
  return (pack_types)pack;
}



Two_Byte_Naturals _VocReadThis2(VOCH handle)
{
  Two_Byte_Naturals n;
  
  if (Bread(handle->file_handle, (char *)&n, 2) == -1)
  {
    printf("Error reading this2!\n");
    return -1;
  }
  
  return n;
}

int _VocReadGetSound(VOCH handle, blocks *block)
{
  byte_counts chunk_length;
  
  if (handle->voice_info.remaining_length > (blklens)MAX_SOUND_LENGTH)
    chunk_length=MAX_SOUND_LENGTH;
  else chunk_length=handle->voice_info.remaining_length;
  
  block->block_type=voice_data;
  block->block_length=block_length=(block_lengths)chunk_length;
  block->dat.voice_data.sample_rate=handle->voice_info.sample_rate;
  block->dat.voice_data.packing=handle->voice_info.packing;
  
  memset(&block->dat.voice_data.data, '\0', MAX_BLOCK_LENGTH);
  
  if (Bread(handle->file_handle, (char *)&block->dat.voice_data.data,
           (int)chunk_length) == -1)
  {
    printf("Error reading data!\n");
    return -1;
  }
  
  handle->voice_info.remaining_length -= (blklens)chunk_length;
  
  return 0;
}


int VocClose(VOCH handle)
{
  Bclose(handle->file_handle);
  free(handle);
  return 0;
}


