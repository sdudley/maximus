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
static char rcs_id[]="$Id: ansi2bbs.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=ANSI2BBS and ANSI2MEC, ANSI to .BBS/MEC conversion utilities
*/

#define MAX_INCL_VER

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max.h"

#define ANSIBUFLEN 80

void WriteColour(int colour,FILE *bbsfile);
char blink;

#ifdef ANSI2MEC
    #define A_DESC "An ANSI to .MEC file conversion program"
#ifndef UNIX
    #define A_NAME "ANSI2MEC"
    #define EXT    ".MEC"
#else
    #define A_NAME "ansi2mec"
    #define EXT    ".mec"
#endif

    #define PutRepChr(ch,count) for (x=0;x < count+1;x++) \
                        fprintf(bbsfile,"%c",ch);

#else
    #define A_DESC "An ANSI to .BBS file conversion program"

#ifndef UNIX
    #define A_NAME "ANSI2BBS"
    #define EXT    ".BBS"
#else
    #define A_NAME "ansi2bbs"
    #define EXT    ".bbs"
#endif

    #define PutRepChr(ch,count) \
              ((ch==25 || ch==13 || ch==10) ? \
              fprintf(bbsfile,"%c\x19%c%c",ch,ch,count) : \
              fprintf(bbsfile,"\x19%c%c",ch,count+1));
#endif

#define Compress_Sequence()                                       \
      {                                                           \
        if (lastch != -1)                                         \
        {                                                         \
          if (lastcount <= 2)                                     \
          {                                                       \
            for (x=0;x <= lastcount;x++)                          \
              putc(lastch,bbsfile);                               \
          }                                                       \
          else PutRepChr(lastch,lastcount);                       \
                                                                  \
          linelen += lastcount+1;                                 \
                                                                  \
          lastcount=0;                                            \
          lastch=-1;                                              \
        }                                                         \
                                                                  \
        if (lastch != -1)                                         \
        {                                                         \
          linelen++;                                              \
          putc(lastch,bbsfile);                                   \
        }                                                         \
                                                                  \
        lastch=ch;                                                \
      }

int linelen;

#define MAX_LLEN  100

