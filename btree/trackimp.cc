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
static char rcs_id[]="$Id: trackimp.cc,v 1.1 2002/10/01 17:49:32 sdudley Exp $";
#pragma on(unreferenced)

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "dbase.h"
#include "track.h"

#define MAX_DBF_LINE  255


static char comma[]=",\n";


int getword(char *strng,char *dest,char *delim,int findword)
{
  int x,
      isw,
      sl_d,
      sl_s,
      wordno=0;

  char *string,
       *oldstring,
       *firstchar;

  if (findword==0)
  {
    *dest='\0';
    return 0;
  }

  string=oldstring=strng;

  sl_d=strlen(delim);

  for (string=strng;*string;string++)
  {
    for (x=0,isw=0;x <= sl_d;x++) if (*string==delim[x]) isw=1;

    if (isw==0)
    {
      oldstring=string;
      break;
    }
  }

  sl_s=strlen(string);

  for (firstchar=0,wordno=0;(string-oldstring) <= sl_s;string++)
  {
    for (x=0,isw=0;x <= sl_d;x++)
      if (*string==delim[x])
      {
        isw=1;
        break;
      }

    if ((! isw) && (string==oldstring))
      wordno++;

    if (wordno != findword)
    {
      if (isw && (string != oldstring))
      {
        for (x=0,isw=0;x <= sl_d;x++) if (*(string+1)==delim[x])
        {
          isw=1;
          break;
        }

        if (isw==0)
          wordno++;
      }
    }
    else
    {
      if (isw && (string != oldstring))
      {
        for (x=0,isw=0;x <= sl_d;x++)
          if (*(string-1)==delim[x])
          {
            isw=1;
            break;
          }

        if (isw==0)
          wordno++;
      }
    }

    if ((wordno==findword) && (firstchar==0)) firstchar=string;

    if (wordno==(findword+1))
    {
      strncpy(dest,(firstchar==oldstring ? firstchar : ++firstchar),(unsigned int)(string-firstchar));
      dest[(unsigned int)(string-firstchar)]='\0';
      break;
    }
  }

  if (firstchar==0 || (firstchar && *firstchar=='\0'))
    *dest='\0';

  return ((int)(firstchar-strng));
}


/* Get a quote-delimited field from the input line */

static void near get_delim(char *line, int wordno, char *output, int max_output, char *delim)
{
  char word[MAX_DBF_LINE];

  getword(line, word, delim, wordno);

  if (*word=='"')
    memmove(word, word+1, strlen(word+1)+1);

  int i;

  if (word[i=strlen(word)-1]=='"')
    word[i]=0;

  strncpy(output, word, max_output);
  output[max_output]=0;
}


/* Export the owner database from TRKOWN.DBF */

static void near ImportOwners(BTREE *pbtOwner)
{
  char line[MAX_DBF_LINE];
  TRK_OWNER_NDX ton;
  FILE *fp;

  printf("Importing owners from TRKOWN.DBF...\n");

  if ((fp=fopen("trkown.dbf", "r"))==0)
  {
    printf("Error opening trkown.dbf for read!\n");
    exit(1);
  }

  while (fgets(line, MAX_DBF_LINE, fp))
  {
    if (*line != '"')
      continue;

    get_delim(line, 1, ton.to, sizeof(ton.to)-1, comma);
    get_delim(line, 2, ton.szOwner, sizeof(ton.szOwner)-1, comma);

    if (!pbtOwner->insert((void *)&ton, 0))
      printf("Couldn't insert owner \"%s\"!\n", ton.szOwner);
  }

  fclose(fp);
}


/* Import all of the area/owner records from TRKAREA.DBF */

static void near ImportAreas(BTREE *pbtArea)
{
  char line[MAX_DBF_LINE];
  TRK_AREA_NDX tan;
  FILE *fp;

  printf("Importing areas from TRKAREA.DBF...\n");

  if ((fp=fopen("trkarea.dbf", "r"))==0)
  {
    printf("Error opening trkarea.dbf for read!\n");
    exit(1);
  }

  while (fgets(line, MAX_DBF_LINE, fp))
  {
    if (*line != '"')
      continue;

    get_delim(line, 1, tan.szArea, sizeof(tan.szArea)-1, comma);
    get_delim(line, 2, tan.to, sizeof(tan.to)-1, comma);

    if (!pbtArea->insert((void *)&tan, 0))
      printf("Couldn't insert area \"%s\"!\n", tan.szArea);
  }

  fclose(fp);
}


/* Import all of the messages from TRKMSG.DBF */

static void near ImportMsgs(DBASE *pdbMsg)
{
  char line[MAX_DBF_LINE];
  TRK_MSG_NDX tmn;
  FILE *fp;

  printf("Importing messages from TRKMSG.DBF...\n");

  if ((fp=fopen("trkmsg.dbf", "r"))==0)
  {
    printf("Error opening trkmsg.dbf for read!\n");
    exit(1);
  }

  while (fgets(line, MAX_DBF_LINE, fp))
  {
    char buf[MAX_DBF_LINE];

    if (*line != '"')
      continue;

    get_delim(line, 1, tmn.szTrackID, sizeof(tmn.szTrackID)-1, comma);
    get_delim(line, 2, tmn.to, sizeof(tmn.to)-1, comma);
    get_delim(line, 3, tmn.tl.szArea, sizeof(tmn.tl.szArea)-1, comma);
    get_delim(line, 4, buf, sizeof(buf)-1, comma);
    tmn.tl.uid=atol(buf);
    get_delim(line, 5, buf, sizeof(buf)-1, comma);
    tmn.ts=(TRK_STATUS)atoi(buf);
    get_delim(line, 6, buf, sizeof(buf)-1, comma);
    tmn.tp=(TRK_PRIORITY)atoi(buf);
    get_delim(line, 7, buf, sizeof(buf)-1, comma);

    tmn.scDateWritten.msg_st.date.mo = (short)atoi(buf);
    tmn.scDateWritten.msg_st.date.da = (short)atoi(buf+3);
    tmn.scDateWritten.msg_st.date.yr = (short)(atoi(buf+6)-1980);

    get_delim(line, 8, buf, sizeof(buf)-1, comma);

    tmn.scDateWritten.msg_st.time.hh = (short)atoi(buf);
    tmn.scDateWritten.msg_st.time.mm = (short)atoi(buf+3);
    tmn.scDateWritten.msg_st.time.ss = (short)atoi(buf+6);

    if (!pdbMsg->insert((void *)&tmn))
      printf("Couldn't insert message \"%s\"!\n", tmn.szTrackID);

      if (!pdbMsg->get_btrees()[2].validate())
      {
      printf("Error in validate after inserting %s!\n", tmn.szTrackID);
      getch();
      break;
      }
  }

  fclose(fp);
}

int main(void)
{
  TRACKER t;
  BTREE *pbtOwner;
  BTREE *pbtArea;
  DBASE *pdbMsg;

  printf("\nTRKIMP  Tracking Database Importer, Version 3.0\n"
         "Copyright 1995 by Lanius Corporation.  All rights reserved.\n\n");

  if (!t.open("trk", TRUE))
  {
    printf("Can't open tracking database!\n");
    exit(1);
  }

  /* Get the physical handles behind the database */

  pbtOwner=t.GetOwnerBtree();
  pbtArea=t.GetAreaBtree();
  pdbMsg=t.GetMsgDbase();

  ImportOwners(pbtOwner);
  ImportAreas(pbtArea);
  ImportMsgs(pdbMsg);

  t.close();
  printf("Done!\n");
  return 0;
}

