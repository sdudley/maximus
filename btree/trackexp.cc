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
static char rcs_id[]="$Id: trackexp.cc,v 1.1.1.1 2002/10/01 17:49:31 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include "dbase.h"
#include "track.h"

/* Export the owner database to TRKOWN.DBF */

static void near ExportOwners(BTREE *pbtOwner)
{
  TRK_OWNER_NDX *pton;
  FILE *fp;
  PALIST pal;

  printf("Exporting owners to TRKOWN.DBF...\n");

  if ((fp=fopen("trkown.dbf", "w"))==0)
  {
    printf("Error opening trkown.dbf for write!\n");
    exit(1);
  }

  while ((pton=(TRK_OWNER_NDX *)pbtOwner->lookup(0, &pal)) != 0)
    fprintf(fp, "\"%s\",\"%s\"\n", pton->to, pton->szOwner);

  fclose(fp);
}


/* Export all of the area/owner records to TRKAREA.DBF */

static void near ExportAreas(BTREE *pbtArea)
{
  TRK_AREA_NDX *ptan;
  FILE *fp;
  PALIST pal;

  printf("Exporting areas to TRKAREA.DBF...\n");

  if ((fp=fopen("trkarea.dbf", "w"))==0)
  {
    printf("Error opening trkarea.dbf for write!\n");
    exit(1);
  }

  while ((ptan=(TRK_AREA_NDX *)pbtArea->lookup(0, &pal)) != 0)
    fprintf(fp, "\"%s\",\"%s\"\n", ptan->szArea, ptan->to);

  fclose(fp);
}


/* Export all of the messages to TRKMSG.DBF */

static void near ExportMsgs(DBASE *pdbMsg)
{
  void *ppvFields[3]={0, 0, 0};
  TRK_MSG_NDX tmn;
  FILE *fp;
  PALIST pal;

  printf("Exporting messages to TRKMSG.DBF...\n");

  if ((fp=fopen("trkmsg.dbf", "w"))==0)
  {
    printf("Error opening TRKMSG.DBF for write!\n");
    exit(1);
  }

  while (pdbMsg->lookup(ppvFields, &pal, (void *)&tmn, 2))
  {
/*
    printf("\"%s\",\"%s\",\"%s\",\"%lu\",\"%u\",\"%u\",\"%02u/%02u/%04u\","
            "\"%02u:%02u:%02u\"\n",
            tmn.szTrackID,
            tmn.to,
            tmn.tl.szArea,
            tmn.tl.uid,
            (int)tmn.ts,
            (int)tmn.tp,
            tmn.scDateWritten.msg_st.date.mo,
            tmn.scDateWritten.msg_st.date.da,
            tmn.scDateWritten.msg_st.date.yr + 1980,
            tmn.scDateWritten.msg_st.time.hh,
            tmn.scDateWritten.msg_st.time.mm,
            tmn.scDateWritten.msg_st.time.ss);
*/
    fprintf(fp, "\"%s\",\"%s\",\"%s\",\"%lu\",\"%u\",\"%u\",\"%02u/%02u/%04u\","
            "\"%02u:%02u:%02u\"\n",
            tmn.szTrackID,
            tmn.to,
            tmn.tl.szArea,
            tmn.tl.uid,
            (int)tmn.ts,
            (int)tmn.tp,
            tmn.scDateWritten.msg_st.date.mo,
            tmn.scDateWritten.msg_st.date.da,
            tmn.scDateWritten.msg_st.date.yr + 1980,
            tmn.scDateWritten.msg_st.time.hh,
            tmn.scDateWritten.msg_st.time.mm,
            tmn.scDateWritten.msg_st.time.ss);
  }

  fclose(fp);
}


#if 0
void ExportStruct(BTREE *, char *) { }
#else
void ExportStruct(BTREE *pbt, char *name)
{
  FILE *fp;

  printf("Exporting structure to %s...\n", name);

  if ((fp=fopen(name, "w"))==0)
  {
    printf("Error opening '%s' for write!\n");
    exit(1);
  }

  pbt->print(fp);

  fclose(fp);
}
#endif



int main(int argc)
{
  TRACKER t;
  BTREE *pbtOwner;
  BTREE *pbtArea;
  DBASE *pdbMsg;

  printf("\nTRKEXP  Tracking Database Exporter, Version 3.0\n"
         "Copyright 1995 by Lanius Corporation.  All rights reserved.\n\n");

  if (!t.open("trk", FALSE))
  {
    printf("Can't open tracking database!\n");
    exit(1);
  }

  /* Get the physical handles behind the database */

  pbtOwner=t.GetOwnerBtree();
  pbtArea=t.GetAreaBtree();
  pdbMsg=t.GetMsgDbase();

/*  printf("validating tree...\n");
  pdbMsg->pbIndex[2].validate();*/

  ExportOwners(pbtOwner);
  ExportAreas(pbtArea);
  ExportMsgs(pdbMsg);

  BTREE *pbtDbase=pdbMsg->get_btrees();

  if (argc >= 2)
  {
    ExportStruct(pbtOwner, "trkown.bts");
    ExportStruct(pbtArea, "trkarea.bts");

    ExportStruct(pbtDbase, "trkmid.bts");
    ExportStruct(pbtDbase+1, "trkmown.bts");
    ExportStruct(pbtDbase+2, "trkmloc.bts");

    printf("Validating owners:\n");
    pbtOwner->validate();

    printf("Validating areas:\n");
    pbtArea->validate();

    printf("Validating message index IDs\n");
    pbtDbase[0].validate();

    printf("Validating message index owners\n");
    pbtDbase[1].validate();

    printf("Validating message index location\n");
    pbtDbase[2].validate();
  }

  t.close();

  printf("Done!\n");
  return 0;
}


