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

/* $Id: track.cc,v 1.2 2004/01/22 09:02:29 wmcbrine Exp $ */

//#define DEBUGTRACK

#include <stdio.h>
#include <string.h>
#include "dbase.h"
#include "track.h"

#define NUM_TRACK_FIELDS  (sizeof(afDbFields)/sizeof(*afDbFields))

// Compare two ASCIIZ strings

int asciizcomp(void *a1, void *a2)
{
  return stricmp((char *)a1, (char *)a2);
}

// Compare routine for two entries in the owner database

int owncomp(void *a1, void *a2)
{
  return (stricmp(((TRK_OWNER_NDX *)a1)->to,
		  ((TRK_OWNER_NDX *)a2)->to));
}

// Compare routine for two entries in the area database

int areacomp(void *a1, void *a2)
{
  return (stricmp(((TRK_AREA_NDX *)a1)->szArea,
                  ((TRK_AREA_NDX *)a2)->szArea));
}

#if 0

/* TRK_MSG_NDX: Comparison function for tracking IDs, including the record no */

int rec_id_comp(void *a1, void *a2)
{
  TRK_MSG_NDX *ptmn;
  int rc;

  if ((rc=stricmp((char *)a1, (char *)a2)) != 0)
    return rc;

  return (int)(*(NNUM *)((char *)a1 + sizeof ptmn->szTrackID) -
               *(NNUM *)((char *)a2 + sizeof ptmn->szTrackID));
}


/* TKR_MSG_NDX:  Comparison function for owners, including the record number */

int rec_own_comp(void *a1, void *a2)
{
  TRK_MSG_NDX *ptmn;
  int rc;

  if ((rc=stricmp((char *)a1, (char *)a2)) != 0)
    return rc;

  return (int)(*(NNUM *)((char *)a1 + sizeof ptmn->to) -
               *(NNUM *)((char *)a2 + sizeof ptmn->to));
}


/* TRK_MSG_NDX:  Comparison function for location (areas) including record no */

int rec_loc_comp(void *a1, void *a2)
{
  TRK_MSG_NDX *ptmn;
  int rc;

  if ((rc=stricmp((char *)a1, (char *)a2)) != 0)
    return rc;

  return (int)(*(NNUM *)((char *)a1 + sizeof ptmn->tl.szArea) -
               *(NNUM *)((char *)a2 + sizeof ptmn->tl.szArea));
}
#endif




// Field definitions for the TRK_MSG_NDX structure

static FIELD afDbFields[]=
{
  {"TrackID",       0,              MAX_TRACK_LEN, asciizcomp, asciizcomp},
  {"Owner",         MAX_TRACK_LEN,  sizeof(TRK_OWNER), asciizcomp, asciizcomp},
  {"Location",      MAX_TRACK_LEN+sizeof(TRK_OWNER), sizeof(TRK_LOCATION), asciizcomp, asciizcomp},
  {"Rest",          0, sizeof(TRK_MSG_NDX) - (MAX_TRACK_LEN+sizeof(TRK_OWNER)+sizeof(TRK_LOCATION)), 0}
};

// Constructor for the tracker database.  Just open the tracking
// data files and get ready to process transactions.

CPPEXPORT TRACKER::TRACKER()
{
  fOpen=FALSE;
}


// Destructor for the tracker database.  Close the tracking data files.

CPPEXPORT TRACKER::~TRACKER()
{
  if (fOpen)
    close();
}



// Open a tracking database

int CPPEXPORT TRACKER::open(char *szName, unsigned fNewFile)
{
  if (fOpen)
    return FALSE;

  char szFile[120];

  strcpy(szFile, szName);
  strcat(szFile, "msg");

  // Try to open database for tracking file

  if (dbMsg.open(szFile, afDbFields, NUM_TRACK_FIELDS, fNewFile) != 0)
    return FALSE;

  strcpy(szFile, szName);
  strcat(szFile, "own.i00");

  if (!btOwner.open(szFile, owncomp, owncomp, sizeof(TRK_OWNER_NDX),
                    fNewFile, 8))
  {
    dbMsg.close();
    return FALSE;
  }

  strcpy(szFile, szName);
  strcat(szFile, "area.i00");

  if (!btArea.open(szFile, areacomp, areacomp, sizeof(TRK_AREA_NDX),
                   fNewFile, 32))
  {
    btOwner.close();
    dbMsg.close();
    return FALSE;
  }

  fOpen=TRUE;

  return TRUE;
}


