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
#include "exec.h"

int cdecl do_spawn (int swapping,
              char *execfname,
              char *cmdtail,
              unsigned envlen,
              char *envp);

char *max_swap_filename = "test.swp";

int main()
{
  int rc;

  printf("ready to swap\n");

  rc = do_exec("command.com", "", USE_EMS | USE_XMS | USE_FILE,
               0xffff, NULL);

  printf("rc = %d\n", rc);
  printf("done swap\n");
  return 0;
}

