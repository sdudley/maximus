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
static char rcs_id[]="$Id: max_rest.c,v 1.1.1.1 2002/10/01 17:52:00 sdudley Exp $";
#pragma on(unreferenced)


/*# name=System restart logic (for "-r" cmd-line param)
*/

#define MAX_INCL_COMMS

#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "max_file.h"

#ifdef __MSDOS__

int System_Restart(char *restart_name)
{
  struct _restart *rst;

  char temp[PATHLEN];

  int type;
  int userfile;


  /* Read the LASTUSER info */

  if (task_num)
    sprintf(temp, lastusxx_bbs, original_path, task_num);
  else sprintf(temp, lastuser_bbs, original_path);

  if ((userfile=shopen(temp, O_RDONLY | O_BINARY))==-1)
  {
    cant_open(temp);
    quit(ERROR_FILE);
  }

  read(userfile, (char *)&usr, sizeof(struct _usr));
  close(userfile);

  /* Restore the user's 'delflag' bits, since the usr.delflag was used      *
   * to write the speed for external programs.                              */

  usr.delflag=usr.df_save;

  /* Convert the user's name/alias into one general string */

  SetUserName(&usr, usrname);

  /* And now read the system restart info */

  sprintf(temp, restarxx_bbs, original_path, task_num);

  if ((userfile=shopen(temp, O_RDONLY | O_BINARY))==-1)
  {
    cant_open(temp);
    quit(ERROR_FILE);
  }
  
  if ((rst=malloc(sizeof(struct _restart)))==NULL)
  {
    logit(mem_none);
    quit(ERROR_CRITICAL);
  }

  read(userfile, (char *)rst, sizeof(struct _restart));
  close(userfile);

  /* Delete it, now that we have it */
  unlink(temp);

  strcpy(restart_name, rst->restart_name);
  strcpy(menupath, rst->menupath);
  strcpy(main_menu, rst->menuname);
  strcpy(firstname, rst->firstname);
  next_ludate=rst->next_ludate;
  strcpy(last_onexit, rst->last_onexit);

  strcpy(fix_menupath, rst->fix_menupath);

  mn_dirty=rst->mn_dirty;


  lastmenu=rst->lastmenu;
  local=rst->local;
  port=rst->port;
  snoop=rst->snoop;
  keyboard=rst->keyboard;
  protocol_letter=rst->protocol_letter;
  chatreq=rst->chatreq;

  timeon=rst->timeon;
  timeoff=rst->timeoff;
  timestart=rst->timestart;
  rst_offset=rst->restart_offset;

  usr.time=rst->usr_time;
  ultoday=rst->ultoday;
  
  no_zmodem=rst->no_zmodem;

  locked=rst->locked;

  lockpriv=rst->lockpriv;

  written_echomail=rst->written_echomail;
  written_matrix=rst->written_matrix;

  current_baud=rst->current_baud;
  steady_baud=rst->steady_baud;
  steady_baud_l=rst->steady_baud_l;
  date_newfile=rst->date_newfile;
  baud=rst->baud;
  event_num=rst->event_num;

  max_time=rst->max_time;
  getoff=rst->getoff;
  origusr=rst->origusr;
  
  barricade_ok=rst->barricade_ok;

  restore_tag_list(NULL,FALSE);

  ChatRestoreStatus(&rst->css);

  /* Read our message-area tags */

  TagReadTagFile(&mtm);

  /* Now open the old log file */

  strcpy(log_name, rst->log_name);
  
  if (! LogOpen())
    quit(ERROR_CRITICAL);
  
/*  memcpy(echo_written_in, rst->echo_written_in, sizeof(echo_written_in));*/

  caller_online=TRUE;

  /* Do this check for the status line here...  We couldn't do so          *
   * previously, since we didn't figure out whether or not the user was    *
   * local or not until now.                                               */

  if (local)
    prm.flags &= ~FLAG_statusline;

  /* Find class number & validate runtime settings */

  Validate_Runtime_Settings();

  Switch_To_Language();
  Fossil_Install(TRUE);

  if (! local)
    mdm_baud(current_baud);

  if ((prm.flags & FLAG_watchdog) && !local)
    mdm_watchdog(0);

  logit(log_ret_from_app);
  fLoggedOn=TRUE;

  mdm_attr=curattr=-1;
  
  Puts(GRAY);
  
  if (*rst->returning)
  {
    Display_File(0, NULL, rst->returning);
    vbuf_flush();
  }

  /* Handle an xtern_erlvl protocol doodad */

  if (rst->ctltype != CTL_NONE)
  {
    struct _proto *pro;
    word fn;
    word got=0;

    if ((pro=malloc(sizeof(struct _proto))) != NULL &&
        FindProtocol(rst->last_protocol, pro))
    {
      FENTRY fent;

      MaxReadLog(pro, rst->ctltype==CTL_UPLOAD);
      free(pro);
      
      for (fn=0; GetFileEntry(fn, &fent); fn++)
      {
        if (fent.fFlags & FFLAG_SENT)
        {
          if (!got)
            got=1;
          MaxSentFile(fn, TRUE, -1L);
        }
      }

      for (fn=0; GetFileEntry(fn, &fent); fn++)
      {
        if ((got != -1) && (fent.fFlags & FFLAG_GOT))
        {
          got=-1;
          File_Process_Uploads(0L, rst->last_protocol, FAS(fah, uppath));
          break;
        }
      }

      if (got==1)
      {
        Puts(xfercomplete);
        Putc('\n');
      }
      else if (got==0)
        Puts(xferaborted);
      
      Free_Filenames_Buffer(0);
    }
  }

  type=rst->restart_type;
  free(rst);

  return type;
}