int _stdc main(int argc,char *argv[])
{
  FILE *ansifile,
       *bbsfile;

  char ansibuf[ANSIBUFLEN],
       temp[ANSIBUFLEN],
       inname[PATHLEN],
       outname[PATHLEN],
       high,
       lasthigh,
       *p;

  unsigned char colour;

  int ch,
      lastch,
      lastcount,
      last_x,
      x,y;

  long offset;

  lastch=-1;
  lastcount=0;
  last_x=1;

  colour=7;   /* White-on-black, starting colour */
  high=blink=lasthigh=FALSE; /* NOT high-intensity */

  Hello(A_NAME, A_DESC, VERSION, "1990, " THIS_YEAR);

  if (argc < 2)
  {
    printf("Usage:\n\n");

    printf("  %s <infile> <outfile>\n\n",A_NAME);

    printf("If no extension is specified for <infile>, then %s will use .ANS by\n",A_NAME);
    printf("default.  If no <outfile> is specified, then <infile> will be used, but with\n");
    printf("an extension of %s.\n",EXT);

    exit(1);
  }

  strcpy(inname,argv[1]);

  if ((p=strchr(inname,'.'))==NULL)
#ifndef UNIX
    strcat(inname,".ANS");
#else
    strcat(inname,".ans");
#endif

  if (argc < 3)
  {
    strcpy(outname,inname);

    if ((p=strrchr(outname,'.'))==NULL)
      strcat(outname,EXT);
    else strcpy(p,EXT);
  }
  else strcpy(outname,argv[2]);

  if ((p=strrchr(outname,'.'))==NULL)
    strcat(outname,EXT);


  /* printf("Compiling %s to %s...\n",strupr(inname),strupr(outname)); */
  printf("Compiling %s to %s...\n", fancy_fn(inname), fancy_fn(outname)); 


  if ((ansifile=fopen(inname,"rb"))==NULL)
  {
    printf("Error opening `%s' for read!\n",inname);
    exit(1);
  }

  if ((bbsfile=fopen(outname,"wb"))==NULL)
  {
    printf("Error opening `%s' for write!\n",outname);
    exit(1);
  }

  linelen=0;

  for (;;)
  {
#ifdef ANSI2MEC
    if (linelen > MAX_LLEN)
    {
      fprintf(bbsfile,"[\r\n]");
      linelen=0;
    }
#endif

    if ((ch=getc(ansifile))==EOF)
      break;

#ifdef ANSI2MEC
    if (ch=='[') /* Double up the `['s */
    {
      if (lastch=='[')
        lastcount++;
      else Compress_Sequence();
    }
#endif

    if (ch=='\n')
    {
      lastch=-1;
      lastcount=0;
      putc('\r',bbsfile);
      putc('\n',bbsfile);
      linelen=0;
    }
    else if (ch != '\x1b')
    {
      if (ch==lastch)
        lastcount++;
      else Compress_Sequence();
    }
    else
    {
      if (lastch != -1)
      {
#ifndef ANSI2MEC
        if (lastcount <= 2)
        {
#endif
          for (x=0;x <= lastcount;x++)
            putc(lastch,bbsfile);
#ifndef ANSI2MEC
        }
        else if (lastcount==25)
          fprintf(bbsfile,"\x19%c\x19%c",lastch,lastch);
        else fprintf(bbsfile,"\x19%c%c",lastch,lastcount+1);
#endif

        linelen += lastcount;
        lastcount=0;
        lastch=-1;
      }

      for (x=0;x < ANSIBUFLEN-1;x++)
      {
        ansibuf[x]=(char)(ch=getc(ansifile));

        if (isalpha(ch))
        {
          ansibuf[x+1]='\0';
          break;
        }
      }

      /* In case ANSI string was too long */
      ansibuf[ANSIBUFLEN-1]='\0';

      /* Delete the beginning '[' */
      memmove(ansibuf,ansibuf+1,strlen(ansibuf)+1);

      switch (ch)
      {
        /* We can't really handle the save/restore cursor commands, so     *
         * just strip everything between here and the restore command...   *
         * This will disrupt SOME screens, but it will make converting     *
         * TheDraw files possible...                                       */

        case 'u':
          break;

        case 's':
          offset=ftell(ansifile);

          for (;;)
          {
            while ((ch=getc(ansifile)) != '\x1b' && ch != EOF)
              ;

            /* Got to EOF before save_restore, so continue! */

            if (ch==EOF)
            {
              fseek(ansifile,offset,SEEK_SET);
              break;
            }

            if ((ch=getc(ansifile)) != '[')
            {
              ungetc(ch,ansifile);
              continue;
            }

            if ((ch=getc(ansifile))=='u')
              break;
            else        /* back-track */
            {
              fseek(ansifile,offset,SEEK_SET);
              break;
/*
              ungetc(ch,ansifile);
              continue;
*/
            }
          }
          break;

        case 'm':       /* Attribute! */
          if (lasthigh)
            colour += 8;

          high=lasthigh;

          for (x=1,*temp='\x01';*temp;x++)
          {
            getword(ansibuf,temp,";",x);

            switch (*temp)
            {
              case '\0':
                break;

              case '0':
                colour=7;
                high=lasthigh=blink=FALSE;
                break;

              case '1':
                if (! high)
                {
                  colour += 8;
                  high=TRUE;
                }
                break;

              case '5':             /* Blink */
                if (!blink)
                {
                  colour += 128;
                  blink=TRUE;
                }
                break;

              case '7':
                high=blink=FALSE;
                colour=112;
                break;

              case '3':
                colour=(char)((colour/16)*16);

                if (high)
                  colour += 8;

                switch(temp[1])
                {
                  case '1':
                    colour += 4;
                    break;

                  case '2':
                    colour += 2;
                    break;

                  case '3':
                    colour += 6;
                    break;

                  case '4':
                    colour++;
                    break;

                  case '5':
                    colour += 5;
                    break;

                  case '6':
                    colour += 3;
                    break;

                  case '7':
                    colour += 7;
                    break;
                }
                break;

              case '4':
                while (colour > 15) /* While we still have a background... */
                  colour -= 16;

                switch(temp[1])
                {
                  case '\0':  /* Just a "4m", means we go to underline */
                    while (colour > 15) /* While we still have a background... */
                      colour -= 16;

                    colour += 16;       /* Blue bkgr for underline */
                    break;

                  case '1':
                    colour += 4*16;
                    break;

                  case '2':
                    colour += 2*16;
                    break;

                  case '3':
                    colour += 6*16;
                    break;

                  case '4':
                    colour += 16;
                    break;

                  case '5':
                    colour += 5*16;
                    break;

                  case '6':
                    colour += 3*16;
                    break;

                  case '7':
                    colour += 7*16;
                    break;
                }
                break;
            }
          }

          lasthigh=high;

          WriteColour(colour,bbsfile);

          if (high)
            colour -= 8;
          break;

        case 'J':       /* CLS */
#ifdef ANSI2MEC
          linelen += 5;
          fprintf(bbsfile,"[cls]");
#else
          fprintf(bbsfile,"\x0c");
#endif
          break;

        case 'K':       /* CLEOL */
#ifdef ANSI2MEC
          linelen += 7;
          fprintf(bbsfile,"[cleol]");
#else
          fprintf(bbsfile,"\x16\x07");
#endif
          break;

        case 'A':       /* UP */
          if (*ansibuf != 'A')
            y=atoi(ansibuf);
          else y=1;

#ifdef ANSI2MEC
          if (y > 3)
          {
            linelen += 19;
            fprintf(bbsfile,"[repeatseq 2 up %d]",y);
          }
          else
          {
            linelen += 2+(3*y);

            fprintf(bbsfile,"[");

            for (x=0;x < y;x++)
              fprintf(bbsfile,"%sup",x ? " " : "");

            fprintf(bbsfile,"]");
          }
#else
          if (y > 3)
            fprintf(bbsfile,"\x16\x19\x02\x16\x03%c",(char)y);
          else for (x=0;x < y;x++)
            fprintf(bbsfile,"\x16\x03");
#endif
          break;

        case 'B':       /* DOWN */
          if (*ansibuf != 'B')
            y=atoi(ansibuf);
          else y=1;

#ifdef ANSI2MEC
          if (y > 3)
          {
            linelen += 21;
            fprintf(bbsfile,"[repeatseq 2 down %d]",y);
          }
          else
          {
            linelen += 2+(5*y);

            fprintf(bbsfile,"[");

            for (x=0;x < y;x++)
              fprintf(bbsfile,"%sdown",x ? " " : "");

            fprintf(bbsfile,"]");
          }
#else
          if (y > 3)
            fprintf(bbsfile,"\x16\x19\x02\x16\x04%c",(char)y);
         else for (x=0;x < y;x++)
            fprintf(bbsfile,"\x16\x04");
#endif
          break;

        case 'C':       /* RIGHT */
          if (*ansibuf != 'C')
            y=atoi(ansibuf);
          else y=1;

#ifdef ANSI2MEC
          if (y > 3)
          {
            linelen += 22;
            fprintf(bbsfile,"[repeatseq 2 right %d]",y);
          }
          else
          {
            linelen += 2+(6*y);

            fprintf(bbsfile,"[");

            for (x=0;x < y;x++)
              fprintf(bbsfile,"%sright",x ? " " : "");

            fprintf(bbsfile,"]");
          }
#else
          if (y > 3)
            fprintf(bbsfile,"\x16\x19\x02\x16\x06%c",(char)y);
          else for (x=0;x < y;x++)
              fprintf(bbsfile,"\x16\x06");
#endif
          break;

        case 'D':       /* LEFT */
          if (*ansibuf != 'C')
            y=atoi(ansibuf);
          else y=1;

#ifdef ANSI2MEC
          if (y > 3)
          {
            linelen += 21;
            fprintf(bbsfile,"[repeatseq 2 left %d]",y);
          }
          else
          {
            linelen += 2+(5*y);

            fprintf(bbsfile,"[");

            for (x=0;x < y;x++)
              fprintf(bbsfile,"%sleft",x ? " " : "");

            fprintf(bbsfile,"]");
          }
#else
          if (y > 3)
            fprintf(bbsfile,"\x16\x19\x02\x16\x05%c",(char)y);
          else for (x=0;x < y;x++)
            fprintf(bbsfile,"\x16\x05");
#endif
          break;

        case 'H':       /* Set position */
        case 'f':
          if (*ansibuf==';')
          {
            getword(ansibuf,temp,";",1);
            y=atoi(temp);
            x=--last_x;
          }
          else
          {
            getword(ansibuf,temp,";",1);
            x=last_x=atoi(temp);

            getword(ansibuf,temp,";",2);
            y=atoi(temp);
          }

#ifdef ANSI2MEC
          linelen += 14;

          fprintf(bbsfile,"[locate %d %d]",x ? x : 1,y ? y : 1);
#else
          {
            int x_26=(x==26);
            int y_26=(y==26);

            if (x_26)
              x--;

            if (y_26)
              y--;

            fprintf(bbsfile,"\x16\x08%c%c",(char)(x ? x : 1),(char)(y ? y : 1));

            if (x_26)
              fprintf(bbsfile, "\x16\x06");

            if (y_26)
              fprintf(bbsfile, "\x16\x04");
          }
#endif
          break;

        default:
          printf("\aWarning!  Unknown ANSI command: `[%s'\n",ansibuf);
          break;
      }
    }
  }

  Compress_Sequence();

  fclose(bbsfile);
  fclose(ansifile);

  printf("Done!\n");
  return 0;
}



