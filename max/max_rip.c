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
static char rcs_id[]="$Id: max_rip.c,v 1.2 2003/06/04 23:46:22 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=RIP support routines
*/

#define MAX_INCL_COMMS

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include "prog.h"
#include "mm.h"
#include "max_file.h"
#include "strbuf.h"

#define MAX_RIPSENDFAIL 3
#define RS_RETRIES_MAX  3

static strbuf *ripfilebuf=NULL;
static int ripsend_failcount=0;

static int near ripfilesent(char *szFilename)
{
  return !(!ripfilebuf || sb_inbuf(ripfilebuf,szFilename,TRUE)==(word)-1);
}

static void near addripfile(char *szFilename)
{
  if (ripfilebuf || (ripfilebuf=sb_new(STRBUFSZ))!=NULL)
  {
    if (!ripfilesent(szFilename))
    {
      if (sb_alloc(ripfilebuf,szFilename)==NULL)  /* Out of space */
      {
        strbuf *newsb=sb_realloc(ripfilebuf,STRBUFSZ,0);
        if (newsb==NULL)
          sb_alloc(ripfilebuf,szFilename);
      }
    }
  }
}

static int near ripsendbatch(int fDisplay)
{
  int rc=TRUE;

  logit("@ripsendbatch(%s,fnames=%d)", fDisplay ?"true":"false", FileEntries());

  if (FileEntries())
  {
    long bytes;
    int fn;

    FENTRY fent;

    bytes=0L;
    for (fn=0; GetFileEntry(fn, &fent); ++fn)
      bytes += fent.ulSize;

    /* Tell the remote system we're sending some files...
     * 4=composite dynamic file: just send, remote sorts out where they go
     * 5=active dynamic file: send, remote sorts out & displays as well
     */

    Printf("\r!|0\x1b""070%c0000<>\n", (fDisplay) ? '5' : '4');

    /* Now, send 'em, forcing ZModem */

    if ((rc=File_Send_Files_Sub(PROTOCOL_ZMODEM, NULL, NULL, bytes, FALSE))!=FALSE)
    {

      /* Add the file names sent to the "sent" list to avoid both
       * having to to ask the user's terminal again or having to
       * resend at all unless we're forced to do so via '!'
       */

      for (fn=0; GetFileEntry(fn, &fent); fn++)
        addripfile(fent.szName);
      
    }

    Free_Filenames_Buffer(0);

  }

  return rc;
}


/* sendrip -- transmits a RIP file to the remote user, if necessary.
 *
 * The return code is one of the following:
 *
 *  0 - file transmitted normally
 *  1 - user already has file
 *  2 - the file could not be found
 *  3 - some other error
 */

#define SENDRIP_FILESENT      0
#define SENDRIP_ALREADYHAS    1
#define SENDRIP_FILENOTEXIST  2
#define SENDRIP_ERROR         3

static int near sendrip(char *pszFile, int fDisplay, int force)
{
  int sendit;
  long filesize;
  char temp[PATHLEN];

  if (!hasRIP())
    return SENDRIP_ERROR;

  logit("@sendrip(%s,%s,%d)", pszFile, fDisplay ? "true" : "false", force);

  /* Get qualified filename for file to send */

  strcpy(temp, rippath);
  Add_Trailing(temp, PATH_DELIM);
  strcat(temp, pszFile);

  /* See if it exists in the current RIP path */

  if ((filesize=fsize(temp))==-1L)
  {

    /* Ok, then try the default RIP path */

    strcpy(temp, PRM(rippath));
    Add_Trailing(temp, PATH_DELIM);
    strcat(temp, pszFile);

    if ((filesize=fexist(temp))==-1)
    {

      /* Report it as an error and quit */

      cant_open(temp);
      return SENDRIP_FILENOTEXIST;
    }
  }

  sendit=!ripfilesent(temp);

  /* Query remote if send is forced or file hasn't
   * yet been sent in this session
   */

  if (force || sendit)
  {
    int hasit=RIP_HasFile(pszFile,&filesize);

    if (hasit==-1)
      return SENDRIP_ERROR;    /* RIP query aborted without success */

    if (!hasit)
      sendit=TRUE;
    else
    {

      /* If we know that the remote has the file, add it to our list
       * of "sent" files. Path must also be added to ensure that the
       * same icon will be sent in case rippath changes during session
       */

      addripfile(temp);
      sendit=FALSE;
    }

  }

  /* If remote already has the file and display is true
   * Then simply tell the remote system to display it
   */

  if (!sendit)
  {
    if (fDisplay)
    {

      /* Send any files "batched" for sending prior this now
       * to ensure that the display order remains the same
       */

      if (!FileEntries() || (sendit=ripsendbatch(fDisplay)))
      {
        /* "play RIP scene" - note: icons are NOT supported here */
        /* You must send icons in batch without playback, then   */
        /* use them within RIP scenes                            */

        Printf("\r!|1R00000000%s\n", pszFile);
        sendit=TRUE;
      }
    }
  }

  /* We need to send them the file
   * Just queue it for now
   */

  else
  {

    /* Send all batched so far if we've run out of space */

    if (CanAddFileEntry() || (sendit=ripsendbatch(fDisplay)))
    {
      AddFileEntry(temp, FFLAG_NOBYTES, filesize);
      sendit=TRUE;
    }
  }

  return sendit ? SENDRIP_FILESENT : SENDRIP_ALREADYHAS;
}


