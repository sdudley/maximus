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
static char rcs_id[]="$Id: piper.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

#error Obsolete.  This program is no longer in the distribution

/*# name=Conversion program for Opus-style SYSTEM*.BBS/DAT files
*/

#define MAX_INCL_VER

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "prog.h"
#include "ffind.h"
#include "max.h"


   #define _TWIT      0x10 /* Twit .......... Minimum access. Eg, Problems */
   #define _DISGRACE  0x30 /* Disgraced ..... Eg, 1st time callers         */
   #define _LIMITED   0x40 /* Limited ....... Eg, 1st time callers         */
   #define _NORMAL    0x50 /* Normal ........ Eg, Regular callers          */
   #define _WORTHY    0x60 /* Worthy ........ Eg, Approved callers         */
   #define _PRIVEL    0x70 /* Privileged .... Eg, Full access callers      */
   #define _FAVORED   0x80 /* Favored ....... Eg, Friends or Helpers       */
   #define _EXTRA     0x90 /* Extra ......... Eg, Friends or Helpers       */
   #define _CLERK     0xA0 /* Clerk ......... Eg, Occasioanal helper       */
   #define _ASSTSYSOP 0xB0 /* Assistant Sysop High access. Eg, Co-Sysops   */
   #define _SYSOP     0xD0 /* Sysop ......... HIGHEST ACCESS. #1 Sysop     */

   #define _HIDDEN    0xE0 /* Hidden ........ HIDES THINGS / NOT FOR USERS */



   struct _sys110
   {
      /*........ (mostly) Common System Data ..............................*/

      word version;           /* System Record version = 110 = v1.10       */
      word menu;              /* Alternate Menu file extension, 0=MNU      */
      word attrib;            /* Area attributes (see below)               */
      byte fillc1[10];        /* Reserved filler                           */
      byte barrpath[ 40 ];    /* Barricade File path.                      */
      byte fillc2[24];        /* Reserved filler                           */

      /*........ File System Information ..................................*/

      byte filtitle[ 50 ];    /* File Area Title                           */
      byte filepath[ 40 ];    /* Path to the file download directory       */
      byte uppath[   40 ];    /* Path to the file upload directory         */
      byte listpath[ 40 ];    /* Path to FILES.BBS equivalent              */
      byte fillf1[22];        /* Reserved filler                           */

      byte FilePriv;          /* Min priv for file area                    */
      byte DownPriv;          /* If not 0, min priv to download            */
      byte UpPriv;            /* If not 0, min priv to upload              */
      byte FileExtPriv;       /* If not 0, min priv to go Outside          */
      byte fillf2[12];        /* Reserved filler                           */

      long FileLock;          /* Locks for File Area                       */
      long DownLock;          /* If not 0, keys needed to download         */
      long UpLock;            /* If not 0, keys needed to upload           */
      long FileExtLock;       /* If not 0, keys needed to go Outside       */
      byte fillf3[32];        /* Reserved filler                           */

      /*........ Message System Information ...............................*/

      byte msgtitle[ 50 ];    /* Msg  Area Title                           */
      byte msgpath[  40 ];    /* Path to messages                          */
      byte fillm1[ 22 ];      /* Reserved filler                           */

      byte MsgPriv;           /* Min priv for msg area                     */
      byte EditPriv;          /* If not 0, min priv to Enter or Reply      */
      byte MsgExtPriv;        /* If not 0, min priv to go Outside          */
      byte fillm2[13];        /* Reserved filler                           */

      long MsgLock;           /* Locks for Msg Area                        */
      long EditLock;          /* If not 0, keys needed to Enter or Reply   */
      long MsgExtLock;        /* If not 0, keys needed to go Outside       */
      byte fillm3[4];         /* Reserved filler                           */
      byte EchoName[32];      /* Echo Area 'Tag' Name                      */

      /*=================================== Total Record Size   = 512 =====*/
   };


struct _ab
{
  char path[60];
  char tag[40];
};

#define NUM_ABBS 512

/* for qsort */
static int _stdc stcomp(const void *arg1, const void *arg2)
{
  static char a1[10],
              a2[10];

  if (strlen((char *)arg1) <= 1)
  {
    a1[0]='0';
    a1[1]=*(char *)arg1;
    a1[2]='\0';
  }
  else strcpy(a1,(char *)arg1);

  if (strlen((char *)arg2) <= 1)
  {
    a2[0]='0';
    a2[1]=*(char *)arg2;
    a2[2]='\0';
  }
  else strcpy(a2,(char *)arg2);

  return (strcmp(a1,a2));
}


