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

/*
 * Caller information API
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include "prog.h"
#include "mm.h"

void ci_login(void)
{
  Get_Dos_Date(&sci.login);
  sci.logon_priv=usr.priv;
  sci.logon_xkeys=usr.xkeys;
  sci.calls=usr.times+1;
}

void ci_init(void)
{
  if (*PRM(caller_log))
  {
    memset(&sci, 0, sizeof sci);
    strcpy(sci.name, usrname);
    sci.task=task_num;
    ci_login();
  }
}

void ci_filename(char * buf)
{
  char temp[PATHLEN];

  *buf='\0';
  if (*PRM(caller_log))
  {
    char *p;

    strcpy(temp, PRM(caller_log));
    Convert_Star_To_Task(temp);
    Parse_Outside_Cmd(temp,buf);
    p=strrchr(buf,'\\');
    if (p==NULL)
      p=buf;
    if (strchr(p,'.')==NULL)
      strcat(buf, dotbbs);
  }
}

void ci_save(void)
{
  char temp[PATHLEN];

  ci_filename(temp);
  if (*sci.name && *temp)
  {
    int fd;

    /* Complete caller information */

    strcpy(sci.name, usrname);
    strcpy(sci.city, usr.city);
    sci.task=task_num;
    Get_Dos_Date(&sci.logoff);
    sci.logoff_priv=usr.priv;
    sci.logoff_xkeys=usr.xkeys;

    fd=shopen(temp, O_RDWR|O_CREAT|O_APPEND|O_BINARY);
    if (fd!=-1)
    {
      write(fd, (char *)&sci, sizeof sci);
      close(fd);
    }
  }
}


