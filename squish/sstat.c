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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: sstat.c,v 1.5 2004/01/13 00:42:14 paltas Exp $";
#pragma on(unreferenced)
#endif

#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "sstat.h"
#include "sstatp.h"

static struct _ahlist *ahlist=NULL;
static struct _nodtot *nodtot=NULL;
static struct _sscfg sc;

char statfile[128];

void _fast NoMem(void);


static int near MatchNN(NETADDR *n1, NETADDR *n2)
{
  return (n1->zone==n2->zone &&
          n1->net ==n2->net  &&
          n1->node==n2->node &&
          n1->point==n2->point);
}



static int near DoThisArea(char *tag)
{
  struct _arealist *al;

  if (sc.do_all)
    return TRUE;
  
  for (al=sc.area; al; al=al->next)
    if (eqstri(tag, al->tag))
      return TRUE;
    
  return FALSE;
}

static int near DoThisNode(NETADDR *n)
{
  struct _nodelist *nl;
  
  for (nl=sc.node; nl; nl=nl->next)
    if (MatchNN(&nl->n, n))
      return TRUE;
    
  return FALSE;
}



/* Read all of the specified nodes in from the SQUISH.STA stats file */

static void near ReadArea(int fd, struct _ahlist *al, struct _tarea *ta)
{
  struct _tnode tn;
  struct _stlist *sl;
  struct _nodtot *nt;


  al->in_msgs += ta->in_msgs;
  al->in_bytes += ta->in_bytes/100;

  while (ta->n_nodes--)
  {
    if (read(fd, (char *)&tn, sizeof tn) != sizeof tn)
      return;
    
    #ifdef DEBUG
    printf("    Node         = %s\n", Address(&tn.node));
    printf("        OutMsgs  = %ld\n", (unsigned long) tn.out_msgs);
    printf("        OutBytes = %ld\n", (unsigned long) tn.out_bytes);
    #endif

    /* Only process the specified nodes */
      
    if (! DoThisNode(&tn.node))
      continue;
    
    for (sl=al->slist; sl; sl=sl->next)
      if (MatchNN(&sl->node, &tn.node))
        break;
      
    if (sl==NULL)
    {
      sl=smalloc(sizeof(struct _stlist));
      
      sl->next=al->slist;
      al->slist=sl;
    }

    /* Make sure that this node is already in the llist of node totals */

    for (nt=nodtot; nt; nt=nt->next)
      if (MatchNN(&nt->node, &tn.node))
        break;

    /* Not there, so add it */

    if (nt==NULL)
    {
      nt=smalloc(sizeof(struct _nodtot));

      nt->node=tn.node;

      nt->next=nodtot;
      nodtot=nt;
    }
    
    sl->node=tn.node;
    sl->out_msgs += tn.out_msgs;
    sl->out_bytes += tn.out_bytes/100;
  }
}





static void near ParseStats(int fd)
{
  int found;
  struct _thdr th;
  struct _tarea tarea;
  struct _ahlist *al; 

  while (read(fd, (char *)&th, sizeof th)==sizeof th)
  {
    found = FALSE;

    if (th.type != TYPE_AREA)
    {
      lseek(fd, th.len, SEEK_CUR);
      continue;
    }

    /* Read frame from file and then copy to the tarea structure */

    if (read(fd, (char *)&tarea, sizeof tarea) != sizeof tarea)
      break;

    #ifdef DEBUG
    printf("Area: %s\n", tarea.tag);
    printf(" InMsgs: %lu\n", (unsigned long) tarea.in_msgs);
    printf("InBytes: %lu\n", (unsigned long) tarea.in_bytes);
    #endif

    for (al=ahlist; al; al=al->next)
      if (eqstri(tarea.tag, al->tag))
      {
        found = TRUE;
	break;
      }

    /* This area not found */

    if (found == FALSE)
    {
      al=smalloc(sizeof(struct _ahlist));

      strcpy(al->tag, tarea.tag);

      al->next=ahlist;
      ahlist=al;
    }

    ReadArea(fd, al, &tarea);
  }
}





