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

/* $Id: f_intrin.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=File area routines: Intrinsic functions
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max_file.h"
#include "display.h"
#include "max_menu.h"


int Exec_File(int type, char **result, char *menuname)
{
  *result=NULL;

  switch (type)
  {
    case newfiles:
      *linebuf='*';
      linebuf[1]='\0';
      File_Locate();
      break;

      /* fall-through */

    case file_area:     File_Area();              break;
    case locate:        File_Locate();            break;
    case file_titles:   File_Titles();            break;
    case file_type:     File_Type();              break;
    case upload:        File_Upload(menuname);    break;
    case download:      File_Download(menuname);  break;
    case raw:           File_Raw();               break;
    case file_kill:     File_Kill();              break;
    case contents:      File_Contents();          break;
    case file_hurl:     File_Hurl();              break;
    case override_path: File_Override_Path();     break;
    case file_tag:      File_Tag(FALSE);          break;
    default:            logit(bad_menu_opt, type); return 0;
  }
  
  return 0;
}

char * Protocol_Name(sword protocol,char *temp)
{
  struct _proto_str *ps;

  *temp='\0';

  if (protocol >= 0 && protocol < MAX_EXTERNP)    /* External protocol */
  {
    strcpy(temp, PRM(protoname[protocol]));
  }
  else
  {
    for (ps=intern_proto;ps->name;ps++)
      if (ps->num==protocol)
      {
        strcpy(temp, ps->name);
        break;
      }
  }

  return temp;
}



long XferTime(sword protocol,long bytes)
{
  long cps,
       seconds;

  if (!bytes)
    return 0L;

  if (local || !baud)
    baud=38400L;

  cps=baud/10;
  seconds=bytes/cps;

  switch(protocol)
  {
    case PROTOCOL_XMODEM: /* 72% efficiency: 100/.72==138 */
/*    case PROTOCOL_TELINK:*/
      seconds=(seconds*138)/100;
      break;

    case PROTOCOL_XMODEM1K: /* 91% efficiency: 100/.91==110 */
    case PROTOCOL_YMODEM:
      seconds=(seconds*110)/100;
      break;

    case PROTOCOL_SEALINK:/* 93% efficiency: 100/.93==107 */
      seconds=(seconds*107)/100;
      break;

    case PROTOCOL_NONE:     /* ?!? */
    case PROTOCOL_ZMODEM: /* 96% efficiency: 100/.96==104 */
    case PROTOCOL_YMODEMG:
      seconds=(seconds*104)/100;
      break;

    default:              /* External protocol */
      seconds=(seconds*111)/100;  /* Avg. 90% efficiency: 100/.90==111 */
      break;
  }

  return seconds;
}



void File_IsOffline(char *filename)
{
  Printf(file_offl, No_Path(fancy_fn(filename)));
  *filename='\0';
}


void File_NotExist(char *filename)
{
  Printf(iseenoxhere, No_Path(fancy_fn(filename)));
  *filename='\0';
}




void Strip_Path(char *filename)
{
  byte *p;

  if (filename==NULL)
    return;

  while ((p=strrchr(filename, '\\')) != NULL ||
         (p=strrchr(filename, '/')) != NULL ||
         (p=strrchr(filename, ':')) != NULL)
  {
    if (p==NULL)
      return;
    
    *p='\0';
    logit(supp_path, fancy_fn(filename));
    strocpy(filename, p+1);
  }

  if (*filename && is_devicename(filename))
  {
    logit(udev, fancy_fn(filename));
    *filename='\0';
    return;
  }

  filename[MAX_FN_LEN]='\0';
}


sword IsBatch(sword protocol)
{
  if (protocol==PROTOCOL_XMODEM || protocol==PROTOCOL_XMODEM1K ||
      (protocol >= 0 && !(prm.protoflag[protocol] & XTERNBATCH)))
    return FALSE;
  else return TRUE;
}





