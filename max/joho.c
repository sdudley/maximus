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
static char rcs_id[]="$Id: joho.c,v 1.2 2003/06/04 23:23:43 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "msgapi.h"
#include "fdnode.h"

/* My reverse-engineered lookup engine for JoHo's nodelist files */

int JoHoLookup(int fd, size_t block_size, size_t rec_size, void *find, void *found, int (*compare)(void *, void *))
{
  struct _control c;
  char *fdb;
  word max, rec, lastrec;
  int i, diff, gotit;
  struct _gdx *g=NULL;
  long lbsize=(long)block_size;
  
  /* Read beginning control record */

  lseek(fd, 0L, SEEK_SET);

  if (read(fd, (char *)&c, sizeof(c)) != sizeof(c))
    return FALSE;
  
  /* Allocate memory to hold one block */

  if ((fdb=malloc(block_size))==NULL)
    return FALSE;

  /* Now trace down the tree, starting at the master index */

  lastrec=-1;
  rec=c.hdr.master_idx;
  max=(word)(lseek(fd, 0L, SEEK_END)/lbsize);
  gotit=FALSE;
  
  /* Do one index record at a time... */

  while (!gotit && rec > 0 && rec < max && rec != lastrec &&
         lseek(fd, rec*lbsize, SEEK_SET)==rec*lbsize &&
         read(fd, fdb, block_size)==(signed)block_size)
  {
    struct _inf *inf=(struct _inf *)fdb;
    unsigned last;
    
    last=inf->index;

    /* Scan all nodes in this block to find a match */

    for (i=0; i < inf->nodes; i++)
    {
      g=(struct _gdx *)(fdb+sizeof(struct _inf)+rec_size*i);

      if ((diff=(*compare)(find, g)) <= 0)
      {
        /* If it was a direct match AND this is a leaf node, we found it. */

        if (diff==0 /*&& !inf->index*/)
          gotit=TRUE;

        break;
      }
      
      last=g->block_num;
    }

    /* If we haven't already found it, set the new "search record" to the   *
     * entry before the last node which was <= our search key.  If it       *
     * wasn't found, simply use the 32nd node in the table for anything     *
     * that was above all entries listed in the table.                      */

    if (!gotit)
      rec=last;
  }
  
  free(fdb);
  
  /* If we found it, make a copy of the new record */
  if (gotit && g)
  {
    memmove(found, g, rec_size);
    return TRUE;
  }
  
  return FALSE;
}



/* Create a JoHo nodelist structure from the raw nodelist information */

int JoHoFetch(void *f, struct _johofile *jf, struct _johonode *jn)
{
  struct _gdx *g=(struct _gdx *)f;
  char *buf;
  char comma[]=",";
  char *p;
  long ofs;
  int pvt, point, fd;

  pvt=!!(g->nlofs & PVT_NODELIST);
  point=!!(g->nlofs & POINT_NODELIST);
  
  fd=pvt ? jf->pfd : point ? jf->ppfd : jf->nfd;
  ofs=g->nlofs & ~(PVT_NODELIST|POINT_NODELIST);

  if (lseek(fd, ofs, SEEK_SET) != ofs)
    return FALSE;

  if ((buf=malloc(MAX_NL_SIZE))==NULL)
    return FALSE;

  buf[MAX_NL_SIZE-1]='\0';

  if (read(fd, buf, MAX_NL_SIZE-1) != MAX_NL_SIZE && !strchr(buf, '\r'))
  {
    free(buf);
    return FALSE;
  }

  if ((p=strchr(buf, '\r')) != NULL)
    *p='\0';

  for (p=buf; (p=strchr(p, '_')) != NULL; p++)
    *p=' ';

  p=strtok(buf,  comma);  strncpy(jn->system, p, 79);   jn->system[79]='\0';
  p=strtok(NULL, comma);  strncpy(jn->city, p, 29);     jn->city[29]='\0';
  p=strtok(NULL, comma);  strncpy(jn->sysop, p, 35);    jn->sysop[35]='\0';
  p=strtok(NULL, comma);  strncpy(jn->phone, p, 39);    jn->phone[39]='\0';
  p=strtok(NULL, comma);  jn->baud=atol(p);
  p += strlen(p)+1;       strncpy(jn->flags, p, 79);    jn->flags[79]='\0';

  free(buf);

  return TRUE;
}


/* Open the JoHo nodelist files.  If ufd==NULL, no userlist will be used. */

int JoHoOpen(char *path, struct _johofile *jf, int userlist)
{
  struct _control c;
  char name[PATHLEN];
  char ext[4];

  jf->fdfd=-1;
  jf->nfd=-1;
  jf->pfd=-1;
  jf->ppfd=-1;
  jf->ufd=-1;

  /* Open the nodelist binary file first */

#ifndef UNIX
  sprintf(name, "%sNODELIST.FDX", path);
#else
  sprintf(name, "%nodelist.fdx", path);
#endif

  if ((jf->fdfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    return FALSE;

  /* Read the control block up front */

  if (read(jf->fdfd, (char *)&c, sizeof(c)) != sizeof(c))
  {
    close(jf->fdfd);
    return FALSE;
  }

  /* Now use that info to open the primary nodelist */

  strncpy(ext, c.nl_ext, 3);
  ext[3]='\0';

#ifndef UNIX
  sprintf(name, "%sNODELIST.%3s", path, ext);
#else
  ext[0] = tolower((int)ext[0]);
  ext[1] = tolower((int)ext[1]);
  ext[2] = tolower((int)ext[2]);
  sprintf(name, "%snodelist.%3s", path, ext);
#endif

  if ((jf->nfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    close(jf->fdfd);
    return FALSE;
  }
  
  /* Open the userlist file, if requested */
  
  if (userlist)
  {
#ifndef UNIX
    sprintf(name, "%sUSERLIST.FDX", path);
#else
    sprintf(name, "%userlist.fdx", path);
#endif
    
    if ((jf->ufd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    {
      close(jf->nfd);
      close(jf->fdfd);
      return FALSE;
    }
  }
  
  /* Failing to open the private nodelist is not an error, since there may  *
   * not be one!                                                            */

#ifndef UNIX
  sprintf(name, "%sFDNET.PVT", path);
#else
  sprintf(name, "%sfdnet.pvt", path);
#endif
  
  jf->pfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT);

  /* Failing to open the point nodelist is not an error, since there may    *
   * not be one!                                                            */

#ifndef UNIX
  sprintf(name, "%sFDPOINT.PVT", path);
#else
  sprintf(name, "%sfdpoint.pvt", path);
#endif
  
  jf->ppfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT);
  
  return TRUE;
}



int JoHoClose(struct _johofile *jf)
{
  if (jf->fdfd != -1)
    close(jf->fdfd);
  
  if (jf->nfd != -1)
    close(jf->nfd);
  
  if (jf->pfd != -1)
    close(jf->pfd);
  
  if (jf->ppfd != -1)
    close(jf->ppfd);
  
  if (jf->ufd != -1)
    close(jf->ufd);

  jf->fdfd=-1;
  jf->nfd=-1;
  jf->pfd=-1;
  jf->ppfd=-1;
  jf->ufd=-1;
  return 0;
}

