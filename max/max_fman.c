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
static char rcs_id[]="$Id: max_fman.c,v 1.1.1.1 2002/10/01 17:51:38 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Miscellaneous file-manipulation routines
*/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "prog.h"
#include "mm.h"
#include "max_area.h"

/* Writes the LASTUSxx.BBS file to the right directory. */

word Write_LastUser(void)
{
  char temp[PATHLEN];
  struct _usr user;
  int file;

  /* Do a bit of manipulation of the _usr part of the LASTUSER file */

  user=usr;


  /* Compatibility with Max 1.02 */

  Adjust_User_Record(&user);

  /* Compatibility with Max 2.xx */

  user.max2priv=max2priv(user.priv);

  user.timeremaining=timeleft();
  user.time += timeonline();/* Temporarily add on time during this call    *
                             * (Normally, usr.time contains the time of    *
                             * all previous calls made today.)             */

  /* Save a copy of the real usr.delflag */

  user.df_save=(byte)usr.delflag;

  /* If we have to stay compatible with Opus, use 9600 for the local  *
   * baud rate!                                                       */

  if (local)
    user.delflag=0; /*(prm.flags & FLAG_lbaud96) ? 9600 : 0;*/
  else
    user.delflag=(unsigned int)baud;

  sprintf(temp, task_num ? lastusxx_bbs : lastuser_bbs, 
          original_path, task_num);

  file=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY | O_NOINHERIT,
             SH_DENYWR, S_IREAD | S_IWRITE);

  if (file==-1)
    cant_open(temp);
  else
  {
    write(file, (char *)&user, sizeof(struct _usr));
    close(file);
  }

  return user.priv;
}


int move_file(char *fromfile,char *tofile)
{
  if (fexist(tofile))
    return -1;

  if (rename(fromfile,tofile))
  {
    /* not same device - MSC returns EXDEV for this, and TC returns         *
     * ENOTSAM.  Unfortunately, TC defines BOTH exdev and, enotsam,         *
     * so we have to check for both...                                      */

/*  if (errno==EXDEV || errno==ENOTSAM)
    {*/
      if (lcopy(fromfile,tofile)==0)
      {
        unlink(fromfile);
        return 0;
      }
      else return -1;
/*    }
    else return -1;*/
  }

  return 0;
}


#ifdef __TURBOC__     /* MSC and others already have this function */

/* always uses modification time */
int cdecl utime(char *name, struct utimbuf *times)
{
  int handle;
  struct date d;
  struct time t;
  struct ftime ft;

  unixtodos(times->modtime, &d, &t);
  ft.ft_tsec = t.ti_sec / 2;
  ft.ft_min = t.ti_min;
  ft.ft_hour = t.ti_hour;
  ft.ft_day = d.da_day;
  ft.ft_month = d.da_mon;
  ft.ft_year = d.da_year - 1980;

  if ((handle=shopen(name, O_RDWR | O_NOINHERIT)) == -1)
    return -1;

  setftime(handle, &ft);
  close(handle);
  return 0;
}

#endif


