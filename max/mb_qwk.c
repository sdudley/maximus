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
static char rcs_id[]="$Id: mb_qwk.c,v 1.10 2004/01/12 23:28:14 wmcbrine Exp $";
#pragma on(unreferenced)

/*# name=QWK creation code for the BROWSE command
*/

#define MAX_LANG_m_browse
#define MAX_LANG_f_area
#define MAX_LANG_max_chat

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <stddef.h>
#include "dr.h"
#include "prog.h"
#include "ffind.h"
#include "max_msg.h"
#include "max_file.h"
#include "m_browse.h"
#include "mb_qwk.h"
#include "qwk.h"
#ifdef MAX_TRACKER
#include "trackc.h"
#include "trackm.h"
#endif
#include "l_attach.h"

static void near QWKAddToCdat(PMAH pmah);

/* Count for # of msgs packed in this area */

static dword area_toyou, area_packed;
static dword total_toyou, total_packed, total_msgs;

static int n_qidx, n_len;
static int qwkfile, ndxfile, perndx;
static int per_msgs;

#ifdef __MSDOS__
#define MAX_QWK_BUF 8     /* buffer up to 8 blocks */
#else
#define MAX_QWK_BUF 64
#endif

static char *mdat_buf;    /* 4K buffer for writing to messages.dat */
static int mdat_nblock;

static word this_conf;
static word num_conf;
static long cdatpos;
static FILE *cdat;
static struct _qmndx *qidxs;
static union stamp_combo now;
static byte qwk_ctr='k';

struct _akh akh;
struct _akd *akd;

struct _qfile
{
  char *fname;
  dword attachID;
  struct _qfile *next;
};

char *qwk_path=NULL;

static char ps_dats[]="%sDATS";
static char o8lxdat[]=PATH_DELIMS "%08lx.DAT";
static char ndx_name[]="%s%03d.NDX";
static char personal_name[]="%sPERSONAL.NDX";
static char mdat_template[]="%smessages.dat";

static struct _qfile *qwk_atts=NULL;
static int file_attaches=0;

static int near add_fileattach(char * szFileName, dword ulAttachID)
{
  int rc = TRUE;
  char * p;
  struct _qfile *q;

  for (p = strtok(szFileName,cmd_delim);
       rc && p;
       p=strtok(NULL,cmd_delim))
  {

    if ((q=malloc(sizeof(struct _qfile)))==NULL)
      rc=FALSE;
    else
    {
      q->attachID=ulAttachID;
      if ((q->fname=strdup(szFileName))!=NULL)
      {
        ++file_attaches;
        q->next=qwk_atts;
        qwk_atts=q;
      }
      else
      {
        free(q);
        rc=FALSE;
      }
    }
  }
  return rc;
}

static void near clean_attaches(int rcvd)
{
  DBASE OS2FAR *pdb;
  struct _qfile *q;

  if (rcvd)
    pdb=LFAdbOpen(PRM(attach_base));
  else
    pdb=NULL;
  for (q=qwk_atts; q; )
  {
    struct _qfile *n=q->next;
    if (pdb && q->attachID)
    {
      int rc;
      void *pvLookId[] = { NULL, NULL, NULL };
      PALIST * pplLook;
      LFA_REC lfa;

      pvLookId[0] = &q->attachID;
      pplLook = PalistNew();
      rc=LFAdbLookup(pdb, pvLookId, pplLook, &lfa);
      PalistDelete(pplLook);
      if (rc && (lfa.ulAttributes & LFA_NODELETEATTACH)==0)
      {
        pvLookId[0] = &lfa.ulAttachID;
        pvLookId[1] = lfa.szTo;
        pvLookId[2] = lfa.szArea;
        LFAdbRemove(pdb, pvLookId);
      }
    }
    free(q->fname);
    free(q);
    q=n;
  }
  if (pdb)
    LFAdbClose(pdb);
  qwk_atts=NULL;
  file_attaches=0;
}

static int copy_attaches(void)
{
  int rc=TRUE;

  if (file_attaches)
  {
    struct _qfile *q;

    for (q=qwk_atts; q; q=q->next)
    {
      char  temp[PATHLEN];

      /* Invent a new filename for each attach */

      sprintf(temp, "%s" PATH_DELIMS "filatt%02d%s", qwk_path, FileEntries(), strrchr(q->fname,'.'));
      if (lcopy(q->fname,temp)==-1)
      {
        q->attachID=0;  /* Don't delete this one */
        rc=FALSE;
      }
      else AddFileEntry(temp, FFLAG_NOBYTES, -1L);
    }
  }
  return rc;
}

/* Buffer a block to be written to messages.dat */

static int near queue_block(char *block)
{
  int bytes;

  /* Transfer block to buffer */

  if (block)
    memmove(mdat_buf + mdat_nblock++*QWK_RECSIZE, block, QWK_RECSIZE);

  /* If we're s'posta flush the buffer */

  if (!block || mdat_nblock==MAX_QWK_BUF)
  {
    bytes=QWK_RECSIZE*mdat_nblock;

    /* If error on writing, return false */

    if (write(qwkfile, mdat_buf, (unsigned)bytes) != bytes)
    {
      char mdatname[PATHLEN];

      sprintf(mdatname, mdat_template, qwk_path);
      logit(cantwrite, mdatname);
      return FALSE;
    }

    mdat_nblock=0;
  }

  return TRUE;
}


