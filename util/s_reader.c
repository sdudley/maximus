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
static char rcs_id[]="$Id: s_reader.c,v 1.1.1.1 2002/10/01 17:57:52 sdudley Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Reader' processing logic
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


int Parse_Reader(FILE *ctlfile)
{
  char temp[MAX_LINE],  /* Contains entire line */
       p[MAX_LINE];     /* First word on line */

  linenum++;

  while (fgets(line,MAX_LINE,ctlfile))
  {
    Strip_Comment(line);

    if (*line)
    {
      strcpy(temp,line);

      getword(line,p,ctl_delim,1);

      if (! *p)
        ;
      else if (eqstri(p,"end"))
        break;
      else if (eqstri(p,"archivers"))
      {
        getword(line,p,ctl_delim,2);
        Make_Filename(prm.arc_ctl,p);
      }
      else if (eqstri(p,"packet"))
      {
        getword(line,p,ctl_delim,3);
        p[8]='\0';
        Make_String(prm.olr_name,p);
      }
      else if (eqstri(p,"work"))
      {
        getword(line,p,ctl_delim,3);
        Make_Path(prm.olr_dir,p);
        
        if (! direxist(p))
          makedir(p);
      }
      else if (eqstri(p,"phone"))
      {
        Make_String(prm.phone_num, fchar(line, ctl_delim, 3));
      }
      else if (eqstri(p, "max"))
      {
        getword(line, p, ctl_delim, 3);
        prm.max_pack=(word)atoi(p);
      }
      else if (eqstri(p,"app") || eqstri(p,"application"))
        ;
      else Unknown_Ctl(linenum,p);
    }

    linenum++;
  }

  linenum++;

  return 0;
}