void WriteColour(int colour,FILE *bbsfile)
{
#ifdef ANSI2MEC
  char *colours[]={"black",
                   "blue",
                   "green",
                   "cyan",
                   "red",
                   "magenta",
                   "brown",
                   "gray",
                   "darkgray",
                   "lightblue",
                   "lightgreen",
                   "lightcyan",
                   "lightred",
                   "lightmagenta",
                   "yellow",
                   "white"};

  char string[120];

  if ((colour & 0x7f) < 16)
    sprintf(string,"[%s]",colours[colour & 0x7f]);
  else sprintf(string,"[%s on %s]",colours[(colour & 0x7f) % 16],
               colours[(colour & 0x7f)/16]);

  linelen += strlen(string);
  fprintf(bbsfile,"%s",string);

#else
  if (colour < 128)
    fprintf(bbsfile,"\x16\x01\x10%c",colour+128);
  else if (colour > 160)
    fprintf(bbsfile,"\x16\x01\x10%c",colour-128);
  else fprintf(bbsfile,"\x16\x01%c",colour-128);
#endif

  if (blink)
#ifdef ANSI2MEC
  {
    fprintf(bbsfile,"[blink]");
    linelen += 7;
  }
#else
    fprintf(bbsfile,"\x16\x02");
#endif
}

