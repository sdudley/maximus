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
static char rcs_id[]="$Id: s_proto.c,v 1.1.1.1 2002/10/01 17:57:52 sdudley Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Protocol XXXXX processing logic
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
#include "dr.h"


int Parse_Protocol(FILE *ctlfile, char *name)
{
  struct _proto pro;

  char temp[MAX_LINE];  /* Contains entire line */
  char p[MAX_LINE];     /* First word on line */

  linenum++;

  memset(&pro, '\0', sizeof(struct _proto));

  while (fgets(line,MAX_LINE,ctlfile))
  {
    char word2[PATHLEN];
    char quote2[PATHLEN];
    char *line2;

    Strip_Comment(line);

    if (*line)
    {
      strcpy(temp,line);

      getword(line, p, ctl_delim, 1);
      getword(line, word2, ctl_delim, 2);
      line2=fchar(line, ctl_delim, 2);
      strcpy(quote2, line2);

      /* Now handle a quoted string */

      if (*line2=='"')
      {
        char *p;
        char *o;
        
        for (p=line2+1, o=quote2; *p; p++)
        {
          if (*p=='"')
          {
            if (p[1]=='"')
              *o++='"';
            else break;
          }
          else *o++=*p;
        }

        *o='\0';
      }

      if (! *p)
        ;
      else if (eqstri(p, "end"))
        break;
      else if (eqstri(p, "logfile"))
        strnncpy(pro.log, word2, PATHLEN);
      else if (eqstri(p, "controlfile"))
        strnncpy(pro.ctl, word2, PATHLEN);
      else if (eqstri(p, "downloadcmd"))
        strnncpy(pro.dlcmd, line2, PATHLEN);
      else if (eqstri(p, "uploadcmd"))
        strnncpy(pro.ulcmd, line2, PATHLEN);
      else if (eqstri(p, "downloadstring"))
        strnncpy(pro.dlstr, line2, 40);
      else if (eqstri(p, "uploadstring"))
        strnncpy(pro.ulstr, line2, 40);
      else if (eqstri(p, "downloadkeyword"))
        strnncpy(pro.dlkey, quote2, 40);
      else if (eqstri(p, "uploadkeyword"))
        strnncpy(pro.ulkey, quote2, 40);
      else if (eqstri(p, "filenameword"))
        pro.fnamword=atoi(word2);
      else if (eqstri(p, "descriptword"))
        pro.descword=atoi(word2);
      else if (eqstri(p, "type"))
      {
        if (eqstri(word2, "batch"))
          pro.flag |= P_BATCH;
        else if (eqstri(word2, "bi"))
          pro.flag |= P_BI;
        else if (eqstri(word2, "opus"))
          pro.flag |= P_OPUS;
        else if (eqstri(word2, "errorlevel"))
          pro.flag |= P_ERL;
        else Unknown_Ctl(linenum, word2);
      }
      else if (eqstri(p,"app") || eqstri(p,"application"))
        ;
      else Unknown_Ctl(linenum,p);
    }

    linenum++;
  }

  linenum++;
  
  strnncpy(pro.desc, name, 40);
  

  if (*pro.desc && do_prm)
  {
    int fd;
    
    if (protocol_num==MAX_EXTERNP)
    {
      printf("\n\aError!  Too many external protocols, "
             "line %d.\n",linenum);
      exit(1);
    }
    
    pro.flag |= P_ISPROTO;
    
    Make_String(prm.protoname[protocol_num], name)
    prm.protocols[protocol_num]=0;
    prm.protoflag[protocol_num]=0;

    if (pro.flag & P_BATCH)
      prm.protoflag[protocol_num] |= XTERNBATCH;

    if (pro.flag & P_ERL)
      prm.protoflag[protocol_num] |= XTERNEXIT;

    protocol_num++;
  
    if (strings[prm.protocol_max])
      strcpy(temp, strings+prm.protocol_max);
    else sprintf(temp, protocol_max, strings+prm.sys_path);

    if ((fd=open(temp, O_CREAT | O_WRONLY | O_APPEND | O_BINARY |
                 (protocol_num==1 ? O_TRUNC : 0),
                 S_IREAD | S_IWRITE))==-1)
    {
      printf("\n\aCan't open %s!\n", temp);
      exit(1);
    }
    
    write(fd, (char *)&pro, sizeof(struct _proto));
    
    close(fd);
  }

  return 0;
}





