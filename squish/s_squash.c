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

/**
 * @file	s_squash.c
 * @author	Scott J. Dudley
 * @version	$Id: s_squash.c,v 1.8 2004/01/22 08:04:28 wmcbrine Exp $
 *
 * $Log: s_squash.c,v $
 * Revision 1.8  2004/01/22 08:04:28  wmcbrine
 * Changed all the "static char rcs_id[]=" stuff to comments. Which works just
 * as well, but doesn't produce any warnings. :-)
 *
 * Revision 1.7  2003/12/14 17:40:19  paltas
 * Fixed different things (can't remember all of it.. )
 *
 * Revision 1.6  2003/11/18 23:04:18  paltas
 * This _might_ remove the .Flo issue..
 *
 * Revision 1.5  2003/09/03 13:51:33  paltas
 * /Linux instead of /UNIX on Linux machines
 *
 * Revision 1.4  2003/07/26 00:03:58  rfj
 * Squish (and MSGAPI) updates as suggested by Bo Simonsen, including correcting
 * a \ to / for UNIX systems, changes concerning packet file name case, via line
 * time stamp change, and s_toss.c table filled in (only for UNIX compiles for
 * now).
 *
 * Also updated squish version number to 1.12 beta.
 *
 * Revision 1.3  2003/06/18 02:00:17  wesgarland
 * Modified to detect when compressed packets wind up with the compressor's
 * extension rather than Squish's intended (e.g. .su0, mo3) extension.
 *
 * Based on changes submitted by Bo Simonsen; modified to have lowercase extensions
 * for the ?ut filenames, where the ? is the mail flavour (FLO)
 *
 */

#define NOVARS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <limits.h>
#include <process.h>
#include "alc.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_squash.h"
#ifdef UNIX
# include <errno.h>
#endif

extern char bsy_pkt_queued[]; /* "%s busy - packet queued" error msg */

static void near MakeArcBase(char *where, NETADDR *dest);

static void near RV_Attach          (byte *line,byte *ag[],NETADDR nn[],word num) {NW(config);NW(line);NW(ag);NW(nn);NW(num);}
static void near RV_Get             (byte *line,byte *ag[],NETADDR nn[],word num) {NW(config);NW(line);NW(ag);NW(nn);NW(num);}
static void near RV_Update          (byte *line,byte *ag[],NETADDR nn[],word num) {NW(config);NW(line);NW(ag);NW(nn);NW(num);}


static void near NotForFrodo(char *cmd)
{
  S_LogMsg("!Cmd `%s' can't be used with ArcmailAttach", cmd);
}

static void near RV_Define(byte *line,byte *ag[],NETADDR nn[],word num)
{
  struct _defn *def;
  byte *lp;
  
  NW(config);
  NW(nn);
  NW(num);

  
  def=smalloc(sizeof(struct _defn));
  
  def->name=sstrdup(ag[1]);
    
  lp=firstchar(line, routedelim, 3);

  if (lp)
    def->xlat=sstrdup(lp);
  else def->xlat=sstrdup("");
  
  def->next=defns;
  defns=def;
}



static void near RV_Dos(byte *line,byte *ag[],NETADDR nn[],word num)
{
  byte *p;
  byte *cmd;
  byte *comspec;

  NW(config);NW(ag);NW(nn);NW(num);

  p=firstchar(line,routedelim,2);

  if (p)
  {
    S_LogMsg("+DOS command: `%s'", p);

    #ifdef __WATCOMC__
      _heapshrink();
    #endif

    cmd=smalloc(strlen(p)+80);
    comspec=getenv("COMSPEC");
    
#ifndef UNIX
    if (comspec==NULL)
      comspec="COMMAND.COM";

    (void)sprintf(cmd, "%s /c %s", comspec, p);
#else
    if (comspec==NULL)
      comspec="/bin/sh";

    (void)sprintf(cmd, "%s %s", comspec, p);
#endif
  
    (void)CallExtern(cmd, FALSE);

    free(cmd);
  }
}

/* Figure out which type of archiver to use for the specified node... */
  
static struct _arcinfo * near Get_Archiver_Index( NETADDR *a)
{
  struct _arcinfo *ai;
  struct _sblist *pack;


  for (ai=config.arc; ai; ai=ai->next)
    for (pack=ai->nodes; pack; pack=pack->next)
      if (AddrMatchNS(a, pack))
        return ai;
  
  /* Node not found in any lists, so use the first archiver by default */
  return config.def_arc;
}




static int near Add_To_Archive(byte *arcname, NETADDR *found, byte *pktname, NETADDR *via)
{
  struct _arcinfo *ai;
  FFIND *ff;
  char cmd[PATHLEN*2];
  char temp[PATHLEN];
  char del[PATHLEN];
  byte *dot, *p;

  int arcret;
  int rc, x;

  rc=TRUE;
  ai=Get_Archiver_Index(via ? via : found);

  if (!ai)
    return FALSE;
  
  Form_Archiver_Cmd(arcname, pktname, cmd, ai->add);


  /* Now try to delete any old arcmail bundles */
  
  (void)strcpy(del, arcname);
  
  if ((dot=strrchr(del, '.')) != NULL)
  {
    for (x=0; x < 7; x++)
    {
      /* Hack on the right extension */

      (void)strcpy(del, arcname);
      (void)sprintf(dot, "%s?", arcm_exts[x]);
      
      if ((ff=FindOpen(del,0)) != NULL)
      {
        do
        {
          /* Now try to unlink all zero-length files int he same directory */

          (void)strcpy(temp, del);

          if (ff->ulSize==0L && (p=strrchr(temp, PATH_DELIM)) != NULL)
          {
            (void)strcpy(p+1, ff->szName);
            (void)unlink(temp);
          }
        }
        while (FindNext(ff)==0);
        
        FindClose(ff);
      }
    }
  }
  
  if (via && !MatchNN(found, via, FALSE))
    (void)sprintf(temp, ", via %s", Address(via));
  else *temp='\0';

  if ((config.flag & FLAG_QUIETARC)==0)
    (void)printf("\n");

  (void)printf("Packet for %s%s (%ld bytes) - Method: %s\n",
               Address(found), temp, fsize(pktname), ai->arcname);

  S_LogMsg(" %sing mail for %s (%ld bytes)%s",
           ai->arcname, Address(found), fsize(pktname), temp);

  arcret=CallExtern(cmd, TRUE);

  if (arcret==0 && !fexist(arcname) && fexist(pktname))
  {
    /* Wes: Sometimes, for reasons not well understood by man,
     * software just doesn't do what it's told. One example of
     * such software would be LHarc for UNIX v1.02. It ignores
     * the requested extension (e.g. .SU0) and instead replaces
     * it with its own extension (.LZH). How foolish!
     *
     * This is a generic fix to try and guess the name of the
     * archive, and rename it properly.
     */

    char 	filespec[FILENAME_MAX];
    char	rootname[FILENAME_MAX];
    char 	*filename = NULL;
    char 	*dot;
    FFIND      	*ff;

    strncpy(rootname, arcname, sizeof(rootname));
    rootname[sizeof(rootname) - 1] = (char)0;
    dot = strchr(rootname, '.');
    if (dot)
      *dot = (char)0;

    snprintf(filespec, sizeof(filespec), "%s.%s", rootname, ai->extension ? : "???");
    if ((ff = FindOpen(filespec, 0)))
    {
      filename = strdup(ff->szName);
      if (!filename)
	NoMem();

      if (FindNext(ff) == 0)
      {
	S_LogMsg("!Found more than one compressed bundle matching %s!", filespec);
	S_LogMsg("!Offending bundles: %s and %s", filename, ff->szName);
	free(filename);
	filename=NULL;
      }
      else
      {
	S_LogMsg("+Archiver generated filename %s; expected %s (renaming)",
		 filename, arcname);
	if (rename(filename, arcname))
	  S_LogMsg("!Unable to rename %s to %s! (%s)", filename, arcname, strerror(errno));
	free(filename);
      }
    }
  }

  if (arcret==0 && !fexist(arcname) && fexist(pktname))
  {
    arcret=-1;
    errno=0;
    S_LogMsg("!Compressed bundle not found after calling archiver!");
    rc=FALSE;
  }

  if (arcret != 0)
  {
    HandleArcRet(arcret, cmd);

    (void)sprintf(temp,
                  "%s%08lx.out",
                  FixOutboundName(0xffff),
                  get_unique_number());

    if (rename(pktname, temp)==0)
      S_LogMsg("!Packet %s moved back to %s", pktname, temp);
    else S_LogMsg("!Couldn't rename %s back to %s", pktname, temp);

    rc=FALSE;
  }
  else
  {
    (void)unlink(pktname);
  }

  return rc;
}


