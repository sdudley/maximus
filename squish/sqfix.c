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
static char rcs_id[]="$Id: sqfix.c,v 1.1.1.1 2002/10/01 17:56:09 sdudley Exp $";
#pragma on(unreferenced)

#define NOVARS
#define NOVER
#define MSGAPI_HANDLERS

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include <errno.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "sqfix.h"
#include "sqver.h"

#define VERSION   SQVERSION
#define BSIZ      32767u

static int free_frames=0;
static int damaged_frames=0;
static int ok_frames=0;

static char *begin, *here;

static char *fixname="$SQFIXED";
static char *sqd_name="%s.sqd";
static char *sqi_name="%s.sqi";









static int near snuggle_up(char *bufr, ptrdiff_t need_len, int *got, int old_sqd)
{
  word len, shift;

  /* If this message is at the edge of our buffer, refill */

  if (here+(size_t)need_len > bufr+(size_t)BSIZ && *got==BSIZ)
  {
    len=*got-(word)(here-bufr);

    memmove(bufr, here, len);
    here=bufr;

    shift=read(old_sqd, bufr+len, BSIZ-len);

    if (shift <= 0)
      return FALSE;

    *got = len+shift;
  }

  return TRUE;
}











static void near ErrOpen(char *name)
{
  printf("Error opening `%s' for read!\n", name);
  exit(1);
}


static void near ErrRead(char *name)
{
  printf("Error reading `%s'!\n", name);
  exit(1);
}

static void near ErrWrite(char *name)
{
  printf("Error writing `%s' (%d)!\n", name, errno);
  exit(1);
}

static void near ErrSeek(char *name)
{
  printf("Error seeking `%s'!\n", name);
  exit(1);
}


void _fast NoMem(void)
{
  printf("Out of memory!\n");
  exit(1);
}

static int _stdc rlcomp(const void *i1, const void *i2)
{
  union stamp_combo s1, s2;

  s1=((RLNK *)i1)->date;
  s2=((RLNK *)i2)->date;

  return (s1.ldate==s2.ldate) ? 0 : GEdate(&s1, &s2) ? 1 : -1;
}

static void near usage(void)
{
  printf("Command-line format:\n\n");

  printf("SQFix <path/filename of damaged area>\n\n"

         "SQFix will automatically restore a damaged Squish area.  If the\n"
         "area can be fixed, the old data files will be renamed to\n"
         "<areaname>.XXD and <areaname>.XXI.\n");
  exit(1);
}


