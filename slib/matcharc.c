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

/* used by \max\src\mb_qwkup.c */

static int near MatchArcType(int fd, struct _arcinfo *ai)
{
  char temp[50];
  int id_len;

  id_len=strlen(ai->id);

  if (ai->id_ofs < 0L)
    lseek(fd,-(long)id_len+ai->id_ofs+1,SEEK_END);
  else lseek(fd,ai->id_ofs,SEEK_SET);

  read(fd,(char *)temp,id_len);

  if (strncmp(ai->id,temp,id_len)==0)
    return TRUE;
  else return FALSE;
}

