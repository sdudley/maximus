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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: s_stat.c,v 1.2 2004/01/13 00:42:14 paltas Exp $";
#pragma on(unreferenced)
#endif

#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "msgapi.h"
#include "squish.h"

static int fd=-1;

/* Open the statistics file */

void StatsOpen(void)
{
  if ((config.flag & FLAG_STATS)==0)
    return;


  /* Open the file */

  if ((fd=sopen(config.statfile, O_CREAT | O_WRONLY | O_BINARY | O_APPEND | O_NOINHERIT,
                SH_DENYNO, S_IREAD | S_IWRITE))==-1)
  {
    S_LogMsg(cantopen, config.statfile);
    return;
  }

  /* Make sure we're at the end */

  (void)lseek(fd, 0L, SEEK_END);

  /* Mark the beginning of a run */

  StatsWriteBlock(TYPE_BEGIN, sizeof(struct _tstamp), 0, NULL);
}





/* Write a block of data to the statistics file.  This function builds the  *
 * stats header automatically.                                              */

/* len: length of block to report in squish.stt header
   actual_len: number of bytes to write from 'data' */

void StatsWriteBlock(word type, word len, word actual_len, void *data)
{
  struct _thdr th;
  struct _tstamp ts;

  /* Don't process if no stats file or if stats are disabled */

  if (fd==-1 || (config.flag & FLAG_STATS)==0)
    return;

  /* Make sure that a value is specified for the actual length */
  if (actual_len==0)
    actual_len=len;

  /* Automatically fill in timestamps */

  if (type==TYPE_BEGIN || type==TYPE_END)
  {
    ts.date=time(NULL);
    data=&ts;
    actual_len=len=sizeof ts;
  }

  /* Fill in the header */

  th.type=type;
  th.len=len;

  /* Write the header and then the data */

  if (fastwrite(fd, (char *)&th, sizeof th) != (int)sizeof(th) ||
      (data && fastwrite(fd, (char *)data, actual_len) != (int)actual_len))
  {
    S_LogMsg("!Error writing to statistics file!");
  }
}






/* Close the statistics file */

void StatsClose(void)
{
  /* Write the ending timestamp */

  if (fd != -1)
  {
    StatsWriteBlock(TYPE_END, sizeof(struct _tstamp), 0, NULL);
    (void)close(fd);
    fd=-1;
  }
}



/* Write the statistics for all message areas */

void StatsWriteAreas(void)
{
  struct _statlist *sl;
  struct _cfgarea *ar;
  struct _tarea ta;
  struct _tnode tn;
  
  /* Write stats for all the areas we have */
    
  for (ar=SkipFirst(config.area); ar; ar=SkipNext(config.area))
  {
    if (ar->flag & AFLAG_STATDATA)
    {
      /* Turn off the "area has stats" flag for the next run */

      ar->flag &= ~AFLAG_STATDATA;

      /* Fill in stats header for area */

      (void)strncpy(ta.tag, ar->name, AH_TAGLEN);
      ta.in_msgs=ar->statarea.in_msgs;
      ta.in_bytes=ar->statarea.in_bytes;
      ta.taflag=(ar->flag & AFLAG_PASSTHRU) ? TAFLAG_PASSTHRU : 0;

      /* Count the number of nodes to which messages were sent */

      for (sl=ar->statlist, ta.n_nodes=0; sl; sl=sl->next)
        if (sl->out_msgs)
          ta.n_nodes++;

      /* Reset stats for next run */

      ar->statarea.in_msgs=0;
      ar->statarea.in_bytes=0;


      /* Write the header block */

      StatsWriteBlock(TYPE_AREA, (word)(sizeof(ta) + sizeof(tn)*ta.n_nodes),
                      sizeof ta, &ta);


      /* Now loop around and write the data for each node too */

      for (sl=ar->statlist; sl; sl=sl->next)
      {
        if (!sl->out_msgs)  /* skip if no msgs sent */
          continue;

        tn.node=sl->node;
        tn.out_msgs=sl->out_msgs;
        tn.out_bytes=sl->out_bytes;

        if (fastwrite(fd, (char *)&tn, sizeof tn) != (int)sizeof tn)
          S_LogMsg("!Error writing to stats file");

        /* Reset the stats for the next run */

        sl->out_msgs=0;
        sl->out_bytes=0;
      }
    }

    /* Write statistics relating to duplicate messages in this area */

    if (ar->dupes)
    {
      struct _tdupe td;

      (void)strncpy(td.tag, ar->name, AH_TAGLEN);
      td.n_dupes=ar->dupes;
      StatsWriteBlock(TYPE_DUPE, sizeof td, 0, &td);
    }
  }
}

