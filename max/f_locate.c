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

/* $Id: f_locate.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=File area routines: L)ocate function
*/

#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <mem.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"
#include "max_menu.h"
#include "fb.h"
#include "display.h"



      
static sword near LocateDisplayJunk(byte *fnam, byte *desc, word *attr, char *nonstop, PFAH pfah, FDAT *dp)
{
  byte *descwork;
  char junk[10];
  sword ret;
  
  if ((descwork=malloc(MAX_FBBS_ENTRY+1))==NULL)
    return DRET_NOMEM;

  /* Reinsert the download counter, if there is one. */

  if (dp->times_dl || (dp->flag & FF_DLCTR))
  {
    /* [  12] */
    sprintf(junk, fbbs_counter, dp->times_dl);

    /* Copy this into the right place */

    strocpy(desc+strlen(junk), desc);
    memmove(desc, junk, strlen(junk));
  }

  ret=ShowFileEntry(attr, nonstop, (dp->flag & FF_OFFLINE),
                    &dp->fdate, &dp->udate, fnam, dp->fsize, TRUE,
                    desc, descwork, pfah);

  free(descwork);

  return ret;
}

      
static word near LocateAddColour(byte *fnam, byte *desc)
{
  int l;
  char *p, *after, *found;

  if ((found=stristr_nochin(fnam, searchfor))==NULL)
    found=stristr_nochin(desc, searchfor);

  if (!found)
    return FALSE;

  if ((prm.charset & CHARSET_CHINESE)==0)
  {
    cstrlwr(fnam);
    cstrlwr(desc);
  }

  /* Make room to insert the three 'normal attrib' bytes after the        *
   * found text.                                                          */

  after=found+strlen(searchfor);

  p=file_desc_col;
  l=strlen(p);
  strocpy(after+l, after);
  memmove(after,file_desc_col,l);

  /* Convert the found part to upper case */

  if ((prm.charset & CHARSET_CHINESE)==0)
    for (p=found; p < after; p++)
      *p=(char)toupper(*p);

  /* Now add the change-colour attributes just before this starts */

  p=file_found_col;
  l=strlen(p);
  strocpy(found+l, found);
  memmove(found, p, l);

  return TRUE;
}





static byte * near LocateSeekDesc(FDAT *dp, int dmp, byte *curdmp, dword *dmploc, word *dmpb)
{
  byte *desc;

  /* First, load the description from the dump file.  If the description    *
   * is too big to fit into one block that we've read in, adjust the block  *
   * to grab it.  If the descripton is before this block, after this block, *
   * or only partially in this block, then we need to readjust the file.    */

  if (dp->desc < *dmploc ||
      dp->desc >= *dmploc+DMP_BUF_SIZE-2 ||
      dp->desc+*(word *)(curdmp+(size_t)(dp->desc-*dmploc)) >=
                                             *dmploc+DMP_BUF_SIZE-2)
  {
    *dmploc=lseek(dmp, dp->desc, SEEK_SET);
    *dmpb=read(dmp, (char *)curdmp, DMP_BUF_SIZE);
  }


  /* Find the location of the description string in the heap plus two,      *
   * since the first word in the file is the length of the description.     */

  desc = curdmp + (size_t)(dp->desc-*dmploc) + sizeof(word);


  /* If it's out of bounds, there's nothing we can do. */

  return ((desc > curdmp + *dmpb) ? NULL : desc);
}






static sword near LocateShowFile(char *nstop, PFAH pfah, FDAT *dp, byte *curdmp, int dmp, dword *dmploc, word *dmpb, word *attr)
{
  byte *file_desc=LocateSeekDesc(dp, dmp, curdmp, dmploc, dmpb);
  byte *desc=NULL;
  byte *fnam, *p, *s;
  sword ret=-1;
  word showit=TRUE;


  /* Alloc memory to hold the new name/desc, since we may need to add       *
   * colour changing bytes to them.                                         */

  if ((desc=malloc(MAX_FBBS_ENTRY+10))==NULL)
    return DRET_NOMEM;
  
  if ((fnam=malloc(MAX_FBBS_ENTRY+10))==NULL)
  {
    free(desc);
    return DRET_NOMEM;
  }
  
  /* Copy the description */
  
  if (!file_desc)
    *desc='\0';
  else
  {
    strncpy(desc, file_desc, MAX_FBBS_ENTRY);
    desc[MAX_FBBS_ENTRY]='\0';
  }
  

  /* Copy the filename */
  
  strcpy(fnam, dp->name);


  /* Pad the filename out to 12 characters */

  for (s=fnam+strlen(fnam), p=fnam+MAX_FN_LEN; s < p; s++)
    *s=' ';

  /* Cap the string */

  *s='\0';
  

  /* Handle a search for text in the filename/desc */

  if (*attr & DISPLAY_SEARCH)
    if (! LocateAddColour(fnam, desc))
      showit=FALSE;

  if (showit)
    ret=LocateDisplayJunk(fnam, desc, attr, nstop, pfah, dp);
  
  free(fnam);
  free(desc);
  
  return ret;
}