static int near Percent1(dword a, dword b)
{
  return (int)(a*100L/b);
}




static int near Percent2(dword a, dword b)
{
  return (int)((a*10000L/b) % 100L);
}

#define Percent(a, b) Percent1(a,b), Percent2(a,b)




static void near CalcTotals(dword *total_in_bytes, dword *total_in_msgs)
{
  struct _ahlist *al;
  struct _stlist *sl;

  total_in_bytes=total_in_msgs=0;

  for (al=ahlist; al; al=al->next)
  {
    if (! DoThisArea(al->tag))
      continue;
    
    al->total_out_bytes=al->total_out_msgs=0;

    for (sl=al->slist; sl; sl=sl->next)
    {
      al->total_out_bytes += sl->out_bytes;
      al->total_out_msgs  += sl->out_msgs;
    }

    if (al->total_out_bytes==0 && al->total_out_msgs==0)
      continue;

    total_in_bytes += al->in_bytes;
    total_in_msgs  += al->in_msgs;
    #ifdef DEBUG
	printf("Total in bytes: %d\nTotal in msgs: %d\n\n", (int) total_in_bytes, (int) total_in_msgs);
    #endif
  }
}




static void near CalculateStats(dword total_in_bytes, dword total_in_msgs)
{
  struct _ahlist *al;
  struct _stlist *sl;
  struct _nodtot *nt;

  for (al=ahlist; al; al=al->next)
  {
    if (! DoThisArea(al->tag))
      continue;

    printf("\nArea %s\n", al->tag);

    printf("  BYTES IN : %8ld (%02d.%02d%% of total bytes in)\n",
    	   (unsigned long) al->in_bytes*100L,
           (int)(al->in_bytes*100/total_in_bytes),
           (int)((al->in_bytes*10000/total_in_bytes) % 100));

    printf("  BYTES OUT: %8ld\n",
           (unsigned long) al->total_out_bytes*100L);

    printf("   MSGS IN : %8ld (%02d.%02d%% of total msgs in)\n",
           (unsigned long) al->in_msgs,
           (int) (al->in_msgs*100/total_in_msgs),
           (int) ((al->in_msgs*10000/total_in_msgs) % 100));

    printf("   MSGS OUT: %8ld\n\n",
           (unsigned long) al->total_out_msgs);

    /* Don't log areas with no output */

    if (total_in_bytes==0 || total_in_msgs==0 ||
        al->total_out_bytes==0 || al->total_out_msgs==0)
    {
      printf("   (No outbound traffic for specified nodes.)\n");
      continue;
    }

    printf("   Node            Byte Out MsgOut %% Bytes %%  Msgs %% TBCst %% TMCst\n");
    printf("   --------------- -------- ------ ------- ------- ------- -------\n");

    for (sl=al->slist; sl; sl=sl->next)
    {
      double area_percent_bytes, area_percent_msgs;

      area_percent_bytes=((double)sl->out_bytes/(double)al->total_out_bytes)*
                         ((double)al->in_bytes /(double)total_in_bytes)*
                          (double)100;

      area_percent_msgs =((double)sl->out_msgs/(double)al->total_out_msgs)*
                         ((double)al->in_msgs /(double)total_in_msgs)*
                          (double)100;

      printf("   %-15s %8ld %6ld %3d.%02d%% %3d.%02d%% %6.02f%% %6.02f%%\n",
             Address(&sl->node),
             (unsigned long) sl->out_bytes*100L,
             (unsigned long) sl->out_msgs,
             Percent(sl->out_bytes, al->total_out_bytes),
             Percent(sl->out_msgs,  al->total_out_msgs),
             (float)area_percent_bytes,
             (float)area_percent_msgs);

      for (nt=nodtot; nt; nt=nt->next)
        if (MatchNN(&nt->node, &sl->node))
        {
          nt->total_percent_bytes += area_percent_bytes;
          nt->total_percent_msgs  += area_percent_msgs;
          break;
        }
    }
  }

  printf("\nNODE TOTALS:\n\n");

  printf("   Node             %%Bytes %% Msgs\n");
  printf("   ---------------- ------ ------\n");

  for (nt=nodtot; nt; nt=nt->next)
  {
    printf("   %-15s  %05.02f%% %05.02f%%\n",
           Address(&nt->node),
           (float) nt->total_percent_bytes,
           (float) nt->total_percent_msgs);
  }

}