int OpusToMaxPriv(byte opriv);


int _stdc main(int argc,char *argv[])
{
  FILE *areasfile,
       *dirbbs,
       *areasbbs;

  FFIND *ff,
        *ffo;

  struct _sys sys;
  struct _sys110 sys110;
  static struct _ab abbs[NUM_ABBS];

  char temp[80],
       *p;

  int x,
      y,
      ret,
      sysfile,
      maxsys,
      abbs_num,
      dat,
      sn;

  char sysname[255][3];

  NW(argc); NW(argv);


  abbs_num=0;

  Hello("PIPER", "Maximus Control File Converter", VERSION,
        "1990, " THIS_YEAR_LAST);

#warning hard-coded filenames inappropriate for unix

  if (fexist("AREAS.CTL"))
  {
    printf("`AREAS.CTL' already exists;  Overwrite [y,N]? ");
    fgets(temp,80,stdin);

    printf("\n");

    if (toupper(*temp) != 'Y')
    {
      printf("Aborted!\n");
      return -1;
    }

    if (fexist("AREAS.BAK"))
      unlink("AREAS.BAK");

    rename("AREAS.CTL","AREAS.BAK");
  }

#ifndef UNIX
  if ((areasbbs=fopen("AREAS.BBS","r")) != NULL ||
      (areasbbs=fopen("ECHO.CTL","r")) != NULL)
#else
  if ((areasbbs=fopen("etc/areas.bbs","r")) != NULL ||
      (areasbbs=fopen("etc/echo.ctl","r")) != NULL)
#endif
  {
    while (fgets(temp,80,areasbbs) != NULL)
    {
      if (abbs_num >= NUM_ABBS-1)
      {
        printf("Too many areas in echomail control file -- continuing...\n");
        break;
      }

      if (strchr(temp,';'))
        *strchr(temp,';')='\0';

      if (*temp)
      {
        p=temp;

        /* Passthrough areas... */
        while (*p=='#' || isdigit(*p) || isspace(*p))
          p++;

        getword(strupr(p),abbs[abbs_num].path," \t\n",1);

        if (abbs[abbs_num].path[strlen(abbs[abbs_num].path)-1] != PATH_DELIM)
          strcat(abbs[abbs_num].path, PATH_DELIMS);;

        getword(p,abbs[abbs_num++].tag," \t\n",2);
      }
    }

    fclose(areasbbs);
  }

  if (!fexist("SYSTEM*.BBS") && !fexist("SYSTEM*.DAT"))
  {
    printf("No SYSTEMxx.BBS/DAT files exist.  Continue anyway [y,N]? ");
    fgets(temp,80,stdin);

    printf("\n");

    if (toupper(*temp) != 'Y')
    {
      printf("Aborted!\n");
      return -1;
    }
  }

  if ((areasfile=fopen("AREAS.CTL","w"))==NULL)
  {
    printf("Fatal error opening `AREAS.CTL' for write!\n");
    exit(1);
  }

  printf("Scanning system files...\n");

  ff=FindOpen("SYSTEM*.BBS", 0);

  if (ff)
    dat=FALSE;
  else
  {
    ff=FindOpen("SYSTEM*.DAT",0);
    dat=TRUE;
  }

  maxsys=0;

  if (ff)
  {
    for (ret=0;ret==0;maxsys++)
    {
      if (ff->szName[6]=='.')   /* SYSTEM.BBS */
      {
        sysname[maxsys][0]='0';
        sysname[maxsys][1]='\0';
      }
      else
      {
        sysname[maxsys][0]=ff->szName[6];

        if (ff->szName[7]=='.')
          sysname[maxsys][1]='\0';
        else
        {
          sysname[maxsys][1]=ff->szName[7];
          sysname[maxsys][2]='\0';
        }
      }

      ret=FindNext(ff);
    }

    FindClose(ff);
  }

  qsort(sysname,maxsys,sizeof(sysname[0]),stcomp);

  for (sn=0;sn < maxsys;sn++)
  {
    printf("\rProcessing area %s...  ",sysname[sn]);

    if (dat)
    {
      sscanf(sysname[sn],"%x",&x);

      if (x < 100)
        fprintf(areasfile,"Area %d\n",x);
      else
      {
        y='A';

        while (x >= 100+26)
        {
          y++;
          x -= 26;
        }

        x -= 100;

        fprintf(areasfile,"Area %c%c\n",y,'A'+x);
      }

      sprintf(temp,"SYSTEM%s.DAT",sysname[sn]);

      if ((sysfile=open(temp,O_RDONLY | O_BINARY))==-1)
        printf("Can't open `%s' -- Continuing...\n",temp);
      else
      {
        read(sysfile,(char *)&sys110,sizeof(struct _sys110));
        close(sysfile);

        if (*sys110.msgpath)
          fprintf(areasfile,"        MsgAccess       %s\n",Priv_Level(OpusToMaxPriv(sys110.MsgPriv)));

        if (*sys110.filepath)
        {
          fprintf(areasfile,"        FileAccess      %s\n\n",Priv_Level(OpusToMaxPriv(sys110.FilePriv)));
          fprintf(areasfile,"        FileInfo        %s\n",sys110.filtitle);
          fprintf(areasfile,"        Download        %s\n",sys110.filepath);
        }

        if (*sys110.uppath)
          fprintf(areasfile,"        Upload          %s\n",sys110.uppath);

        if (*sys110.listpath)
          fprintf(areasfile,"        FileList        %s\n",sys110.listpath);

        if (*sys110.msgpath)
        {
          fprintf(areasfile,"\n        MsgInfo         %s\n",sys110.msgtitle);

          sprintf(temp,"%sORIGIN.*",sys110.msgpath);

          ffo=FindOpen(temp,0);

          if (ffo && (sys110.attrib & ECHO))
          {
            sprintf(temp,"%s%s", sys110.msgpath, ffo->szName);

            if ((dirbbs=fopen(temp,"r")) != NULL)
            {
              fgets(temp,80,dirbbs);
              fclose(dirbbs);

              if (temp[x=strlen(temp)-1]=='\n')
                temp[x]='\0';

              if (strlen(ffo->szName) <= 7)
                x=0;
              else if (sscanf(ffo->szName,"ORIGIN.%x",&x) != 1)
                x=0;

              fprintf(areasfile,"        Origin          %d %s\n",x,temp);
            }
          }

          FindClose(ffo);

          if ((sys110.attrib & ECHO) && *sys110.EchoName)
            fprintf(areasfile,"        MsgName         %s\n",sys110.EchoName);

          if (*sys110.barrpath)
            fprintf(areasfile,"        Barricade       %s\n",sys110.barrpath);

          if (sys110.attrib & ECHO)
            strcpy(temp,   "EchoMail");
          else if (sys110.attrib & SYSMAIL)
            strcpy(temp,   "Matrix  ");
          else strcpy(temp,"Local   ");

          fprintf(areasfile,  "        %s        %s\n\n",temp,sys110.msgpath);

          if ((sys110.attrib & NOPRIVATE) && (sys110.attrib & NOPUBLIC))
            fprintf(areasfile,"        Read-Only\n");
          else if (sys110.attrib & NOPRIVATE)
            fprintf(areasfile,"        Public Only\n");
          else if (sys110.attrib & NOPUBLIC)
            fprintf(areasfile,"        Private Only\n");
          else fprintf(areasfile,"        Public and Private\n");

          if (sys110.attrib & ANON_OK)
            fprintf(areasfile,"        Anonymous OK\n");
        }
      }
    }
    else
    {
      fprintf(areasfile,"Area %s\n",sysname[sn]);

      if (eqstri(sysname[sn],"0"))
        strcpy(temp,"SYSTEM.BBS");
      else sprintf(temp,"SYSTEM%s.BBS",sysname[sn]);

      if ((sysfile=open(temp,O_RDONLY | O_BINARY))==-1)
        printf("Can't open `%s' -- Continuing...\n",temp);
      else
      {
        read(sysfile,(char *)&sys,sizeof(struct _sys));
        close(sysfile);

        if (*sys.msgpath)
          fprintf(areasfile,"        MsgAccess       %s\n",Priv_Level(sys.priv));

        if (*sys.filepath)
          fprintf(areasfile,"        FileAccess      %s\n",Priv_Level(sys.priv));

        if (*sys.filepath)
        {
          fprintf(areasfile,"\n");

          sprintf(temp,"%sDIR.BBS",sys.filepath);

          if ((dirbbs=fopen(temp,"r")) != NULL)
          {
            fgets(temp,80,dirbbs);
            fclose(dirbbs);

            if (temp[x=strlen(temp)-1]=='\n')
              temp[x]='\0';

            fprintf(areasfile,"        FileInfo        %s\n",temp);
          }

          fprintf(areasfile,  "        Download        %s\n",sys.filepath);
        }

        if (*sys.uppath)
          fprintf(areasfile,  "        Upload          %s\n",sys.uppath);

        if (*sys.msgpath)
        {
          sprintf(temp,"%sDIR.BBS",sys.msgpath);

          fprintf(areasfile,"\n");

          if ((dirbbs=fopen(temp,"r")) != NULL)
          {
            fgets(temp,80,dirbbs);
            fclose(dirbbs);

            if (temp[x=strlen(temp)-1]=='\n')
              temp[x]='\0';

            fprintf(areasfile,"        MsgInfo         %s\n",temp);
          }

          sprintf(temp,"%sORIGIN.*",sys.msgpath);

          ffo=FindOpen(temp,0);

          if (ffo && (sys.attrib & ECHO))
          {
            sprintf(temp,"%s%s",sys.msgpath,ffo->szName);

            if ((dirbbs=fopen(temp,"r")) != NULL)
            {
              fgets(temp,80,dirbbs);
              fclose(dirbbs);

              if (temp[x=strlen(temp)-1]=='\n')
                temp[x]='\0';

              if (strlen(ffo->szName) <= 7)
                x=0;
              else if (sscanf(ffo->szName,"ORIGIN.%x",&x) != 1)
                x=0;

              fprintf(areasfile,"        Origin          %d %s\n",x,temp);
            }
          }

          FindClose(ffo);

          if (sys.attrib & ECHO)
          {
            for (x=0;x < abbs_num;x++)
              if (eqstri(sys.msgpath,abbs[x].path))
              {
                fprintf(areasfile,"        MsgName         %s\n",abbs[x].tag);
                break;
              }

            if (x==abbs_num)
              fprintf(areasfile,    "        MsgName         ?\n");
          }

          /* If the area is currently barricaded, or the BBSPATH points to   *
           * a file, instead of a path.                                      */

          if (fexist(sys.bbspath))
            fprintf(areasfile,"        Barricade       %s\n",sys.bbspath);

          if (sys.attrib & ECHO)
            strcpy(temp,   "EchoMail");
          else if (sys.attrib & SYSMAIL)
            strcpy(temp,   "Matrix  ");
          else strcpy(temp,"Local   ");

          fprintf(areasfile,  "        %s        %s\n\n",temp,sys.msgpath);

          if ((sys.attrib & NOPRIVATE) && (sys.attrib & NOPUBLIC))
            fprintf(areasfile,"        Read-Only\n");
          else if (sys.attrib & NOPRIVATE)
            fprintf(areasfile,"        Public Only\n");
          else if (sys.attrib & NOPUBLIC)
            fprintf(areasfile,"        Private Only\n");
          else fprintf(areasfile,"        Public and Private\n");

          if (sys.attrib & ANON_OK)
            fprintf(areasfile,"        Anonymous OK\n");
        }
      }
    }

    fprintf(areasfile,"End Area\n\n");
  }

  fclose(areasfile);

  printf("\r                     \rAll done!\n");

  return 0;
}

int OpusToMaxPriv(byte opriv)
{
  switch(opriv)
  {
    case _SYSOP:
      return SYSOP;

    case _ASSTSYSOP:
      return ASSTSYSOP;

    case _CLERK:
      return CLERK;

    case _EXTRA:
      return EXTRA;

    case _FAVORED:
      return FAVORED;

    case _PRIVEL:
      return PRIVIL;

    case _WORTHY:
      return WORTHY;

    case _NORMAL:
      return NORMAL;

    case _LIMITED:
      return LIMITED;

    case _DISGRACE:
      return DISGRACE;

    case _TWIT:
      return TWIT;

    default:
      return HIDDEN;
  }
}



