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
static char rcs_id[]="$Id: mr.c,v 1.1.1.1 2002/10/01 17:57:32 sdudley Exp $";
#pragma on(unreferenced)

#define MAX_INCL_VER

#define INCL_DOS
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <dos.h>
#ifdef OS_2
#include <os2.h>
#endif
#include "dr.h"
#include "prog.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#include "bfile.h"
#include "max.h"
#include "old_msg.h"
#include "mr.h"
#include "areaapi.h"
#include "prmapi.h"

char *orig_path[MAX_DRIVES];
char orig_disk;

extrn int _stdc brk_trapped;
static long total = 0L;

#ifdef COPY
  int copymsg(FILE *outfile, char *fname);
  static FILE *outf = NULL;
#endif

static char *pu_msg="%u.msg";

static void _stdc restore_dirs(void)
{
  /* Now restore the current directory to what it was before */

  Restore_Dir(&orig_disk, orig_path);
}


void _fast NoMem(void)
{
  printf("\nRan out of memory!\n");
  exit(1);
}

static void near usage(void)
{
  printf("Format:\n\n"

         "    MR [options] [area...]\n\n"

         "[area...] are one or more area names to be renumbered.  For typical use, to\n"
         "renumber all areas, run:\n\n"

         "    MR all\n\n"

         "The supported options are:\n\n"

         "    -m<file>  - use <file> as the message area data file\n"
         "    -p<file>  - use the PRM file from <file> instead of MAXIMUS environment var\n");

#ifdef COPY
  printf("    -f<file>   - append erased messages to this ascii file.\n");
#endif

  exit(1);
}



/* Compare function for qsorting message array */

static int _stdc msgcomp(const void *i1, const void *i2)
{
  return (((REN *)i1)->old - ((REN *)i2)->old);
}







static REN * near scan_area(word *nummsg)
{
  FFIND *ff;
  REN *ren;
  RNUM *ra=NULL, *rp, *rpnext;
  word num_msg=0;
  word i;

  if ((ff=FindOpen("*.msg", 0))==NULL)
  {
    printf("No messages ");
    return NULL;
  }

  printf("- Scan ");
  fflush(stdout);


  /* Create a linked list of message numbers in the area */
  
  do
  {
    if ((rp=malloc(sizeof(REN)))==NULL)
      NoMem();
    
    memset(rp, '\0', sizeof(REN));

    if ((rp->num=atoi(ff->szName)) == 0)
      free(rp);
    else
    {
      /* The message was good, so add node to the linked list */

      rp->next=ra;
      ra=rp;
      
      num_msg++;
    }
  }
  while (FindNext(ff)==0);
  
  FindClose(ff);

  if (num_msg==0)
  {
    printf("- Nothing to process ");
    fflush(stdout);
    return NULL;
  }
  
  *nummsg=num_msg;

  if ((ren=malloc(num_msg*sizeof(REN)))==NULL)
    NoMem();

  memset(ren, '\0', num_msg*sizeof(REN));


  /* Copy messages to array */

  for (rp=ra; rp && num_msg--; rpnext=rp->next, free(rp), rp=rpnext)
    ren[num_msg].old=rp->num;

  /* Sort the array */
  
  qsort(ren, *nummsg, sizeof(REN), msgcomp);

  /* Scan all of the messages in the area to gather up/down and date info */

  for (i=0; i < *nummsg; i++)
  {
    char msgname[PATHLEN];
    int fd;
    
    if (brk_trapped)
      return NULL;

    sprintf(msgname, pu_msg, ren[i].old);
    
    if ((fd=shopen(msgname, O_RDONLY | O_BINARY))==-1)
      printf("\nCan't open %s", msgname);
    else
    {
      struct _omsg omsg;
      
      if (read(fd, (char *)&omsg, sizeof(omsg)) != sizeof(omsg))
        printf("\nCan't read %s", msgname);
      else
      {
        ren[i].date.msg_st=omsg.date_arrived;

        /* If msg has invalid date, get the stamp from the *.msg file */

        if (omsg.date_arrived.date.da==0 ||
            omsg.date_arrived.date.da > 31 ||
            omsg.date_arrived.date.yr > 50 ||
            omsg.date_arrived.time.hh > 23 ||
            omsg.date_arrived.time.mm > 59 ||
            omsg.date_arrived.time.ss > 59)
        {
          Get_Dos_Date(&ren[i].date);
          get_fdt(fd, &ren[i].date);
        }

        ren[i].up=omsg.reply;
        ren[i].down=omsg.up;
      }
      
      close(fd);
    }
  }
  
  return ren;
}