static void near link_base(char *origname)
{
  char name[PATHLEN];
  RLNK *rl, *rp, *re;
  struct _sqbase sqb;
  SQHDR hdr;
  SQIDX ix;
  XMSG msg;
  FOFS pos;
  long mn, max;
  int sqd, sqi;

  printf("Scanning...\n");

  /* Open the index and data files */
  
  sprintf(name, sqi_name, origname);

  if ((sqi=sopen(name, O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
                 SH_DENYRW, S_IREAD | S_IWRITE))==-1)
    ErrOpen(name);

  sprintf(name, sqd_name, origname);
  
  if ((sqd=sopen(name, O_RDWR | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE))==-1)
    ErrOpen(name);

  if (read(sqd, (char *)&sqb, sizeof(sqb)) != sizeof(sqb))
    ErrRead(name);

  if (sqb.num_msg==0)
  {
    close(sqd);
    return;
  }

  #if defined(__MSDOS__) || defined(OS_2)
    if (sqb.num_msg*(long)sizeof(RLNK) > 65000L)
      NoMem();
  #endif

  /* Allocate memory for the array of msg info */

  if ((rl=malloc((word)sqb.num_msg*sizeof(RLNK)))==NULL)
    NoMem();

  /* Now scan through the file to grab all of the message positions and     *
   * dates.                                                                 */

  for (pos=sqb.begin_frame, mn=0;
         mn < (long)sqb.num_msg &&
         lseek(sqd, pos, SEEK_SET)==pos && 
         read(sqd, (char *)&hdr, sizeof(hdr))==sizeof(hdr);
       pos=hdr.next_frame, mn++)
  {
    if (read(sqd, (char *)&msg, sizeof(XMSG)) != sizeof(XMSG))
      ErrRead(name);
    
    rl[(size_t)mn].date.msg_st=((union stamp_combo *)&msg.date_arrived)->ldate
                         ? msg.date_arrived : msg.date_written;
    rl[(size_t)mn].pos=pos;
    
    if ((mn % 5)==0)
      printf("%ld\r", mn);
  }

  max=mn;
  
  printf("          \rSorting...\n");

  /* Now sort the messages by date arrived */

  qsort(rl, (word)max, sizeof(RLNK), rlcomp);
  
  sqb.uid=1L;
  sqb.begin_frame=rl[0].pos;
  sqb.last_frame=rl[(size_t)mn-1].pos;
  
  printf("Threading headers...\n");

  for (rp=rl, re=rp+(size_t)mn, mn=0; rp < re; rp++)
  {
    if (lseek(sqd, rp->pos, SEEK_SET) != rp->pos)
      ErrSeek(name);
    
    if (read(sqd, (char *)&hdr, sizeof(hdr)) != sizeof(hdr))
      ErrRead(name);
    
    /* Relink the forward/back chain */

    hdr.prev_frame=(rp==rl ? NULL_FRAME : rp[-1].pos);
    hdr.next_frame=(rp==re-1 ? NULL_FRAME : rp[1].pos);
    
    /* Seek back to the SQHDR and rewrite it with the new link */

    if (lseek(sqd, rp->pos, SEEK_SET) != rp->pos)
      ErrSeek(name);
    
    if (write(sqd, (char *)&hdr, sizeof(hdr)) != sizeof(hdr))
      ErrWrite(name);

    
    /* Now read in the message header, so that we can use it to recreate    *
     * the index.                                                           */

    if (read(sqd, (char *)&msg, sizeof(msg)) != sizeof(msg))
      ErrRead(name);

    /* Fill in the index fields */

    ix.ofs=rp->pos;
    ix.umsgid=sqb.uid++;

    ix.hash=SquishHash(msg.to);

    if (msg.attr & MSGREAD)
      ix.hash |= 0x80000000Lu;

    /* And write the index out */

    if (write(sqi, (char *)&ix, sizeof(ix)) != sizeof(ix))
      ErrWrite(name);

    if ((++mn % 5)==0)
      printf("%ld\r", mn);
  }
  
  lseek(sqd, 0L, SEEK_SET);
  
  if (write(sqd, (char *)&sqb, sizeof(sqb)) != sizeof(sqb))
    ErrWrite(name);

  printf("\r         \r");

  free(rl);
  
  close(sqi);
  close(sqd);
}



static void near rename_base(char *origname, char *toname)
{
  char from[PATHLEN];
  char to[PATHLEN];
  
  sprintf(from, sqd_name, origname);
  sprintf(to,   "%s.xxd", origname);
  
  if (rename(from, to) != 0)
  {
    unlink(to);
    rename(from, to);
    unlink(from);
  }

  sprintf(from, sqi_name, origname);
  sprintf(to,   "%s.xxi", origname);
  
  if (rename(from, to) != 0)
  {
    unlink(to);
    rename(from, to);
    unlink(from);
  }
  

  /* Now copy the fixed ones in appropriately */

  sprintf(from, sqi_name, toname);
  sprintf(to,   sqi_name, origname);
  
  if (rename(from, to))
  {
    printf("Can't rename index file %s to %s\n", from, to);
    exit(1);
  }

  sprintf(from, sqd_name, toname);
  sprintf(to,   sqd_name, origname);
  
  if (rename(from, to))
  {
    printf("Can't rename data file %s to %s\n", from, to);
    exit(1);
  }
}