int QWK_Begin(BROWSE *b)
{
  NW(b);

  Get_Dos_Date(&now);
  
  total_packed=total_toyou=total_msgs=0L;
  num_conf=0;
  
  logit(log_qwk_download);

  Lmsg_Free();

  if ((qidxs=malloc(sizeof(struct _qmndx)*MAX_QIDX))==NULL ||
      (qwk_path=malloc(PATHLEN))==NULL ||
      (len_chain=malloc(sizeof(struct _len_ch)*MAX_LEN_CHAIN))==NULL ||
      Make_QWK_Directory()==-1 ||
      Create_Control_DAT()==-1 ||
      Create_Messages_DAT()==-1 ||
      Read_Kludge_File(&akh, &akd)==-1)
  {
    if (qidxs)
    {
      free(qidxs);

      if (qwk_path)
      {
        free(qwk_path);
        
        if (len_chain)
          free(len_chain);
      }
    }

    qidxs=NULL;
    qwk_path=NULL;
    len_chain=NULL;
    
    return -1;
  }

  n_qidx=0;
  n_len=0;

  Puts(qwk_pack_start);

  return 0;
}



int Make_QWK_Directory(void)
{
  /* Make sure that the appropriate control file sections are filled out */

  if (*PRM(olr_dir)=='\0' || *PRM(olr_name)=='\0' || *PRM(arc_ctl)=='\0')
    return -1;

  sprintf(qwk_path,"%snode%02x", PRM(olr_dir), task_num);

  return Make_Clean_Directory(qwk_path);
}


void Clean_QWK_Directory(int rdir)
{
  Clean_Directory(qwk_path, rdir);
}



static int near Create_Control_DAT(void)
{
  char temp[PATHLEN];
  union stamp_combo sc;
  static char ps_n[]="%s\r\n";

  
  sprintf(temp, "%scontrol.dat", qwk_path);
    
  if ((cdat=shfopen(temp, fopen_writep, O_RDWR | O_CREAT | O_BINARY |
		    O_NOINHERIT))==NULL)
  {
    cant_open(temp);
    return -1;
  }

  Get_Dos_Date(&sc);

  /* The name of the system */

  fprintf(cdat, ps_n, PRM(system_name));
  
  /* The name of this city - we don't know, so leave it blank */
  
  fprintf(cdat, "\r\n");

  /* Phone number of this place */

  fprintf(cdat, ps_n, PRM(phone_num));
  fprintf(cdat, ps_n, PRM(sysop));

  /* Name of the xxxxxxxx.QWK file */

  fprintf(cdat,"0 ,%s\r\n", PRM(olr_name));

  /* The current date */

  fprintf(cdat,"%02d-%02d-%4d,%02d:%02d:%02d\r\n",
               sc.msg_st.date.mo,
               sc.msg_st.date.da,
               sc.msg_st.date.yr+1980,
               sc.msg_st.time.hh,
               sc.msg_st.time.mm,
               sc.msg_st.time.ss << 1);

  /* Now add the user's name */

  fprintf(cdat, "%s\r\n", usrname);

  fprintf(cdat, "\r\n");  /* Name of custom menu to display; none in this case*/
  fprintf(cdat, "0\r\n"); /* ?? Unknown. */
  fprintf(cdat, "0\r\n"); /* ?? Unknown. */


  /* Save this position in the file for later */
  
  cdatpos=ftell(cdat);
  
  /* Write the highest conference number - this will be updated later */
	
  fprintf(cdat, "%-5u\r\n", 0);
  
  /* Following this is a list of all the conferences containing messages.   *
   * We write these out as we're searching through the areas, so leave      *
   * the file open.                                                         */

  return 0;
}