static void near ParseConfigLine(char *line)
{
  static char *cfgdelim=" \t\n\r,";
  char *s;

  /* Strip off any comments */
  
  if ((s=strchr(line, ';')) != NULL)
    *s='\0';


  /* Grab a word from the config */
  
  s=strtok(line, cfgdelim);
  
  /* Nodes to track */
  
  if (s==NULL)
    return;

  if (eqstri(s, "track"))
  {
    struct _nodelist *node;
    NETADDR last;
  
    memset(&last, '\0', sizeof(NETADDR));


    /* Parse all net/node numbers off this line */
    
    while ((s=strtok(NULL, cfgdelim)) != NULL)
    {
      node=smalloc(sizeof(struct _nodelist));

      node->n=last;
      
      ParseNN(s, &node->n.zone, &node->n.net,
              &node->n.node, &node->n.point, FALSE);
      
      last=node->n;

      /* Append to linked list */
      
      node->next=sc.node;
      sc.node=node;
    }
  }
  else if (eqstri(s, "area"))
  {
    struct _arealist *area;
    
    while ((s=strtok(NULL, cfgdelim)) != NULL)
    {
      if (eqstri(s, "all"))
        sc.do_all=TRUE;
      else
      {
        area=smalloc(sizeof(struct _arealist));
      
        area->tag=sstrdup(s);
      
        area->next=sc.area;
        sc.area=area;
      }
    }
  }
  else if(eqstri(s, "statfile"))
  {
    while ((s=strtok(NULL, cfgdelim)) != NULL)
    {
	strcat(statfile, s);
    }
    
  }
  
  else
  {
    printf("Invalid keyword in config file: `%s'\n", s);
  }
}





static void near ParseConfig(char *cfg)
{
  FILE *fp;
  char * envConfig = NULL;
  char * tmp = NULL;
  char line[PATHLEN];
  
  sc.node=NULL;
  sc.area=NULL;
  sc.do_all=FALSE;

  if ((envConfig = getenv("SQUISH")))
  {
    if((tmp = strrchr(envConfig, '/')))
    {
	strncpy(cfg, envConfig, tmp - envConfig);
	cfg[tmp - envConfig] = '\0';
#ifdef UNIX
	strcat(cfg, "/sstat.cfg");
#else
	strcat(cfg, "\SSTAT.CFG");
#endif	
    }
  }
  
  if ((fp=shfopen(cfg, "r", O_RDONLY))==NULL)
  {
    printf("Error opening `%s'!\n", cfg);
    exit(1);
  }
  
  while (fgets(line, PATHLEN, fp))
    ParseConfigLine(line);
  
  fclose(fp);
}





int _stdc main(int argc, char *argv[])
{
  dword total_in_bytes, total_in_msgs;
  int fd;
  char cfg[128];

  memset(statfile, 0, 128);

  NW(argc);
  NW(argv);
  
  ParseConfig(cfg);

  if(statfile[0] == 0)
  {
    #ifndef UNIX
	strcpy(statfile, "SQUISH.STT");
    #else
	strcpy(statfile, "squish.stt");		
    #endif
  }

  if ((fd=open(statfile, O_RDONLY | O_BINARY))==-1)
  {
    printf("Error!  No statistics file to read!\n");
    return 1;
  }

  ParseStats(fd);
  close(fd);

  CalcTotals(&total_in_bytes, &total_in_msgs);
  CalculateStats(total_in_bytes, total_in_msgs);

  return 0;
}

void _fast NoMem(void)
{
  printf("Ran out of memory!\n");
  exit(1);
}

