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
static char rcs_id[]="$Id: s_match.c,v 1.1 2002/10/01 17:56:26 sdudley Exp $";
#pragma on(unreferenced)

#define NOVARS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "alc.h"
#include "prog.h"
#include "max.h"
#include "squish.h"
#include "s_match.h"

MATCHOUT * MatchOutOpen(NETADDR *who,int type,byte flavour)
{
  MATCHOUT *mo;
  extern struct _hpkt *hpl;

  if (config.flag & FLAG_FRODO)
    return (HoleMatchOutOpen(who, type, flavour));
  
  if ((mo=(MATCHOUT *)malloc(sizeof(MATCHOUT)))==NULL)
    return NULL;
  
  if (who->zone==ZONE_ALL)
  {
    /* Process all zones, so range is 'config.ob[0 ... num_ob]' */
    mo->high_ob=(sword)config.num_ob;
    mo->cur_ob=0;
  }
  else
  {
    /* Process only the one area 'config.ob[their_zone]' */
    
    mo->cur_ob=(sword)FindOutbound(who->zone);
    mo->high_ob=mo->cur_ob+1;
  }
  
  if (mo->cur_ob==-1 || mo->high_ob==-1)
  {
    free(mo);
    return (HoleMatchOutOpen(who, type, flavour));
  }

  mo->ff=mo->parentff=NULL;
  mo->who=*who;
  mo->flavour=flavour;
  mo->type=(sword)type;
  mo->config=&config;
  mo->hpkt=hpl;
  
  if (! MatchOutNext(mo))
  {
    free(mo);
    return NULL;
  }
  
  return (mo);
}

