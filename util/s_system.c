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
static char rcs_id[]="$Id: s_system.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section System' processing logic
*/

#define SILT
#define NOVARS
#define NOINIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"
#include "dr.h"

int Parse_System(FILE *ctlfile)
{
  int x;

  char temp[MAX_LINE];  /* Contains entire line */
  char p[MAX_LINE];     /* First word on line */
  char *s2;

  linenum++;

  while (fgets(line, MAX_LINE, ctlfile))
  {
    Strip_Comment(line);

    if (*line)
    {
      strcpy(temp, line);

      getword(line, p, ctl_delim, 1);

      if (! *p)
        ;
      else if (eqstri(p, "end"))     /* End system section */
        break;
      else if (eqstri(p, "name"))
      {
        s2=fchar(line, ctl_delim, 2);

        if (strlen(s2) > 60)
          s2[61]='\0';

        Make_String(prm.system_name, s2);
      }
      else if (eqstri(p, "sysop"))
      {
        s2=fchar(line,ctl_delim,2);
        Make_String(prm.sysop,s2);
      }
      else if (eqstri(p, "snoop"))
        prm.flags |= FLAG_snoop;
      else if (eqstri(p, "video"))
      {
        getword(line, p, ctl_delim, 2);

#if 0
        if (eqstri(p, "fossil"))
          prm.video=VIDEO_FOSSIL;
        else if (eqstri(p, "dos"))
          prm.video=VIDEO_DOS;
        else if (eqstri(p, "fast"))
          prm.video=VIDEO_FAST;
        else
#endif
        if (eqstri(p, "bios"))
          prm.video=VIDEO_BIOS;
        else if (eqstri(p, "ibm"))
          prm.video=VIDEO_IBM;
        else if (eqstri(p, "ibm/snow"))
        {
          prm.video=VIDEO_IBM;
          prm.flags2 |= FLAG2_has_snow;
        }
        else Unknown_Ctl(linenum, p);
      }
      else if (eqstri(p, "path"))
      {
        getword(line, p, ctl_delim, 2);

        if (eqstri(p,"system"))
        {
          s2=fchar(line, ctl_delim, 3);

#if 0
          if (*s2=='\0' || s2[1] != ':' || s2[2] != '\\')
          {
            printf("\n\aError!  `Path System' on line %d must have a fully-qualified path!\n"
                   "ie.  Path System D:\\Path\\\n",linenum);
            exit(1);
          }
#endif

          Make_Path(prm.sys_path,s2);

          if (! direxist(s2))
            makedir(s2);

          if (! prm.net_info)
            prm.net_info=prm.sys_path;
        }
        else if (eqstri(p, "misc"))
        {
          s2=fchar(line, ctl_delim, 3);
          Make_Path(prm.misc_path, s2);

          if (! direxist(s2))
            makedir(s2);
        }
        else if (eqstri(p,"language"))
        {
          s2=fchar(line, ctl_delim, 3);
          Make_Path(prm.lang_path, s2);

          if (! direxist(s2))
            makedir(s2);
        }
        else if (eqstri(p,"temp"))
        {
          s2=fchar(line, ctl_delim, 3);
          Make_Path(prm.temppath, s2);

          if (! direxist(s2))
            makedir(s2);

          sprintf(temp, "%s$TEMP$.$$$", strings+prm.temppath);

          /* Create an empty file in this directory, to make sure we     *
           * don't have the NetBIOS problem...                           */

          if ((x=open(temp, O_WRONLY | O_CREAT | O_TRUNC,S_IREAD | S_IWRITE)) != -1)
            close(x);
        }
        else if (eqstri(p, "outbound"))
        {
          s2=fchar(line, ctl_delim, 3);
          Make_Path(prm.junk, s2);
        }
        else if (eqstri(p, "inbound"))
        {
          s2=fchar(line, ctl_delim, 3);
          Make_Path(prm.inbound_files, s2);
        }
        else if (eqstri(p,"ipc"))
        {
          s2=fchar(line, ctl_delim, 3);
          Make_Path(prm.ipc_path, s2);

          if (! direxist(s2))
            makedir(s2);
        }
        else Unknown_Ctl(linenum, p);
      }
      else if ((x=1,eqstri(p, "uses")) || (x=2,eqstri(p, "file")))
      {
        getword(line, p, ctl_delim, 2);
        s2=fchar(line, ctl_delim, 3);

        Add_Filename(s2);

        if (eqstri(p,"password"))
          Make_Filename(prm.user_file, s2)
        else if (eqstri(p,"access"))
          Make_Filename(prm.access, s2)
        else if (eqstri(p,"callers"))
          Make_Filename(prm.caller_log, s2)
        else Unknown_Ctl(linenum, p);

        if (x==1 && do_prm)           /* USES statement */
          if (! fexist(s2))
          {
            *strrchr(s2,'.')='\0';
            printf(cantfind_file_ctl, s2, linenum);
            Compiling(-1, NULL, NULL);
          }
      }
      else if (eqstri(p,"log"))         /* Logging */
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"file"))
        {
          char *p;
          
          strcpy(temp, fchar(line, ctl_delim, 3));
          Make_Filename(prm.log_name, temp);

          /* Make sure that the logfile path exists */

          if ((p=strrchr(temp, '\\')) != NULL)
          {
            if (p[1]==':' || p[0]=='\\')
              p[1]=0;
            else
              *p='\0';
            
            if (!direxist(temp))
              makedir(temp);
          }
        }
        else if (eqstri(p, "mode") || eqstri(p, "level"))
        {
          getword(line, p, ctl_delim, 3);

          if (eqstri(p,"terse"))
            prm.log_mode=LOG_terse;
          else if (eqstri(p,"verbose"))
            prm.log_mode=LOG_verbose;
          else if (eqstri(p,"trace"))
            prm.log_mode=LOG_trace;
          else if (isdigit(*p))
            prm.log_mode=(byte)atoi(p);
          else
          {
            Unknown_Ctl(linenum, p);
            prm.log_mode=LOG_terse;
          }
        }
        else Unknown_Ctl(linenum, p);
      }
      else if (eqstri(p, "task"))         /* Task number */
        prm.task_num=(char)atoi(fchar(line, ctl_delim, 2));
      else if (eqstri(p, "multitasker"))  /* Multitasker */
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p, "none"))
          prm.multitasker=MULTITASKER_NONE;
        else if (eqstri(p, "auto"))
          prm.multitasker=MULTITASKER_AUTO;
        else if (eqstri(p, "doubledos"))
          prm.multitasker=MULTITASKER_DOUBLEDOS;
        else if (eqstri(p, "desqview"))
          prm.multitasker=MULTITASKER_DESQVIEW;
        else if (eqstri(p, "topview"))
          prm.multitasker=MULTITASKER_TOPVIEW;
        else if (eqstri(p, "multilink"))
          prm.multitasker=MULTITASKER_MLINK;
        else if (eqstri(p, "mswindows"))
          prm.multitasker=MULTITASKER_MSWINDOWS;
        else if (eqstri(p, "pc-mos"))
          prm.multitasker=MULTITASKER_PCMOS;
        else if (eqstri(p, "os/2"))
          prm.multitasker=MULTITASKER_OS2;
        else if (eqstri(p, "UNIX"))
          prm.multitasker=MULTITASKER_UNIX;
        else Unknown_Ctl(linenum, p);
      }
      else if (eqstri(p, "mcp")) /* MCP stuff */
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"pipe"))      /* Close all standard files */
        {
          s2=fchar(line, ctl_delim, 3);
          Make_String(prm.mcp_pipe, s2);
        }
        else if (eqstri(p, "sessions"))
        {
          getword(line, p, ctl_delim, 3);

          prm.mcp_sessions = atoi(p);

          if (prm.mcp_sessions==0)
          {
            printf("\n\aError!  Session count cannot be 0 in '%s' (line %d)!\n",
                   p, linenum);
            Compiling(-1, NULL, NULL);
          }
        }
        else
          Unknown_Ctl(linenum, p);
      }
      else if (eqstri(p, "reboot"))  /* Watchdog */
        prm.flags |= FLAG_watchdog;
      else if (eqstri(p, "swap"))
        prm.flags2 |= FLAG2_SWAPOUT;
      else if (eqstri(p, "dos"))     /* DOS stuff */
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"close"))      /* Close all standard files */
          prm.flags |= FLAG_close_sf;
        else Unknown_Ctl(linenum, p);
      }
      else if (eqstri(p, "no"))
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"share") || eqstri(p,"share.exe"))
          prm.flags2 |= FLAG2_noshare;
        else if (eqstri(p,"password"))
          prm.flags2 |= FLAG2_NOENCRYPT;
        else Unknown_Ctl(linenum, p);
      }
      else if (eqstri(p, "app") || eqstri(p, "application"))
        ;
      else Unknown_Ctl(linenum, p);
    }

    linenum++;
  }

  linenum++;

  return 0;
}


