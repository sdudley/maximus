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

#include <stdio.h>
#include <stdlib.h>
#include "prog.h"

/* Obtain the name of the current Maximus .prm file, either through a
 * command-line option or via the MAXIMUS environment variable.
 */

char *GetMaximus(int argc, char **argv, int iBadErrorlevel)
{
  char *pszMaximus = NULL;

  (void)argc;

  /* First search the command-line for an explicit '-p' override */

  if (argv)
  {
    while (*argv)
    {
      /* Ignore everything except -p options */

      if (**argv=='-' && (*argv)[1]=='p')
      {
        pszMaximus = *argv + 2;
        break;
      }

      argv++;
    }
  }

  /* If we couldn't get that, look for the environment variable */

  if (!pszMaximus && (pszMaximus = getenv("MAXIMUS"))==NULL)
  {
    printf("Error!  The 'MAXIMUS' environment variable could not be found.\n"
           "This variable must point to your main Maximus .PRM file.\n\n"

#ifdef OS_2
           "For example, the following can be added to your CONFIG.SYS:\n\n"
#else
           "For example, the following can be added to your AUTOEXEC.BAT:\n\n"
#endif
           "    SET MAXIMUS=C:\\MAX\\MAX.PRM\n");

    exit(iBadErrorlevel);
  }

  return pszMaximus;
}