static int near Create_Messages_DAT(void)
{
  /*
  char *text="Produced by MaxQWK...Copyright (c) 1991 by Scott J. Dudley.  "
             "All rights reserved.";
  */

  /* some readers barf if they don't get this exact (c) message. */

  char *text="Produced by Qmail...Copyright (c) 1987 by Sparkware.  "
             "All Rights Reserved";


  char block[QWK_RECSIZE];
  char mdatname[PATHLEN];

  if ((mdat_buf=malloc(MAX_QWK_BUF * QWK_RECSIZE))==NULL)
  {
    logit(mem_none);
    return -1;
  }

  mdat_nblock=0;

  /* Now create the beginning of the message data file */
  
  sprintf(mdatname, mdat_template, qwk_path);
  
  if ((qwkfile=sopen(mdatname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY | O_NOINHERIT,
                     SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
  {
    cant_open(mdatname);
    free(mdat_buf);
    return -1;
  }

  memset(block, ' ', QWK_RECSIZE);
  memmove(block, text, strlen(text));
  
  if (!queue_block(block))
  {
    close(qwkfile);
    return -1;
  }


  /* Now handle the "Personal Messages" index file */

  per_msgs=0;

  return 0;
}




int Read_Kludge_File(struct _akh *akh, struct _akd **akd)
{
  char temp[PATHLEN];
  unsigned howmuch;
  int kfd, got;



  /* Allocate memory for the kludge data file for this user */

  if ((*akd=malloc(sizeof(struct _akd)*MAX_QWK_AREAS))==NULL)
    return -1;

  /* Initialize it to zeroes */

  memset(*akd, '\0', sizeof(struct _akd)*MAX_QWK_AREAS);

  /* Create the base directory name, and ensure that it exists */

  sprintf(temp, ps_dats, PRM(olr_dir));
  
  if (! direxist(temp))
    if (mkdir(temp) != 0)
    {
      free(*akd);
      return -1;
    }
    
  /* Append the name of the data file we need to access */
    
  sprintf(temp+strlen(temp), o8lxdat, (long)usr.lastread_ptr);
  

  /* If we can't open or read it, or if it's for some other user, then      *
   * initialize a new struct.                                               */

  if ((kfd=shopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1 ||
      read(kfd, (char *)akh, sizeof(struct _akh)) != sizeof(struct _akh) ||
      !eqstri(akh->name, usr.name))
  {
    strcpy(akh->name, usr.name);
    akh->num_areas=0;
    return 0;
  }

  /* Read in all of the _akd structs into memory */
  
  howmuch=akh->num_areas * sizeof(struct _akd);
  
  /* If we only got some of the structs, set the pointer to the actual      *
   * number we got.                                                         */

  if ((got=read(kfd, (char *)*akd, howmuch)) != (int)howmuch)
    akh->num_areas=got/sizeof(struct _akd);

  close(kfd);
  return 0;
}





int Write_Kludge_File(struct _akh *akh, struct _akd **akd)
{
  char temp[PATHLEN];
  unsigned howmuch;
  int kfd, ret;

  ret=-1;
  
  sprintf(temp, ps_dats, PRM(olr_dir));
  sprintf(temp+strlen(temp), o8lxdat, (long)usr.lastread_ptr);
  
  if ((kfd=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY | O_NOINHERIT,
                 SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
  {
    return -1;
  }
  
  if (write(kfd, (char *)akh, sizeof(struct _akh))==sizeof(struct _akh))
  {
    howmuch=akh->num_areas * sizeof(struct _akd);
  
    if (write(kfd, (char *)*akd, howmuch)==(signed)howmuch)
      ret=0;
  }
  
  close(kfd);
  return ret;
}



/* Insert an area in the current message area -> QWK area translation list. *
 * If tossto != -1, insert the area in that specific toss slot.             */

int InsertAkh(char *aname, int tossto)
{
  int old, an;

  if (tossto != -1 && tossto <= MAX_QWK_AREAS)
    an=tossto;
  else
  {
    /* Scan the list of areas we've already processed, and if we haven't    *
     * found the one we're looking for, add a new one.                      */

    for (an=0; an < akh.num_areas; an++)
      if (eqstri(aname, akd[an].name))
        break;

    /* If we couldn't find a slot to stick this area in, then figure out    *
     * what to do...                                                        */

    if (an==akh.num_areas && akh.num_areas >= MAX_QWK_AREAS)
    {
      /* All areas are full, so find one that isn't being used.             *
       * Search through all of the areas we have, and recycle the oldest    *
       * one, such that it's hopefully not being used again.                */

      for (an=0, old=-1; an < akh.num_areas; an++)
        if (old==-1 || !GEdate(&akd[an].used, &akd[old].used))
          old=an;

      /* If the oldest area is equal to now, then more than MAX_QWK_AREAS   *
       * must have been tagged for this call.  In that case, give a         *
       * warning message, and skip this area.                               */

      if (akd[old].used.ldate==now.ldate)
      {
        Puts(qwk_toomany);
        return 1;
      }

      an=old;
    }
  }

  strcpy(akd[an].name, aname);
  akd[an].used=now;

  if (an >= akh.num_areas)
    akh.num_areas=an+1;

  return an;
}




/* Switch to a new QWK area */

int QWK_Status(BROWSE *b, char *aname, int colour)
{
  char tp[PATHLEN];

  NW(b);
  
  
  this_conf=InsertAkh(aname, -1) + 1;
  
  /* Prepare the index file for this area */

  sprintf(tp, ndx_name, qwk_path, this_conf);
  
  if ((ndxfile=sopen(tp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY | O_NOINHERIT,
                     SH_DENYNONE,S_IREAD | S_IWRITE))==-1)
  {
    cant_open(tp);
    return -1;
  }

  n_qidx=0;

  if (!b->fSilent)
  {
    Rev_Up();
    Printf(srchng, (colour % 7)+9, aname);
    vbuf_flush();
  }

  area_toyou=area_packed=0;
  

  /* Add the conference "number" to the CONTROL.DAT file */

  fprintf(cdat, "%d\r\n", this_conf);


  QWKAddToCdat(&mah);

  vbuf_flush();
  return 0;
}


static void near QWKAddToCdat(PMAH pmah)
{
  char temp[PATHLEN];

  /* Now add the area name.  If we have a msgname-type tag, then use        *
   * it, making sure to fancy_str() everything appropriately.  Otherwise,   *
   * just copy the MsgInfo line...                                          */

  if (*PMAS(pmah, echo_tag)==0)
  {
    strcpy(temp, *PMAS(pmah, descript)==0
                    ? PMAS(pmah, name)
                    : PMAS(pmah, descript));
  }
  else
  {
    strcpy(temp, MAS(*pmah, echo_tag));
    cfancy_str(temp);
  }


  /* Now add the appropriate name to the file.  Truncate to 14 chars to     *
   * handle brain-dead readers.                                             */

  temp[12]='\0';
  fprintf(cdat, "%s\r\n", *temp ? temp : "Unknown");
}



/* Call the listidle function to display dots */

int QWK_Idle(BROWSE *b)
{
  NW(b);
  
  return (List_Idle(b));
}


/* Queue the current block, if we have exceeded the block size */

static int near maybe_queue(char **blpos, char *block, int *n_blocks)
{
  /* If it's more than the length of one block, write the block to      *
   * disk, and shift this string back to the beginning of the block.    */

  while (*blpos >= block+QWK_RECSIZE)
  {
    if (!queue_block(block))
      return FALSE;

    *blpos -= QWK_RECSIZE;

    memmove(block, block+QWK_RECSIZE, *blpos-block);
    (*n_blocks)++;
  }

  return TRUE;
}


/* Create a .QWK message header for the current message */

static int near BuildQWKHeader(BROWSE *b)
{
  struct _qmhdr qm;

  /* Now fill out the QMail header for this message.  BASIC structures -    *
   * ich ech bleuch!                                                        */
  
  /* ' ' - public message
     '-' - received public message
     '*' - private message
     '+' - received private message
  */

  qm.status=(byte)((b->msg.attr & MSGPRIVATE) ? '*' : ' ');
  
  if (b->msg.attr & MSGREAD)
    qm.status=(byte)(qm.status=='*' ? '+' : '-');
  
  bprintf(qm.msgn, "%-7ld", MsgMsgnToUid(b->sq, b->msgn));
  
  bprintf(qm.date, "%02d-%02d-%02d",
          b->msg.date_written.date.mo,
          b->msg.date_written.date.da,
          (b->msg.date_written.date.yr+80) % 100);
        
  bprintf(qm.time, "%02d:%02d",
          b->msg.date_written.time.hh,
          b->msg.date_written.time.mm);
        
  bprintf(qm.from, "%-25.25s", b->msg.from);
  bprintf(qm.to,   "%-25.25s", b->msg.to);
  bprintf(qm.subj, "%-25.25s", b->msg.subj);
  bprintf(qm.pwd,  "            ");
  bprintf(qm.replyto, "%-8ld", b->msg.replyto);
  
  /* Just an approximation - we fill this value in for real later */
  
  bprintf(qm.len, "%-6ld", MsgGetTextLen(b->m) / QWK_RECSIZE + 1);
  qm.msgstat=QWK_ACTIVE;
  qm.confLSB=(byte)(this_conf & 0xffu);
  qm.confMSB=(byte)((this_conf >> 8) & 0xffu);
  /*qm.wasread=(b->msg.attr & MSGREAD) ? '*' : ' ';*/
  /*qm.wasread=1;*/
  memset(qm.rsvd, ' ', sizeof(qm.rsvd));

  return queue_block((char *)&qm);
}


/* Add this message to PERSONAL.NDX, if necessary */

static int near AddPersonalIndex(BROWSE *b, struct _qmndx *pqn)
{
  char fname[PATHLEN];
  int rc=TRUE;

  /* If the message is either to us or to our alias... */

  if (eqstri(b->msg.to, usr.name) ||
      (*usr.alias && eqstri(b->msg.to, usr.alias)))
  {

    sprintf(fname, personal_name, qwk_path);

    if ((perndx=sopen(fname, O_CREAT | O_APPEND | O_WRONLY | O_BINARY | O_NOINHERIT,
                      SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
    {
      cant_open(fname);
    }
    else
    {
      if (write(perndx, (char *)pqn, sizeof *pqn) != sizeof *pqn)
        rc=FALSE;

      close(perndx);

      per_msgs++;
    }
  }

  return rc;
}


/* Generate the index record for this message */

static int near BuildIndex(BROWSE *b, unsigned long this_rec, word this_conf)
{
  struct _qmndx qmndx;
  int rc=TRUE;
  byte exp=152;
  
  /* Create the MSBinary-format .QWK index */

  this_rec++;
  while (!(this_rec & 0x800000L)) {
    exp--;
    this_rec <<= 1;
  }

  qmndx.mks_rec[0] = this_rec & 0xff;
  qmndx.mks_rec[1] = (this_rec >> 8) & 0xff;
  qmndx.mks_rec[2] = (this_rec >> 16) & 0x7f;
  qmndx.mks_rec[3] = exp;

  qmndx.conf=(byte)(this_conf & 0xffu);

  if (!AddPersonalIndex(b, &qmndx))
    rc=FALSE;

  /* Make sure that the buffer isn't full */

  if (n_qidx==MAX_QIDX)
    Flush_Qidx();

  /* Add this index entry to the buffer */

  qidxs[n_qidx++]=qmndx;

  return rc;
}


/* Structure passed to the QWK line-output callback function */

struct _qwk_callback
{
  int line_type;
  char **blpos;
  char *block;
  int *n_blocks;
};


/* Callback function used to add a line to the .QWK output file */

int QWKCallBack(char *line, void *args, int inbrowse)
{
  struct _qwk_callback *pqc=args;
  int len;

  NW(inbrowse);
  /* If the user can't see sb's or kludges, don't insert them. */

  if (pqc->line_type & MSGLINE_SEENBY && !GEPriv(usr.priv,prm.seenby_priv))
    return 0;
  else if (pqc->line_type & MSGLINE_KLUDGE && !GEPriv(usr.priv,prm.ctla_priv))
    return 0;


  strcat(line, QWK_EOL_STR); /* PCBored end-of-line character */
  len=strlen(line);

  /* Add an '@' to the beginning of all kludges */

  if (pqc->line_type & MSGLINE_KLUDGE)
  {
    **pqc->blpos='@';
    (*pqc->blpos)++;
  }

  /* Copy this line into the buffer */

  memmove(*pqc->blpos, line, len);
  (*pqc->blpos) += len;

  /* Queue this block, if necessary */

  if (!maybe_queue(pqc->blpos, pqc->block, pqc->n_blocks))
    return -1;

  return 0;
}


/* Initialize the headers used for writing the .QWK message */

static int near QWKInitializeHeaders(BROWSE *b, int qwkfile,
                                     unsigned long *pthis_rec)
{
  int rc=0;

  /* Determine the current record number */

  *pthis_rec=tell(qwkfile)/QWK_RECSIZE + mdat_nblock;

  /* Build the header for this message and write to disk */

  if (!BuildQWKHeader(b))
    rc=-1;

  /* Generate the index file for this msg */

  if (!BuildIndex(b, *pthis_rec, this_conf))
    rc=-1;

  /* Now seek to the beginning of the message */

  MsgSetCurPos(b->m, 0L);

  return rc;
}


/* Add all of the stuff to the packet that belongs before the message       *
 * body itself.                                                             */

#ifdef MAX_TRACKER
static void near QWKAddHeaderText(BROWSE *b, char *block, char **pblpos,
                                  int *pn_blocks, TRK_MSG_NDX *ptmn,
                                  char *ctrl, int *pdo_we_own)
#else
static void near QWKAddHeaderText(BROWSE *b, char *block, char **pblpos,
				  int *pn_blocks, char *ctrl, int *pdo_we_own)
#endif
{
  struct _qwk_callback qc;      /* Used for passing info the QWK callback fn */

  /* Initialize the start/end of the blocks */

  *pn_blocks=1;
  *pblpos=block;

  /* Add a "From:" string if we're downloading packets from a netmail area */

  if (mah.ma.attribs & MA_NET)
  {
    sprintf(*pblpos, "From: %s" QWK_EOL_STR QWK_EOL_STR,
            Address(&b->msg.orig));

    (*pblpos) += strlen(*pblpos);
  }

  /* Perform message auditing in this area */

#ifdef MAX_TRACKER
  **pblpos=0;
  TrackAddQWKFirst(ptmn, pdo_we_own, *pblpos, ctrl);
  (*pblpos) += strlen(*pblpos);
#endif


  /* Initialize the QWK callback function variables to point to our         *
   * automatic variables.                                                   */

  qc.blpos=pblpos;
  qc.block=block;
  qc.n_blocks=pn_blocks;
  qc.line_type=MSGLINE_KLUDGE;

  /* Add all of the kludge lines to the output packet */

  ShowKludgeLines(ctrl, QWKCallBack, &qc, FALSE);
}


/* Process the text message body itself */

static int QWKAddMsgBody(BROWSE *b, char *block, char **pblpos, int *pn_blocks)
{
  struct _qwk_callback qc;      /* Used for passing info to QWK callback fn */
  byte *ol[MAX_MSGDISPLAY];     /* Output lines from Msg_Read_Lines */
  byte lt[MAX_MSGDISPLAY];      /* Type of lines from Msg_Read_Lines */
  char last_attr=0;             /* Last attribute read (for Msg_Read_Lines) */
  int n_ol;                     /* Number of lines allocated by Alloc_Outline */
  int got, ln;                  /* Max and current line in msg_read_lines buf */

  qc.blpos=pblpos;
  qc.block=block;
  qc.n_blocks=pn_blocks;

  if ((n_ol=Alloc_Outline(ol)) < 1)
    return -1;

  while ((got=Msg_Read_Lines(b->m, n_ol, 80-HARD_SAFE,
                             80-SOFT_SAFE, ol, lt, &last_attr, MRL_QEXP)) > 0)
  {
    for (ln=0; ln < got; ln++)
    {
      qc.line_type=lt[ln];

      if (QWKCallBack(ol[ln], &qc, FALSE)==-1)
        break;
    }
  }

  Dealloc_Outline(ol);
  return 0;
}


/* Update the counters used for displaying the number of msgs in this area */

static int near UpdateCounters(BROWSE *b)
{
  if (OkToFixLastread(b))
    Lmsg_Set(b, b->msgn);

  if (MsgToThisUser(b->msg.to))
  {
    area_toyou++;
    total_toyou++;
  }

  area_packed++;

  /* If the user packed too many messages, and there is a set limit... */
  
  if (++total_packed >= (dword)prm.max_pack && prm.max_pack)
    return -1;

  return 0;
}

/* "Display" a message by adding it to messages.dat */

int QWK_Display(BROWSE *b)
{
#ifdef MAX_TRACKER
  TRK_MSG_NDX tmn;              /* Tracking record for this message */
#endif
  char *block;                  /* Block used for output */
  char *blpos;                  /* Current position in block */
  char *ctrl;                   /* Kludges for this message */
  dword ctrl_len;               /* Size of control info in this msg */
  unsigned long this_rec;       /* This record number */
  int n_blocks;                 /* Number of blocks in this msg */
  int ret=0;                    /* Return code */
  int do_we_own=FALSE;          /* TRUE if we can see trking info in this msg */


  /* display a dot every couple of seconds */
  
  if (List_Idle(b)==-1)
    return -1;

  /* Don't download messages which are older than the specified date */

  if (((union stamp_combo *)&b->msg.date_arrived)->ldate != 0 &&
      !GEdate((union stamp_combo *)&b->msg.date_arrived, &scRestrict))
  {
    return 0;
  }

  /* Allocate memory to buffer the message in memory */

  if ((block=malloc(QWK_RECSIZE*3))==NULL)
    return -1;

  /* Initialize the packet headers and stuff */

  if (QWKInitializeHeaders(b, qwkfile, &this_rec)==-1)
    ret=-1;


  /* Allocate and read the control information for this msg */

  ctrl_len=MsgGetCtrlLen(b->m);

  if ((ctrl=malloc(ctrl_len+10))==NULL)
    ctrl_len=0;

  MsgReadMsg(b->m, NULL, 0L, 0L, NULL, ctrl_len+1, ctrl);

  /* Add all of the pre-header text information */

#ifdef MAX_TRACKER
  QWKAddHeaderText(b, block, &blpos, &n_blocks, &tmn, ctrl, &do_we_own);
#else
  QWKAddHeaderText(b, block, &blpos, &n_blocks, ctrl, &do_we_own);
#endif
  QWKAddMsgBody(b, block, &blpos, &n_blocks);

  /* See if there's a file attached to this message */

  if ((b->msg.attr & MSGFILE) && MsgToThisUser(b->msg.to))
  {
    DBASE OS2FAR * pdb;           /* File attach database handle */
    LFA_REC lfa;                  /* File attach database record */

    /* See if the attach is still valid */

    if((pdb=Read_Attach(&lfa,&b->msg,ctrl,!!(mah.ma.attribs & MA_NET)))!=NULL)
    {
      char temp[PATHLEN*3];

      /* It is, so save the path until later */

      if (Attach_File(&lfa,temp,NULL)) 
        add_fileattach(temp,lfa.ulAttachID);

      /* .. and close the database */

      LFAdbClose(pdb);
    }
  }

#ifdef MAX_TRACKER
  /* Add tracking information to the end of the message body */

  if (do_we_own && ret != -1)
  {
    TrackAddQWKTail(&tmn, blpos, ctrl);
    blpos += strlen(blpos);

    /* Write to disk, if necessary */

    if (!maybe_queue(&blpos, block, &n_blocks))
      ret=-1;
  }
#endif


  /* Now fill it with spaces to the next 128byte boundary */

  if (blpos-block && ret != -1)
  {
    memset(blpos, ' ', QWK_RECSIZE-(blpos-block));
    queue_block(block);
    n_blocks++;
  }
  
  /* Update the received bit, and set the length of this msg in the .DAT file */

  Recd_Msg(b->m, &b->msg, TRUE);
  Update_Length(this_rec, n_blocks);

  usr.msgs_read++;
  
  /* Free memory used by routine */

  if (ctrl)
    free(ctrl);

  free(block);
  
  if (UpdateCounters(b)==-1)
    ret=-1;

  return ret;
}



/* Processing after all of the messages have been scanned in one area */

int QWK_After(BROWSE *b)
{
  char temp[PATHLEN];
  dword num_msg;
  
  NW(b);

  Flush_Qidx();
  close(ndxfile);
  
  /* Don't send zero-length .NDX files */
  
  if (area_packed==0)
  {
    sprintf(temp, ndx_name, qwk_path, this_conf);
    unlink(temp);
  }
  else
  {
    num_msg=MsgGetNumMsg(b->sq);

    if (!b->fSilent)
    {
      Rev_Up();

      Printf(qwk_pack_fmt, MAS(mah, name), MAS(mah, descript),
             num_msg, area_toyou, area_packed);
    }

    total_msgs += num_msg;
  }

  num_conf++;

  return 0;
}




int QWK_End(BROWSE *b)
{
  int ret=0;
  
  NW(b);

  Rev_Up();
  Flush_Len_Chain();

  queue_block(NULL);  /* flush buffer */
  close(qwkfile);

  FinishControlDAT();

  if (!b->fSilent)
    Printf(qwk_pack_end, total_msgs, total_toyou, total_packed);
  
  if (total_packed >= (dword)prm.max_pack && prm.max_pack)
    Printf(qwk_too_many, (long)prm.max_pack);
  
  /* If we got something, write the conference list and pack the mail. */

  if (!num_conf)
    Press_ENTER();
  else
  {
#ifdef MAX_TRACKER
    /* If we have database moderators, add a report to their .QWK packet */

    if (!b->fSilent)
      TrackReportOurMessages(qwk_path);
#endif

    Write_Kludge_File(&akh, &akd);
    ret=QWK_Compress_Mail(b);
    
    /* Only save the bundle if we are local, and the return code was good. */
    if (! (local && ret==0))
      Clean_QWK_Directory(TRUE);
  }

  clean_attaches((ret==0));

  free(akd);
  free(len_chain);
  free(qidxs);
  free(qwk_path);

  qwk_path=NULL;
  akd=NULL;
  len_chain=NULL;
  qidxs=NULL;
  
  return ret;
}






static void near FinishControlDAT(void)
{
/*  char temp[PATHLEN];*/

  /* Delete PERSONAL.NDX if no messages for user */
  
  /* obsolete - file is simply not opened if no personal msgs */

  /*
  if (per_msgs==0)
  {
    sprintf(temp, personal_name, qwk_path);
    unlink(temp);
  }
  */


  fprintf(cdat, "HELLO\r\n");
  fprintf(cdat, "NEWS\r\n");
  fprintf(cdat, "GOODBYE\r\n");
  fprintf(cdat, "0\r\n");

  /* Now, finally update the "number of conferences" pointer */

  fseek(cdat, cdatpos, SEEK_SET);
  fprintf(cdat,"%-5u\r\n", num_conf ? num_conf-1 : 0);

  fclose(cdat);
}






static int near QWK_Compress_Mail(BROWSE *b)
{
  char qwkname[PATHLEN];
  char files[PATHLEN];
  char cmd[PATHLEN];
  struct _css css;
  sword protocol;
  sword good, ret;
  struct _arcinfo *pai;

  

/*  Printf(xxx_msgs_packed, total_packed);*/
  
  if (total_packed==0 || GetYnAnswer(download_msgs,0)==NO)
    return -1;
  
  Puts(wait_doing_compr);
  
  GenerateStupidFiles();
    
  sprintf(qwkname, "%s%s.qw%c", qwk_path, PRM(olr_name), qwk_ctr);
  unlink(qwkname);

#ifndef UNIX
  sprintf(files, "%s*.*", qwk_path);
#else
  sprintf(files, "%s*", qwk_path);
#endif  
  
  Load_Archivers();
  
  while (usr.compress==0 || 
         usr.compress > MAX_ARI ||
         (pai=UserAri(usr.compress))==NULL)
  {
    usr.compress=Get_Archiver();
  }

  if (!pai)
  {
    Puts(unknown_compr);
    ret=-1;
  }
  else
  {
#ifndef UNIX
    char tmp[PATHLEN * 2];
#endif

    Form_Archiver_Cmd(qwkname, files, cmd, pai->add);

    /* Add MaxPipe to the call */

#ifndef UNIX
    sprintf(tmp, maxpipe_cmd, cmd);
    strcpy(cmd, tmp);
#endif

    ret=Outside(NULL, NULL, OUTSIDE_RUN | OUTSIDE_NOFIX, cmd, FALSE,
                CTL_NONE, 0, NULL);
  }
  
  Clean_QWK_Directory(FALSE);
  
#ifdef UNIX
    adaptcase(qwkname);
#endif
  
  if (ret != 0 || !fexist(qwkname))
  {
    logit(log_err_compr, ret);
    Puts(err_compr_mail);
    Press_ENTER();
    return -1;
  }

  good=FALSE;

  if (local)
    good=TRUE;
  else
  {
    Putc('\n');

    if (File_Get_Protocol(&protocol, FALSE, TRUE)==-1)
      return -1;

    save_tag_list(NULL);
    Free_Filenames_Buffer(0);

    if (AddFileEntry(qwkname, FFLAG_NOBYTES, -1L)==-1)
      return -1;
    
    /* Save caller's chat status */
    
    ChatSaveStatus(&css);
    ChatSetStatus(FALSE, cs_file_xfer);

    /* Add any file attaches to the queue */

    copy_attaches();
    
    /* If the file was sent okay, update the user's lastread pointers */

    if (File_Send_Files(protocol, menuname, NULL, 1))
      good=TRUE;

    /* In case user ran over time limit, make sure that LRPs are updated */
    
    do_timecheck=FALSE;

    /* Restore caller's chat status */
    
    ChatRestoreStatus(&css);

    Free_Filenames_Buffer(0);
    restore_tag_list(NULL,FALSE);
  }

  if (good)
  {
    extern struct _lrptr *lrptr;

    if (qwk_ctr=='k')
      qwk_ctr='0';

    qwk_ctr++;

    if (qwk_ctr=='9'+1)
      qwk_ctr='0';

    if (lrptr)
    {
      Puts(qwk_update_lr);
      vbuf_flush();
      Lmsg_Update(b);
      Putc('\n');
    }
  }

  /* Restore timecheck value */

  do_timecheck=TRUE;


  return ret;
}



/* If the specified file doesn't exist in the qwk_path, make it blank */

#ifdef NEVER
static void near MakeBlank(char *s)
{
  char fname[PATHLEN];
  int fd;
  
  strcpy(fname, qwk_path);
  strcat(fname, s);
  
  if ((fd=open(fname, O_CREAT | O_WRONLY | O_BINARY,
               S_IREAD | S_IWRITE)) != -1)
    close(fd);
}
#endif


static void near CopyOLRdir(char *fspec)
{
  char fname[PATHLEN];
  char toname[PATHLEN];
  FFIND *ff;
  
  strcpy(fname, PRM(olr_dir));
  strcat(fname, fspec);
  
  if ((ff=FindOpen(fname, 0))==NULL)
    return;
  
  do
  {
    /* Copy the file FROM the olr dir TO this directory */

    strcpy(fname, PRM(olr_dir));
    strcat(fname, ff->szName);
    
    strcpy(toname, qwk_path);
    strcat(toname, ff->szName);
    
    lcopy(fname, toname);
  }
  while (FindNext(ff)==0);
  
  FindClose(ff);
}




static void near GenerateStupidFiles(void)
{
  char fname[PATHLEN];
  FILE *fp;
  
  /* Write the stupid DOOR.ID file */
  
  sprintf(fname, "%sDOOR.ID", qwk_path);
  
  if ((fp=fopen(fname, "wb")) != NULL)
  {
    fprintf(fp, "DOOR = %s\r\n", us_short);
    fprintf(fp, "VERSION = %s\r\n", version);
    fprintf(fp, "SYSTEM = %s\r\n", xfer_id);
    fprintf(fp, "CONTROLNAME = %s\r\n", cprog_name);
/*    fprintf(fp, "CONTROLTYPE = ADD\r\n");
    fprintf(fp, "CONTROLTYPE = DROP\r\n");
*/
    fclose(fp);
  }

  /* Copy everything from \max\olr to \max\olr\nodeXX */

  CopyOLRdir(WILDCARD_ALL);

  /* Make sure that these files exist */
  
/*MakeBlank("HELLO");*/
/*MakeBlank("NEWS");*/
/*MakeBlank("GOODBYE");*/
}



/* BASIC printf() function.  Copies to a destination, without terminating   *
 * nuls.                                                                    */

static void _stdc bprintf(char *dest, char *format,...)
{
  char string[120];
  va_list var_args;

  va_start(var_args,format);
  vsprintf(string,format,var_args);
  va_end(var_args);

  memmove(dest, string, strlen(string));
}


static void near Flush_Qidx(void)
{
  if (n_qidx)
  {
    write(ndxfile, (char *)qidxs, sizeof(struct _qmndx)*n_qidx);
    n_qidx=0;
  }
}



/* Update the dummy length of a particular record, and make it reflect the  *
 * true message length.  This is buffered, so we only update the actual     *
 * message file once in a while.                                            */

static void near Update_Length(long rec, int blocks)
{
  if (n_len==MAX_LEN_CHAIN)
    Flush_Len_Chain();
  
  len_chain[n_len].rec=rec;
  len_chain[n_len++].blocks=blocks;
}


/* Seek back to the header block, and update the position count for each    *
 * message.                                                                 */

static void near Flush_Len_Chain(void)
{
  struct _len_ch *lc, *end;
  char b_len[12];
  
  /* Flush the message block buffer, if any */

  queue_block(NULL);

  for (lc=len_chain, end=len_chain+n_len; lc < end; lc++)
  {
    lseek(qwkfile,
          (lc->rec*QWK_RECSIZE) + offsetof(struct _qmhdr, len),
          SEEK_SET);

    bprintf(b_len, "%-6d", lc->blocks);
    write(qwkfile, b_len, 6);
  }
  
  if (n_len)
  {
    n_len=0;
    lseek(qwkfile, 0L, SEEK_END);
  }
}

