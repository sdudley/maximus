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
static char rcs_id[]="$Id: max_inif.c,v 1.1.1.1 2002/10/01 17:51:41 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File-oriented initialization functions for Max
*/

#define MAX_LANG_max_init
#define MAX_INCL_COMMS

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include "prog.h"
#include "ffind.h"
#include "mm.h"

#ifndef ORACLE

/* If the date of the .CTL file is greater than that of the .PRM, then
   make a ruckus! */

void Compare_Dates(char *ctlname,char *prmname)
{
  union stamp_combo st_c,
                    st_p;

  FileDate(ctlname,&st_c);
  FileDate(prmname,&st_p);

  if (! GEdate(&st_p,&st_c))
  {
    Lputs(old_prm_file);
    vbuf_flush();
    Local_Beep(3);
    Delay(300);
  }
}



int Read_Stats(struct _bbs_stats *bstats)
{
  union stamp_combo now;
  
  int bfile;

  char temp[PATHLEN];

  sprintf(temp,bbs_stats,PRM(sys_path),task_num);

  if ((bfile=shopen(temp, O_RDONLY | O_BINARY)) != -1)
  {
    read(bfile, (char *)bstats, sizeof(struct _bbs_stats));
    close(bfile);
    
    Get_Dos_Date(&now);
    now.dos_st.time=0;
    
    if (bstats->version==0 || now.ldate != bstats->date.ldate)
    {
      bstats->version=STATS_VER;
      bstats->today_callers=0;
    }
  }
  
  Get_Dos_Date(&bstats->date);
  bstats->date.dos_st.time=0;

  if (bfile==-1 || bstats->version != STATS_VER)
  {
    memset(bstats, '\x00', sizeof(struct _bbs_stats));

    bstats->version=STATS_VER;
    bstats->online_date=time(NULL);
  }

  return 0;
}


void Write_Stats(struct _bbs_stats *bstats)
{
  int bfile;

  char temp[PATHLEN];

  sprintf(temp,bbs_stats,PRM(sys_path),task_num);

  if ((bfile=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                   SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
  {
    cant_open(temp);
    return;
  }

  write(bfile, (char *)bstats, sizeof(struct _bbs_stats));
  close(bfile);
}


#if 0
byte Assign_Unique_Task(void)
{
  FFIND *ff;
  
  char temp[PATHLEN];
  
  int high=0,
      ret,
      x;
  
  sprintf(temp, ut_star, original_path);
  
  for (ff=FindOpen(temp,0), ret=0; ff && ret==0; ret=FindNext(ff))
  {
    if ((sscanf(ff->szName, ut_name, &x))==1)
      if (x > high)
        high=x;
  }
  
  if (++high >= 255)  /* Return error if there are too many task numbers */
    return (byte)0;
  else return (byte)high;
}
#endif

#endif  /* !ORACLE */
