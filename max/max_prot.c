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
static char rcs_id[]="$Id: max_prot.c,v 1.1.1.1 2002/10/01 17:52:00 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <stdlib.h>
#include <string.h>
#include "mm.h"
#include "max_file.h"


/* Tries to find the entry for 'protocol' in the protocol.max file */

word FindProtocol(sword protocol, struct _proto *pro)
{
  char fname[PATHLEN], *p;
  long seekit;
  word ret=FALSE;
  int fd;
  
  /* Handle a user-specified protocol.max file */

  p=PRM(protocol_max);

  if (*p)
    strcpy(fname, p);
  else sprintf(fname, "%sprotocol.max", PRM(sys_path));
  
  if ((fd=shopen(fname, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    cant_open(fname);
    return FALSE;
  }
  
  seekit=sizeof(struct _proto)*protocol;

  if (lseek(fd, seekit, SEEK_SET)==seekit &&
      read(fd, (char *)pro, sizeof(struct _proto))==sizeof(struct _proto))
  {
    ret=TRUE;
  }
  
  close(fd);
  
  return ret;
}



/* Write the .CTL file for the specified external protocol */

void MaxWriteCtl(struct _proto *pro, word ul)
{
  FILE *ctl;
  char fname[PATHLEN];
  word n;
  long bd=steady_baud_l ? steady_baud_l : baud;

  FENTRY fent;

  /* Delete any preexisting log file */

  Parse_Outside_Cmd(pro->log, fname);
  unlink(fname);
  
  
  /* Handle the "%" translation characters */

  Parse_Outside_Cmd(pro->ctl, fname);
  
  if ((ctl=fopen(fname, fopen_write))==NULL)
  {
    cant_open(fname);
    return;
  }

  /* If we need to handle an Opus-style control file, do so now */

  if (pro->flag & P_OPUS)
  {
    if (local)
      fprintf(ctl, xctl_keyboard);
    else fprintf(ctl, xctl_port_baud, port+1, bd);

    if (!local)
      fprintf(ctl, xctl_modem,
              port+1, bd,
              prm.handshake_mask /* 3 */, prm.carrier_mask, baud);

    fprintf(ctl, xctl_time, timeleft());

    if (log_name && *log_name)
      fprintf(ctl, xctl_log, log_name);

    fprintf(ctl, xctl_msgs, MAS(mah, path));
    fprintf(ctl, xctl_uploads, FAS(fah, uppath));
    fprintf(ctl, xctl_downloads, FAS(fah, downpath));
    fprintf(ctl, xctl_help,  original_path);
  
    if (fah.heap)
    {
      if (*FAS(fah, filesbbs))
        fprintf(ctl, xctl_filesbbs, FAS(fah, filesbbs));
    }
  }

  /* Now write out the names of the files to send */

  for (n=0; GetFileEntry(n, &fent); n++)
  {
    fprintf(ctl, ul ? pro->ulstr : pro->dlstr, fent.szName);
    fputc('\n', ctl);
  }

  /* Close the control file */

  fclose(ctl);
}






word MaxReadLog(struct _proto *pro, word doing_ul)
{
  FILE *log;

  #define LOGLEN 255
  char fname[PATHLEN];
  char line[LOGLEN];
  word gotfile=FALSE;
  char tempname[PATHLEN];

  FENTRY fent;

  /* Handle the "%" translation characters */

  Parse_Outside_Cmd(pro->log, fname);
  
  if ((log=fopen(fname, fopen_read))==NULL)
  {
    cant_open(fname);
    return FALSE;
  }

  while (fgets(line, LOGLEN, log))
  {
    char *ulstr, *dlstr, *got, *desc;
    word ul;
    word fn;
    
    /* Find the positions of the upload and download strings */

    dlstr=strstr(line, pro->dlkey);
    ulstr=strstr(line, pro->ulkey);

    if (ulstr==NULL && dlstr==NULL)
      continue;

    if (pro->flag & P_BI)
    {
      /* Use whichever token comes FIRST in the log line, since we might    *
       * see either.                                                        */
      
      ul=(ulstr && (dlstr==NULL || ulstr < dlstr));
      got=(ul ? ulstr : dlstr);
    }
    else
    {
      /* Otherwise, just get the one that was requested */

      ul=doing_ul;
      
      got=(ul ? ulstr : dlstr);

      if (!got)
        continue;
    }

    /* Now copy the filename, based on the word# we're supposed to get. */

    getword(got, fname, ctl_delim, pro->fnamword+1);

    /* Skip it if we didn't get anything */

    if (*fname=='\0')
      continue;
    
    /* Now figure out if we requested this file or not... */
    
    for (fn=0; GetFileEntry(fn, &fent); fn++)
      if (eqstri(No_Path(fent.szName), No_Path(fname)))
        break;

    if (fn==FileEntries())
      memset(&fent, 0, sizeof fent);
      
    /* Grab the description, if any */

    desc=pro->descword ? firstchar(got, ctl_delim, pro->descword) : NULL;

    if (desc)
      fent.szDesc=desc;

    /* If we're doing an upload, strip the path specification */

    if (ul)
    {
      strcpy(tempname, No_Path(fname));
      fent.szName=tempname;

      /* Now figure out the fully-qualified path/name of the file so
       * that we can get the correct size.
       */

      sprintf(fname, ss, FAS(fah, uppath), fent.szName);
    }
    else
    {
      fent.szName=fname;
    }

    /* Now get the size of this file */
    
    fent.ulSize = fsize(fent.szName);

    if (ul)
    {
      logit(log_ul, *pro->desc, FAS(fah, uppath), fent.szName);
      fent.fFlags |= FFLAG_GOT;
    }
    else
    {
      fent.fFlags |= FFLAG_SENT;
    }

    UpdFileEntry(fn, &fent);
    gotfile=TRUE;
  }

  fclose(log);

  /* If the transfer was successful, delete both the .CTL and .LOG files */

  if (gotfile)
  {
    Parse_Outside_Cmd(pro->log, fname);
    unlink(fname);

    Parse_Outside_Cmd(pro->ctl, fname);
    unlink(fname);
  }
  
  return gotfile;
}