static void near delete_msgs(PMAH pmah, REN *ren, word num_msg)
{
  word i;
  word gone=0;

  if (pmah->ma.killbyage || pmah->ma.killbynum)
  {
    printf("- Delete ");
    fflush(stdout);
  }
  else return;

  if (pmah->ma.killbynum)
  {
    sword j;

    j=(sword)num_msg - (sword)pmah->ma.killbynum - !(pmah->ma.attribs & MA_ECHO);

    while (j >= 0)
    {
      if (ren[j].old==1 && (pmah->ma.attribs & MA_ECHO))
        j--;
      else ren[j--].flag |= REN_DELETE;
    }
  }
  

  /* Kill messages by date arrived */

  if (pmah->ma.killbyage)
  {
    union stamp_combo today;
    sword dd, mm, yy;

    /* Calculate the date of oldest msg to keep */

    Get_Dos_Date(&today);

    dd=today.msg_st.date.da;
    mm=today.msg_st.date.mo;
    yy=today.msg_st.date.yr;

    dd -= pmah->ma.killbyage;

    while (dd < 1)
    {
      dd += 31;
      mm--;
    }

    while (mm < 1)
    {
      mm += 12;
      yy--;
    }
    
    if (yy < 0)
      yy=0;
    
    today.msg_st.date.da=dd;
    today.msg_st.date.mo=mm;
    today.msg_st.date.yr=yy;

    /* Now scan thru and mark for deletion */

    for (i=0; i < num_msg; i++)
    {
      if (ren[i].old==1 && (pmah->ma.attribs & MA_ECHO))
        continue;

      if (!GEdate(&ren[i].date, &today))
        ren[i].flag |= REN_DELETE;
    }
  }

#ifdef COPY
  if (outf)
  {
    fprintf(outf, "\n");
    fprintf(outf, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    fprintf(outf, "  Area %s ", area->name);

    if (*area->msgname)
      fprintf(outf, " (%s) ", area->msgname);

    if (*area->msginfo)
      fprintf(outf, ", %s", area->msginfo);

    fprintf(outf, "\n");
  }
#endif
  
  /* Now loop through everything marked for deletion and kill it */

  for (i=0; i < num_msg; i++)
  {
    char msgname[PATHLEN];
    
    /* Never delete the HWM in an echo */

    if (brk_trapped)
      return;

    if ((pmah->ma.attribs & MA_ECHO) && ren[i].old==1)
      continue;

    if ((ren[i].flag & REN_DELETE)==0)
      continue;

    ren[i].new=-1;

    sprintf(msgname, pu_msg, ren[i].old);

#ifdef COPY
    if (outf)
        copymsg(outf, msgname);
#endif

    if (unlink(msgname) != 0)
      printf("\nCan't delete %s", msgname);
    
    gone++;
  }
  
  printf("(%4u) ", gone);
  fflush(stdout);
  total += gone;
}

static void near lastread(char *path, char *file, REN *ren, word num_msg)
{
  char temp[PATHLEN];
  word *lr;
  word size;
  word i, max;
  int fd;

  /* Create the name of the file */

  strcpy(temp, path);
  strcat(temp, file);

  
  /* Open the file for update */

  if ((fd=shopen(temp, O_RDWR | O_BINARY))==-1)
    return;
  
  size=(word)fsize(temp);
  max=size/2;

  if (size==0)
  {
    close(fd);
    return;
  }

  if ((lr=malloc(size))==NULL)
    NoMem();
  
  read(fd, (char *)lr, size);

  /* Now scan through and update the user's lastread pointers */

  for (i=0; i < max; i++)
  {
    word j;

    if (! lr[i])
      continue;

    /* Renumber the lastread pointer */

    for (j=0; j < num_msg; j++)
      if (lr[i]==ren[j].old && (ren[j].flag & REN_DELETE)==0)
      {
        lr[i]=ren[j].new;
        break;
      }

    /* Not found, so set to zero */

    if (j==num_msg)
      lr[i]=0;
  }
  
  lseek(fd, 0L, SEEK_SET);
  write(fd, (char *)lr, size);

  free(lr);

  close(fd);
}


static void near renumber_msgs(REN *ren, word num_msg)
{
  word new_num=1;
  word i;

  printf("- Renum ");
  fflush(stdout);

  for (i=0; i < num_msg; i++)
  {
    char from[PATHLEN];
    char to[PATHLEN];
    
    if (ren[i].flag & REN_DELETE)
    {
      ren[i].new=-1;
      continue;
    }
    
    ren[i].new=new_num++;
    
    if (ren[i].old != ren[i].new)
    {
      sprintf(from, pu_msg, ren[i].old);
      sprintf(to, pu_msg, ren[i].new);
    
      if (rename(from, to) != 0)
        printf("\nCan't rename %s to %s", from, to);
    }
  }
  
  printf("- Link ");
  fflush(stdout);

  /* Now redo the reply chains */

  for (i=0; i < num_msg; i++)
  {
    REN this;

    if (ren[i].flag & REN_DELETE)
      continue;

    /* Search for the 'up' reply link */

    if (ren[i].up)
    {
      REN *f;

      this.old=ren[i].up;

      f=bsearch(&this, ren, num_msg, sizeof(REN), msgcomp);

      if (f==NULL)
      {
        ren[i].up=0;
        ren[i].flag |= REN_DELTA;
      }
      else if (ren[i].up != f->new)
      {
        ren[i].flag |= REN_DELTA;

        ren[i].up=f->new;
      }
    }


    /* Search for the 'down' reply link */

    if (ren[i].down)
    {
      REN *f;

      this.old=ren[i].down;

      f=bsearch((void *)&this, ren, num_msg, sizeof(REN), msgcomp);

      if (f==NULL)
      {
        ren[i].down=0;
        ren[i].flag |= REN_DELTA;
      }
      else if (ren[i].down != f->new)
      {
        ren[i].flag |= REN_DELTA;
        ren[i].down=f->new;
      }
    }
  }
  

  
  /* Now perform the actual update of the headers */
  
  for (i=0; i < num_msg; i++)
  {
    char msgname[PATHLEN];
    int fd;

    /* Only change modified messages, and don't renumber deleted ones */

    if ((ren[i].flag & REN_DELTA)==0 || (ren[i].flag & REN_DELETE))
      continue;

    if (brk_trapped)
      return;

    sprintf(msgname, pu_msg, ren[i].new);
    
    if ((fd=shopen(msgname, O_RDWR | O_BINARY))==-1)
      printf("\nCan't open %s", msgname);
    else
    {
      struct _omsg omsg;
      union stamp_combo mdate;
      
      get_fdt(fd, &mdate);
      
      if (read(fd, (char *)&omsg, sizeof(omsg)) != sizeof(omsg))
        printf("\nCan't read %s", msgname);
      else
      {
        omsg.reply=ren[i].up;
        omsg.up=ren[i].down;

        lseek(fd, 0L, SEEK_SET);

        if (write(fd, (char *)&omsg, sizeof(omsg)) != sizeof(omsg))
          printf("\nCan't write %s", msgname);
        
        set_fdt(fd, &mdate);
      }
      
      close(fd);
    }
  }
}



/*
Area 0123456789012345678901234567890 - Scan - Sort - Delete - Renumber - Done
*/



static void near renumber_area(HPRM hp, PMAH pmah)
{
  char path[PATHLEN];
  word num_msg;
  REN *ren;

  PrmRelativeString(hp, PMAS(pmah, path), path);

  if ((pmah->ma.type & MSGTYPE_SDM)==0 || *path==0)
    return;
  
  if (Save_Dir(&orig_disk, orig_path, path)==-1)
  {
    printf("Can't access area '%s'\n", path);
    return;
  }
  
  printf("Area %-27s ", path);
  fflush(stdout);

  if ((ren=scan_area(&num_msg)) != NULL)
  {
    if (brk_trapped)
      return;

    delete_msgs(pmah, ren, num_msg);

    if (brk_trapped)
      return;

    renumber_msgs(ren, num_msg);

    if (brk_trapped)
      return;

    lastread(path, "lastread", ren, num_msg);
    lastread(path, "lastread.bbs", ren, num_msg);
    free(ren);
  }
  
  printf("- Done\n");
  fflush(stdout);
}



static int near scan_adat(char *adat, char **astart, HPRM hp)
{
  HAF haf;
  HAFF haff;
  MAH mah={0};
  word fAll = FALSE;
  char **ap;
  char **aend;

  /* Find the end of the list */

  for (aend=astart; *aend; aend++)
    if (eqstri(*aend, "all"))
    {
      *aend=0;
      fAll = TRUE;
    }
  
  if ((haf=AreaFileOpen(adat, TRUE))==NULL)
  {
    printf("Fatal error opening message area database '%s' for read.  Aborting.\n",
           adat);
    exit(1);
  }
  
  if ((haff=AreaFileFindOpen(haf, NULL, 0)) != NULL)
  {
    while (AreaFileFindNext(haff, &mah, FALSE)==0)
    {
      /* If nothing specified, renumber ALL areas */

      if (fAll)
        renumber_area(hp, &mah);
      else
      {
        for (ap=astart; ap < aend; ap++)
          if (*ap && eqstri(*ap, MAS(mah, name)))
          {
            renumber_area(hp, &mah);
            *ap=NULL;
            break;
          }
      }

      if (brk_trapped)
        break;
    }

    AreaFileFindClose(haff);
  }

  DisposeMah(&mah);
  AreaFileClose(haf);

  /* Print a list of areas that were not processed. */

  for (ap=astart; ap < aend; ap++)
    if (*ap && **ap != '-')
      printf("Warning: unknown area '%s'\n", *ap);
  
  printf("\n%lu messages purged.\n", total);

  return 0;
}

int _stdc main(int argc, char **argv)
{
  char **ap;
  char adat[PATHLEN];
  char *pszMaximus;
  HPRM hp;
  int ret;

#if defined(__FLAT__) && defined(OS_2)
  EXCEPTIONREGISTRATIONRECORD err;

  brktrapos2(&err, TRUE);
#else
  brktrap();
#endif

  atexit(brkuntrap);

  NW(argc);

  #ifdef DMALLOC
  dmalloc_on(1);
  #endif

  printf("\nMR  Maximus *.MSG Maintenance Program  Version " VERSION "\n");
  printf("Copyright 1991, "THIS_YEAR" by Lanius Corporation.  All rights reserved.\n\n");
  
  orig_disk=(char)getdisk();
  atexit(restore_dirs);

  install_24();
  atexit(uninstall_24);
  
  if (argc < 2 ||
      (eqstri(argv[1], "-?") ||
       eqstri(argv[1], "/?") ||
       eqstri(argv[1], "?")))
  {
    usage();
  }

  pszMaximus = GetMaximus(argc, argv, 1);

  hp = PrmFileOpen(pszMaximus, 1);
  strcpy(adat, PrmFileString(hp, marea_name));

#ifdef COPY
  if(!strnicmp(argv[1], "-f", 2)){ /*PLF Wed  10-02-1991  02:32:05 */
    outf = fopen(argv[1]+2, "ab");
    if(!outf){
        perror(argv[1]+2);
        return 1;
    }
    argc--;
    argv++;
  }
#endif

  for (ap=argv+1; *ap; ap++)
  {
    if (**ap=='-')
    {
      switch ((*ap)[1])
      {
        case 'm': strcpy(adat, *ap+2); break;
        case 'p': break;    /* handled by GetMaximus */
        case '?': usage();
      }
    }
  }

  ret=scan_adat(adat, argv+1, hp);
  PrmFileClose(hp);
  
  printf(brk_trapped ? "Aborted.\n" : "Done.\n");
  
  return ret;
}