int Merge_Pkts(byte *from,byte *to)
{
  byte *copybuf;
  
  int fromfile, tofile;
  unsigned got, dont_unlink;
  word tempint;

  if (eqstri(from, to))
    return 0;

  /* If the destination file doesn't exist, then try the quick way, by      *
   * renaming the original.  If that works, then it's OK -- otherwise,      *
   * continue and do it the long way.                                       */

  if (!fexist(to))
  {
    if (rename(from, to)==0)
      return 0;
  }
  
  if ((fromfile=sopen(from, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    return -1;
  
  if ((tofile=open(to, O_RDWR | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1 ||
      lseek(tofile, -(long)sizeof(word), SEEK_END)==-1 ||
      read(tofile, (char *)&tempint, (unsigned)sizeof(word)) != (int)sizeof(word) ||
      tempint != 0 ||
      (copybuf=malloc(COPYSIZE))==NULL)
  {
    if (tofile != -1)
      (void)close(tofile);

    (void)close(fromfile);
    return -1;
  }
  
  /* Skip over the terminating NUL word, and set the position in the from   *
   * file to AFTER the packet header.                                       */
     
  (void)lseek(tofile, -(long)sizeof(word), SEEK_END);
  (void)lseek(fromfile, (long)sizeof(struct _pkthdr), SEEK_SET);

  dont_unlink=FALSE;

  while ((got=(unsigned)fastread(fromfile, copybuf, (unsigned)COPYSIZE)) != 0 &&
         got != (unsigned)-1)
  {
    if (fastwrite(tofile, copybuf, got) != (int)got)
    {
      S_LogMsg("!Error copying %s to %s", from, to);
      dont_unlink=TRUE;
      break;
    }
  }

  /* The terminating NUL should have been copied from the other packet,     *
   * so we don't need to do anything...                                     */
  
  free(copybuf);
  
  (void)close(tofile);
  (void)close(fromfile);
  
  if (!dont_unlink && unlink(from)==-1)
    return -1;
  
  return 0;
}

static void near Add_To_Flo(FILE *flo,byte *txt)
{
  char temp[PATHLEN];
  char wrd[PATHLEN];
  byte *p;

  /* Seek to beginning of file */

  (void)fseek(flo, 0L, SEEK_SET);

  /* Read each existing line to see if file was already attached... */

  while (fgets(temp, PATHLEN, flo) != NULL)
  {
    (void)Strip_Trailing(temp,'\n');

    /* Strip any comments */

    if (strchr(temp,';'))
      *temp='\0';


    /* Skip blank lines */

    if (! *temp)
      continue;


    /* Strip off any "#" or "^" characters for trunc/delete */

    if (*temp=='#' || *temp=='^')
      p=temp+1;
    else p=temp;

    /* Now, parse the first filename out of it */

    (void)getword(p, wrd, " ,;\t\n!", 1);

    /* If it matches, it already existed, so no work necessary */

      /* the things i do for lint ------------------v */
    if (eqstri(wrd, txt + ((*txt=='#' || *txt=='^') ? 1 : 0)))
      return;
  }

  /* If we got here, then it doesn't exist.  Just add it ourselves,         *
   * and then return...                                                     */

  (void)fseek(flo, 0L, SEEK_END);

  /* If there is any text to add, place it here.  Otherwise, maybe it's     *
   * just a dry poll.                                                       */

  if (*txt)
    (void)fprintf(flo, "%s\n", txt);
}




int Add_To_FloFile(byte *fmtxt, byte *from, byte *to)
{
  char temp[PATHLEN];
  FILE *fromfile=NULL;
  FILE *tofile;
  byte *mode, *p;
  long fs_to;

  fs_to=fsize(to);

  if (from && (fromfile=shfopen(from, "r", O_RDONLY | O_NOINHERIT))==NULL)
  {
    S_LogMsg("!Can't open %s", from);
    return -1;
  }

  if (fexist(to))
    mode="r+";
  else mode="w+";

  if ((tofile=shfopen(to, mode, O_RDWR | O_CREAT | O_NOINHERIT))==NULL)
  {
    if (fromfile)
      (void)fclose(fromfile);
    
    S_LogMsg("!Can't write to %s", to);
    return -1;
  }

  /* Just in case the last addition to the 'tofile' didn't leave a blank    *
   * line...  (But this is only needed if 'tofile' was non-blank!)          */

  if (fs_to > 0L)
  {
    /* Seek to the last character in the file */

    (void)fseek(tofile, -1L, SEEK_END);

    if (fgetc(tofile) != '\n')
      (void)fputc('\n', tofile);
  }

  (void)fseek(tofile, 0L, SEEK_END);

  /* Now copy everything which was in the old one into the new one */

  if (!from)
    Add_To_Flo(tofile, fmtxt);
  else
  {
    while (fgets(temp, sizeof(temp), fromfile))
    {
      (void)Strip_Trailing(temp, '\n');


      /* Strip any comments */

      if ((p=strchr(temp, ';')) != NULL)
        *p='\0';


      /* Skip blank lines */

      if (! *temp)
        continue;

      Add_To_Flo(tofile, temp);
    }
  }

  (void)fclose(tofile);

  if (!from)
    return 0;
  
  (void)fclose(fromfile);

  if (unlink(from)==0)
    return 0;

  S_LogMsg("!Err merging `%s' into `%s'", from ? from : fmtxt, to);
  return -1;
}


void FloName( byte *out, NETADDR * n, byte flavour, word addmode)
{
  FFIND *ff;
  byte *flavptr;
  byte flav, foundflav;
  
  NW(config);
  
  flav=(flavour=='O' ? (byte)'F' : flavour);

  MakeOutboundName(n, out);
  flavptr=out+strlen(out);

  flavptr[0]=(byte) (addmode ? '?' : tolower(flav));
  flavptr[1]='l';
  flavptr[2]='o';
  flavptr[3]='\0';
  
  /* If we're running in "Add Mode", if a ?LO is already waiting for the    *
   * same node (but with a different flavour), use that file instead of     *
   * the requested one.                                                     */

  if (addmode)
  {
    if ((ff=FindOpen(out, 0)) != NULL)
    {
      /* If we don't find anything useful, assume that we're using the      *
       * given flavour.                                                     */

      *flavptr=tolower(flav);


      /* Copy the flo style of the found filename into the filename that    *
       * we got.                                                            */

      do
      {
        foundflav=(byte)toupper(ff->szName[strlen(ff->szName) - 3]);

        /* Don't use .FLO files for addmode! */

        if (foundflav != 'F')
        {
          *flavptr=tolower(foundflav);
          break;
        }
      }
      while (FindNext(ff)==0);

      FindClose(ff);
    }
    else
    {
      /* Didn't find anything, so use the requested flavour */

      *flavptr=tolower(flav);
    }
  }
}



static void near OutName(byte *out, NETADDR *n, byte flavour)
{
  MakeOutboundName(n, out);
  out += strlen(out);
  
  *out++=flavour;
  *out++='u';
  *out++='t';
  *out='\0';
}





/* Many different flavours keep the fun going on and on... :-) */
  
static byte * near LifeSavers(byte flavour)
{
  struct _flotype *f;

  for (f=flo_str;f->name;f++)
    if (f->flavour==flavour)
      return (f->name);

  return "";
}



static byte near Get_Routing_Flavour(byte *ag[], int vn, int flo)
{
  struct _flotype *f;
  int x;

  for (x=vn; x < MAX_ROUTE_ARGS && *ag[x]; x++)
  {
    for (f=flo_str; f->name; f++)
      if (eqstri(ag[x], f->name))
        return (byte)(flo ? f->flavour
                          :  ((f->flavour=='F') 
                              ? (byte)'O' 
                              : f->flavour) );

    if (isalpha(*ag[x]) && !eqstri(ag[x],"world") && !eqstri(ag[x], "all"))
    {
      (void)printf("Invalid %s type: `%s'!\n", flo ? "FLO" : "packet", ag[x]);
      break;
    }
  }

  return (byte)(flo ? 'F' : 'O');
}





static void near ErrRename(byte *old,byte *new)
{
  S_LogMsg("!Err renaming `%s' to `%s'", old, new);
}



/* Change the flavour of a particular file */

static void near Change_Style(NETADDR nn[], unsigned num, byte from, byte to)
{
  MATCHOUT *mo;

  char temp[PATHLEN];
  byte *p;

  unsigned i;

  for (i=0; i < num; i++)
  {
    if (from=='L' || from=='U')
      mo=MatchOutOpen(&nn[i], MATCH_ALL, from);
    else mo=MatchOutOpen(&nn[i], MATCH_FLO | MATCH_OUT, from);

    if (!mo)
      continue;

    do
    {
      /* Make sure that we can open the .bsy file */

      if (BusyFileOpen(mo->name, FALSE)==-1)
      {
        S_LogMsg(bsy_pkt_queued, mo->name);
        HoleRemoveFromList(mo->name);
        continue;
      }
      
      (void)strcpy(temp, mo->name);

      /* Find the 'dot' in the filename */

      if ((p=strrchr(temp,'.')) != NULL)
      {
        switch(to)
        {
          case 'L':           /* Leave */

            /* Don't do .PNT dirs */

            if (p[1]=='P')
              break;

            /* Make sure that extension is nul-padded for at least 3 nuls */

            if (p[3]=='T' || p[3]=='O' || p[3]=='Q' ||
                (p[1]=='Z' && p[2]=='\0'))
            {
              if (p[2]=='\0')
                p[3]='\0';

              p[2]=p[1];
              p[1]='N';
            }
            break;

          case 'U':           /* Unleave */
            /* Don't do .PNT dirs */

            if (p[1]=='P')
              break;

            p[1]=p[2];

            if (p[3]=='T')
              p[2]='U';
            else if (p[3]=='O')
              p[2]='L';
            else if (p[3]=='Q')
              p[2]='E';
            else if (p[3]=='\0')
              p[2]='\0';
            break;

          case 'C':           /* Normal flow/out styles */
          case 'D':
          case 'H':
          case 'F':
          case 'O':
            /* Change the FLO type as appropriate */

            p[1]=(byte)to;

            /* If it's a ?UT file, and we've changed it to a 'F' (which is  *
             * the normal type of routing for FLO), then change it to OUT.  */

            if (toupper(p[2])=='U' && p[1]=='F')
              p[1]='O';
            break;

          default:
            /* happy lint */
            break;
        }

        if (! eqstri(mo->name,temp))
        {
          (void)printf("CHANGE: %s -> %s\n", mo->name, temp);

          if (fexist(temp))
          {
            if (toupper(p[2])=='U')
              (void)Merge_Pkts(mo->name, temp);
            else (void)Add_To_FloFile(NULL, mo->name, temp);
          }
          else if (rename(mo->name, temp)==-1)
          {
            ErrRename(mo->name, temp);
            {
              BusyFileClose(mo->name);
              return;
            }
          }
          
          /* Now rename the packet in our internal OUT.SQ list */
          
          HoleRename(mo->name, temp);
        }
      }

      BusyFileClose(mo->name);
    }
    while (MatchOutNext(mo));

    MatchOutClose(mo);
  }
}




static void near RV_Leave(byte *line,byte *ag[],NETADDR nn[],word num)
{
  NW(ag); NW(line);
  
  if (config.flag & FLAG_FRODO)
  {
    NotForFrodo("Leave");
    return;
  }

  Change_Style(nn, num, (byte)'U', (byte)'L');
}

static void near RV_Unleave(byte *line,byte *ag[],NETADDR nn[],word num)
{
  NW(ag); NW(line);

  if (config.flag & FLAG_FRODO)
  {
    NotForFrodo("Unleave");
    return;
  }

  Change_Style(nn, num, (byte)'L', (byte)'U');
}


static void near RV_Change(byte *line,byte *ag[],NETADDR nn[],word num)
{
  NW(line);
  
  if (config.flag & FLAG_FRODO)
  {
    NotForFrodo("Change");
    return;
  }

  if (! *ag[2])
  {
    S_LogMsg("!Line %d of route cfg: need 2 flavours and at least 1 node",
             linenum);
    return;
  }


  Change_Style(nn, num,
               Get_Routing_Flavour(ag, 1, TRUE),
               Get_Routing_Flavour(ag, 2, TRUE));
}



/* Create a dummy, 60-byte packet */

static void near Generate_Dummy_Pkt(int fd, struct _sblist *us,NETADDR *them)
{
  struct _pkthdr ph;
  word zero=0;

  Fill_Out_Pkthdr(&ph, us, them->zone, them->net, them->node, them->point);

  (void)fastwrite(fd, (char *)&ph, sizeof(struct _pkthdr));
  (void)fastwrite(fd, (char *)&zero, sizeof(word));
}


static void near RV_Poll(byte *line,byte *ag[],NETADDR nn[],word num)
{
  word express;
  word node;
  int fd;

  byte flavour;


  NW(line);

  if (config.flag & FLAG_FRODO)
  {
    NotForFrodo("Poll");
    return;
  }

  if (!num)
  {
    S_LogMsg("!Line %d of route file: Must specify at least one node.\n",
             linenum);
    return;
  }

  flavour=Get_Routing_Flavour(ag,1,TRUE);

  for (node=0; node < num; node++)
  {
    if (nn[node].zone==ZONE_ALL || nn[node].net==NET_ALL ||
        nn[node].node==NODE_ALL || nn[node].point==POINT_ALL)
    {
      S_LogMsg("!Line %d of route file: can't use 'All' or 'World' with POLL cmd",linenum);
      continue;
    }

    if (eqstri(ag[1],"express"))
    {
      OutName(scratch, &nn[node], (byte)flavour);
      express=TRUE;
    }
    else
    {
      FloName(scratch, &nn[node], (byte)flavour, FALSE);
      express=FALSE;
    }

    (void)printf("POLL: %hu:%hu/%hu.%hu (%s)\n",
                 nn[node].zone, nn[node].net, nn[node].node, nn[node].point,
                 LifeSavers(flavour));

    /* Only create if it doesn't already exist */

    if (fexist(scratch))
    {
      #ifdef DEBUG_ROUTE
      S_LogMsg("@%s exists: no PollGen",scratch);
      #endif
    }
    else
    {
      if ((fd=sopen(scratch, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                    SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
      {
        S_LogMsg("!Err creating `%s'",scratch);
        continue;
      }

      if (express)
        Generate_Dummy_Pkt(fd, config.addr, &nn[node]);

      (void)close(fd);
    }
  }
}



/* Change the .* extension on an ARCmail packet into either the             *
 * current one (which is non-zero length), or the one numbered              *
 * AFTER the last zero-length file.                                         */


static void near Unique_ArcName(byte *base,byte *arcname)
{
  char temp[PATHLEN];
  char tmpname[PATHLEN];
  static byte pspspc[]="%s%s%c";
  byte *ext;

  FFIND *ff;

  struct tm *lt;
  SCOMBO fdate, today;
  time_t longt;
  unsigned long fsiz;
  int high, add, i;

#if 0
  int arcm_num = 10;
  char arcm_letters[]="0123456789";
#else
  int arcm_num = 36;
  char arcm_letters[]="0123456789abcdefghijklmnopqrstuvwxyz";
#endif



  /* Get today's date and store it */

  (void)Get_Dos_Date(&today);
  longt=time(NULL);
  lt=localtime(&longt);

  if (lt->tm_wday >= 0 && lt->tm_wday <= 6 &&
      (config.flag & FLAG_OLDARCM)==0)
  {
    ext=arcm_exts[lt->tm_wday];
  }
  else ext=arcm_exts[1]; /* Use *.MO? by default */


  /* First, scan for all existing ARCmail files with today's extension... */

  for (add=0, high=-1; add < arcm_num; add++)
  {
    /* Set the extension to '.MOx', where 'x' is from 0-9. */

    (void)sprintf(temp, pspspc, base, ext, arcm_letters[add]);

    /* If it doesn't exist, use a size of -1 */

    if ((ff=FindInfo(temp))==NULL)
    {
      fsiz=(dword)-1;      /* Use this as a flag */
      fdate.ldate=0L;
    }
    else
    {
      /* Otherwise grab info from the filefind bug */

      fsiz=ff->ulSize;
      fdate=ff->scWdate;
      FindClose(ff);
    }

    /* If the file exists, and it's higher than the last one we saw,        *
     * increment the 'high' pointer appropriately.                          */
       
    if (fsiz != (dword)-1 && add > high)
      high=add;
    
    /* If it exists, and if it's a non-zero file (has some mail in it),     *
     * AND if it's today's date, we can (probably) use it.                  */
    
    if (fsiz > 0L && fdate.dos_st.date==today.dos_st.date)
    {
      /* This is the one we're looking for, so exit if it falls within      *
       * the MaxArchive range.                                              */

      if (config.max_archive==0 || fsiz/1000 <= config.max_archive)
      {
        (void)strcpy(arcname, temp);
        return;
      }
    }
  }


  /* We didn't find an existing one, so try to create a new one, which is   *
   * one above the highest extension.                                       */
  
  if (high != -1 && high < arcm_num-1)
  {
    (void)sprintf(arcname, pspspc, base, ext, arcm_letters[high+1]);
    return;
  }


  /* OK, time to call the shots.  We couldn't find any empty spaces, and    *
   * the highest one was '9', so it's time to unlink some zero-length files.*/

  (void)sprintf(temp, "%s%s%c", base, ext, '?');

  if ((ff=FindOpen(temp, 0)) != NULL)
  {
    do
    {
      (void)sprintf(tmpname, "%s%s%c",
                    base,
                    ext,
                    ff->szName[strlen(ff->szName)-1]);

      if (fsize(tmpname)==0L)
        (void)unlink(tmpname);
    }
    while (FindNext(ff)==0);

    FindClose(ff);
  }

  /* Find a file that does not exist.  If they all exist, fall through and  *
   * add to the last one, *.xx9.                                            */

  for (i=0; i < arcm_num; i++)
  {
    (void)sprintf(arcname, pspspc, base, ext, arcm_letters[i]);

    if (!fexist(arcname))
      break;
  }
}

  
unsigned long get_unique_number(void)
{
  static time_t last_time=0L;
  static unsigned ctr=0;
  time_t now;

  now=time(NULL);

  /* Make sure that we don't spit out two packets with the same number... */

  if (now != last_time)
    ctr=0;
  else
  {
    /* If we've produced more than 15 msgs in this second, wait until       *
     * the next second.                                                     */

    if (ctr++==15)
    {
      while (last_time==time(NULL))
        ;

      now=time(NULL);
      ctr=0;
    }
  }

  last_time=now;
  now = (now << 4) + (time_t)ctr;
  return now;
}


/* Renames a packet from its XXXXyyyy.OUT format to a serialized            *
 * packet name.                                                             */

static sword near Unique_Pktname(byte *outname,byte *pktname,NETADDR *found)
{
  NW(found);

  (void)sprintf(pktname,"%s%08lx.pkt",
                FixOutboundName(0xffff /*found->zone*/),
                get_unique_number());

  if (rename(outname, pktname)==-1)
  {
    ErrRename(outname, pktname);
    return -1;
  }
  
  return 0;
}


/*
static void near CantFileRouteNormal(void)
{
  S_LogMsg("!Err line %d: `File' can't be used with NORMAL flavour", linenum);
}
*/



static void near MustSpecOne(void)
{
  S_LogMsg("!Err line %d of route file:  Must specify at least 1 node", linenum);
}






/* Rewrite a packet header based on the true destination address */

static void near StompPacketHeader(char *fname, PNETADDR pna, int check_point)
{
  struct _pkthdr ph, phold;
  int fd;

  /* Don't stomp over the packet header if we're told not to do so */

  if (config.flag2 & FLAG2_NOSTOMP)
    return;

  /* Open packet file */

  if ((fd=sopen(fname, O_RDWR | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    return;

  /* Read in the old packet header */

  if (read(fd, (char *)&phold, sizeof phold)==(int)sizeof(phold))
  {
    /* Create a new packet header */

    Fill_Out_Pkthdr(&ph, config.addr, pna->zone, pna->net,
                    pna->node, pna->point);

    /* Now copy the origination address info from the original packet */

    ph.orig_zone=ph.qm_orig_zone=phold.orig_zone;
/*    ph.dest_zone=ph.qm_dest_zone=phold.dest_zone;*/
    ph.orig_net=phold.orig_net;
    ph.orig_node=phold.orig_node;
    ph.orig_point=phold.orig_point;

    /* Make sure that the dest/orig fields match */

    if (*ph.password ||
        (phold.dest_zone==ph.dest_zone &&
        (!check_point || phold.dest_point==ph.dest_point) &&
        phold.dest_zone==ph.dest_zone &&
        *phold.password=='\0'))
    {
      /* Stomp over with the new header and close file */

      (void)lseek(fd, 0L, SEEK_SET);

      if (write(fd, (char *)&ph, sizeof ph) != (int)sizeof ph)
        S_LogMsg("!Error stomping packet header %s", fname);
    }
  }

  (void)close(fd);
}




static void near RV_Send(byte *line,byte *ag[],NETADDR nn[],word num)
{
  MATCHOUT *mo;

  char temp[PATHLEN];
  char arcname[PATHLEN];

  word noarc;
/*  word fil;*/
  word nod, an;

  byte flavour;
  
  NW(line);

  noarc=/*fil=*/FALSE;

  for (an=2; ag[an]; an++)
    if (eqstri(ag[an], "file"))
      ; /*fil=TRUE;*/
    else if (eqstri(ag[an], "noarc"))
      noarc=TRUE;
    else break;

  flavour=Get_Routing_Flavour(ag,1,TRUE);

  if (! *ag[an])
  {
    MustSpecOne();
    return;
  }

  for (nod=0; nod < num; nod++)
  {
    NETADDR nop;

    mo=MatchOutOpen(&nn[nod], MATCH_OUT | MATCH_FLO, 'F');

    if (!mo)
      continue;
    
    do
    {
      nop=mo->found;
      nop.point=0;

      if (mo->found.point != 0 &&
          nn[nod].point==POINT_ALL &&
          !DestIsHereA(&nop) &&
          (config.flag2 & FLAG2_BINKPT))
      {
        /* Unless we're performing explicit routing, send all mail for      *
         * other systems' points through their boss.                        */

        mo->found.point=0;

        /* Now twiddle the packet header to fix the pwd, if necessary */

        if (mo->got_type & MATCH_OUT)
          StompPacketHeader(mo->name, &mo->found, FALSE);
      }


      /* Create a busy file for this node */

      if (BusyFileOpenNN(&mo->found, FALSE)==-1)
      {
        /* If we can't move it to the outbound area, pretend that it        *
         * doesn't exist, and leave it for processing on the next run.      */

        S_LogMsg(bsy_pkt_queued, mo->name);
        HoleRemoveFromList(mo->name);
        continue;
      }

      /* Route file attaches, unless they're already normal flavour */

      if (mo->got_type & MATCH_FLO)
      {
        if (flavour != 'F' && flavour != 'O' &&
            (config.flag & FLAG_FRODO)==0)
        {
          FloName(temp, &mo->found, flavour,
                  config.flag & FLAG_ADDMODE);

          (void)Add_To_FloFile(NULL, mo->name, temp);
        }
      }
      else if (noarc)
      {
          /* Nothing to archive, so just add to a packet */

          MakeOutboundName(&mo->found, temp);
	  if(flavour == 'F' || flavour == 'O')
	    (void)sprintf(temp+strlen(temp), "%cut", (int) 'o');
	  else
            (void)sprintf(temp+strlen(temp), "%cut", (int) tolower(flavour));

          if (! eqstri(mo->name, temp))
          {
            (void)Merge_Pkts(mo->name, temp);
            HoleRemoveFromList(mo->name);
          }
          
      }
      else
      {
        /* The name of the compressed bundle we're putting packet in... */

        MakeArcBase(temp, &mo->found);

        /* Now, find the right extension to use */

        Unique_ArcName(temp, arcname);


        /* Convert the *.OUT filename into a packet name */

        if (Unique_Pktname(mo->name, temp, &mo->found) != -1)
        {
          HoleRemoveFromList(mo->name);

          if (Add_To_Archive(arcname, &mo->found, temp, NULL))
          {
            /* Find the name of the FLO file to put this into */

            if ((config.flag & FLAG_FRODO)==0)
              FloName(temp, &mo->found, flavour,
                      (config.flag & FLAG_ADDMODE));


            /* Insert the # truncation character at the beginning of the line */

            (void)strocpy(arcname+1, arcname);
            *arcname='#';


            /* And add it, if necessary */

            if (config.flag & FLAG_FRODO)
              (void)Hole_Add_To_Net(&mo->found, arcname, flavour);
            else (void)Add_To_FloFile(arcname, NULL, temp);
          }
        }
      }

      BusyFileCloseNN(&mo->found);
    }
    while (MatchOutNext(mo));

    MatchOutClose(mo);
  }
}



static void near RV_Route(byte *line,byte *ag[],NETADDR nn[],word num)
{
  MATCHOUT *mo;
  NETADDR dest;
  NETADDR host;

  char temp[PATHLEN];
  char arcname[PATHLEN];
  byte flavour;

  word hostroute, noarc, fil;
  word nod, an;
  
  NW(line);


  noarc=fil=FALSE;
  hostroute=eqstri(ag[0],"hostroute");


  /* Get some sort of routing flavour */
  
  flavour=Get_Routing_Flavour(ag, 1, TRUE);

  
  /* Now parse for "File" and "NoArc" */

  for (an=2; ag[an]; an++)
    if (eqstri(ag[an], "file"))
      fil=TRUE;
    else if (eqstri(ag[an], "noarc"))
      noarc=TRUE;
    else break;

  if (! *ag[an])
  {
    MustSpecOne();
    return;
  }
    
  
  /* nn[0] is parsed with TRUE for the "wildcard" flag, so that             *
   * something like "1:249/122" will be turned into "1:249/122.All".        *
   * However, we can't route to a wildcard, so adjust it back for           *
   * the route-to address.                                                  */

  host=nn[0];
  
  if (host.zone==ZONE_ALL)
    host.zone=config.def.zone;
  
  if (host.net==NET_ALL)
    host.net=config.def.net;
  
  if (host.node==NODE_ALL)
    host.node=config.def.node;
  
  if (host.point==POINT_ALL)
    host.point=0;
    

  for (nod=0; nod < num; nod++)
  {
    mo=MatchOutOpen(&nn[nod], MATCH_OUT | MATCH_FLO, 'F');

    /* Got nothing, so keep looping */
    
    if (!mo)
      continue;

    do
    {
      /* If we can't open the busy file for either the routed-to node or    *
       * the routed-from node, remove this file from the queue.             */

      if (BusyFileOpenNN(&host, FALSE)==-1)
      {
        S_LogMsg(bsy_pkt_queued, mo->name);
        HoleRemoveFromList(mo->name);
        continue;
      }
      

      /* Now check the routed-from node, if it's not the same as above */

      if (!AddrMatch(&host, &mo->found) &&
          BusyFileOpen(mo->name, FALSE)==-1)
      {
        /* Error - close the first busy file */

        BusyFileCloseNN(&host);
                      
        S_LogMsg(bsy_pkt_queued, mo->name);
        HoleRemoveFromList(mo->name);
        continue;
      }

      if (!hostroute)
        dest=host;
      else
      {
        dest.zone=mo->found.zone;
        dest.net=mo->found.net;
        dest.node=0;
        dest.point=0;
      }

      /* Route a file-attach */

      if (mo->got_type & MATCH_FLO)
      {
        /* Only route a file if it's destined to the 'host' of a route      *
         * statement, or if there's an explicit "File" modifier.            */

        if ((AddrMatch(&host, &mo->found) || fil) &&
            (config.flag & FLAG_FRODO)==0)
        {
          /* If the packet is to be routed FROM the host, TO the host,      *
           * and has a flavour of normal, then don't perform any routing.   *
           * (We can't route it to itself!)                                 */

          if (AddrMatch(&host, &mo->found) && (flavour=='F' || flavour=='O'))
            ;
          else
          {
            FloName(temp, &dest, flavour,
                    config.flag & FLAG_ADDMODE);

            (void)Add_To_FloFile(NULL, mo->name, temp);
          }
        }
      }
      else if (noarc)
      {
        /* Route without archiving - just copy the packet */

        MakeOutboundName(&dest, temp);

        (void)sprintf(temp+strlen(temp), "%cut", (int)(flavour=='F' ? 'o' : tolower(flavour)));

        /* Remap the packet header, if necessary */

        StompPacketHeader(mo->name, &host, FALSE);

        if (! eqstri(mo->name, temp))
        {
          (void)Merge_Pkts(mo->name, temp);
          HoleRemoveFromList(mo->name);
        }
      }
      else  /* ARC it */
      {
        /* The name of the compressed bundle we're putting packet in. */

        MakeArcBase(temp, &dest);

        /* Now, find the right extension to use */

        Unique_ArcName(temp, arcname);


        /* Remap the packet header, if necessary */

        StompPacketHeader(mo->name, &host, FALSE);


        /* Convert the *.OUT filename into a packet name */

        if (Unique_Pktname(mo->name, temp, &mo->found) != -1)
        {
          HoleRemoveFromList(mo->name);

          if (Add_To_Archive(arcname, &mo->found, temp, &dest))
          {
            /* Find the name of the FLO file to put this into */

            if ((config.flag & FLAG_FRODO)==0)
              FloName(temp, &dest, flavour,
                      (config.flag & FLAG_ADDMODE));


            /* Insert the # trunc character at the beginning of the line */

            (void)strocpy(arcname+1, arcname);
            *arcname='#';


            /* And add it, if necessary */

            if (config.flag & FLAG_FRODO)
              (void)Hole_Add_To_Net(&dest, arcname, flavour);
            else (void)Add_To_FloFile(arcname, NULL, temp);
          }
        }
      }


      /* Close the other busy file, if it isn't the same as the first. */

      if (! AddrMatch(&host, &mo->found))
        BusyFileClose(mo->name);


      /* Close the routed-from busy file */

      BusyFileCloseNN(&host);

    }
    while (MatchOutNext(mo));

    MatchOutClose(mo);
  }
}





#ifdef NEVER /* obsolete.  See V_GateRoute() in s_config.c */

/* GateRoute Crash 1:1/2 2:All */

static void near RV_GateRoute(byte *line,byte *ag[],NETADDR nn[],word num)
{
  MATCHOUT *mo;

  struct _pktprefix pp;
  struct _pkthdr ph;

  char intlstr[PATHLEN];
  char outfn[PATHLEN];
  byte *orig, *buf, *p;

  int infile=-1, outfile=-1;
  int first, got, intllen, num_str, flavour, x;

  #define RDBUFSIZE 2048
  
  NW(line);

  if (! *ag[1])
  {
    S_LogMsg("!Line %d of route file: must specify at least 1 node",linenum);
    return;
  }

  flavour=Get_Routing_Flavour(ag,1,TRUE);

  sprintf(outfn,asdfasdfasdf /*old obout */ ,
          FixOutboundName(nn[0].zone),
          nn[0].net,
          nn[0].node,
          flavour=='F' ? 'O' : flavour);

  BusyFileOpen(outfn, TRUE);

  for (x=0; x < num; x++)
  {
    /* Use only MATCH_OUT - files are NEVER gaterouted. */

    mo=MatchOutOpen(&nn[x], MATCH_OUT, 'F');

    while (mo)
    {
      /* Find the name of the new packet to create... */


      BusyFileOpen(mo->name, TRUE);

      if ((infile=open(mo->name,O_RDONLY | O_BINARY))==-1)
        continue;
      
      if ((outfile=open(outfn,O_CREAT | O_RDWR | O_BINARY,
                        S_IREAD | S_IWRITE))==-1)
      {
        close(infile);
        continue;
      }

      lseek(outfile,0L,SEEK_END);

      /* Now, if it's not empty, skip over the packet end.  If it IS      *
       * empty, then generate a new packet header.                        */

      if (tell(outfile) != 0L)
        lseek(outfile,-2L,SEEK_END);
      else
      {
        Fill_Out_Pkthdr(&ph,
                        config.addr,
                        nn[0].zone,nn[0].net,nn[0].node,nn[0].point);

        fastwrite(outfile,(char *)&ph,sizeof(struct _pkthdr));
      }

      /* Skip over the packet header in the new file */

      if (fastread(infile,(char *)&ph,sizeof(struct _pkthdr)) !=
                   sizeof(struct _pkthdr))
      {
        close(outfile);
        close(infile);
        continue;
      }

      while (fastread(infile,(char *)&pp,sizeof(struct _pktprefix))==
                 sizeof(struct _pktprefix))
      {
        (void)sprintf(intlstr,"\x01INTL %hu:%hd/%hd %hu:%hd/%hd",
                      mo->found.zone,mo->found.net,mo->found.node,
                      config.us[0]->zone,
                      config.us[0]->net,
                      config.us[0]->node);

        intllen=strlen(intlstr);

        pp.dest_net=nn[0].net;
        pp.dest_node=nn[0].node;


        /* Dump out the updated packet header */

        fastwrite(outfile,(char *)&pp,sizeof(struct _pktprefix));

        num_str=0;

        if ((orig=malloc(RDBUFSIZE+intllen+intllen))==NULL)
          break;

        first=TRUE;
        
        while ((got=fastread(infile,orig,RDBUFSIZE)) > 0 && num_str <= 4)
        {
          buf=orig;
          
          if (num_str==4 && first)
          {
            if (!memstr(buf,intlstr,got,intllen))
            {
              (void)memmove(buf+intllen+1, buf, got);
              (void)memmove(buf, intlstr, intllen);
              buf[intllen]='\r';

              buf += intllen+1;
            }

            first=FALSE;
          }

          if ((p=memchr(buf,'\0',got)) != NULL)
          {
            num_str++;
            p++;
          }
          else p=buf+got;

          fastwrite(outfile,orig,(int)(p-orig));

          /* Seek back to the prior packet header */
          lseek(infile,-(got-(p-buf)),SEEK_CUR);
        }

        free(orig);
      }
      
      /* And terminate the output file */

      x=0;
      fastwrite(outfile,(char *)&x,2);
      close(outfile);
      
      close(infile);

      /* Mention what we just did... */

      (void)printf("GateRoute %s -> ",Address(&mo->found));
      (void)printf("%s\n",Address(&nn[0]));

      /* And delete the input file */
      
      unlink(mo->name);

      BusyFileClose(mo->name);
      
      /* Get next match... */
      if (! MatchOutNext(mo))
        break;
    }

    MatchOutClose(mo);
  }

  BusyFileClose(outfn);
}

#endif /* NEVER */





/* Advance the file pointer to the next "Sched" token */

static void near Skip_To_Next_Sched(FILE *in)
{
  char temp[MAX_ROUTE_LINELEN];
  char wrd[PATHLEN];
  long lastpos;
  
  lastpos=ftell(in);
  
  while (fgets(temp, MAX_ROUTE_LINELEN, in))
  {
    (void)getword(temp, wrd, routedelim, 1);
    
    if (eqstri(wrd, "sched"))
    {
      (void)fseek(in, lastpos, SEEK_SET);
      return;
    }
    
    lastpos=ftell(in);
  }
}


static void near Exec_Sched(byte *name)
{
  (void)printf("\nExecuting packer schedule %s...\n\n", name);
}
    
static void near Parse_Sched(FILE *in,byte *args[],byte *tag)
{
  char temp[PATHLEN];
  byte *s;

  time_t tim;
  struct tm *lt;

  unsigned btime, etime, ctime;
  int start_hour;
  int start_min;
  int end_hour;
  int end_min;
  int doit;

  tim=time(NULL);
  lt=localtime(&tim);



  /* Sched MySched All 00:14 22:05 */
  /* 0     1       2   3     4     */



  /* If the user specified a tag, and this is equal to it, then do       *
   * this with no further questions...                                   */

  if (tag && *tag)
  {
    if (eqstri(args[1],tag))
      Exec_Sched(args[1]);
    else
    {
      #ifdef DEBUG_ROUTE
      (void)printf("@Skipping schedule %s due to cmd-line override.\n", args[1]);
      #endif

      Skip_To_Next_Sched(in);
    }

    return;
  }

  for (s=strtok(args[2],"|");s;s=strtok(NULL,"|"))
  {
    /* If it's the right day, then continue */
    if (eqstri(s,weekday_ab[lt->tm_wday]) || eqstri(s,"all"))
      break;
    else if ((eqstri(s,"week") || eqstri(s,"wkday")) && lt->tm_wday > 0 && lt->tm_wday < 6)
      break;
    else if ((eqstri(s,"wkend") || eqstri(s,"weekend")) &&
             (lt->tm_wday==0 || lt->tm_wday==6))
      break;
  }

  if (!s)  /* If we didn't exit through 'break', then there was no match */
  {
    #ifdef DEBUG_ROUTE
    (void)printf("@Skipping schedule %s due to day-of-week.\n", args[1]);
    #endif

    Skip_To_Next_Sched(in);
    return;
  }

  /* Now make sure that we're in the right time slot */

  (void)getword(args[3], temp, ":", 1);
  start_hour=atoi(temp);

  (void)getword(args[3], temp, ":", 2);
  start_min=atoi(temp);

  (void)getword(args[4], temp, ":", 1);
  end_hour=atoi(temp);

  (void)getword(args[4], temp, ":", 2);
  end_min=atoi(temp);

  /* Automatically do it if there's no time given */

  if (*args[3]=='\0' || *args[4]=='\0')
    doit=TRUE;
  else doit=FALSE;

  /* Convert times to something that we can easily handle */

  ctime=lt->tm_hour * 100 + lt->tm_min;
  btime=start_hour * 100 + start_min;
  etime=end_hour * 100 + end_min;

  /* If ending time is less than starting time, assume it wraps around      *
   * midnight.                                                              */

  if (etime < btime)
  {
    if (ctime < etime || ctime >= btime)
      doit=TRUE;
  }
  else
  {
    if (ctime >= btime && ctime < etime)
      doit=TRUE;
  }

  if (doit)
    Exec_Sched(args[1]);
  else
  {
    #ifdef DEBUG_ROUTE
    (void)printf("@Skipping schedule %s due to time.\n", args[1]);
    #endif

    Skip_To_Next_Sched(in);
  }
}

static word near TokDelim(byte ch)
{
  return (ch==' ' || ch=='\t' || ch==',' || ch=='\\' || ch=='/' ||
          ch==':' || ch=='\0' || ch=='.' || ch=='\r' || ch=='\n');
}

static void near ProcessDefToken(byte *str, struct _defn *def)
{
  byte *s;
  unsigned toklen=strlen(def->name);
  unsigned replen=strlen(def->xlat);
  

  while ((s=stristr(str, def->name)) != NULL)
  {
    /* If it's surrounded by sufficient whitespace */
    
    if ((s==str || TokDelim(s[-1])) && TokDelim(s[toklen]))
    {
      /* Shift the string to make room */

      if (strlen(s)+replen-toklen >= MAX_ROUTE_LINELEN-5)
      {
        S_LogMsg("!Define macro expansion is too long");
        break;
      }

      (void)memmove(s+replen, s+toklen, strlen(s+toklen)+1);
      (void)memmove(s, def->xlat, strlen(def->xlat));
    }
    else /* not found */
    {
      /* continue search AFTER this */
      str=s+1;
    }
  }
}


static void near ProcessDefines(byte *s)
{
  struct _defn *def;
  
  for (def=defns; def; def=def->next)
    ProcessDefToken(s, def);
}






void Munge_Outbound_Area(byte *cfgname,byte *tag)
{
  static struct _verbtable
  {
    /* Name of this command */
    
    byte *verb;
    
    
    /* Name of the handler to call */
    
    void (near *vp)(byte *line,
                    byte *ag[],NETADDR n[],word num);
                  
    /* Word at which to start parsing for net addresses */

    unsigned np_at;
    

    /* Word at which to start allowing 1:123/All or similar notation */
    
    sword slush_at;

  } cv[]=
    {
      {"send",            RV_Send,              2, 3},
      {"route",           RV_Route,             2, 3},
      {"leave",           RV_Leave,             2, 2},
      {"unleave",         RV_Unleave,           2, 2},
      {"change",          RV_Change,            4, 4},
      {"poll",            RV_Poll,              2, -1},
/**/  {"attach",          RV_Attach,            3, -1},
/**/  {"get",             RV_Get,               3, -1},
/*    {"gateroute",       RV_GateRoute,         2}, */
      {"dos",             RV_Dos,    (unsigned)-1, -1},
/**/  {"update",          RV_Update,            3, -1},
      {"hostroute",       RV_Route,             2, 3},
      {"define",          RV_Define, (unsigned)-1, -1}
    };

  static byte *args[MAX_ROUTE_ARGS];

  static NETADDR n[MAX_ROUTE_NODES];
  NETADDR d;

  FILE *cfgfile;

  byte in[MAX_ROUTE_LINELEN];
  byte temp[MAX_ROUTE_LINELEN];
  byte *p;

  word num;
  unsigned i, arg, v;

  defns=NULL;
  
  if (config.flag & FLAG_FRODO)
    Hole_Read_Netmail_Area();

  if (config.flag2 & FLAG2_NUKE)
    Hole_Nuke_Bundles();

  (void)printf("\nScanning outbound areas...\n\n");

  Check_Outbound_Areas();

#if 0
  if (convert)
  {
    switch(convert)
    {
      case CVT_FLO:   FloToArc();   break;
      case CVT_ARC:   ArcToFlo();   break;
      case CVT_KILL:  KillArc();    break;
      case CVT_SFLO:  OutToSflo();  break;
    }

    return;
  }
#endif

  if ((cfgfile=shfopen(cfgname, "rb", O_RDONLY | O_BINARY | O_NOINHERIT))==NULL)
    ErrOpening("config", cfgname);

  for (i=0; i < MAX_ROUTE_ARGS; i++)
    if ((args[i]=malloc(MAX_ROUTE_LEN))==NULL)
      NoMem();
  
  for (linenum=1; fgets(in,MAX_ROUTE_LINELEN,cfgfile); linenum++)
  {
    if (*in=='\x1a')
      break;

    (void)Strip_Trailing(in, '\r');
    (void)Strip_Trailing(in, '\n');
    (void)Strip_Trailing(in, '\r');
    (void)Strip_Trailing(in, '\x1a');
    
    if ((p=strchr(in,';')) != NULL || (p=strchr(in, '%')) != NULL)
      *p='\0';

    ProcessDefines(in);

    for (arg=0; arg < MAX_ROUTE_ARGS; arg++)
    {
      (void)getword(in, args[arg], routedelim, (int)(arg+1u));

      if (! *args[arg])
        break;
    }

    if (! *args[0] || *args[0]=='\x1a')
      continue;
    
    for (v=0; v < cvlen; v++)
      if (eqstri(args[0], cv[v].verb))
      {
        /* If we have to parse the node entries into a list... */

        num=0;

        if (cv[v].np_at != (unsigned)-1)
        {
          /* Set the defaults to point to us */

          d.zone=config.addr->zone;
          d.net=config.addr->net;
          d.node=config.addr->node;
          d.point=0;

          for (i=cv[v].np_at; i < MAX_ROUTE_ARGS; i++)
          {
            /* Grab this node */

            (void)getword(in, temp, routedelim, (int)i);

            /* If it's a node address */

            if (! *temp)
              break;
            else if (isdigit(*temp) || *temp=='.' ||
                     strnicmp(temp, "world", 5)==0 ||
                     strnicmp(temp, "all", 3)==0)
            {
              /* Get address based on defaults */

              d.point=0;
              ParseNN(temp, &d.zone, &d.net, &d.node, &d.point,
                      ((sword)i >= cv[v].slush_at && cv[v].slush_at != -1)
                      ? 1 : 0 /* the things i do for lint :-( */);

              /* Copy to the node array, and increment */
              n[num++]=d;
            }
          }
        }

        (*cv[v].vp)(in, args, n, num);
        break;
      }
      
    if (eqstri(args[0],"sched"))
      Parse_Sched(cfgfile, args, tag);
    else if (v==cvlen)
       S_LogMsg("!Bad cmd in route file (line %d): `%s'", linenum, args[0]);
  }

  
  /* Now free the define tokens */
  
  {
    struct _defn *def, *next;
    
    for (def=defns; def; next=def->next, free(def), def=next)
    {
      free(def->xlat);
      free(def->name);
    }
  }
  
  for (i=0; i < MAX_ROUTE_ARGS; i++)
    free(args[i]);

  (void)fclose(cfgfile);

  if (config.flag & FLAG_FRODO)
    Hole_Free_Netmail_Area();

  (void)printf("\n");
}

/* Figure out how many outbound areas there are, and their numbers */

static void near Check_Outbound_Areas(void)
{
  struct _outb *pob;
  FFIND *ff;
  byte *p;
  
  config.ob[0]=config.addr->zone;
  config.num_ob=1;

  /* Scan all outbound directories into a list of zones */

  for (pob=config.outb; pob; pob=pob->next)
  {
    (void)sprintf(scratch, "%s.*", pob->dir);


    ff=FindOpen(scratch, ATTR_SUBDIR | ATTR_HIDDEN |
                         ATTR_READONLY | ATTR_SYSTEM);

    if (ff)
    {
      do
      {
        p=strchr(ff->szName, '.');

        if (p && sscanf(p+1, "%hx", &config.ob[config.num_ob])==1)
          config.num_ob++;
      }
      while (FindNext(ff)==0);
    }

    FindClose(ff);
  }
}







/*

* Send  [NoArc] [C/H/D/N]  <nodes>    *.OUT -> *.?LO
* Route [NoArc] [C/H/D/N]  <nodes>    *.OUT -> *.?LO
* Leave <nodes>                       *.?UT -> *.N?T
                                      *.?LO -> *.N?O
* Unleave <nodes>                     *.N?T -> *.?UT
                                      *.N?O -> *.?LO
* Change <C/H/D/N> <C/H/D/N> <nodes>  *.?UT -> *.?UT
                                      *.?LO -> *.?LO
* Poll <verb> <nodes>                 *.?LO
* Poll Express <nodes>                *.CUT
  Attach <verb> file1+file2 <nodes>   *.?LO
  Get <verb> file1+file2 <nodes>      *.REQ and *.?LO
* GateRoute <verb> <zonegate> <nodes> *.?UT -> *.?UT
* Dos <command> <arguments>
  Update <verb> <file1+file2> <nodes> *.REQ (update request)
* HostRoute                           *.OUT -> *.OUT
* Define <constant> <expression>

*/


static void near MakeArcBase(char *where, NETADDR *dest)
{
  char *fake;

  fake=FixOutboundName(dest->zone);

  /* Handle FroDo points just in the standard outbound area */

  if (dest->point && (config.flag & FLAG_FRODO))
  {
    (void)sprintf(where, "%sP%04hx%03hx",
                  fake,
                  (unsigned)dest->node,
                  (unsigned)dest->point);
  }
  else if (dest->point && (config.flag2 & FLAG2_BINKPT))
  {
    /* Handle stupid binkleyterm points here */

    (void)sprintf(where, "%s%04hx%04hx.pnt",
                  fake,
                  (unsigned)dest->net,
                  (unsigned)dest->node);

    if (!direxist(where))
      (void)make_dir(where);

    (void)sprintf(where+strlen(where), PATH_DELIMS "%08hx",
                  config.addr->point-dest->point);
  }
  else
  {
    /* Handle normal stuff here */

    (void)sprintf(where, "%s%04hx%04hx",
                  fake,
                  (unsigned)(word)(config.addr->net  - dest->net),
                  (unsigned)(word)(config.addr->node - dest->node));
  }
}



void MakeOutboundName(NETADDR *d, char *s)
{
  char *orig=s;
  
  (void)strcpy(s, FixOutboundName(d->zone));
  s += strlen(s);
  

  (void)sprintf(s, "%04hx%04hx", (unsigned)d->net, (unsigned)d->node);
  s += strlen(s);
  
  if (d->point && (config.flag & FLAG_FRODO)==0 && 
      (config.flag2 & FLAG2_BINKPT))
  {
    (void)strcat(s, ".pnt");
    s += strlen(s);

    if (!direxist(orig))
      (void)make_dir(orig);

    (void)sprintf(s, PATH_DELIMS "%08hx", d->point);
    s += strlen(s);
  }
  
  *s++='.';
  *s='\0';
}

