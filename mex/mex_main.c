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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: mex_main.c,v 1.4 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

#define MEX_INIT

#define MAX_INCL_VER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "prog.h"
#include "max.h"
#include "mex.h"

int sdebug=FALSE;
int mdebug=FALSE;


static void near usage(void)
{
  static char help[]=
    "Usage:\n\n"

    "mex [<args>...] <filename>\n\n"

    "<args> can be any of the following options:\n\n"

    "  -a       Show addresses instead of names in ASCII quad listing\n"
/*  "  -c       Emit subscript checking code\n"*/
    "  -h<size> Set heap size to <size> bytes\n"
    "  -s<size> Set stack size to <size> bytes\n"
    "  -q       Quad output instead of writing .VM file\n";

  puts(help);
  exit(0);
}

int yydebug;

int _stdc main(int argc,char **argv)
{
  char outfile[PATHLEN];
  long lStackSize;
  long lHeapSize;
  char **av;
  char *delim;
  char *dot;
  int ret;
  
  NW(argc);

  fprintf(stderr,
          "\nMEX  Maximus Extension Language Compiler  Version " VERSION "\n"
          "Copyright 1990, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

  lStackSize = DEFAULT_STACKSIZE;
  lHeapSize = DEFAULT_HEAPSIZE;

  if (argc < 2)
    usage();

  /* Process command-line arguments */

  for (av=argv+1; *av; av++)
  {
    if (**av=='-' || **av=='/')
    {
      switch ((*av)[1])
      {
        case 'a':
          show_addr=TRUE;
          break;

/*      case 'c':
          check_subs=TRUE;
          break;*/

        case 'd':
          if ((*av)[2]=='y')
            yydebug=TRUE;
          else if ((*av)[2]=='s')
            sdebug=TRUE;
          else if ((*av)[2]=='m')
            mdebug=TRUE;
          break;

        case 'h':
          lHeapSize = atol(*av + 2);
          break;

        case 's':
          lStackSize = atol(*av + 2);
          break;

        case 'q':
          vm_output=FALSE;
          break;

        default:
        case '?':
          usage();
          break;
      }
    }
    else if (eqstri(*av, "?"))
      usage();
    else
    {
      /* Doesn't begin with a '-' or '/', so it must be a filename */

      strcpy(filename, *av);
    }
  }


  #ifdef DMALLOC
  if (mdebug)
  {
    extern FILE *_stdc efile;

    dmalloc_on(FALSE);

    efile=fopen("dmalloc.log", "w");
  }
  #endif


  /* Add a .MEX extension, if none exists */

  delim=strrstr(filename, "\\/:");
  dot=strrchr(filename, '.');

  if (!dot || (delim && dot && dot < delim))
    strcat(filename, ".mex");

  if (!push_fstk(filename))
    exit(1);

  if (vm_output)
    open_vm(filename, lStackSize, lHeapSize, outfile);

  linenum=1L;
  scope=0;

  offset=maxoffset=0;
  glob_offset=maxgloboffset=GLOB_CONTROL_DATA;
  n_errors=n_warnings=0;


  /* Create main symbol table */

  symtab=st_create(SYMTAB_SIZE, NULL);


  /* Do the actual compilation */

  ret=yyparse();

  if (vm_output)
    close_vm();

  /* Destroy the symbol table, since it's no longer needed */

  st_destroy(symtab);

  if (n_errors)
    unlink(outfile);


  fprintf(stderr,
          "%sCompilation completed (%d errors, %d warnings).\n",
          n_errors + n_warnings ? "\n" : "",
          n_errors, n_warnings);

  return ret;
}



