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
static char rcs_id[]="$Id: sqinfo.c,v 1.2 2003/06/05 03:13:40 wesgarland Exp $";
#pragma on(unreferenced)

#define NOVARS
#define NOVER
#define MSGAPI_HANDLERS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <fcntl.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "sqver.h"

static int quiet=0;
static int findbug=0;
static int batch=0;

void divider(void)
{
  if (!quiet)
    printf("\n=======================================\n");
}

char *frtype(int type)
{
  switch (type)
  {
    case FRAME_NORMAL:
      return "Stored msg";

    case FRAME_FREE:
      return "Free";

    default:
      return "\aUnknown!  (Should be `Stored msg' or `Free'!)";
  }
}

int debug_chain(char *name, FILE *sfd, FILE *ifd, struct _sqbase *sqbase,
                FOFS new_frame, FOFS last_frame, char *type,
                struct _sqbase *sqb, int num_msg, int fFreeFrame)
{
  SQIDX idx;
  SQHDR lframe;
  SQHDR frame;
  FOFS lframeofs;
  long filesize;
  int goterr=FALSE;
  int haderr=FALSE;
  int msgnum=0;
  word count;

  NW(ifd);


  if (new_frame==NULL_FRAME)
  {
    if (!quiet)
      printf("The %s list is empty.\n",type);
  }
  else
  {
    lframe.next_frame=new_frame;
    lframeofs=NULL_FRAME;

    fseek(sfd,0L,SEEK_END);
    filesize=ftell(sfd);

    count=0;

    for (;;)
    {
      if (!quiet)
        printf("\n");

      if (!findbug)
        printf("%s frame at %#010lx: (#%u)\n", type, new_frame, count+1);

      msgnum++;

      if (!quiet)
        printf("\n");

      if (new_frame==NULL_FRAME)
      {
        printf("\aerror! cannot read the NULL frame\n");
        goterr=TRUE;
      }

      if (new_frame > filesize)
      {
        printf("\aframe offset too large (file size is %#010lx)!\n",filesize);
        goterr=TRUE;
      }

      fseek(sfd,new_frame,SEEK_SET);

      if (ftell(sfd) != lframe.next_frame)
      {
        printf("\aerror: incorrect forward link for prior msg, #%d\n",count-1);
        goterr=TRUE;
      }

      if (fread((char *)&frame,1,sizeof(SQHDR),sfd) != sizeof(SQHDR))
      {
        printf("\aerror reading frame at %#010lx\n",new_frame);
        goterr=TRUE;
      }
      else
      {
        /************************************************************************/

        if (ifd)
        {
          fseek(ifd, count*(long)sizeof(SQIDX), SEEK_SET);

          if (ftell(ifd) != count*(long)sizeof(SQIDX))
          {
            printf("\aError seeking index file to right offset!\n");
            goterr=TRUE;
          }

          if (fread((char *)&idx, 1, sizeof(SQIDX), ifd) != sizeof(SQIDX))
          {
            printf("\aerror reading index entry at %#010lx\n",
                   count*(long)sizeof(SQIDX));

            goterr=TRUE;
          }

          if (!quiet)
            printf("idxofs=       %#010lx", idx.ofs);

          if (idx.ofs != new_frame)
          {
            printf("\a (should be %08lx!)\n", new_frame);
            goterr=TRUE;
          }
          else if (!quiet)
            printf(" (OK)\n");

          if (!quiet)
            printf("umsgid=       %#010lx\n",idx.umsgid);
        }

        /************************************************************************/

        if (!quiet)
          printf("id=           %#010lx",frame.id);

        if (frame.id==SQHDRID)
        {
          if (!quiet)
            printf(" (OK)\n");
        }
        else
        {
          printf("\a (Should be %#010lx!)\n",SQHDRID);
          goterr=TRUE;
        }




        if (!quiet)
          printf("prev_frame=   %#010lx",frame.prev_frame);

        if (frame.prev_frame==lframeofs)
        {
          if (!quiet)
            printf(" (OK)\n");
        }
        else
        {
          printf("\a (Should be %#010lx.)\n",lframeofs);
          goterr=TRUE;
        }




        if (!quiet)
          printf("next_frame=   %#010lx",frame.next_frame);

        if (new_frame >= sqbase->end_frame)
        {
          printf("\a (Should be less than end_frame [%#010lx]!)\n",
                 sqbase->end_frame);
          goterr=TRUE;
        }
        else if (new_frame==frame.next_frame)
        {
          printf("\a (Circular frame reference!)\n");
          goterr=TRUE;
        }
        else if (new_frame==last_frame && frame.next_frame != NULL_FRAME)
        {
          printf("\a Last should be NULL_FRAME (0x00000000).\n");
          goterr=TRUE;
        }
        else if (frame.next_frame==NULL_FRAME && new_frame != last_frame)
        {
          printf("\a Premature link end. Chain should end at %#010lx.\n",last_frame);
          goterr=TRUE;
        }
        else if (sqb && frame.next_frame==NULL_FRAME &&
                 (unsigned long)(count+1) != sqb->num_msg)
        {
          printf("\a\nErr!  Last msg is #%ld, but got chain end after %ld msgs!\n",sqb->num_msg,(long)(count+1));
          goterr=TRUE;
        }
        else if (!quiet)
          printf(" (OK)\n");






        if (!quiet)
          printf("frame_length= %ld",frame.frame_length);

        if (new_frame+sizeof(SQHDR)+frame.frame_length > sqbase->end_frame ||
            (frame.next_frame > new_frame &&
             new_frame+sizeof(SQHDR)+frame.frame_length > frame.next_frame))
        {
          printf("\a Too long!\n");
          goterr=TRUE;
        }
        else if (!quiet)
          printf(" (OK)\n");

        if (!quiet)
          printf("msg_length=   %ld",frame.msg_length);

        if ((long)frame.msg_length < (long)sizeof(XMSG) + (long)frame.clen &&
            !fFreeFrame)
        {
          printf("\a  (Control info is too long for frame!)\n");
          goterr=TRUE;
        }
        else if (frame.msg_length > frame.frame_length)
        {
          printf("\a  (Should be <= %ld!)\n", frame.frame_length);
          goterr=TRUE;
        }
        else if (!quiet)
          printf(" (OK)\n");


        if (!quiet)
        {
          printf("clen=         %ld\n",frame.clen);
          printf("type=         %s\n",frtype(frame.frame_type));
        }
      }

      if (goterr)
      {
        printf("Problem in %s.\n"
               "Press <esc> to abort, or any other key to continue: ",
               name);

        if (batch)
          exit(1);

        fflush(stdout);

        if (kgetch()=='\x1b')
          exit(1);

        printf("\n");
        goterr=FALSE;
        haderr=TRUE;
      }

      divider();

      if (new_frame==NULL_FRAME || frame.next_frame==NULL_FRAME)
        break;

      lframeofs=new_frame;
      new_frame=frame.next_frame;
      lframe=frame;
      count++;
    }
  }

