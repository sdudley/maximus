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
#include "ar.h"
#include "prog.h"
#include "arc.h"
#include "ffind.h"

static void near usage(void);
static void near err_open(char *name);
static void near err_write(char *name);
static void store(void);

#define FALSE 0
#define TRUE 1

int main(int argc, char *argv[])
{
  char outname[PATHLEN];
  char line[PATHLEN];
  char temp[PATHLEN];
  char *file;
  FFIND *ff;
  FILE *fp=NULL;
  word dofile, i;

  if (argc < 3)
    usage();

  strcpy(outname, argv[1]);

  if (!strchr(outname, '.'))
    strcat(outname, ".fiz");

  if ((outfile=fopen(outname, "w+b"))==NULL)
    err_open(temp);

  make_crctable();

  for (i=2; i < argc; i++)
  {
    dofile=FALSE;
      
    if (*argv[i]=='@')
    {
      dofile=TRUE;

      argv[i]++;
      
      if ((fp=fopen(argv[i], "r"))==NULL)
      {
        printf("Error reading %s!\n", argv[i]);
        continue;
      }
    }

    while (!dofile || fgets(line, PATHLEN, fp))
    {
      if (fp)
      {
        word x;

        if (line[x=strlen(line)-1]=='\n')
          line[x]='\0';

        file=line;
      }
      else file=argv[i];
      
      if ((ff=FindOpen(file, 0))==NULL)
      {
        printf("can't find %s\n", file);
        continue;
      }

      do
      {
        char *p;

        strcpy(temp, file);

        p=strrchr(temp, '\\');

        if (p)
          p[1]='\0';
        else *temp='\0';

        strcat(temp, ff->szName);

        if (stricmp(temp, outname) != 0)
          arcit(temp);
      }
      while (FindNext(ff)==0);

      FindClose(ff);

      if (!dofile)
        break;
    }
    
    if (dofile)
    {
      fclose(fp);
      fp=NULL;
    }
  }

  fclose(outfile);

  return 0;
}



void arcit(char *inname)
{
  struct _fizhdr fh;
  char *p;
  long pos;

  if ((infile=fopen(inname, "rb"))==NULL)
    err_open(inname);

  printf("archiving %s", inname);
  fflush(stdout);

  origsize=compsize=unpackable=0;
  
  crc=INIT_CRC;
  
  pos=ftell(outfile);
  
  if (fwrite((char *)&fh, sizeof(fh), 1, outfile) != 1)
    err_write("archive");

  /* strip off the path specification */

  if ((p=strrchr(inname, '\\'))==NULL)
    p=inname;
  else p++;

  if (fwrite(p, strlen(p), 1, outfile) != 1)
    err_write("archive");

  
  encode();

  if (unpackable)
  {
    printf("storing");
    fflush(stdout);

    fh.method=0;
    fseek(outfile, pos+sizeof(fh)+strlen(p), SEEK_SET);
    fseek(infile, 0L, SEEK_SET);
    store();
  }
  else
  {
    fh.method=1;
  }

  fh.id=FIZ_ID;
  fh.fnlen=strlen(p);
  fh.origsize=origsize;
  fh.compsize=compsize;
  fh.crc = crc;
  get_fdt(fileno(infile), &fh.date);

  fseek(outfile, pos, SEEK_SET);

  if (fwrite((char *)&fh, sizeof(fh), 1, outfile) != 1)
    err_write("archive");

  fseek(outfile, 0L, SEEK_END);

  fclose(infile);

  printf("\n");
}


static void near usage(void)
{
  printf("format: arc arcname filename...\n");
  exit(1);
}

static void near err_open(char *name)
{
  printf("Fatal error opening `%s'!\n", name);
  exit(1);
}

static void near err_write(char *name)
{
  printf("Fatal error writing to `%s'!\n", name);
  exit(1);
}

static void store(void)
{
    uint n;
    byte *buffer=malloc(DICSIZ);

    if (buffer==NULL)
    {
      printf("nomem\n");
      exit(1);
    }

    origsize = 0;
    crc = INIT_CRC;
    while ((n = fread((char *)buffer, 1, DICSIZ, infile)) != 0) {
        fwrite_crc(buffer, n, outfile);  origsize += n;
    }
    compsize = origsize;

    free(buffer);
}


