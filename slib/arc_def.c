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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alc.h"
#include "prog.h"
#include "arc_def.h"

static char *cfgdelim=" \t\n\b";

struct _arcinfo * _fast Parse_Arc_Control_File(char *afname)
{
  struct _arcinfo *start, *ari;

  FILE *af;
  
  char word1[PATHLEN];
  char temp[PATHLEN];
  
  char *word2, *line, *p, *o, *ln;
  
  int tempx;

  
  if ((line=malloc(PATHLEN))==NULL)
    return NULL;

  if ((af=fopen(afname, "r"))==NULL)
  {
    free(line);
    return NULL;
  }
  
  ln=line;
  start=ari=NULL;
  
  while (fgets(line,PATHLEN,af) != NULL)
  {
    ln=line;

    Strip_Trailing(ln,'\n');

again:
    if ((p=strchr(ln,';')) != NULL)
      *p='\0';

    getword(ln,word1,cfgdelim,1);
    word2=firstchar(ln,cfgdelim,2);

    if (! *word1)
      continue;

#if defined(__MSDOS__)
    if (eqstri(word1, "os2") || eqstri(word1, "nt"))
#elif defined(OS_2)
    if (eqstri(word1, "dos") || eqstri(word1, "nt"))
#elif defined(NT)
    if (eqstri(word1, "dos") || eqstri(word1, "os2"))
#elif defined(UNIX)
    if (eqstri(word1, "dos") || eqstri(word1, "os2") || eqstri(word1, "nt"))
#else
    #error invalid operating system
#endif
      continue;
#if defined(__MSDOS__)
    else if (eqstri(word1, "dos"))
#elif defined(OS_2)
    else if (eqstri(word1, "os2"))
#elif defined(NT)
    else if (eqstri(word1, "nt"))
#elif defined(UNIX)
    else if (eqstri(word1, "unix"))
#else
    #error invalid operating system
#endif
    {
      ln=word2;
      goto again;
    }

    if (! word2)
    {
      printf("Error in archiver config file: `%s'?\n",ln);
      continue;
    }

    if (eqstri(word1, "archiver"))
    {
      if ((ari=malloc(sizeof(struct _arcinfo)))==NULL)
        return NULL;
      
      memset(ari, '\0', sizeof(struct _arcinfo));

      ari->next=start;
      start=ari;

      if ((ari->arcname=strdup(word2))==NULL)
        return NULL;
    }
    else if (eqstri(word1, "extension"))
    {
      if (ari && (ari->extension=strdup(word2))==NULL)
        return NULL;
    }
    else if (eqstri(word1, "add"))
    {
      if (ari && (ari->add=strdup(word2))==NULL)
        return NULL;
    }
    else if (eqstri(word1, "extract"))
    {
      if (ari && (ari->extract=strdup(word2))==NULL)
        return NULL;
    }
    else if (eqstri(word1, "view"))
    {
      if (ari && (ari->view=strdup(word2))==NULL)
        return NULL;
    }
    else if (eqstri(word1, "ident") && ari)
    {
      ari->id_ofs=atol(word2);

      if ((p=strchr(word2, ','))==NULL)
      {
        printf("Invalid archiver definition: `%s'!\n",word2);
        continue;
      }

      /* Now convert the hex bytes into a string */

      for (o=temp, p++; *p; p += 2)
        if (sscanf(p, "%02x", &tempx)==1)
          *o++=(char)tempx;

      *o='\0';

      /* And copy it into the 'ari' structure */

      if ((ari->id=strdup(temp))==NULL)
        return NULL;
    }
    else if (eqstri(word1, "end"))
      ari=NULL;
  }
  
  fclose(af);
  free(ln);
  return start;
}