  if (num_msg != -1 && msgnum != num_msg)
  {
    printf("\aError!  Expected %d msgs in chain, but only found %d\n",
           num_msg, msgnum);
    haderr=TRUE;
  }

  return (haderr ? -1 : 0);
}

int sqvalidate(char *name,FILE *sfd,FILE *ifd)
{
  struct _sqbase sqbase;
  int x,y;

  if (fread((char *)&sqbase,1,sizeof(struct _sqbase),sfd) !=
         sizeof(struct _sqbase))
  {
    printf("\asqvalidate: unable to read sqbase header\n");
    return -1;
  }
  
  divider();
  
  printf("\nGlobal data for %s:\n\n",name);

  if (!quiet)
  {
    printf("len=         %d\n",sqbase.len);
    printf("num_msg=     %ld\n",sqbase.num_msg);
    printf("high_msg=    %ld",sqbase.high_msg);
  }

  if (sqbase.high_msg != sqbase.num_msg)
    printf("\a (should be %ld!)",sqbase.num_msg);

  if (!quiet)
  {
    printf("\n");

    printf("uid=         %ld\n",sqbase.uid);
    printf("base=        %s\n",sqbase.base);
    printf("begin_frame= %#010lx\n",sqbase.begin_frame);
    printf("last_frame=  %#010lx\n",sqbase.last_frame);
    printf("last_free_fr=%#010lx\n",sqbase.last_free_frame);
    printf("free_frame=  %#010lx\n",sqbase.free_frame);
    printf("end_frame=   %#010lx\n",sqbase.end_frame);
    printf("sz_sqhdr=    %d\n",sqbase.sz_sqhdr);
/*  printf("sz_sqidx=    %d\n",sqbase.sz_sqidx);*/
    printf("max_msg=     %ld\n",sqbase.max_msg);
    printf("skip_msg=    %ld\n",sqbase.skip_msg);
    printf("keep_days=   %u\n", sqbase.keep_days);
    printf("high_water=  %ld\n\n",sqbase.high_water);
  }

  divider();
  
  x=debug_chain(name, sfd, ifd,  &sqbase, sqbase.begin_frame,
                sqbase.last_frame, "message", &sqbase, (int)sqbase.num_msg,
                FALSE);

  y=debug_chain(name, sfd, NULL, &sqbase, sqbase.free_frame,
                sqbase.last_free_frame, "free", NULL, -1,
                TRUE);

  return ((x || y) ? 1 : 0);
}



