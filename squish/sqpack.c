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
static char rcs_id[]="$Id: sqpack.c,v 1.1 2002/10/01 17:56:11 sdudley Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "ffind.h"
#include "msgapi.h"
#include "api_sq.h"
#include "max.h"
#include "sqpack.h"
#include "sqver.h"
#include "areaapi.h"
#include "areadat.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#if defined(__FARDATA__) && !defined(__FLAT__)
  #if defined(__WATCOMC__) && !defined(OS_2)
    #define myfarmalloc(p)  halloc(p,1)
    #define myfarfree(p)    hfree(p)
  #else
    #define myfarmalloc(p)  h_malloc(p)
    #define myfarfree(p)    h_free(p)
  #endif
#else
  #define myfarmalloc(p)    malloc(p)
  #define myfarfree(p)      free(p)
#endif


static void near error(void)
{
  printf("\a  Err!  Run SQINFO!");
  return;
}

static void near ErrOpening(byte *name)
{
  printf("Error opening `%s'!\n", name);
}


static word near kill_msg(sword usDays, union stamp_combo *pscDate)
{
  struct tm *ptm;
  time_t t;
  long long_t;
  union stamp_combo scToday;
  
  t=time(NULL);

  long_t = (long)t - 60L*60L*24L*(long)usDays;

  /* Make sure that the date value is more than zero (and that it hasn't    *
   * wrapped around)!                                                       */

  t=(long_t < 0 || long_t > t) ? 0 : long_t;

  ptm=localtime(&t);
  
  TmDate_to_DosDate(ptm, &scToday);

  return (!GEdate(pscDate, &scToday));
}



int pack_base(struct _sqbase *isqb, int infd, int outfd, struct _sqbase *osqb, SQIDX huge *idx)
{
  SQHDR *frame;

  byte *sqbp;

  sdword b_left;

  FOFS new_frame, next_new_frame;
  FOFS lframeofs, loutfofs;

  long ofdpos;

  word mctr=0;
  ptrdiff_t b_get;
  int got, do_bs;
  byte fKill=FALSE;
  long n_msg=0;


  ofdpos=sizeof(struct _sqbase);

  
  /* Start reading from the first message frame */

  new_frame=isqb->begin_frame;
  do_bs=isatty(fileno(stdout));


  /* If there aren't any messages, then there's nothing to do. */
  
  if (new_frame==NULL_FRAME)
  {
    osqb->begin_frame=NULL_FRAME;
    printf(" - ");
    return 0;
  }

  lframeofs=loutfofs=NULL_FRAME;

  do
  {

    frame=(SQHDR *)sqbuf;
    
    /* Read in this frame from the original SQD file, and make sure that    *
     * it's valid.                                                          */

    if (lseek(infd, new_frame, SEEK_SET) != new_frame ||
        read(infd, (char *)frame, sizeof(SQHDR)) != sizeof(SQHDR) ||
        frame->id != SQHDRID ||
        frame->prev_frame != lframeofs ||
        new_frame >= isqb->end_frame ||
        new_frame==frame->next_frame)
    {
      error();
      break;
    }


    fKill=FALSE;
    
    /* If we have to kill by number of days, read the msg hdr and date */

    if (isqb->keep_days)
    {
      XMSG msg;

      if (read(infd, (char *)&msg, sizeof(XMSG)) != sizeof(XMSG))
      {
        error();
        break;
      }
      
      lseek(infd, -(long)sizeof(XMSG), SEEK_CUR);

      if (kill_msg((sword)isqb->keep_days, 
                   (union stamp_combo *)(msg.date_arrived.date.yr 
                     ? &msg.date_arrived
                     : &msg.date_written)))
      {
        fKill=TRUE;
      }
    }

    /* Static packing of the message base by number */

    if (isqb->max_msg && (sdword)mctr < (sdword)isqb->num_msg - (sdword)isqb->max_msg + (sdword)isqb->skip_msg)
      fKill=TRUE;

    if (fKill && (sdword)mctr >= (sdword)isqb->skip_msg)
    {
      lframeofs=new_frame;
      new_frame=frame->next_frame;

      mctr++;
        
      if ((mctr % 10)==0 && do_bs)
      {
        printf("\b\b\b\b\b%5u", mctr);
        fflush(stdout);
      }

#ifdef __FARDATA__
      h_memmove(idx+n_msg, idx+n_msg+1L,
                ((long)isqb->num_msg-(n_msg+1L))*(long)sizeof(SQIDX));
#else
      memmove(idx+n_msg, idx+n_msg+1L,
              ((word)isqb->num_msg-(n_msg+1))*sizeof(SQIDX));
#endif
      continue;
    }

    /* Now adjust the pointers in this frame for the offsets in the new     *
     * file.                                                                */

    next_new_frame=frame->next_frame; /* save a copy of old next_frame */
    frame->prev_frame=loutfofs;
    loutfofs=ofdpos;

    /* Make sure that the last message points to a NULL frame */

    if ((dword)(mctr+1)==isqb->num_msg)
      frame->next_frame=NULL_FRAME;
    else frame->next_frame=ofdpos+sizeof(SQHDR)+frame->msg_length;

    /* The new frame length will be just enough to hold the msg */

    frame->frame_length=frame->msg_length;

    idx[n_msg].ofs=ofdpos;



    /* Position the pointer to just after the frame, for further reading */
    
    sqbp=sqbuf+sizeof(SQHDR);

    b_left=frame->msg_length;
    
    /* While there are still bytes to read from this message... */
    
    while (b_left > 0)
    {
      b_get=(ptrdiff_t)(SQBUF_SIZE-(ptrdiff_t)(sqbp-sqbuf));
      b_get=(ptrdiff_t)min((long)b_get, b_left);

      if ((got=read(infd, sqbp, (unsigned)b_get)) <= 0)
      {
        error();
        break;
      }

      if (got != (int)b_get)
        b_left=0;
      
      /* Advance the pointer past all of the bytes that we read in */
      
      sqbp += (size_t)b_get /* got */;
      b_left -= (sdword)b_get /* got */;
      

      /* Now calculate how many bytes to write */
      
      b_get=sqbp-sqbuf;
        
      if (write(outfd, sqbuf, (unsigned)b_get) != (signed)b_get)
      {
        printf("\nError on write!  (Out of disk space?)\n");
        return TRUE;
      }
      

      /* Increment the file pointer for the output file */
      
      ofdpos += (long)b_get;
      

      /* If we hit the end of the buffer, wrap to the beginning again */
      
      if (b_get==SQBUF_SIZE)
        sqbp=sqbuf;
    }
    

    /* If there was an error in the while() loop */
    
    if (b_left > 0)
      break;
    
    lframeofs=new_frame;
    new_frame=next_new_frame;


    /* Make sure that we don't process too many messages. */
    
    if ((dword)++mctr > isqb->num_msg)
    {
      error();
      break;
    }

    n_msg++;

    if ((mctr % 10)==0 && do_bs)
    {
      printf("\b\b\b\b\b%5u", mctr);
      fflush(stdout);
    }
  }
  while (new_frame != NULL_FRAME);

  if (new_frame==NULL_FRAME)
    printf("%s%5u - ", do_bs ? "\b\b\b\b\b" : "", mctr);

  osqb->num_msg=osqb->high_msg=n_msg;
  
  osqb->last_frame=loutfofs;
  osqb->end_frame=ofdpos;
    
  /* If no messages left, set frame pointers accordingly */

  if (n_msg==0)
    osqb->begin_frame=osqb->last_frame=NULL_FRAME;
  
  /* Return TRUE on error */

  return (new_frame != NULL_FRAME);
}



