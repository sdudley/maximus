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

#ifndef __VOC_DATA_H_DEFINED
#define __VOC_DATA_H_DEFINED

#include "bfile.h"

#define MAX_BLOCK_LENGTH 40000u

typedef byte One_Byte_Naturals;   /* 0 .. 255, defualt=8 */
typedef word Two_Byte_Naturals;   /* 0 .. 65536, default=16 */
typedef One_Byte_Naturals file_type_descriptions[0x14 ]; /*SJD 1991-06-01 14:43:45 */ /* 0x13 */
typedef long blklens; /* 0 .. 2**24-1 */
typedef dword byte_counts;
typedef byte sound_bytes;
typedef word block_lengths;   /* 0 .. MAX_BLOCK_LENGTH */
typedef word sound_indices;   /* 1 .. MAX_BLOCK_LENGTH */

typedef sound_bytes sounds;

enum blk_types
{
  terminator=0, voice_data=1, voice_continuation=2, silence=3, marker=4,
  text=5, start_repeat=6, end_repeat=7, size=8
};

typedef byte block_types; /* blk_types enum used for this type */

typedef word sample_rates;  /* 3906 .. INTMAX */

typedef byte bool;
typedef enum { unpacked=0, halved=1, thirds=2, quartered=3 } pack_types;

typedef word markers;       /* 1 .. 0xfffeu */
typedef word repeat_counts; /* 1 .. 0xffffu */
typedef word duration;      /* 0 .. ????? */

struct voice_data
{
  sample_rates sample_rate;
  pack_types packing;
  sounds data[MAX_BLOCK_LENGTH];
};

typedef struct
{
  block_types block_type;
  block_lengths block_length;

  union
  {
    struct voice_data voice_data;
    duration silence_interval;          /* silence */
    markers mark;                       /* marker */
    char text_string[MAX_BLOCK_LENGTH]; /* text */
    repeat_counts count;                /* start_repeat */
    /* null */                          /* terminator, end_repeat */
   
  } dat;
} blocks;


typedef blocks voice_blocks;

typedef struct
{
  word voice_to_continue;     /* false */
  sample_rates sample_rate;
  pack_types packing;
  blklens remaining_length;
} existing_voice_info;


typedef struct
{
  BFILE file_handle;
  bool is_input;
  bool terminated;
  existing_voice_info voice_info;
} *VOCH;

typedef struct
{
  file_type_descriptions File_Type_Description;
  word offset_of_data_block; /* dword */ /*SJD 1991-06-01 14:43:31 */
  One_Byte_Naturals version_minor;
  One_Byte_Naturals version_major;
  One_Byte_Naturals id_code_minor;
  One_Byte_Naturals id_code_major;
} headers;


typedef struct
{
  block_types block_type;
  blklens blklen;
} block_headers;

#define BLOCK_HEADER_LENGTH 4


#include "voc_snd.h"

#endif /* __VOC_DATA_H_DEFINED */



