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
static char rcs_id[]="$Id: vm_heap.c,v 1.3 2003/12/14 17:40:19 paltas Exp $";
#pragma on(unreferenced)

#define HEAP_PROBLEMS
#define HEAP_SIGNATURE
#define COMPILING_MEX_VM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "vm.h"


void hpinit(void)
{
  pdshDheap=(struct _dsheap *)(pbDs + vmh.lGlobSize + vmh.lStackSize);
#ifdef HEAP_SIGNATURE
  pdshDheap->sig = DSHEAP_SIG;
#endif
  pdshDheap->size=vmh.lHeapSize - sizeof(struct _dsheap);
  pdshDheap->next=END_HEAP;
  pdshDheap->free=TRUE;
  pdshDheap->rsvd=0;
}

VMADDR hpalloc(word len)
{
  struct _dsheap *dh, *fp, *new;
  VMADDR size;
  
#ifdef HEAP_PROBLEMS
  debheap=1;
  if (len >= 6000)
  {
    printf("asdf\n");
  }

  if (hpcheck() != 0)
  {
    printf(__FUNCTION__ " bar\n");
  }
#endif

  for (dh=pdshDheap;
       dh;
       dh=(struct _dsheap *)(pbDs+dh->next))
  {
    /* Only work on free blocks */
    
    if (dh->free)
    {
      /* If this block is smaller than the one we need, see if we can join  *
       * any following blocks to make up for it.                            */
         
      if (dh->size < len)
      {
        size=dh->size;
        fp=dh;

        if (dh->next==END_HEAP)
          break;

        /* Walk the list of blocks following this one in the heap, and      *
         * count how much space they have free.                             */
           
        while (fp &&
               fp->next != END_HEAP &&
               ((struct _dsheap *)(pbDs+fp->next))->free &&
               size < len)
        {
          fp=(struct _dsheap *)(pbDs+fp->next);

          /* Increment the amount of space we received by the length of   *
           * this block, including the _dsheap header chunk.              */

          size += fp->size+sizeof(struct _dsheap);
        }
            
        /* If we have enough space, merge these blocks together */
          
        if (size < len)
          continue;
        else
        {
          dh->size=size;
          dh->next=fp->next;
        }
      }

      /* If there's enough space here to consider lumping this block into   *
       * two pieces...                                                      */

      if (dh->size >= len+sizeof(struct _dsheap)+8)
      {
        /* The new block is located after this header, and after the data   *
         * which follows this header.                                       */

        new=(struct _dsheap *)((byte *)dh + sizeof(struct _dsheap) + len);
        
        
        /* Subtract the size that we're using */
        
        new->size=(dh->size-len-sizeof(struct _dsheap));
        new->next=dh->next;
        new->rsvd=0;
        new->free=TRUE;
#ifdef HEAP_SIGNATURE
        new->sig=DSHEAP_SIG;
#endif

        dh->size=len;
        dh->next=((byte *)new-pbDs);
      }

      dh->free=FALSE;
      dh->rsvd=0;
#ifdef HEAP_SIGNATURE
      dh->sig = DSHEAP_SIG;
#endif

      /* Return the number of the dataseg location which immediately        *
       * follows the block header.                                          */

      #ifdef DEBUGVM
      if (debheap)
        printf("%08lx - hpalloc(%d) from %lx\n",
               (long)((byte *)(dh+1)-pbDs),
               len,
               (long)vaIp);
      #endif

      return ((byte *)(dh+1)-pbDs);
    }
    
    if (dh->next==END_HEAP)
      break;
  }
    
  return END_HEAP;
}



void hpfree(VMADDR ofs)
{
  struct _dsheap *dh;
  
#ifdef HEAP_PROBLEMS
  if (hpcheck() != 0)
  {
    printf(__FUNCTION__ " foo\n");
  }
#endif

  dh=(struct _dsheap *)(pbDs+ofs)-1;
  
  if (dh->free || dh->size==0
#ifdef HEAP_SIGNATURE
   || dh->sig != DSHEAP_SIG
#endif
    )
    vm_err("Invalid hpfree(%08lx)", ofs);
  
  dh->free=TRUE;

  #ifdef DEBUGVM
  if (debheap)
    printf("%08lx - hpfree() from %" UINT32_XFORMAT ")\n", ofs, (long)vaIp);
  #endif
}


#ifdef HEAP_PROBLEMS

int hpcheck(void)
{
  struct _dsheap *dh;
  
  for (dh=pdshDheap;
       dh;
       dh=(struct _dsheap *)(pbDs+dh->next))
  {
    /* If pointer out of the heap */

    if ((byte *)dh - pbDs >= vmh.lGlobSize + vmh.lStackSize + vmh.lHeapSize)
      return -1;

#ifdef HEAP_SIGNATURE
    if (dh->sig != DSHEAP_SIG)
      return -2;
#endif

    if (dh->next==END_HEAP)
      break;
  }

  return 0;
}
#endif


#ifdef DEBUGVM
void hpdbug(void)
{
  struct _dsheap *dh;
  int first=TRUE;
  
  for (dh=pdshDheap;
       dh;
       dh=(struct _dsheap *)(pbDs+dh->next))
  {
    if (!dh->free)
    {
      if (first)
        printf("\n\n");

      printf("heap ofs=%08" UINT32_XFORMAT " (size=%d)\n", (long)((byte *)(dh+1)-pbDs), dh->size);
    }

    if (dh->next==END_HEAP)
      break;
  }
}
#endif

