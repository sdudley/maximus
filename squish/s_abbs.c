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
static char rcs_id[]="$Id: s_abbs.c,v 1.1 2002/10/01 17:56:19 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include "alc.h"
#include "dr.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"

#define MAX_ABBS_LEN 4096

/*static char *abbs_delim=" \t\n,;";*/
static char *abbs_delim=" \t\n;";

void Parse_Areas(char *areas_name)
{
  FILE *abbs;

  struct _cfgarea *ar;
  char *path, *tag, *orig, *line, *s;
  word flag, type;

  int found_origin=FALSE;

  if (areas_name==NULL || *areas_name=='\0')
    return;

  if ((abbs=shfopen(areas_name, "r", O_RDONLY))==NULL)
    ErrOpening("AREAS", areas_name);

  line=smalloc(MAX_ABBS_LEN);
  orig=smalloc(MAX_ABBS_LEN);

  while (fgets(line, MAX_ABBS_LEN-1, abbs) != NULL)
  {
    /* Strip comment characters */

    if ((s=strchr(line,';')) != NULL)
      *s='\0';

    (void)Strip_Trailing(line,'\n');

    if (! *line)
      continue;

      /* This treats 1st non-blank line as origin, if it contains           *
       * a '!'.                                                             */

    if (! found_origin)
    {
      found_origin=TRUE;

      if (strchr(line,'!'))
      {
        line[MAXORIGIN]='\0';

        free(config.origin);

        config.origin=sstrdup(line);

        /* Chop off the SysOp name, if any */
        s=strrchr(config.origin,'!');

        if (s)
          *s='\0';

        continue;
      }
    }

    (void)strupr(line);
    (void)strcpy(orig,line);


    flag=0;
    type=MSGTYPE_ECHO;
    path=strtok(line,abbs_delim);

    /* Now scan for "#" and "$" modifiers */

    while (path && (*path=='$' || *path=='#'))
    {
      if (*path=='#')
      {
        flag |= AFLAG_PASSTHRU;

        if (! *++path)
          path=strtok(NULL,abbs_delim);
      }

      if (path && *path=='$') /* '$' signifies a SquishMail-type area */
      {
        type=MSGTYPE_SQUISH | MSGTYPE_ECHO;

        if (! *++path)
          path=strtok(NULL,abbs_delim);
      }
    }


    tag=strtok(NULL,abbs_delim);

    if (!tag || *tag=='\0')
      continue;

    /* Find the start of the net/node entries */
    s=strtok(NULL,abbs_delim);
    
    /* Convert the location of the net/node start to the orig[] string,     *
     * since it doesn't have any of those pesky nuls that strtok put in.    */
       
    ar=Declare_Area(path, tag, s ? orig+(word)(s-line) : NULL,
                    type, flag);

    if (ar && (ar->flag & (AFLAG_DUPES|AFLAG_NET)))
    {
      S_LogMsg("!Attempted to redeclare area %s as EchoMail", ar->name);
      exit(ERL_ERROR);
    }
  }

  free(orig);
  free(line);

  (void)fclose(abbs);

  if (config.dupes==NULL)
    config.dupes=config.badmsgs;
}


struct _cfgarea * Declare_Area(char *path, char *tag, char *nodes, word type, word flag)
{
  static char unknown_flag[]="Unknown area declaration flag: \"%s\"\n";
  NETADDR naddr;
  struct _cfgarea *ar, *newar;
  struct _sblist *sb, *sbcheck, last;
  struct _perlist *pl;
  unsigned do_path; /* fix up path spec? */
  word exists;
  char *s, *p;

  if (path==NULL || tag==NULL || *path=='\0' || *tag=='\0')
  {
    (void)printf("Error!  Both a path and a tag must be specified for area %s/%s\n",
                 path, tag);

    return NULL;
  }
  
  /* Scan the list of areas to see if this one was already put in there */

  (void)strupr(tag);
  
  newar=smalloc(sizeof(struct _cfgarea));

  newar->name=tag; /* don't strdup because we don't need to yet */
  
  if ((ar=SkipAddNode(config.area, newar, &exists))==NULL)
    NoMem();
  
  /* If the area exists, just use the area pointed to by 'ar', and free     *
   * the 'newar' that we didn't need.                                       */

  ar->flag |= flag;
  ar->type |= type;

  /* Default to *.MSG if nothing else specified */

  if ((ar->type & MSGTYPE_ECHO)==0)
    ar->type |= MSGTYPE_SDM;

  if (exists)
  {
    do_path=FALSE;

    if (!eqstri(path, ar->path))
    {
      S_LogMsg("!Path for area %s redeclared as %s",
             ar->name, path);

      free(ar->path);
      do_path=TRUE;
    }

    free(newar);
  }
  else
  {
    /* Otherwise, it wasn't found, so make a new area */

    if (config.addr==NULL)
    {
      (void)printf("Error!  An 'Address' statement must be given before defining\n"
                   "the first message area!\n");
      exit(ERL_ERROR);
    }
    
    ar->primary=*config.addr;

    /* Add in the new area's path */

    do_path=TRUE;
  }

  if (do_path)
  {
    /* Copy in the area's path */

    ar->path=sstrdup(fancy_str(path));

    /* If the area is a "D:\" format, don't strip the trailing slash */

    if (ar->path[1]==':' && (ar->path[2]=='\\' || ar->path[2]=='/') &&
        ar->path[3]=='\0')
      ;
    else (void)Strip_Trailing(ar->path, '\\');

    ar->name=sstrdup(tag);
  }