// Close the tracking database

int CPPEXPORT TRACKER::close()
{
  if (!fOpen)
    return FALSE;

  btArea.close();
  btOwner.close();
  dbMsg.close();
  fOpen=FALSE;

  return TRUE;
}


// Return the handles for the internal databases

BTREE * CPPEXPORT TRACKER::GetOwnerBtree(void)
{
  return fOpen ? &btOwner : 0;
}

BTREE * CPPEXPORT TRACKER::GetAreaBtree(void)
{
  return fOpen ? &btArea : 0;
}

DBASE * CPPEXPORT TRACKER::GetMsgDbase(void)
{
  return fOpen ? &dbMsg : 0;
}

// Find an area number for a given area name.  Adds records to the
// area database as necessary.

char * CPPEXPORT TRACKER::GetAreaOwner(char *szArea)
{
  TRK_AREA_NDX tan;
  TRK_AREA_NDX *ptan;

  if (!fOpen)
    return 0;

  strcpy(tan.szArea, szArea);

  if ((ptan=(TRK_AREA_NDX *)btArea.lookup(&tan, 0)) != 0)
  {
    #ifdef DEBUGTRACK
    printf("RetrieveArea \"%s\" with default owner \"%s\"\n",
           ptan->szArea, ptan->to);
    #endif

    return ptan->to;
  }

  // Try to find a default for all tracked areas

  strcpy(tan.szArea, "*");

  if ((ptan=(TRK_AREA_NDX *)btArea.lookup(&tan, 0)) != 0)
  {
    #ifdef DEBUGTRACK
    printf("RetrieveArea default owner - \"%s\"\n",
             ptan->to);
    #endif

    return ptan->to;
  }

  #ifdef DEBUGTRACK
  printf("RetrieveArea could not find owner of \"%s\"\n",
         szArea);
  #endif

  return 0;
}



// Find an owner number for a given owner name.  Adds records to the
// owner database as necessary.

char * TRACKER::GetOwnerName(TRK_OWNER to)
{
  TRK_OWNER_NDX ton;
  TRK_OWNER_NDX *pton;

  if (!fOpen)
    return 0;

  strcpy(ton.to, to);

  if ((pton=(TRK_OWNER_NDX *)btOwner.lookup(&ton, 0)) != 0)
  {
    #ifdef DEBUGTRACK
    printf("RetrieveOwner \"%s\" -> \"%s\"\n",
           pton->to, pton->szOwner);
    #endif

    return pton->szOwner;
  }

  #ifdef DEBUGTRACK
  printf("RetrieveOwner could not find record for \"%s\"\n", to);
  #endif

  return 0;
}



// Add a message to the tracking database

int CPPEXPORT TRACKER::AddMsg(TRK_MSG_NDX *ptmn)
{
  char *szOwner;

  if (!fOpen)
    return FALSE;

  #ifdef DEBUGTRACK
  printf("Inserting database record for message %s:%ld\n",
         ptmn->tl.szArea, ptmn->tl.uid);
  #endif

  // If a specific owner name was not specified, use the default owner
  // for this area.

  if (! *ptmn->to)
  {
    if ((szOwner=GetAreaOwner(ptmn->tl.szArea))==0)
      return FALSE;

    strcpy(ptmn->to, szOwner);
  }

  strupr(ptmn->to);

  // Insert this in the message tracking database

  if (!dbMsg.insert(ptmn))
  {
    #ifdef DEBUGTRACK
    printf("Note!  Record %s is already in database!\n", ptmn->szTrackID);
    #endif

    return FALSE;
  }

  return TRUE;
}