/* Pack the Squish index file */

static int near pack_file(int sqd, int ifd, int newfd)
{
  SQIDX *idx=NULL;
  struct _sqbase isqb, osqb;
#ifdef __FARDATA__
  long bytes;
#else
  word bytes;
#endif
  int ret;

  /* Read the sqbase from the original file */
  
  lseek(sqd, 0L, SEEK_SET);
  
  if (read(sqd, (char *)&isqb, sizeof(struct _sqbase)) != 
                                       sizeof(struct _sqbase))
  {
    error();
    return 1;
  }

  /* Copy the struct over to the new file, but change a few things... */
  
  osqb=isqb;
  
  osqb.len=sizeof(struct _sqbase);
  osqb.begin_frame=sizeof(struct _sqbase);
  osqb.last_frame=NULL_FRAME;
  osqb.free_frame=NULL_FRAME;
  osqb.last_free_frame=NULL_FRAME;
  osqb.end_frame=sizeof(struct _sqbase);

  lseek(newfd, 0L, SEEK_SET);
  
  if (write(newfd, (char *)&osqb, sizeof(osqb)) != sizeof(osqb))
  {
    printf("\a  Err!  Can't write to disk!");
    return 1;
  }
  
  /* Allocate enough memory to read in the index */
  
#ifdef __FARDATA__
  bytes=isqb.num_msg * (long)sizeof(SQIDX);
#else
  bytes=(size_t)sizeof(SQIDX) * (size_t)isqb.num_msg;
#endif

  if (bytes)
  {
    if ((idx=myfarmalloc(bytes))==NULL)
      NoMem();

    /* Now read it in */

#ifdef __FARDATA__
    if (h_read(ifd, (char *)idx, bytes) != bytes)
#else
    if ((word)read(ifd, (char *)idx, bytes) != bytes)
#endif
    {
      printf("\a  Err!  Can't read index!");
      myfarfree(idx);
      return 1;
    }
  }


  /* Perform the actual packing of the messages */
  
  ret=pack_base(&isqb, sqd, newfd, &osqb, idx);
  
    
  /* Rewrite the index */
    
  lseek(ifd, 0L, SEEK_SET);

  if (bytes && ret==0)
  {
#ifdef __FARDATA__
    if (h_write(ifd, (char *)idx, bytes) != (long)bytes)
#else
    if ((word)write(ifd, (char *)idx, bytes) != bytes)
#endif
    {
      printf("\a  Err!  Can't write to disk!");
      free(idx);
      return 1;
    }

    /* Truncate the file at the end of the index */

    setfsize(ifd, (long)bytes);
    myfarfree(idx);
  }
    

  /* Now try to rewrite the sqbase header for the new output file */
    
  lseek(newfd, 0L, SEEK_SET);
    
  if (write(newfd, (char *)&osqb, sizeof(osqb)) != sizeof(osqb))
  {
    printf("\a  Err!  Can't write to disk!");
    return 1;
  }
  
  return ret;
}