/* Writes the RESTARxx.BBS file to the right directory. */

void Write_Restart(char restart_type, char *restart_name, long restart_offset, int usrtime, int ctltype, char *parm, long starttime, struct _css *css, char *returning)
{
  struct _restart *rst;
  char temp[PATHLEN];
  int file;

  if ((rst=malloc(sizeof(struct _restart)))==NULL)
  {
    logit(mem_none);
    return;
  }
  
  /* Now fill in the "restart" part of the structure */

  memset(rst, '\0', sizeof(struct _restart));

  rst->rst_ver=RST_VER;
  rst->origusr=origusr;
  rst->restart_type=restart_type;
  strcpy(rst->restart_name, restart_name ? restart_name : PRM(first_menu));
  strcpy(rst->menupath,menupath);
  strcpy(rst->menuname, CurrentMenuName());
  strcpy(rst->firstname,firstname);
  rst->next_ludate=next_ludate;
  strcpy(rst->last_onexit,last_onexit);
  rst->laston=usr.ludate.msg_st;

  strcpy(rst->fix_menupath,fix_menupath);
  rst->mn_dirty=mn_dirty;

  rst->lastmenu=lastmenu;
  rst->local=local;
  rst->port=port;
  rst->snoop=snoop;
  rst->keyboard=keyboard;
  rst->protocol_letter=protocol_letter;
  rst->chatreq=chatreq;

  rst->no_zmodem=no_zmodem;
  rst->timeon=timeon;
  rst->timeoff=timeoff;
  rst->timestart=timestart;
  rst->restart_offset=restart_offset;

  rst->usr_time=usrtime;
  rst->ultoday=ultoday;
  rst->written_echomail=written_echomail;
  rst->written_matrix=written_matrix;
  rst->current_baud=current_baud;
  rst->steady_baud=steady_baud;
  rst->date_newfile=date_newfile;
  rst->steady_baud_l=steady_baud_l;
  rst->baud=baud;
  rst->max_time=max_time;
  rst->getoff=getoff;
  rst->event_num=event_num;

  rst->ctltype=ctltype;
  strcpy(rst->parm,parm);
  rst->starttime=starttime;

/*  memcpy(rst->echo_written_in, echo_written_in, sizeof(echo_written_in));*/

  if (returning)
    strcpy(rst->returning, returning);

  rst->locked=locked;

  rst->lockpriv=lockpriv;

  rst->barricade_ok=barricade_ok;
  
  rst->css=*css;
  rst->last_protocol=last_protocol;

  strcpy(rst->log_name, log_name);

  /* Make sure that we don't delete the RESTARxx.BBS file when exiting... */
  if (rst_offset==-1L)
    rst_offset=0L;

  sprintf(temp,restarxx_bbs,original_path,task_num);

  if ((file=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                  SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
    cant_open(temp);
  else
  {
    write(file, (char *)rst, sizeof(struct _restart));
    close(file);
  }
  
  free(rst);

  save_tag_list(NULL);
}

#endif /* __MSDOS__ */

