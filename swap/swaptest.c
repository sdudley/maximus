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
#include <string.h>
#include <errno.h>
#include <dos.h>
#include <process.h>

#include "swap.h"

/*
 *    This program is an example of how to use swap() to swap out the
 *    current program, execute another in its place, then restore the
 *    original program.  It should work with Turbo C (all versions),
 *    Turbo C++ 1.0, and Microsoft C 5.10.  It should work fine for the
 *    Small and Medium models of Microsoft C 6.00 (see SWAP.DOC)
 *
 */

int swap_return;
unsigned char exec_return;
unsigned char *comspec;
unsigned int dos_ptr;

main (int argc, char *argv[])
   {

   printf ("Hello--we are now in SWAPTEST.  We are about to execute a DOS shell.\n\n");

   if (!xms_installed())
      printf ("No ");
   printf ("XMS driver detected.\n");

   if (!ems4_installed())
      printf ("No ");
   printf ("EMS 4.0 driver detected.\n");


/*
 * Now allocate another DOS block for this program, to demonstrate
 *  new capability of SWAP 3.00 to swap all DOS blocks owned by a program
 *
 * Use _dos_allocmem() for Microsoft C 5.10 and 6.00,
 * use allocmem() for Turbo C++ 1.0 and Turbo C 2.0.
 * Both functions use same parameters.
 * SWAP.ASM must be assembled WITHOUT /DNoFrag for this to work.
 * Allocate a DOS block of 1024 paragraphs (16K bytes).
 */

#if 1
{
  char *p1;
  char *p2;
  char *p3;

  p1=malloc(50000);
  p2=malloc(50000);
  p3=malloc(50000);

  printf("p1=%x\n", p1);
  printf("p2=%x\n", p2);
  printf("p3=%x\n", p3);
}
#else

  #ifdef __TURBOC__
     allocmem (512u, &dos_ptr);
  #else
     _dos_allocmem (1024, (unsigned short *)&dos_ptr);
  #endif
#endif

  printf("\n**test without swapping!\n");

  {
    char *args[2]={"c:\\os2\\mdos\\command.com", NULL};
    int rc;

    rc=spawnvp(P_WAIT, args[0], args);

    printf("rc=%d, errno=%d, args='%s'\n",
           rc, errno, args[0]);
  }

   printf ("\n** Type EXIT to return to SWAPTEST **\n");

   comspec = getenv ("COMSPEC");

   swap_return = swap (comspec, "", &exec_return, "swaptest.fil");

   printf ("\n\nBack in SWAPTEST now.\n\n");

   switch (swap_return)
      {
      case SWAP_OK:        printf ("Successful, executed program returned %d.\n", (int)exec_return);
                           break;

      case SWAP_NO_SHRINK: printf ("Unable to shrink DOS memory block.\n");
                           break;

      case SWAP_NO_SAVE:   printf ("Unable to save program to memory or disk.\n");
                           break;

      case SWAP_NO_EXEC:   printf ("DOS EXEC call failed.  Error is %d: ", (int)exec_return);
                           switch (exec_return)
                              {
                              case BAD_FUNC:       printf ("Bad function.\n");                        break;
                              case FILE_NOT_FOUND: printf ("Program file not found.\n");              break;
                              case ACCESS_DENIED:  printf ("Access to program file denied.\n");       break;
                              case NO_MEMORY:      printf ("Insufficient memory to run program.\n");  break;
                              case BAD_ENVIRON:    printf ("Bad environment.\n");                     break;
                              case BAD_FORMAT:     printf ("Bad format.\n");                          break;
                              default:             printf ("Unexpected error code #%d (decimal).\n", (int)exec_return);
                                                   printf ("Consult DOS technical reference manual.\n");
                                                   break;

                              }
      }

   return (0);

   }

