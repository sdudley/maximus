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
static char rcs_id[]="$Id: max_bar.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Barricade-handling routines
*/

#define MAX_LANG_m_area

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <mem.h>
#include "alc.h"
#include "prog.h"
#include "mm.h"
#include "max_area.h"
#ifdef UNIX
#include <errno.h>
#endif

/* ParseBarPriv
 *
 * Returns TRUE if the privilege level indicates that the user
 * is to be given access to the barricaded area.
 */

static int near ParseBarPriv(char *szPriv, BARINFO *pbi)
{
  word priv;

  if (! *szPriv)
  {
    pbi->priv=usr.priv;
    pbi->keys=usr.xkeys;
    pbi->use_barpriv=FALSE;
    return TRUE;
  }

  if (eqstri(szPriv, "NoAccess"))
    priv=(word)-1;
  else
    priv=ClassAbbrevLevel(szPriv);

  pbi->priv=priv;

  /* Invalid priv level (like "noaccess"), so don't grant access */

  if (priv==(word)-1)
    return FALSE;

  pbi->keys |= SZKeysToMask(szPriv);
  pbi->use_barpriv=TRUE;
  return TRUE;
}


/* ProcessExtBar
 *
 * Returns TRUE if we found a line that matches the current user's name;
 * FALSE otherwise.
 */

static int near ProcessExtBar(char *line, BARINFO *pbi)
{
  char szPriv[PATHLEN];
  char name[PATHLEN];

  getword(line, name, ctl_delim, 1);

  if (eqstri(Strip_Underscore(name), usr.name) || eqstri(name, usr.alias))
  {
    getword(line, szPriv, ctl_delim, 2);
    ParseBarPriv(szPriv, pbi);
    return TRUE;
  }

  if (eqstri(name, all))
  {
    getword(line, szPriv, ctl_delim, 2);

    if (*szPriv)
      ParseBarPriv(szPriv, pbi);

    return TRUE;
  }

  return FALSE;
}


/* GetBarPwd
 *
 * Prompt the user to enter a password for access to this file/msg area.
 */

static void near GetBarPwd(char *pwd)
{
  if (! *linebuf)
    Display_File(0, NULL, PRM(barricade));

  InputGetse(pwd, '.', bar_access);
  display_line=display_col=1;
  cfancy_str(pwd);
}


/* BadBarPwd
 *
 * Display an error to the user if an incorrect barricade password
 * was entered.
 */

static void near BadBarPwd(char *pwd, char *name, int tries)
{
  logit(log_bad_bar_pwd, name, pwd);

  Printf(wrong_pwd, tries);
  Putc('\n');

  if (tries==3)
  {
    logit(l_invalid_pwd);
    Puts(invalid_pwd);
    ci_barricade();
    mdm_hangup();
  }
}


/* ProcessBarFile
 *
 * Process the barricade file specified by 'fp'.
 *
 * 'pbi' will be updated with information pertaining to the user's new
 * access level.
 *
 * 'name' is the name of the message area (to be displayed in error
 * messages).
 *
 * 'ext_only' is true if we are to only consider the effects of the
 * extended barricade lines (and to pretend that the user has no
 * access if the extended barricade fails).  When this is true, Maximus
 * will never prompt the user to enter a password.
 *
 * This function returns TRUE if the user is to be given access to
 * the specified area.
 */

static int near ProcessBarFile(FILE *fp, BARINFO *pbi, char *name, int ext_only)
{
  char line[PATHLEN];
  char pwd[PATHLEN];
  int asked_pwd;
  int tries=0;

  do
  {
    fseek(fp, 0L, SEEK_SET);
    asked_pwd=FALSE;

    while (fgets(line, PATHLEN, fp) != NULL)
    {
      Strip_Trailing(line, '\n');

      if (*line=='!')
      {
        if (ProcessExtBar(line+1, pbi))
          return (pbi->priv != (word)-1);
      }
      else if (*line && !ext_only)  /* ask for a pwd only if ext_only is false */
      {
        char try_pwd[PATHLEN];
        char priv[PATHLEN];

        if (!asked_pwd)
        {
          asked_pwd=TRUE;
          GetBarPwd(pwd);
        }

        getword(line, try_pwd, ctl_delim, 1);

        if (eqstri(pwd, try_pwd))
        {
          getword(line, priv, ctl_delim, 2);

          if (*priv)
            return ParseBarPriv(priv, pbi);

          return TRUE;
        }
      }
    }

    if (asked_pwd && *pwd)
      BadBarPwd(pwd, name, ++tries);
  }
  while (asked_pwd && *pwd);

  /* No matching pwd */

  return FALSE;
}


/* GetBarPriv
 *
 * Get the user's virtual privilege level for a specified barricade file.
 *
 * 'is_msg' is true if processing a message barricade; otherwise a file
 * barricade.
 *
 * 'pmah' and 'pfah' are the area handles for the appropriate msg/file area.
 *
 * 'pbi' is updated with information about the user's virtual privilege
 * level.
 *
 * 'ext_only' tells this function to only consider extended barricades.
 * See the description of this parameter in ProcessBarFile for more info.
 *
 * This function returns TRUE if the user is allowed to access the
 * specified area.
 */

int GetBarPriv(char *barfile, int is_msg, PMAH pmah, PFAH pfah, BARINFO *pbi, int ext_only)
{
  FILE *fp;
  int rc;

  /* Initialize barricade structure to zeroes */

  if (pbi)
    memset(pbi, 0, sizeof *pbi);

  if ((fp=shfopen(barfile, fopen_read, O_RDONLY | O_NOINHERIT))==NULL)
  {
    logit(cantopen, barfile, errno);
    return FALSE;
  }

  rc=ProcessBarFile(fp, pbi, is_msg ? PMAS(pmah, name) : PFAS(pfah, name),
                    ext_only);

  fclose(fp);
  return rc;
}