// Update an existing track message

int CPPEXPORT TRACKER::UpdateMsg(TRK_MSG_NDX *ptmnOld, TRK_MSG_NDX *ptmnNew)
{
  if (!fOpen)
    return FALSE;

#ifdef DEBUGTRACK
  printf("Updating database record for message %s:%ld\n",
         ptmnOld->tl.szArea, ptmnOld->tl.uid);
#endif

  strupr(ptmnNew->to);

  if (!dbMsg.update(ptmnOld, ptmnNew))
  {
    #ifdef DEBUGTRACK
    printf("Error!  Can't update record %s!\n", ptmnOld->szTrackID);
    #endif

    return FALSE;
  }

  return TRUE;
}



// Delete a message from the tracking database

int CPPEXPORT TRACKER::DeleteMsg(TRK_MSG_NDX *ptmn)
{
  void *pvFields[3];

  if (!fOpen)
    return FALSE;

  pvFields[0]=ptmn->szTrackID;
  pvFields[1]=ptmn->to;
  pvFields[2]=&ptmn->tl;

  if (!dbMsg.remove(pvFields))
  {
    #ifdef DEBUGTRACK
    printf("Error!  Can't remove message %s from db!\n",
           ptmn->szTrackID);
    #endif

    return FALSE;
  }

  return TRUE;
}


// Look up a message in the tracking database

int CPPEXPORT TRACKER::LookupMsg(char *szTrackID, char *szOwner, char *szLocation, PALIST *ppl, TRK_MSG_NDX *ptmn, unsigned uiIdx)
{
  void *apv[NUM_TRACK_FIELDS]={0};
  void **ppv;

  if (!fOpen)
    return FALSE;

  if (!szTrackID && !szOwner && !szLocation)
    ppv=0;
  else
  {
    apv[0]=szTrackID;
    apv[1]=szOwner;
    apv[2]=szLocation;
    ppv=apv;
  }

  // Now call the database lookup function

  return dbMsg.lookup(ppv, ppl, ptmn, uiIdx);
}


// Returns the priority of the message specified by ptio

char * CPPEXPORT TRACKER::GetPriority(TRK_MSG_NDX *ptmn)
{
  switch (ptmn->tp)
  {
    case TP_CRIT:     return "Critical";
    case TP_URGENT:   return "Urgent";
    case TP_NORMAL:   return "Normal";
    case TP_LOW:      return "Low";
    case TP_NOTIFY:
    default:          return "Notification";
  }
}


// Returns the status of the message specified by ptio

char * CPPEXPORT TRACKER::GetStatus(TRK_MSG_NDX *ptmn)
{
  switch (ptmn->ts)
  {
    case TS_NEW:      return "New";
    case TS_WORKING:  return "Working";
    case TS_CLOSED:   return "Closed";
    case TS_OPEN:
    default:          return "Open";
  }
}

// Set the link between owner name and owner ID

int CPPEXPORT TRACKER::SetOwner(TRK_OWNER to, char *szOwner)
{
  TRK_OWNER_NDX ton;

  if (!fOpen)
    return FALSE;

  strcpy(ton.to, to);
  strupr(ton.to);

  /* If there is no owner, simply delete the specified record */

  if (!szOwner)
    return btOwner.remove(&ton);

  strcpy(ton.szOwner, szOwner);

  if (!btOwner.insert(&ton, IF_REPLACE))
  {
    #ifdef DEBUGTRACK
    printf("Error!  Can't insert owner %s/%s in database!\n", to, szOwner);
    #endif

    return FALSE;
  }

  #ifdef DEBUGTRACK
  printf("Added owner %s/%s\n", to, szOwner);
  #endif

  return TRUE;
}

// Get the link between owner ID and name

