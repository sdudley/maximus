/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/***************************************************************************/
/*                                                                         */
/*            DW-TEXT: Simple double linked list manager                   */
/*                     .................................                   */
/*                                                                         */
/*            Created: 20/Aug/90                                           */
/*            Updated: 20/Apr/97                                           */
/*                                                                         */
/*            Copyright (C) 1990-1997 JV DIALOGUE                          */
/*            Written by Pete I. Kvitek                                    */
/*                                                                         */
/***************************************************************************/

#include "dw-lst.h"

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   s p e c i f i c   d e c l a r a t i o n s                 //
/////////////////////////////////////////////////////////////////////////////

  // Make up the compiler dependant embedded assembler code prefix

#ifndef __FLAT__
#ifndef ASM
#if defined (__TURBOC__)
#define ASM asm
#elif defined (_MSC_VER)
#define ASM _asm
#endif
#endif
#endif

//#undef ASM

  // Set up compiler optimization

#ifdef __TURBOC__
#pragma option -r+
#pragma option -Z
#pragma option -G
#endif

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   l o c a l   s u b r o u t i n e s                         //
/////////////////////////////////////////////////////////////////////////////

  // None so far ...

/////////////////////////////////////////////////////////////////////////////
// P u b l i c   r o u t i n e s                                           //
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************
* This routine creates the new list element with the given size
*/

  PLE APIENTRY LstCreateElement(USHORT cb)
  {
    return (cb >= sizeof(LE)) ? MemAlloc(cb, MA_CLEAR) : NULL;
  }

/***************************************************************************
* This routine destroys the given list element
*/

  VOID APIENTRY LstDestroyElement(PLE ple)
  {
    if (ple != NULL) MemFree(ple);
  }

/***************************************************************************
* This routine links element at the given position assuming null references
*/

  USHORT APIENTRY LstLinkElement(PPLE pple1st, PLE ple, USHORT ile)
  {
    USHORT ileReturn = 0;
    PLE pleAfter;

    //////////////////////////////////
    // Check if no elements in list //
    //////////////////////////////////

    if (*pple1st == NULL) {

      // Insert very first element

      if (ile != 0 && ile != LST_END) return LST_ERROR;

      *pple1st = ple;                           // Pointer to first

    } else

    ///////////////////////////////////
    // Check if inserting at the end //
    ///////////////////////////////////

    if (ile == LST_END) {

      // Scan to the end of the list

      pleAfter = *pple1st;

#ifndef ASM

      while (ileReturn++, pleAfter->pleNext != NULL)
        pleAfter = pleAfter->pleNext;

#else
        // Init element counter and pointer

ASM     xor     cx, cx
ASM     les     bx, pleAfter

        // Increment element counter and check if next pointer is null
Loop1:
ASM     inc     cx
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next1
ASM     or      dx, dx
ASM     jz      Done1

        // Advance to the next element
Next1:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop1

        // Store element counter and pointer
Done1:
ASM     mov     ileReturn, cx
ASM     mov     word ptr pleAfter, bx
ASM     mov     word ptr pleAfter[2], es

#endif

      // Update links

      pleAfter->pleNext = ple;          // Prev to this
      ple->plePrev = pleAfter;          // This to prev

    } else

    /////////////////////////////////////////
    // Check if inserting at the beginning //
    /////////////////////////////////////////

    if (ile == 0) {

      // Insert at the beginning of the elements list
      // Note that there should be at least one element
      // in the list due to the previous logic

      ple->pleNext = *pple1st;          // This to next
      ple->pleNext->plePrev = ple;      // Next to this
      *pple1st = ple;                   // Pointer to first

    } else {

    ////////////////////////////////////
    // Otherwize insert in the middle //
    ////////////////////////////////////

      // Insert at the middle of the elements list.
      // Note that this may be the last element as well
      // and there is at list one element in list

      pleAfter = *pple1st;

#ifndef ASM

      while (++ileReturn != ile)
        if ((pleAfter = pleAfter->pleNext) == NULL)
          return LST_ERROR;
#else
        // Init element counter and pointer

ASM     xor     cx, cx
ASM     mov     di, ile
ASM     les     bx, pleAfter

        // Increment element counter and compare to the target
Loop2:
ASM     inc     cx
ASM     cmp     cx, di
ASM     je      Done2

        // Load the next pointer and check if null

ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next2
ASM     or      dx, dx
ASM     jz      Fail2

        // Advance to the next element
Next2:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop2

        // Invalid index error

Fail2:  return  LST_ERROR;

        // Store element counter and pointer
Done2:
ASM     mov     ileReturn, cx
ASM     mov     word ptr pleAfter, bx
ASM     mov     word ptr pleAfter[2], es

#endif

      // Update links

      ple->pleNext = pleAfter->pleNext; // This to next
      pleAfter->pleNext = ple;          // Prev to this
      ple->plePrev = pleAfter;          // This to prev
      if (ple->pleNext != NULL)         // If not the last element
        ple->pleNext->plePrev = ple;    // Next to this
    }

    return ileReturn;
  }