static void near rebuild_file(int old_sqd,MSG *new,char *bufr)
{
  MSGH *msgh;
  SQHDR hdr;
  XMSG msg;

  char *search="SD\xae\xaf";
  char *txt;

  long msgn=0L;
  int got;
  word lens;

  lens=strlen(search);
  
  printf("Rebuilding...\n");
  
  while ((got=read(old_sqd, bufr, BSIZ)) > 0)
  {
    /* Try and shift everything to the beginning of the buffer */
    
    begin=bufr;

    while ((here=memstr(begin, search, got-(word)(begin-bufr), lens)) != NULL)
    {
      /* Make sure that we haven't overflowed the buffer */

      if (! snuggle_up(bufr, (ptrdiff_t)sizeof(SQHDR), &got, old_sqd))
        break;


      /* If we got something, move it to the header and see if it's ok */
      
      memmove(&hdr, here, sizeof(SQHDR));

      /* If the frame header is damaged... */
      
      if (hdr.frame_type != FRAME_NORMAL || hdr.id != SQHDRID)
      {
        if (hdr.frame_type==FRAME_FREE)
          free_frames++;
        else damaged_frames++;

        /* Start searching from just after this frame */
        
        begin=here+1;
        continue;
      }

      here += sizeof(SQHDR);
      
      /* Make sure that we haven't overflowed the buffer */

      if (! snuggle_up(bufr, (ptrdiff_t)hdr.frame_length, &got, old_sqd))
        break;
      

      /* Copy out the message header */
      
      memmove(&msg, here, sizeof(XMSG));
      
      /* Now jump to the control text */
      
      here += sizeof(XMSG);

      /* Make sure that it's as long as it really says it is */
      
      if (hdr.clen)
        hdr.clen=min(hdr.clen, (unsigned long)strlen(here)+1L);

      txt=here+(word)hdr.clen;

      /* Do the same check on the message text */
      
      if (hdr.msg_length)
        hdr.msg_length=min(hdr.msg_length, (dword)strlen(txt)+1);

      /* Open a new message for creation in the new .SQD */
      
      if ((msgh=MsgOpenMsg(new,MOPEN_CREATE,0L))==NULL)
      {
        printf("Error opening message for write!  Aborting...\n");
        exit(1);
      }

      msg.replyto=0L;
      memset(msg.replies,'\0',sizeof(UMSGID)*MAX_REPLY);

      /* So that the 'fixed' msgs don't get scanned out to everyone         *
       * else!                                                              */

      msg.attr |= MSGSCANNED; 

      if (MsgWriteMsg(msgh,
                      FALSE,
                      &msg,
                      txt,
                      hdr.msg_length,
                      hdr.msg_length,
                      hdr.clen,
                      here)==-1)
      {
        printf("Error writing message!  (Out of disk space?)\n");
        exit(1);
      }

      MsgCloseMsg(msgh);

      if ((++msgn % 5)==0)
      {
        printf("%ld\r", msgn);
        fflush(stdout);
      }
      
      ok_frames++;

      begin=here+(size_t)hdr.msg_length;

      if ((word)(begin-bufr) > got)
        break;
    }

    /* Seek back the size of one header, to allow for overlap */

    if (got==BSIZ)
      lseek(old_sqd, -(signed long)sizeof(SQHDR)+1, SEEK_CUR);
  }

/* (void)tell(old_sqd);*/
}


/* Rewrite the lastread file based on the new message numbers */

static void near rewrite_lastreads(char *base_name, dword huge *pmsg, dword num_msg)
{
  char from[PATHLEN];
  char to[PATHLEN];
  dword size=num_msg * (long)sizeof(dword);
  char sql_name[PATHLEN];
  int fd;

  /* Rename the old lastread pointers */

  sprintf(from,   "%s.sql", base_name);
  sprintf(to,     "%s.xxl", base_name);
  unlink(to);
  rename(from, to);

  if (!pmsg)
    return;


  strcpy(sql_name, base_name);
  strcat(sql_name, ".sql");

  printf("Rewriting lastread pointers...\n");

  if ((fd=sopen(sql_name, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, SH_DENYRW,
               S_IREAD | S_IWRITE))==-1)
  {
    printf("Error opening %s for write!\n");
    return;
  }

  if (h_write(fd, (char huge *)pmsg, size) != size)
  {
    printf("Error writing to %s!\n", sql_name);
    return;
  }

  close(fd);
}


/* Search the index file for a particular UMSGID */

static dword near find_idx_uid(SQIDX huge *pi, long num_idx, UMSGID uid)
{
  SQIDX huge *pcur, huge *pend;

  if (uid)
    for (pcur=pi, pend=pi+num_idx; pcur < pend; pcur++)
      if (pcur->umsgid==uid)
        return pcur-pi+1;

  return 0L;
}


#if !defined(__FLAT__) && !defined(OS_2)

static int near bitch_size(char *where, long size)
{
  if (size >= 65500L)
  {
    printf("Warning!  %s file size is too large.  Lastread\n"
           "pointers will not be saved.  (Run SQFIX32 if you need support\n"
           "for larger files.)\n",
           where);
    return TRUE;
  }

  return FALSE;
}

#endif /* !__FLAT__ && !OS_2 */


static void near bitch_memory(char *where)
{
  printf("Error!  Not enough memory to read in %s file.  Lastread\n"
         "pointers will not be corrected.  (Try using SQFIX32 if you have\n"
         "a very large %s file.)\n",
         where, where);
}


/* Try to save all of the lastread pointers */