int CPPEXPORT TRACKER::GetOwner(TRK_OWNER to, char *szOwner)
{
  TRK_OWNER_NDX ton;
  TRK_OWNER_NDX *pton;

  if (!fOpen)
    return FALSE;

  strcpy(ton.to, to);

  if ((pton=(TRK_OWNER_NDX *)btOwner.lookup(&ton, 0))==0)
  {
    #ifdef DEBUGTRACK
    printf("Error!  Can't find owner record %s in database!\n", to);
    #endif

    return FALSE;
  }

  strcpy(szOwner, pton->szOwner);
  return TRUE;
}


// Set the default owner for a message area

int CPPEXPORT TRACKER::SetDefaultOwner(char *szArea, TRK_OWNER to)
{
  TRK_AREA_NDX tan={0};

  if (!fOpen)
    return FALSE;

  strcpy(tan.szArea, szArea);

  /* If no owner specified, remove area link from database */

  if (!to)
    return btArea.remove(&tan);

  /* Otherwise, modify the existing owner */

  strcpy(tan.to, to);
  strupr(tan.to);

  if (!btArea.insert(&tan, IF_REPLACE))
  {
    #ifdef DEBUGTRACK
    printf("Error!  Can't insert area %s/%s in database!\n", szArea, to);
    #endif

    return FALSE;
  }

  #ifdef DEBUGTRACK
  printf("Added area record %s/%s\n", szArea, to);
  #endif

  return TRUE;
}


// Get the default owner record for a message area

int CPPEXPORT TRACKER::GetDefaultOwner(char *szArea, TRK_OWNER to)
{
  TRK_AREA_NDX tan;
  TRK_AREA_NDX *ptan;

  if (!fOpen)
    return FALSE;

  strcpy(tan.szArea, szArea);

  if ((ptan=(TRK_AREA_NDX *)btArea.lookup(&tan, 0))==0)
  {
    #ifdef DEBUGTRACK
    printf("Error!  Can't find %s!\n", szArea);
    #endif
    return FALSE;
  }

  strcpy(to, ptan->to);
  strupr(to);

  #ifdef DEBUGTRACK
  printf("Got area record %s/%s\n", szArea, to);
  #endif
  return TRUE;
}


#ifdef TEST

int main()
{
  TRACKER tk;

  TRK_MSG_NDX test1=
  {
    "930630120012.0000",
    "",                         // Default owner
    {"MUFFIN", 9999L},
    TS_NEW,
    TP_NORMAL
  };

  TRK_MSG_NDX test3=
  {
    "930101200010.0000",
    "",                         // Default owner
    {"BATH", 2L},
    TS_NEW,
    TP_NORMAL
  };

  TRK_MSG_NDX test2=
  {
    "930701190234.0000",
    "DD",                       // Specify explicit owner
    {"MUFFIN", 1235L},
    TS_NEW,
    TP_CRIT
  };


  tk.SetOwner("ADMN", "Paul Ashmore");
  tk.SetOwner("DD", "Tim Snape");
  tk.SetOwner("SJD", "Scott Dudley");

  tk.SetDefaultOwner("MUFFIN", "ADMN");
  tk.SetDefaultOwner("BATH", "SJD");

  tk.AddMsg(&test1);
  tk.AddMsg(&test2);
  tk.AddMsg(&test3);

  PALIST pl1, pl2, pl3, pl4;

  printf("\n*** Looking up second message\n\n");

  while (tk.LookupMsg(test2.szTrackID, 0, 0, &pl1, &test2))
    tk.PrintMsg(&test2);

  printf("\n*** Looking up first message\n\n");

  while (tk.LookupMsg(test1.szTrackID, 0, 0, &pl2, &test1))
    tk.PrintMsg(&test1);

  printf("\n*** Looking up DD messages\n\n");

  while (tk.LookupMsg(0, "DD", 0, &pl3, &test1))
    tk.PrintMsg(&test1);

  printf("\n*** Looking up all messages\n\n");

  while (tk.LookupMsg(0, 0, 0, &pl4, &test1))
    tk.PrintMsg(&test1);

  return 0;
}

#endif