/***************************************************************************
* This routine unlinks the given element from list
*/

  PLE APIENTRY LstUnlinkElement(PPLE pple1st, USHORT ile)
  {
    PLE ple = *pple1st;

    // Check for trivial case: empty list

    if (ple == NULL)
      return NULL;

    // Check if we have to unlink last element
    // Note that this can be the first element as well

    if (ile == LST_END) {

#ifndef ASM

      while (ple->pleNext != NULL)      // Get to the last element
        ple = ple->pleNext;

#else
        // Init element pointer

ASM     les     bx, ple

        // Load next pointer and check if null
Loop1:
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next1
ASM     or      dx, dx
ASM     jz      Done1

        // Advance to the next element
Next1:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop1

        // Store element pointer
Done1:
ASM     mov     word ptr ple, bx
ASM     mov     word ptr ple[2], es

#endif

      // Update links

      if (ple->plePrev != NULL)         // If not the 1st element
        ple->plePrev->pleNext = NULL;   //   fix prev's next ptr;
      else                              //   otherwise reset ptr to
        *pple1st = NULL;                //   the first el in list

    } else {

      // Unlink the middle element. Note that it may be
      // also the last or the first element in list

#ifndef ASM

      while (ile--)                             // Get to the element
        if ((ple = ple->pleNext) == NULL)
          return NULL;

#else
        // Init element counter and pointer

ASM     les     bx, ple
ASM     mov     cx, ile

        // Check element counter not zero and decrement it
Loop2:
ASM     or      cx, cx
ASM     jz      Done2
ASM     dec     cx

        // Load next pointer and check if null
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next2
ASM     or      dx, dx
ASM     jz      Fail2

        // Advance to the next element
Next2:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop2

        // Invalid index error

Fail2:  return  NULL;

        // Store element pointer
Done2:
ASM     mov     word ptr ple, bx
ASM     mov     word ptr ple[2], es

#endif

      // Update links

      if (ple->plePrev == NULL)                 // If first el in list,
        *pple1st = ple->pleNext;                //   fix pointer to first
      else                                      //   otherwise;
        ple->plePrev->pleNext = ple->pleNext;   //   point prev to next link;

      if (ple->pleNext != NULL)                 // If not the last,
        ple->pleNext->plePrev = ple->plePrev;   //   point next to prev link
    }

    // Reset references so we can link it in later

    ple->plePrev = NULL;
    ple->pleNext = NULL;

    return ple;
  }

/***************************************************************************
* This routine returns the given list element pointer
*/

  PLE APIENTRY LstElementFromIndex(PLE ple, USHORT ile)
  {
    if (ile == LST_END) {

      // Look for the last element in list

#ifndef ASM

      if (ple != NULL)
        while (ple->pleNext != NULL)
          ple = ple->pleNext;

#else
        // Check if null head

        if (ple == NULL) return NULL;

        // Init element pointer

ASM     les     bx, ple

        // Load next pointer and check if null
Loop1:
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next1
ASM     or      dx, dx
ASM     jz      Done1

        // Advance to the next element
Next1:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop1

        // Store element pointer
Done1:
ASM     mov     word ptr ple, bx
ASM     mov     word ptr ple[2], es

#endif

    } else {

      // Look for the requested element

#ifndef ASM

      while (ple != NULL)
        if (ile--)
          ple = ple->pleNext;
        else
          break;

#else
        // Check if null head

        if (ple == NULL) return NULL;

        // Init element counter and pointer

ASM     les     bx, ple
ASM     mov     cx, ile

        // Check element counter not zero and decrement it
Loop2:
ASM     or      cx, cx
ASM     jz      Done2
ASM     dec     cx

        // Load next pointer and check if null
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next2
ASM     or      dx, dx
ASM     jz      Fail2

        // Advance to the next element
Next2:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop2

        // Invalid index error

Fail2:  return  NULL;

        // Store element pointer
Done2:
ASM     mov     word ptr ple, bx
ASM     mov     word ptr ple[2], es

#endif
    }

    return ple;
  }

/***************************************************************************
* This routine the given list element index
*/

  USHORT APIENTRY LstIndexFromElement(PLE ple1st, PLE ple)
  {
    USHORT ile;

    // Check if null head

    if (ple1st == NULL) return LST_ERROR;

    // Locate the requested element

#ifndef ASM

    for (ile = 0; ple1st != NULL; ple1st = ple1st->pleNext, ile++)
      if (ple1st == ple)
        return ile;

    return LST_ERROR;

#else
        // Init element counter and pointer

ASM     les     bx, ple1st
ASM     xor     cx, cx

        // Init target and first time pointer

ASM     mov     di, word ptr ple
ASM     mov     si, word ptr ple[2]
ASM     mov     ax, bx
ASM     mov     dx, es

        // Check if match found
Loop:
ASM     cmp     di, ax
ASM     jne     Next
ASM     cmp     si, dx
ASM     je      Done

        // Increment counter, load next pointer and check if null
Next:
ASM     inc     cx
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Skip
ASM     or      dx, dx
ASM     jz      Fail

        // Advance to the next element
Skip:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop

        // Not found
Fail:   return LST_ERROR;

        // Store and return element index
Done:
ASM     mov     ile, cx
        return ile;

#endif
  }

/***************************************************************************
* This routine returns number of elements in list
*/

  USHORT APIENTRY LstQueryElementCount(PLE ple)
  {
    USHORT ile;

#ifndef ASM

    for (ile = 0; ple != NULL; ple = ple->pleNext, ile++);

#else
        // Check if null head

        if (ple == NULL) return 0;

        // Init element counter and pointer

ASM     les     bx, ple
ASM     xor     cx, cx

        // Increment counter, load next pointer and check if null
Loop:
ASM     inc     cx
ASM     mov     ax, word ptr es:[bx + 4]
ASM     mov     dx, word ptr es:[bx + 6]
ASM     or      ax, ax
ASM     jnz     Next
ASM     or      dx, dx
ASM     jz      Done

        // Advance to the next element
Next:
ASM     mov     bx, ax
ASM     mov     es, dx
ASM     jmp     Loop

        // Store element count
Done:
ASM     mov     ile, cx

#endif

    return ile;
  }

/***************************************************************************
* End of LST-MNGR.C                                                        *
****************************************************************************/