int squishtest(char *name)
{
  char temp[PATHLEN];

  FILE *sfd;
  FILE *ifd;
  char *p, *s;

  int x;
  
  p=strrchr(name, '.');
  s=strrstr(name, "/\\");

  if (p && (!s || p > s))
    *p='\0';

  strcpy(temp, name);
  strcat(temp, ".sqd");

  if ((sfd=shfopen(temp, "rb", O_RDONLY | O_BINARY))==NULL)
  {
    printf("error opening data file %s for read.\n",temp);
    exit(1);
  }
  
  strcpy(temp, name);
  strcat(temp,".sqi");

  if ((ifd=shfopen(temp, "rb", O_RDONLY | O_BINARY))==NULL)
  {
    printf("error opening index file %s for read.\n",temp);
    exit(1);
  }
  
  x=sqvalidate(name, sfd, ifd);
  
  fclose(ifd);
  fclose(sfd);
  
  return x;
}

int _stdc main(int argc,char *argv[])
{
  if (argc > 2)
  {
    if (eqstri(argv[2],"-q") || eqstri(argv[2], "/q"))
      quiet=1;
    else if (eqstri(argv[2],"-b") || eqstri(argv[2], "/b"))
      quiet=findbug=1;
    else if (eqstri(argv[2], "-e") || eqstri(argv[2], "/e"))
      quiet=findbug=batch=1;
  }

  if (!quiet)
  {
    printf("\nSQINFO  SquishMail Database Diagnostic Utility, Version " SQVERSION ".\n");
    printf("Copyright 1990, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");
  }

  if (argc < 2)
  {
    printf("Command-line format:\n\n");

    printf("%s <area name> [<options>...]\n\n", argv[0]);

    printf("SQINFO will display a diagnostic dump of the headers for the specified\n"
           "message area, and it will report any problems which are found.  SQINFO\n"
           "supports the following options:\n\n");

    printf("      -q   Quiet mode.  Only display one line per message.\n"
           "      -b   Bugfind mode.  Only print messages if an error occurs.\n"
           "      -e   Errorlevel mode.  Only print messages if an error occurs,\n"
           "           and SQINFO returns a non-zero errorlevel instead of pausing\n"
           "           when it discovers a problem.\n");

    exit(1);
  }

  return (squishtest(argv[1]));
}