/* This routine tries to locate the file's description in FILES.DMP, and    *
 * if found, it searches both the filename and description for the          *
 * specified string.                                                        */

static word near LocateSearchString(FDAT *dp, int dmp, byte *curdmp, dword *dmploc, word *dmpb)
{
  char *desc;
  
  desc=LocateSeekDesc(dp, dmp, curdmp, dmploc, dmpb);

  return (desc && 
          (stristr_nochin(dp->name, searchfor) ||
           stristr_nochin(desc, searchfor)));
}


static sword near LocateReadFdat(FDAT *da, int dat, int dmp, word attr, char *nstop, PFAH pfah)
{
  FDAT *dp, *dend;

  sword got;
  word doit;
  sword ret=0;

  byte *curdmp;
  dword dmploc;
  word dmpbytes;

  
  /* Initialize the buffer for the .DMP file */

  dmploc=(dword)-1L;
  dmpbytes=0;
  
  if ((curdmp=malloc(DMP_BUF_SIZE))==NULL)
    return DRET_NOMEM;

  while (ret==0 &&
         (got=read(dat, (char *)da, sizeof(FDAT)*MAX_SEARCH_DAT)) >=
                                                      sizeof(FDAT))
  {
    got /= sizeof(FDAT);
    
    if (halt())
    {
      ret=DRET_BREAK;
      break;
    }

    for (dp=da, dend=da+got; dp < dend; dp++)
    {
      if (dp->flag & (FF_COMMENT|FF_DELETED))
        continue;

      doit=FALSE;

      if ((attr & DISPLAY_NEWFILES) && GEdate(&dp->udate, &new_date) &&
          (dp->flag & FF_OFFLINE)==0)
        doit=TRUE;
      else if ((attr & DISPLAY_SEARCH) &&
               LocateSearchString(dp, dmp, curdmp, &dmploc, &dmpbytes))
        doit=TRUE;

      if (doit)
      {
        matches++;

        ret=LocateShowFile(nstop, pfah, dp, curdmp, dmp, &dmploc, &dmpbytes,
                           &attr);
      }
      
      if (ret != 0)
        break;
    }
  }
  
  free(curdmp);
  
  return ret;
}
      