  /* Parse the addresses to scan to */

  last=ar->primary;

  if (!nodes)
    s=NULL;
  else s=strtok(nodes, abbs_delim);

  while (s)
  {
    last.point=0;
    
    if (*s=='/' || *s=='-')
    {
      (void)strlwr(s);
      
      switch (s[1])
      {
        case 'h':
          ar->flag |= AFLAG_HIDEMSG;
          break;

        case 's':     /* Strip private flag */
          ar->flag |= AFLAG_STRIPPVT;
          break;

        case '0':     /* Passthru */
          ar->flag |= AFLAG_PASSTHRU;
          break;

        case 'f':     /* FTSC (*.msg) area */
          ar->type |= MSGTYPE_SDM;
          ar->type &= ~MSGTYPE_SQUISH;
          break;

        case '$':     /* Squish area */
          ar->type |= MSGTYPE_SQUISH;
          ar->type &= ~MSGTYPE_SDM;

          switch(s[2])
          {
            case 'm': ar->sq_max_msgs= (dword)atol(s+3); break;
            case 's': ar->sq_save_msgs=(dword)atol(s+3); break;
            case 'd': ar->sq_keep_days=(word) atoi(s+3); break;
            case '\0': break;
            default:  (void)printf(unknown_flag, s);
          }
          break;
          
        case 't': /* -t3 for a type-3 message base */
          ar->type &= ~(MSGTYPE_SDM|MSGTYPE_SQUISH);
          ar->type |= (word)atoi(s+2);
          break;

        case 'p':     /* Primary address for this area */
          ar->primary.point=0;

          ParseNN(s+2, &ar->primary.zone, &ar->primary.net,
                       &ar->primary.node, &ar->primary.point, FALSE);
          last=ar->primary;
          break;
          
        case '+':     /* add a node to the seen-bys */
          sb=smalloc(sizeof(struct _sblist));
          *sb=ar->primary;
          sb->point=0;

          ParseNN(s+2, &sb->zone, &sb->net, &sb->node, &sb->point, FALSE);

          sb->next=ar->add;
          ar->add=sb;
          
          ar->num_add++;
          break;

        case 'u':     /* allow remote updates */
          sb=smalloc(sizeof(struct _sblist));
          *sb=ar->primary;
          sb->point=0;

          ParseNN(s+2, &sb->zone, &sb->net, &sb->node, &sb->point, FALSE);

          sb->next=ar->update_ok;
          ar->update_ok=sb;
          break;

        case 'i':       /* send only personal msgs */
          /* format: "-i249/106;Scott_Dudley" */

          pl=smalloc(sizeof(struct _perlist));

          pl->node=config.def;
          ParseNN(s+2, &pl->node.zone, &pl->node.net, &pl->node.node,
                  &pl->node.point, FALSE);

          if ((p=strchr(s, ','))==NULL)
          {
            free(pl);
            S_LogMsg("!Invalid personal flag: %s", s);
          }
          else
          {
            /* Create copy of name */

            pl->name=sstrdup(p+1);

            /* Convert underscores to spaces */

            for (p=pl->name; (p=strchr(p, '_')) != NULL; )
              *p++=' ';

            /* Add to linked list */

            pl->next=ar->plist;
            ar->plist=pl;
          }
          break;

        case 'x':       /* exclude a node from sending msgs into an echo */
          sb=smalloc(sizeof(struct _sblist));
          *sb=ar->primary;
          sb->point=0;

          ParseNN(s+2, &sb->zone, &sb->net, &sb->node, &sb->point, FALSE);

          sb->next=ar->norecv;
          ar->norecv=sb;
          break;

        default:
          (void)printf(unknown_flag, s);
      }
    }
    else if (*s=='#')
      ar->flag |= AFLAG_PASSTHRU;
    else if (isdigit(*s) || *s=='.')
    {
      struct _statlist *sl;
      
      sb=smalloc(sizeof(struct _sblist));

      *sb=last;

      Parse_NetNode(s, &sb->zone, &sb->net, &sb->node, &sb->point);

      last=*sb;
      
      for (sbcheck=ar->scan; sbcheck; sbcheck=sbcheck->next)
        if (MatchSS(sbcheck, sb, FALSE))
        {
          S_LogMsg("!Area %s: node %s specified twice",
                   ar->name, Address(SblistToNetaddr(sbcheck, &naddr)));
          continue;
        }

      sb->next=ar->scan;
      ar->scan=sb;
      
      /* Add this to our statistics list */
      
      if (config.flag & FLAG_STATS)
      {
        sl=smalloc(sizeof(struct _statlist));
        
        /* Copy the node's address to this list */

        (void)SblistToNetaddr(sb, &sl->node);

        sl->next=ar->statlist;
        ar->statlist=sl;
      }
    }
    else
    {
      S_LogMsg("!Junk in area definition:  `%s'", s);
    }

    s=strtok(NULL, abbs_delim);

    ar->num_scan++;
  }

  if ((ar->type & (MSGTYPE_SDM|MSGTYPE_SQUISH))==0)
    ar->type |= MSGTYPE_SDM;

  return ar;
}

