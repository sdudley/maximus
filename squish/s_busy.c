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
static char rcs_id[]="$Id: s_busy.c,v 1.1 2002/10/01 17:56:19 sdudley Exp $";
#pragma on(unreferenced)

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "keys.h"
#include "msgapi.h"
#include "squish.h"

static char *nodot_bsy="bsy";
static char *dot_bsy=".bsy";


/* Create a busy flag for a particular node */

static char * near MakeBusyName(NETADDR *n, char *temp)
{
  MakeOutboundName(n, temp);

  if ((config.flag & FLAG_FRODO) && n->point)
    (void)sprintf(temp+strlen(temp), "%03x", n->point & 0xfff);
  else
    (void)strcat(temp, nodot_bsy);

  return temp;
}


/* Open a busy flag for a specified node number */

int BusyFileOpenNN(NETADDR *n, int wait)
{
  char temp[PATHLEN];

  return (BusyFileOpen(MakeBusyName(n, temp), wait));
}


/* Close a node number-based busy flag */

void BusyFileCloseNN(NETADDR *n)
{
  char temp[PATHLEN];

  BusyFileClose(MakeBusyName(n, temp));
}


/* Add a .bsy extension to a filename */

static char * near bsy_extension(char *name)
{
  char *bsyname, *point;

  /* Handle InterMail-style busy flags */

  /* Find the last dot in the filename */
  
  bsyname=smalloc(strlen(name)+10);
  (void)strcpy(bsyname, name);

  point=strrchr(bsyname, '.');

  /* Change the extension to .bsy */
  
  if (point==NULL || strchr(bsyname, '\\') > point)
    (void)strcat(bsyname, dot_bsy);
  else
  {
    /* If we're in FD mode and we have a ".XXX" extension, leave it as-is   *
     * for intermail support.                                               */

    if ((config.flag & FLAG_FRODO) &&
        isxdigit(point[1]) && isxdigit(point[2]) && isxdigit(point[3]))
    {

    }
    else
    {
      (void)strcpy(point, dot_bsy);
    }
  }
  
  return bsyname;
}


/* Create a busy file for the specified filename */

int BusyFileOpen(char *name, int wait)
{
  char *bsyname, *dir, *point;
  char temp[PATHLEN];
  int bfile;
  unsigned bs;

  /* Only use if FLAG_BSY is set, and FLAG_FRODO is *not* set. */

  if ((config.flag & FLAG_BSY)==0)
    return 0;

  bsyname=bsy_extension(name);
  
  /* Open the file with O_EXCL, such that the create fails if the file      *
   * already exists.                                                        */

  while ((bfile=cshopen(bsyname, O_EXCL | O_CREAT | O_WRONLY | O_BINARY))==-1)
  {
    if (!wait)
      return -1;

    dir=sstrdup(bsyname);
    
    point=strrchr(dir, PATH_DELIM);
    
    if (point)
      *point='\0';
    
    (void)make_dir(dir);
    free(dir);

    (void)sprintf(temp," (%s busy; wait)", name);
    (void)printf(temp);
    
    while (fexist(bsyname))
    {
      if (khit() && kgetch()==K_ESC)
        break;

      tdelay(200);
    }

    for (bs=strlen(temp); bs--; )
      (void)printf("\b \b");
  }

  if (bfile != -1)
    (void)close(bfile);
  
  free(bsyname);
  return 0;
}


/* Close a busy flag for a named file */

void BusyFileClose(char *name)
{
  char *bsyname;

  
  if ((config.flag & FLAG_BSY)==0)
    return;
  
  bsyname=bsy_extension(name);

  (void)unlink(bsyname);
  free(bsyname);
}


/* See if a specified busy file exists for a certain node */

int BusyFileExist(NETADDR *n)
{
  char bsyname[PATHLEN];
  char *fname;

  if ((config.flag & (FLAG_FRODO|FLAG_BSY)) != FLAG_BSY)
    return FALSE;

  fname=bsy_extension(MakeBusyName(n, bsyname));

  return (fexist(fname));
}

