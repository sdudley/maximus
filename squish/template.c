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

#define INCL_DOS
#include <string.h>
#include <os2.h>
#include "msgapi.h"
#include "sqfeat.h"

static char szAppName[]="Template";

/****************************************************************************
 * FeatureInit:                                                             *
 ****************************************************************************
 *                                                                          *
 * This is called at Squish startup, when the "Feature" line in SQUISH.CFG  *
 * is processed.  This routine can be used to perform feature-specific      *
 * initialization needs.                                                    *
 *                                                                          *
 * The only action that this function must perform is to fill out the       *
 * 'feature information' structure that is passed to it by Squish.          *
 * At present, the only field in this structure is the 'Config Name'        *
 * option.  This can be used to register keywords for which the             *
 * FeatureConfig function will be called.  This can be used to              *
 * implement feature-specific keywords in the configuration file.           *
 *                                                                          *
 * NOTE:  External features should NOT display anything on the screen       *
 * in this function.                                                        *
 *                                                                          *
 * A return value of 0 indicates that the feature initialized successfully. *
 * A non-zero return value indicates failure and instructs Squish to        *
 * terminate.                                                               *
 *                                                                          *
 ****************************************************************************/

word FEATENTRY _export FeatureInit(struct _feat_init far *pfi)
{
  /* The szConfigName field should contain a list of the keywords           *
   * for which FeatureConfig should be called.  These tokens are only       *
   * matched if they are the first word on a given line.  More than one     *
   * token can be given, as long as a "\r" is used to separate adjacent     *
   * tokens, such as "Killrcat\rKillrdog\rKillrbird".                       */

  strcpy(pfi->szConfigName, szAppName);
  return 0;
}


/****************************************************************************
 * FeatureConfig                                                            *
 ****************************************************************************
 *                                                                          *
 * This function is called when Squish detects one of the specified         *
 * feature configuration keywords in SQUISH.CFG.  The feature should        *
 * the information on this line as required, and then return to Squish.     *
 *                                                                          *
 * A return value of 0 indicates success.  A non-zero return value          *
 * instructs Squish to abort.                                               *
 *                                                                          *
 ****************************************************************************/

word FEATENTRY _export FeatureConfig(struct _feat_config far *pfc)
{
  char *p;

  printf("Config: got args: \"");

  for (p=pfc->ppszArgs[1]; p && *p; p++)
    printf("%s ", p);

  printf("\"\n");

  return 0;
}



/****************************************************************************
 * FeatureNetMsg                                                            *
 ****************************************************************************
 *                                                                          *
 * This function is called just before Squish packs a mesage from a         *
 * netmail area.  Squish will call this function for each netmail message,  *
 * regardless of the status of the MSGSENT bit, unless otherwise defined    *
 * in the feature initialization structure (see FeatureInit).               *
 *                                                                          *
 * Information in the feat_netmsg structure describes the current message   *
 * being processed, in addition to pointers to the message header, body     *
 * text, and control information.                                           *
 *                                                                          *
 * If any special actions are necessary, the feature should fill out the    *
 * ulAction field in the structure before this function terminates.         *
 *                                                                          *
 * A return value of 0 indicates success.  A non-zero return value          *
 * instructs Squish to terminate execution.                                 *
 *                                                                          *
 ****************************************************************************/


word FEATENTRY _export FeatureNetMsg(struct _feat_netmsg far *pfn)
{
  printf("Packed message to \"%s\"\n", pfn->pMsg->to);
  return 0;
}


word FEATENTRY _export FeatureTossMsg(struct _feat_toss far *pft)
{
  printf("Tossed message to \"%s\"\n", pft->pMsg->to);
  return 0;
}


word FEATENTRY _export FeatureScanMsg(struct _feat_scan far *pfs)
{
  printf("Scanned message to \"%s\"\n", pfs->pMsg->to);
  return 0;
}

word FEATENTRY _export FeatureTerm(struct _feat_term far *pft)
{
  printf("Feature termination.\n");
  (void)pft;
  return 0;
}


#ifdef __FLAT__
void FEATENTRY _export Feature32Bit(void)
#else
void FEATENTRY _export Feature16Bit(void)
#endif
{
}