static int near pack_squish_file(char *path)
{
  byte sqdname[PATHLEN];
  byte ifdname[PATHLEN];
  byte newname[PATHLEN];
  
  long oldsize, newsize;
  int sqd, ifd, newfd, ret;

  printf("Packing %-22s -      ", path);

  sprintf(sqdname, "%s.sqd", path);
  sprintf(ifdname, "%s.sqi", path);
  sprintf(newname, "%s.~~~", path);

  /* Try to open all three files */
  
  if ((sqd=sopen(sqdname, O_RDONLY | O_BINARY, SH_DENYRW,
                 S_IREAD | S_IWRITE))==-1)
  {
    ErrOpening(sqdname);
    return 1;
  }
  
  if ((ifd=sopen(ifdname, O_RDWR | O_BINARY, SH_DENYRW,
                 S_IREAD | S_IWRITE))==-1)
  {
    ErrOpening(ifdname);
    close(sqd);
    return 1;
  }

  lock(sqd, 0L, 1L);
  lock(ifd, 0L, 1L);

  if ((newfd=sopen(newname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                   SH_DENYRW, S_IREAD | S_IWRITE))==-1)
  {
    ErrOpening(newname);
    close(ifd);
    close(sqd);
    return 1;
  }

  /* Now pack from the .SQD file to the .~~~ file */
  
  ret=pack_file(sqd, ifd, newfd);

  unlock(ifd, 0L, 1L);
  unlock(sqd, 0L, 1L);

  /* Close everything up */
  
  close(newfd);
  close(ifd);
  close(sqd);
  
  if (ret != 0)
    unlink(newname);
  else
  {
    /* Success! */
    
    oldsize=fsize(sqdname);
    newsize=fsize(newname);

    if (oldsize==0)
      oldsize=1;

    printf("Old=%7ld; New=%7ld",
           oldsize, newsize);

    totold += oldsize;
    totnew += newsize;

    unlink(sqdname);
    rename(newname, sqdname);
  }
  
  printf("\n");

  return ret;
}



/* Remove the extension from the name of the squish file to pack */

static int near pack_extension(char *name)
{
  char *lp, *ld;

  lp=strrstr(name, "/\\:");
  
  if ((ld=strrchr(name, '.')) != NULL && (!lp || ld > lp))
    *ld='\0';
  
  return (pack_squish_file(name));
}





/* Create a list of .SQD files to process, based on a wildcard given        *
 * on the command-line.                                                     */

static int near process_sqd_wildcard(char *fspec)
{
  FFIND *ff;
  char *pth;
  struct _alist *alist=NULL;
  struct _alist *al, *alnext;
  char name[PATHLEN];
  int ret=0;

  ff=FindOpen(fspec, 0);
  
  if (ff==NULL)
  {
    printf("Filename `%s' not found!\n", fspec);
    exit(0);
  }
  
  if ((pth=strrstr(fspec, "/\\:")) != NULL)
  {
    /* Cut off everything after the last path delimiter, and ret this strng */

    pth[1]='\0';
    pth=fspec;
  }
  else
  {
    /* return an empty string */
    
    pth=fspec;
    *pth='\0';
  }

  /* Create linked list of filenames, to ensure that we don't process an    *
   * individual file more than once.                                        */

  totold=totnew=0L;

  do
  {
    strcpy(name, pth);
    strcat(name, ff->szName);

    al=smalloc(sizeof(struct _alist));
    al->name=sstrdup(name);

    al->next=alist;
    alist=al;
  }
  while (FindNext(ff)==0);

  FindClose(ff);


  ret=0;
  
  for (al=alist; al; alnext=al->next, free(al), al=alnext)
  {
    ret=pack_extension(al->name) || ret;
    free(al->name);
  }

  return ret;
}



