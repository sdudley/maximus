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

#pragma off(unreferenced)
static char rcs_id[]="$Id: fixlr.c,v 1.1.1.1 2002/10/01 17:57:23 sdudley Exp $";
#pragma on(unreferenced)

#include <fcntl.h>
#include <io.h>
#include "bfile.h"
#include "max.h"

void FixUser(BFILE b, char *arg, int *piMaxRead)
{
  struct _usr usr;
  int iUserNum=atoi(arg);
  long lSeek=(long)iUserNum * sizeof usr;

  Bseek(b, lSeek, SEEK_SET);

  if (Bread(b, &usr, sizeof usr) != 0)
    puts("Can't read user file");

  printf("User #%d - old lastread %5d - ", iUserNum, usr.lastread_ptr);

  usr.lastread_ptr=++(*piMaxRead);

  printf("new lastread %5d\n", usr.lastread_ptr);

  Bseek(b, lSeek, SEEK_SET);

  if (Bwrite(b, &usr, sizeof usr) != 0)
    puts("Can't write user file!");
}

int main(int argc, char *argv[])
{
  int iMaxRead=0;
  struct _usr usr;
  BFILE b;

  if (argc < 3)
  {
    puts("Usage:\n  fixlr <userbbs_path> <user#> [<user#> ...]");
    return 1;
  }

  if ((b=Bopen(argv[1], BO_RDWR | BO_BINARY, BSH_DENYNO, 8192))==NULL)
  {
    puts("Error opening user file!");
    return 1;
  }

  /* Scan for highest lastread pointer */

  while (Bread(b, &usr, sizeof usr)==sizeof usr)
    if (usr.lastread_ptr > iMaxRead)
      iMaxRead=usr.lastread_ptr;

  printf("Highest lastread pointer is %d\n", iMaxRead);

  for (argv += 2; *argv; argv++)
    FixUser(b, *argv, &iMaxRead);

  Bclose(b);

  return 0;
}


