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

#include "prog.h"

/* qsort  --  ANSI-compatible quicksort function
**
** by Ray Gardner        Englewood, Colorado              May 1985
** Placed in the public domain by Ray Gardner       September 1986
** revised by Ray Gardner                                  8/27/87
**      to fix bug in inner loop of insertion sort
**
** Usage: qsort(base, nbr_elements, width_bytes, compare_function);
**         char *base;
**         unsigned int nbr_elements, width_bytes;
**         int (*compare_function)();
**
** Sorts an array starting at base, of length nbr_elements, each
** element of size width_bytes, ordered via compare_function; which
** is called as  (*compare_function)(ptr_to_element1, ptr_to_element2)
** and returns < 0 if element1 < element2, 0 if element1 = element2,
** > 0 if element1 > element2.  This is compatible with qsort in ANSI
** C standard.  Most of the refinements are due to R. Sedgewick.
** See "Implementing Quicksort Programs", Comm. ACM, Oct. 1978, and
** Corrigendum, Comm. ACM, June 1979.
*/


static void cswap ( char *, char *, unsigned );

#define SWAP(a,b) (cswap((a),(b),width))

#define  T   9          /* subfiles <= T elements are insertion sorted */
                        /* T must be >= 3 */

/* compatible with ANSI qsort()   */
void _fast iqsort (char *base, unsigned int nel, unsigned int width,int (_stdc *comp)(void *,void *))
{
   char *stack[40], **sp;            /* stack and stack pointer        */
   char *i, *j, *limit;              /* scan and limit pointers        */
   unsigned thresh;                  /* size of T elements in bytes    */

   thresh = T * width;               /* init threshold                 */
   sp = stack;                       /* init stack pointer             */
   limit = base + nel * width;       /* pointer past end of array      */
   for ( ;; ) {                      /* repeat until break...          */
      if ( (unsigned)(limit - base) > thresh ) { /*if more than T items, swap mid  */
         SWAP(((((int)(limit-base))/width)/2)*width+base, base);  /* with base*/
         i = base + width;           /* i scans from left to right     */
         j = limit - width;          /* j scans from right to left     */
         if ( (*comp)(i, j) > 0 )    /* Sedgewick's                    */
            SWAP(i, j);              /*    three-element sort          */
         if ( (*comp)(base, j) > 0 ) /*        sets things up          */
            SWAP(base, j);           /*            so that             */
         if ( (*comp)(i, base) > 0 ) /*              *i <= *base <= *j */
            SWAP(i, base);           /* *base is the pivot element     */
         for ( ;; ) {                /* loop until break               */
            do                       /* move i right until *i >= pivot */
               i += width;
            while ( (*comp)(i, base) < 0 );
            do                       /* move j left until *j <= pivot  */
               j -= width;
            while ( (*comp)(j, base) > 0 );
            if ( i > j )             /* break loop if pointers crossed */
               break;
            SWAP(i, j);            /* else swap elements, keep scanning*/
         }
         SWAP(base, j);              /* move pivot into correct place  */
         if ( j - base > limit - i ) {/* if left subfile is larger...  */
            sp[0] = base;            /* stack left subfile base        */
            sp[1] = j;               /*    and limit                   */
            base = i;                /* sort the right subfile         */
         } else {                    /* else right subfile is larger   */
            sp[0] = i;               /* stack right subfile base       */
            sp[1] = limit;           /*    and limit                   */
            limit = j;               /* sort the left subfile          */
         }
         sp += 2;                    /* increment stack pointer        */
      } else {                       /* else use insertion sort        */
         for ( j = base, i = j + width; i < limit; j = i, i += width )
            for ( ; (*comp)(j, j+width) > 0; j -= width ) {
               SWAP(j, j+width);
               if ( j == base )
                  break;
            }
         if ( sp != stack ) {        /* if any entries on stack...     */
            sp -= 2;                 /* pop the base and limit         */
            base = sp[0];
            limit = sp[1];
         } else                      /* else stack empty, all done     */
            break;
      }
   }
}

static void cswap(char *a,char *b,unsigned k)          /* swap chars */
{
  char tmp;

  do
  {
    tmp = *a;
    *a++ = *b;
    *b++ = tmp;
  } while (--k);
}