/* Process the specified file area */

static word near process_this_area(char *name, char *argv[])
{
  char **p;
  
  for (p=argv+2; *p; p++)
    if (eqstri(name, *p))
      return TRUE;
    
  return FALSE;
}


/* Process a Max 2.x area data file.  Returns TRUE if the file was
 * Max 2.x format, or FALSE otherwise.  If returns TRUE, *piRet
 * is the return value from pack_extension.
 */

int process_max2_areas(int fd, int all, int *piRet, char *argv[])
{
  static struct _area a;
  int ret=0;

  /* Start from beginning of file */

  lseek(fd, 0L, SEEK_SET);

  /* If we can't read the area header, or if it isn't a valid Max
   * 2.x-format data file...
   */

  if (read(fd, (char *)&a, sizeof a) != sizeof a ||
      a.id != AREA_ID)
  {
    return FALSE;
  }

  do
  {
    if ((a.type & MSGTYPE_SQUISH) &&
        (all || process_this_area(a.msgpath, argv)))
    {
      ret=pack_squish_file(a.msgpath) || ret;
    }
  }
  while (read(fd, (char *)&a, sizeof a) == sizeof a);

  *piRet=ret;
  return TRUE;
}


/* Process a Max3 area data file.  But first check to see if it is
 * Max2.x format, and if so, call the appropriate routine.
 */

static int near process_area_dat(int argc, char *argv[])
{
  word all=TRUE;
  HAF haf;
  HAFF haff;
  MAH ma={0};
  int ret=0;
  int fd;
  
  if (argc > 2)
    all=FALSE;

  /* Test for Max 2.x format */

  if ((fd=shopen(argv[1], O_RDONLY | O_BINARY)) != -1)
  {
    int fGotMax2;

    fGotMax2=process_max2_areas(fd, all, &ret, argv);
    close(fd);

    if (fGotMax2)
      return ret;
  }

  if ((haf=AreaFileOpen(argv[1], TRUE))==NULL ||
      (haff=AreaFileFindOpen(haf, NULL, 0))==NULL)
  {
    printf("Error opening area database `%s' for read!\n", argv[1]);
    exit(1);
  }
  
  while (AreaFileFindNext(haff, &ma, FALSE)==0)
  {
    if ((ma.ma.type & MSGTYPE_SQUISH) &&
        (all || process_this_area(MAS(ma, name), argv)))
    {
      ret=pack_squish_file(MAS(ma, path)) || ret;
    }
  }
  
  AreaFileFindClose(haff);
  AreaFileClose(haf);

  return ret;
}


static void near format(void)
{
  putss( "Format 1:\n\n"

         "  SQPACK <filespec>\n\n"

         "        This instructs SQPACK to process all of the specified\n"
         "        .SQD areas, as given by a wildcard.\n\n"

         "        ie. SQPACK D:\\MSG\\*.SQD\n\n"

         "Format 2:\n\n"

         "  SQPACK <marea> [area1 area2 ... arean]\n\n"

         "        This instructs SQPACK to read a Maximus-style area data file and to\n"
         "        process the areas within.  For Max 2.x, specify the name and path of\n"
         "        your AREA.DAT file.  For Max 3.x, specify the name and path (but no\n"
         "        extension) of your MAREA database. By default, all areas are\n"
         "        processed.  To process only selected areas, list the names of those\n"
         "        areas on the command line, after the name of your area data file.\n");

  exit(1);
}



static int near process_all_areas(int argc, char *argv[])
{
  char name[PATHLEN];
  int ret;
 
  if (argc < 2)
    format();

  strcpy(name, argv[1]);

  /* If no extension is supplied, use .SQD by default */

  if (!strchr(name, '.'))
  {
    char temp[PATHLEN];

    strcpy(temp, name);
    strcat(temp, ".sqd");

    if (fexist(temp))
      strcpy(name, temp);
  }

  if (stristr(name, ".sqd"))
    ret=process_sqd_wildcard(name);
  else ret=process_area_dat(argc, argv);

  return ret;
}




int _stdc main(int argc, char *argv[])
{
  int ret;

  printf("\nSQPACK  Squish Database Pack Utility; Version " SQVERSION "\n"
     "Copyright 1991, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

  install_24();
  atexit(uninstall_24);

  #ifdef DMALLOC
  dmalloc_on(1);
  #endif

  ret=process_all_areas(argc, argv);

  printf("\nOriginal size=%ld.  Packed size=%ld.\n", totold, totnew);

  return ret;
}



void _fast NoMem(void)
{
  printf("\aRan out of memory!\n");
  exit(1);
}


