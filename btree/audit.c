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

/* $Id: audit.c,v 1.2 2004/01/22 08:04:24 wmcbrine Exp $ */

#include <string.h>
#include <stdio.h>
#include "trackc.h"

int c_main(void)
{
  TRK_MSG_NDX test1=
  {
    "9306301200120000",
    "ADMN",
    {"MUFFIN", 9999L},
    TS_NEW,
    TP_NORMAL
  };

  TRK_MSG_NDX test3=
  {
    "9301012000100000",
    "",                         // Default owner
    {"BATH", 2L},
    TS_NEW,
    TP_NORMAL
  };

  TRK_MSG_NDX test2=
  {
    "9307011902340000",
    "DD",                       // Specify explicit owner
    {"MUFFIN", 1235L},
    TS_NEW,
    TP_CRIT
  };

  TRK_MSG_NDX new;

  PALIST *ppl;
  TRK t=TrkOpen();

  new=test1;

  TrkSetOwner(t, "ADMN", "Paul Ashmore");
  TrkSetOwner(t, "DD", "Tim Snape");
  TrkSetOwner(t, "SJD", "Scott Dudley");

  TrkSetDefaultOwner(t, "MUFFIN", "ADMN");
  TrkSetDefaultOwner(t, "BATH", "SJD");
  TrkSetDefaultOwner(t, "*", "SJD");

  TrkAddMsg(t, &test1);
  TrkAddMsg(t, &test2);
  TrkAddMsg(t, &test3);

  printf("\n*** Looking up second message\n\n");

  ppl=PalistNew();

  while (TrkLookupMsg(t, test2.szTrackID, NULL, NULL, ppl, &test2))
    TrkPrintMsg(t, &test2);

  printf("\n*** Looking up first message\n\n");

  PalistDestroy(ppl);
  ppl=PalistNew();

  while (TrkLookupMsg(t, test1.szTrackID, NULL, NULL, ppl, &test1))
    TrkPrintMsg(t, &test1);

  printf("\n*** Looking up DD messages\n\n");

  PalistDestroy(ppl);
  ppl=PalistNew();

  while (TrkLookupMsg(t, NULL, "DD", NULL, ppl, &test1))
    TrkPrintMsg(t, &test1);

  printf("\n*** Looking up all messages\n\n");

  PalistDestroy(ppl);
  ppl=PalistNew();

  while (TrkLookupMsg(t, NULL, NULL, NULL, ppl, &test1))
    TrkPrintMsg(t, &test1);

  PalistDestroy(ppl);
  ppl=PalistNew();

  printf("\n*** updating 1st msg (owner ADMN -> DD)!\n");

  test1=new;
  strcpy(new.to, "DD");
  new.tl.uid=666666L;
  TrkUpdateMsg(t, &test1, &new);


  printf("\n*** Looking up all messages\n\n");

  PalistDestroy(ppl);
  ppl=PalistNew();

  while (TrkLookupMsg(t, NULL, NULL, NULL, ppl, &test1))
    TrkPrintMsg(t, &test1);

  PalistDestroy(ppl);

  TrkClose(t);

  return 0;
}