static sword near LocateSearchFdat(byte *fdat, byte *fdmp, word attr, char *nstop, PFAH pfah)
{
  int dat, dmp;
  FDAT *da;
  sword ret=0;
  
  if (halt())
    return DRET_BREAK;

  if ((dat=shopen(fdat, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    return DRET_OK;
  
  if ((dmp=shopen(fdmp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    close(dat);
    return DRET_OK;
  }
  
  if ((da=malloc(sizeof(FDAT) * MAX_SEARCH_DAT)) != NULL)
  {
    ret=LocateReadFdat(da, dat, dmp, attr, nstop, pfah);
    free(da);
  }
  
  close(dmp);
  close(dat);
  return ret;
}



/* Search one area for the specified file(s) */

static sword near LocateSearchArea(word attr, char *nstop, PFAH pfah,
                                   byte colour)
{
  byte fbbs[PATHLEN];
  byte fdmp[PATHLEN];
  byte fdat[PATHLEN];
  byte *delim, *dot;

  /* Skip this area if the "nonew" flag is set */

  if ((attr & DISPLAY_NEWFILES) && (pfah->fa.attribs & FA_NONEW))
    return DRET_OK;

  /* Set area name to this area */

  strcpy(usr.files, PFAS(pfah, name));

  /* Standard attributes for displaying file list */

  attr |= DISPLAY_FILESBBS | DISPLAY_AREANAME;

  /* Print a little banner for this area */

  Printf(srchng, (char)((colour % 7)+9), PFAS(pfah, name));
  vbuf_flush();


  /* Check for ^c */
  
  if (halt())
    return DRET_BREAK;

  
  /* Figure out the name of the files.bbs-like file for this area */

  if (*FAS(*pfah, filesbbs))
    strcpy(fbbs, FAS(*pfah, filesbbs));
  else
    sprintf(fbbs, ss, FAS(*pfah, downpath), sfiles);


  /* Now make files.dat the same name, but with a .DAT extension */

  strcpy(fdat, fbbs);

  /* Find the last delimiter in the path */
  
  delim=strrstr(fdat, pdel_only);
  
  
  /* Find the last dot in the path */
  
  dot=strrchr(fdat, '.');
  
  
  /* Ace the dot, if it follows the path */
  
  if (dot && (!delim || dot > delim))
    *dot='\0';


  /* Copy the pruned index to the FDMP name too */

  strcpy(fdmp, fdat);

  strcat(fdmp, dot_dmp);
  strcat(fdat, dot_dat);

  if (fexist(fdat))
    return (LocateSearchFdat(fdat, fdmp, attr, nstop, pfah));
  else
  {
    int rc = DRET_OK;

    if (PushFileAreaSt(pfah, NULL))
    {
      rc = Display_File(attr, nstop, fbbs);
      PopFileArea();
    }

    return rc;
  }
}





/* Search all file areas for certain files */

void File_Locate(void)
{
  FAH fa;
  BARINFO bi, biSave;
  HAFF haff;

  byte savearea[MAX_ALEN];
  byte *s;

  byte colour;
  char nonstop;

  int ret;

  memset(&fa, 0, sizeof fa);

  /* Save the user's priv level */

  if (! *linebuf)
    Puts(loc_banner);

  do
  {
    if (! *linebuf)
      Puts(note_helpnf);

    InputGetsLL(searchfor, BUFLEN, loc_file);

    if (! *searchfor)
      return;
    else if (eqstr(searchfor, qmark))
    {
      Display_File(0, NULL, PRM(hlp_locate));
      *searchfor='\0';
      continue;
    }

    if (((s=strchr(searchfor, '*')) != NULL && s != searchfor) || strchr(searchfor, '?'))
    {
      Dont_Use_Wildcards(ze_loc_cmd);
      *searchfor='\0';
      continue;
    }
  }
  while (! *searchfor);

  nonstop=FALSE;
  matches=0;

  if (*searchfor=='*')
  {
    WhiteN();

    if ((s=firstchar(searchfor+1, cmd_delim, 1)) != NULL && isdigit(*s))
      strcpy(linebuf, s);

    if (Get_New_Date(&new_date, &date_newfile, dtsf)==-1)
      return;

    display_line=display_col=1;
    strcpy(searchfor, "*");

    /* Store the time of this newfiles search */

    Get_Dos_Date(&usr.date_newfile);
  }
  else
  {
    display_line=display_col=1;
    Printf(searchingfor, searchfor, '\0');
    logit(log_searchingfor, searchfor);
  }




  first_search=TRUE;
  
  if ((haff=AreaFileFindOpen(haf, NULL, 0))==NULL)
    return;

  colour=0;
  ret=0;

  biSave.priv=usr.priv;
  biSave.keys=usr.xkeys;
  strcpy(savearea, usr.files);

  for (colour=ret=0;
       !halt() && AreaFileFindNext(haff, &fa, FALSE)==0 &&
       ret != DRET_BREAK && ret != DRET_NOMEM;
       colour++)
  {

    /* Exclude divisions, inaccesible areas or areas which are aliases */

    if (!ValidFileArea(NULL, &fa, VA_VAL | VA_PWD | VA_EXTONLY, &bi) ||
        (fa.fa.attribs & FA_NOINDEX))
    {
      continue;
    }

    Mdm_check();

    ret=LocateSearchArea(*searchfor=='*' ? DISPLAY_NEWFILES : DISPLAY_SEARCH,
                         &nonstop, &fa, colour);

    first_search=FALSE;
  }
  
  Puts(space_over);

  Printf(located, matches, matches==1 ? blank_str : pl_match);

  /* Restore current area and priv level */

  strcpy(usr.files, savearea);
  usr.priv=biSave.priv;
  usr.xkeys=biSave.keys;
}