int MatchOutNext(MATCHOUT *mo)
{
  char net[20];
  char node[20];
  char doname[PATHLEN];
  char temp[PATHLEN];
  byte *p;
  
  int found;

  mo->got_type=0;
  mo->fFromHole=FALSE;

  if (config.flag & FLAG_FRODO)
    return (HoleMatchOutNext(mo));
  
  if (mo->cur_ob >= mo->high_ob)
    return (HoleMatchOutNext(mo));

  /* Keep looping until we obtain a filename */
  
  for (*doname='\0';*doname=='\0';)
  {
    if (! mo->ff) /* If the findfile handle wasn't open, then do so */
    {
      /* Create the wildcard filespec */
      
      /* If either the net or node number is 'ALL', then replace it         *
       * with "????", so we can find all nodes of the appropriate           *
       * type.                                                              */

      if (mo->who.net==NET_ALL)
        (void)strcpy(net, fourqs);
      else
        (void)sprintf(net, p04x, mo->who.net);

      if (mo->who.node==NODE_ALL)
        (void)strcpy(node, fourqs);
      else
        (void)sprintf(node, p04x, mo->who.node);

      /* Now build the final path out of the directory name, the            *
       * four-digit net and node addresses, and the .* trailer.             */

      (void)sprintf(temp, "%s%s%s.*",
                    FixOutboundName(config.ob[mo->cur_ob]),
                    net, node);
                
      mo->ff=FindOpen(temp, MSDOS_SUBDIR);
    }
    else if (FindNext(mo->ff) != 0) /* Else try to get another one */
    {
      /* If not, make it look just as if we were just trying to open the    *
       * handle, but failed because there was nothing there.                */
         
      FindClose(mo->ff);
      mo->ff=NULL;
    }
    
    /*************************************************************/
    /************** STUPID BINKLEYTERM KLUDGE ALERT **************/
    /*************************************************************/

    /* If we ran out of things to look at, try changing back to the         *
     * parent directory, if we were browsing a pointdir.                    */
    
    if (mo->ff==NULL)
    {
      mo->ff=mo->parentff;
      mo->parentff=NULL;

      /* Now go to the next match, since the current match was the .PNT     *
       * directory.                                                         */

      if (FindNext(mo->ff) != 0)
      {
        FindClose(mo->ff);
        mo->ff=NULL;
      }
    }
    
    if (mo->ff==NULL)
    {
      /* If this was the last zoned outbound area to look at, then we're    *
       * SOL.                                                               */
      
      if (++mo->cur_ob >= mo->high_ob)
        return (HoleMatchOutNext(mo));
      else
      {
        /* Otherwise, we can proceed to the next area, and start the        *
         * search all over again.                                           */
        
        continue;
      }
    }
    
    /* Now, we have a filename with the right root filename sitting in      *
     * ff->szName.  Now, check the extension, and if it's what we're       *
     * looking for, copy it into 'doname' to get out of the loop.           */
    
    if ((p=strchr(strupr(mo->ff->szName),'.')) != NULL)
    {
      /*************************************************************/
      /************** STUPID BINKLEYTERM KLUDGE ALERT **************/
      /*************************************************************/

      /* handle the gawd-awful .PNT dir that bink 2.5 uses! */

      if (p[1]=='P' && p[2]=='N' && p[3]=='T' && mo->who.point)
      {
        /* Create the name of the directory to search */

        (void)strcpy(temp, FixOutboundName(config.ob[mo->cur_ob]));
        (void)strcat(temp, mo->ff->szName);
        (void)strcat(temp, "\\");

        /* Figure out which point we need to search for */

        if (mo->who.point==POINT_ALL)
          (void)strcat(temp, "*.*");
        else
          (void)sprintf(temp+strlen(temp), "%08hx.*", mo->who.point);


        /* Now do a kludge so that all of the MatchXX code doesn't have   *
         * to be rewritten; shift the main dir into the 'parentff' handle,*
         * and open a new handle for the point directory with 'ff'.  This *
         * means that we can still act as a state-driven function.        */

        mo->parentff=mo->ff;

        /* Figure out which node that this one is a point off */

        if (sscanf(mo->ff->szName, "%04hx%04hx",
                   &mo->found.net, &mo->found.node)==2)
        {
          /* Fill in zone element */

          mo->found.zone=config.ob[mo->cur_ob];

          /* If there is anything to be found, parse it now */

          mo->ff=FindOpen(temp, 0);
        }
        else
          mo->ff=NULL;

        /* Musta been a blank directory, so keep on chugging, and pretend *
         * that it's just something that we don't understand.             */

        if (mo->ff==NULL)
        {
          mo->got_type=found=MATCH_UNKNOWN;
          mo->ff=mo->parentff;
          mo->parentff=NULL;
        }
        else
        {
          char *oldp=p;
          
          /* Now parse the dot, so we can handle the next entry */

          if ((p=strchr(strupr(mo->ff->szName),'.'))==NULL)
            p=oldp;
        }
      }
      
      if (p[2]=='L' && p[3]=='O')
        mo->got_type=found=MATCH_FLO;
      else if (p[2]=='U' && p[3]=='T')
        mo->got_type=found=MATCH_OUT;
      else
        mo->got_type=found=MATCH_UNKNOWN;
      
      /* Now, compare this to what the calling function asked for */
      
      if (mo->type & found) /* Got it! */
      {
        /* Now, check to see if the flavour types match */
        
        mo->flavour=(byte)toupper(mo->flavour);
        
        if (mo->flavour==0 ||
            mo->flavour==p[1] ||
            (mo->flavour=='F' && p[1]=='O') ||
            (mo->flavour=='O' && p[1]=='F') ||
            (mo->flavour=='L' && p[1]=='N') ||
            (mo->flavour=='U' && p[1] != 'N'))
        {
          /* Looks good, so return the right filename. */

          
          
          /*************************************************************/
          /************** STUPID BINKLEYTERM KLUDGE ALERT **************/
          /*************************************************************/

          /* If we're searching a point dir... */
          
          if (mo->parentff)
          {
            if (sscanf(mo->ff->szName, "%08hx", &mo->found.point) != 1)
              mo->found.point=0;
          }
          else
          {
            if (sscanf(mo->ff->szName,"%04hx%04hx",
                       &mo->found.net,&mo->found.node) != 2)
            {
              mo->found.zone=mo->found.net=mo->found.node=mo->found.point=0;
            }
            else
            {
              /* Fill in the rest of the 'found' structure */
              mo->found.zone=config.ob[mo->cur_ob];
              mo->found.point=0;
            }
          }
          
          (void)strcpy(doname,
                       FixOutboundName(config.ob[mo->cur_ob]));


          /* Add the name of the parent directory */

          if (mo->parentff)
            (void)sprintf(doname+strlen(doname), "%04hx%04hx.pnt\\",
                          (unsigned)mo->found.net, (unsigned)mo->found.node);

          (void)strcat(doname, mo->ff->szName);
          (void)upper_fn(doname);
        }
      }
      /* else do nothing, and we loop around to find next match */
    }
  }
  
  /* If we managed to get this far, we must have found a valid              *
   * filename, so put the name in the right place, and return.              */
  
  (void)strcpy(mo->name,doname);
  (void)strupr(mo->name);
  
  return 1;
}


void MatchOutClose(MATCHOUT *mo)
{
  if (mo)
  {
    if (config.flag & FLAG_FRODO)
    {
      HoleMatchOutClose(mo);
      return;
    }

    if (mo->ff)
      FindClose(mo->ff);
    
    free(mo);
  }
}



static int near FindOutbound(word zone)
{
  int i;
  
  for (i=(int)config.num_ob-1; i >= 0; i--)
    if (config.ob[i]==zone)
      return i;
    
  return -1;
}

