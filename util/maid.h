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

/* TEST.MAD:


:Hunk MAX_INIT
located="";
log_begin_mt="+Begin, v%s";

:Hunk GLOBAL

*/

#if 0
  MAID: MAximus International Definitions specification:

In USER.DEF / SYSOP.DEF (sysop-editable):

  item="text" \
       "moretext"
  item2="second text item"
         
Above file is fed into MAID compiler.  Output of MAID, ASCII text, one to a
line, into MAIDOUT.H:

      MAID(item,"text""moretext")
      MAID(item2,"second text item")

In MAX.H:  (We can define INTERNAL_STRING, to make all of the strings
            internal.  Once we've got the external compiler working, we
            just #undef the INTERNAL_STRING variable, and we're all
            set to go with the multilingulal version.)  */


#ifdef INTERNAL_STRING

  #define MAID(i,t) extrn char *i IS(t);
  #define s(i)      (i)
    
  /* The MAID() macros in MAIDOUT.H just translate into the "extrn char"    *
   * calls mentioned above, which is what we want.                          */
    
  #include "maidout.h"

#else /* !INTERNAL_STRING */


  char *maid_heap;   /* Pointer to table of NULL-terminated string, like    *
                      * the .PRM file.                                      */

  char maid_ofs[MAX_OFS]; /* Array of integer pointers to strings in        *
                           * maid_heap.                                     */

  /* THe s() macro can be used just as before, except without               *
   * INTERNAL_STRING defined, it'll access the MAID heap instead.           */
  
  #define s(i)     (maid_heap+maid_ofs[i])

  /* This part creates an enumeration type for the strings, so that we can  *
   * use the same variable identifiers as before, and not have to define it *
   * separately.  This is the value for the 'i' which is passed to the      *
   * s() function, above.                                                   */

  #define MAID(i,t)  (i),

  enum _maid_str
  {
    #include "maidout.h"
    0xffff /* Last item can't have a trailing "," */
  };

  #ifdef MAID_COMPILER
    
    /* #defining MAID_COMPILER signifies that the program is doing the      *
     * compiling of the *.DEF files.                                        */
    
    struct _mcomp
    {
      char *token;
      int val;
    };
    
    /* If MAID_COMPILER is defined, then we also create an array of structs,*
     * which contains the "stringized" form of the string-name, and the     *
     * enum value, as defined above.                                        */
    
    #define MAID(i,t) {#i,i},
      
    struct _mcomp[]=
    {
      #include "maidout.h"
      {0xffff, "___dummy___"} /* Last item can't have trailing "," */
    };
    
  #endif
#endif

#undef MAID

#endif


