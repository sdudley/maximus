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
static char rcs_id[]="$Id: s_lang.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Language' processing logic
*/

#define SILT
#define MAX_INCL_LANGUAGE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"
#include "dr.h"
#include "language.h"


int Parse_Language(FILE *ctlfile)
{
  struct _gheapinf gi;
  struct _heapdata h0, h1;

  int lf;

  char temp[MAX_LINE],  /* Contains entire line */
       p[MAX_LINE];     /* First word on line */

  linenum++;

  for (;;)
  {
    if (fgets(line,MAX_LINE,ctlfile) != NULL)
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
        else if (eqstri(p,"language"))
        {
          done_language=TRUE;

          getword(line,p,ctl_delim,2);

          Make_String(prm.lang_file[prm.max_lang],p);

          /* Now open the language file, to find out how big the largest     *
           * heap is.                                                        */

#ifndef UNIX
          sprintf(temp,"%s%s.LTF",strings+prm.lang_path,p);
#else
          sprintf(temp,"%s%s.ltf",strings+prm.lang_path,p);
#endif
          if ((lf=shopen(temp,O_RDONLY | O_BINARY))==-1)
          {
            printf("\nFatal error opening language file `%s'!\n",temp);
            exit(1);
          }

          read(lf,(char *)&gi,sizeof(struct _gheapinf));
          lseek(lf, gi.hn_len, SEEK_CUR); /* Skip over names heap */
          read(lf,(char *)&h0,sizeof(struct _heapdata));
          read(lf,(char *)&h1,sizeof(struct _heapdata));
          close(lf);

          /* Find the largest heap size out of everything */

          if (prm.max_heap < gi.max_gheap_len)
            prm.max_heap=gi.max_gheap_len;

          if (prm.max_ptrs < gi.max_gptrs_len)
            prm.max_ptrs=gi.max_gptrs_len;

          /* Now find the size we need for the largest global heap */

          if (prm.max_glh_ptrs < (h0.ndefs+h0.adefs)*sizeof(HOFS))
            prm.max_glh_ptrs=(h0.ndefs+h0.adefs)*sizeof(HOFS);

          if (prm.max_glh_len < h0.hlen)
            prm.max_glh_len=h0.hlen;

          /* We only have one sysop heap, so only check it once */
          if (prm.max_lang==0)
          {
            prm.max_syh_ptrs=(h1.ndefs+h1.adefs)*sizeof(HOFS);
            prm.max_syh_len=h1.hlen;
          }

          prm.max_lang++;
        }
        else if (eqstri(p,"app") || eqstri(p,"application"))
          ;
        else Unknown_Ctl(linenum,p);
      }

      linenum++;
    }
    else break;
  }

  linenum++;

  return 0;
}