dword huge *save_lastreads(char *sqd_name, dword *pnum_lastread)
{
  SQIDX huge *pi;
  dword num_sqi;
  dword huge *plr, huge *curlr;
  UMSGID uid;
  long size;
  FILE *fp;
  int fd;
  char sqi_name[PATHLEN];
  char sql_name[PATHLEN];
  char *p;

  /* Get the .sqi filename */

  if ((p=strrchr(sqd_name, '.')) != NULL)
    *p=0;

  strcpy(sqi_name, sqd_name);
  strcat(sqi_name, ".sqi");

  printf("Reading lastread pointers...\n");

  /* Open the index file */

  if ((fd=sopen(sqi_name, O_RDONLY | O_BINARY, SH_DENYRW, S_IREAD | S_IWRITE))==-1)
    return NULL;

  size=lseek(fd, 0L, SEEK_END);

#if !defined(__FLAT__) && !defined(OS_2)
  if (bitch_size("Index", size))
  {
    close(fd);
    return NULL;
  }
#endif

  num_sqi=size / (long)sizeof(SQIDX);
  lseek(fd, 0L, SEEK_SET);

  /* Allocate enough memory for all of the records */

  if ((pi=h_malloc(size))==NULL)
  {
    bitch_memory("index");
    close(fd);
    return NULL;
  }

  if (h_read(fd, (char huge *)pi, size) != size)
  {
    h_free(pi);
    close(fd);
    return NULL;
  }

  close(fd);

  strcpy(sql_name, sqd_name);
  strcat(sql_name, ".sql");

  if ((fp=fopen(sql_name, "rb"))==NULL)
  {
    h_free(pi);
    return NULL;
  }

  fseek(fp, 0L, SEEK_END);
  size=ftell(fp);
  fseek(fp, 0L, SEEK_SET);

#if !defined(__FLAT__) && !defined(OS_2)
  if (bitch_size("Lastread", size))
  {
    close(fd);
    return NULL;
  }
#endif

  /* Allocate memory for array of new lastreads */

  if ((plr=h_malloc(size))==NULL)
  {
    bitch_memory("lastread");
    return NULL;
  }

  curlr=plr;

  /* Convert all of the lastread UMSGIDs to message numbers */

  while (fread((void *)&uid, sizeof(UMSGID), 1, fp)==1)
  {
    dword lr=find_idx_uid(pi, num_sqi, uid);
    /*printf("uid %ld = msgn %ld\n", uid, lr);*/
    *curlr++=lr;
  }

  close(fd);

  *pnum_lastread=size / (long)sizeof(dword);

  /* Return array of lastread message numbers */

  return plr;
}


static void near fix_base(char *name)
{
  dword num_lastread;
  dword huge *pLastRead;
  static char old_name[PATHLEN];
  char fixbase[PATHLEN];
  char *bufr, *p, *s;

  MSG *new;


  int old_sqd;

  /* thwack away any periods */

  p=strrchr(name, '.');
  s=strrstr(name, "/\\");

  if (p && (!s || p > s))
    *p='\0';

  sprintf(old_name, "%s.sqd", name);

  if ((old_sqd=sopen(old_name, O_RDONLY | O_BINARY, SH_DENYRW,
                     S_IREAD | S_IWRITE))==-1)
    ErrOpen(old_name);
  
  if ((bufr=malloc(BSIZ))==NULL)
  {
    printf("Not enough memory!  (%ld)  Aborting...\n", (long)coreleft());
    exit(1);
  }
  
  strcpy(fixbase, name);
  
  if ((p=strrstr(fixbase, "\\/:")) != NULL)
    p[1]='\0';
  else *fixbase='\0';
  
  strcat(fixbase, fixname);

  if ((new=MsgOpenArea(fixbase, MSGAREA_CREATE, MSGTYPE_SQUISH))==NULL)
  {
    printf("Error creating %s database!  Aborting...\n", fixbase);
    close(old_sqd);
    exit(1);
  }

  MsgLock(new);

  pLastRead=save_lastreads(old_name, &num_lastread);
  rebuild_file(old_sqd, new, bufr);
  rewrite_lastreads(old_name, pLastRead, num_lastread);

  MsgCloseArea(new);
  free(bufr);

  close(old_sqd);
  
  rename_base(name, fixbase);
  link_base(name);

  printf("\rDone rebuild!\n\n", name);

  printf("Free frame headers:    %d\n", free_frames);
  printf("Damaged frame headers: %d\n", damaged_frames);
  printf("Repaired frames:       %d\n", ok_frames);
}




int _stdc main(int argc,char *argv[])
{
  printf("\nSQFIX  Squish Message Area Repair and Reconstruction  Version " VERSION "\n");
  printf("Copyright 1991, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

  if (argc < 2)
    usage();

  fix_base(argv[1]);

  return 0;
}