int RIP_HasFile(char *pszFile, long *plFilesize)
{
  int   retries, rc;
  char  *stacked=NULL;

  if (!hasRIP())
    return -1;

  /* Preserve any stacked keys or pending input since we're going
   * to try talking to the remote terminal directly and we don't want
   * to short out any poked keys or type-ahead if it can be helped.
   */

  rc=strlen(linebuf);
  while (Mdm_keyp())
    linebuf[rc++]=(byte)Mdm_getcw();
  linebuf[rc  ]='\0';

  if (rc)
  {
    stacked=strdup(linebuf);
    Clear_KBuffer();
  }

  rc=-1;
  for (retries=0 ; rc==-1 && retries < RS_RETRIES_MAX; ++retries)
  {
    int x;

    /* Ask the remote system if it has the file */

    mdm_dump(DUMP_INPUT);
    Printf("\r!|1F0%c0000%s\n", plFilesize ? '2' : '0', pszFile);
    Mdm_flush();
    
    x=Mdm_kpeek_tic(1000);
    if (x != -1)            /* Throw away this key unless it timed out */
      Mdm_getcw();

    switch (x)
    {

      case '0':             /* Nope, remote doesn't have it */
        rc=FALSE;
        break;

      case '1':
        if (!plFilesize)    /* Yes he does, and we don't care about size */
        {
          rc=TRUE;
          break;
        }
                            /* Yes, but we still have to check the size */
        if (Mdm_kpeek_tic(200)=='.')
        {
          long fsize;
          char temp[31];

          /* Get the rest and parse it */

          Mdm_getcw();    /* Throw away '.' */
          Input(temp,INPUT_NLB_LINE|INPUT_NOECHO,0,sizeof temp-1,NULL);
          fsize=atol(temp);

          /* If we are just asked for the filesize,
           * then return it to the caller.
           */

          if (*plFilesize==-1L)  /* -1 means that size was requested */
          {
            *plFilesize=fsize;
            rc=TRUE;
          }

          /* Otherwise compare the given size and return the result */

          else
            rc=(*plFilesize==fsize);
        }
        break;

          /* Fallthru */
      default:
        logit(log_err_ripsend,retries+1);
        Delay(50);
        break;
    }
  }

  /* Crisis management:
   * After the maximum number of retrying file query, we have to abort.
   * Since the clueless user may have enabled RIP, we only allow this a
   * set number of times, then turn RIP off and tell him/her so
   */

  if (rc==-1)
  {
    logit(log_abort_ripsend);
    if (++ripsend_failcount >= MAX_RIPSENDFAIL)
    {

      /* Junk the stacked keys, who knows what's there */

      if (stacked)
      {
        free(stacked);
        stacked=NULL;
      }

      usr.bits &= ~BITS_RIP;
      Set_Lang_Alternate(FALSE);
      logit(log_disable_rip);
      no_local_output=FALSE;
      Puts(rip_disabled);

      /* Let's be sure that the user sees this and hasn't tried to get
       * some action during the query by thumping on their enter key
       */

      Clear_KBuffer();
      mdm_dump(DUMP_INPUT);
      Press_ENTER();

    }
  }

  /* Re-stack any previously stacked commands */

  if (stacked != NULL)
  {
    strcpy(linebuf, stacked);
    free(stacked);
  }

  logit("@RIP_HasFile(%s,%ld)=%d", pszFile, *plFilesize);

  /* Tell caller that we aborted RIP */

  return rc;
}


int RIP_SendFile(char *pszFile, int fDisplay)
{
  int   rc=TRUE;
  char *p2;

  if (!hasRIP())
    return SENDRIP_ERROR;

  logit("@rip_sendfile(%s,%s)", pszFile, fDisplay ? "true" : "false");

  /* Save current download tag list so that the user doesn't lose it */

  save_tag_list(NULL);
  Free_Filenames_Buffer(0);

  p2=pszFile;

  while (rc)
  {
    int   filegroup =0,
          forcesend =0;
    char *p1;

    p1=p2 + strspn(p2,cmd_delim);     /* This avoids use of strtok() */
    p2=p1 + strcspn(p1,cmd_delim);    /* which is not reentrant */

    if (*p2)
      *p2++ = '\0';

    if (!*p1)             /* At end of string */
      break;

    for (;;)
    {
      if (*p1=='@')       /* @filename is a list of files */
        ++filegroup;
      else if (*p1=='!')  /* !filename forces file(s) to be sent */
        ++forcesend;          /* even if we already know they have been */
      else
        break;
      ++p1;
    }

    /* Handle single file send */

    if (!filegroup)
    {
      int send_rc = sendrip(p1,fDisplay,forcesend);
      rc = (send_rc==SENDRIP_FILESENT || send_rc==SENDRIP_ALREADYHAS);
    }
    else
    {
      /* Handle multi-file send */

      FILE  *fp = shfopen(p1, fopen_read, O_RDONLY);

      if (fp==NULL)
      {
        cant_open(p1);
        rc=FALSE;
      }
      else
      {
        char  temp[PATHLEN];

        while (rc && fgets(temp,PATHLEN-1,fp)!=NULL)
        {
          char *s2;

          s2=temp;
          for (;;)
          {
            int force = forcesend;
            int send_rc;
            char *s1;

            s1=s2 + strspn(s2,cmd_delim);
            s2=s1 + strcspn(s1,cmd_delim);

            if (*s2)
              *s2++ = '\0';

            if (!*s1)
              break;

            if (*s1 == '!')
            {
              ++s1;
              ++force;
            }

            send_rc=sendrip(s1, fDisplay, force);
            rc = (send_rc==SENDRIP_FILESENT || send_rc==SENDRIP_ALREADYHAS);
          }
        }
        fclose(fp);
      }
    }
  }

  /* If we've got any files to send, send them all now */

  if (rc)
    ripsendbatch(fDisplay);

  /* Restore the user's download tag list */

  restore_tag_list(NULL,FALSE);

  return rc;
}


void RipClear()
{
  if (! *linebuf && hasRIP())
    Puts(rip_autoerase);
}

