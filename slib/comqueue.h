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

#ifndef __COMQUEUE_H_DEFINED
#define __COMQUEUE_H_DEFINED

typedef struct
{
  PBYTE pbBuf;    /* Start of queue buffer */
  PBYTE pbHead;   /* Pointer to head of queue */
  PBYTE pbEnd;    /* Pointer past end of queue buffer */
  PBYTE pbTail;   /* Pointer to tail of queue */
} COMQUEUE;

typedef COMQUEUE *PCOMQUEUE;


/* True if the tail pointer is currently below the head pointer */

#define QueueWrapped(q)  ((q)->pbTail < (q)->pbHead)


/* True if the queue is empty */

#define QueueEmpty(q) ((q)->pbHead == (q)->pbTail || \
                       ((q)->pbHead == (q)->pbEnd && (q)->pbTail == (q)->pbBuf))


/* Purge everything in the specified queue */

#define QueuePurge(q)  ((q)->pbHead = (q)->pbTail = (q)->pbBuf)


/* This function is to be called by tasks performing removals from
 * the queue if QueueGetSizeContig() returns 0.
 */

#define QueueWrapPointersRemove(q) ((q)->pbHead == (q)->pbEnd ? \
                                    ((q)->pbHead = (q)->pbBuf) : 0)


/* Get the number of contiguous bytes available for removal from the queue */

#define QueueGetSizeContig(q) (QueueWrapped(q) ?          \
                               (q)->pbEnd - (q)->pbHead : \
                               (q)->pbTail - (q)->pbHead)

/* Get the total number of bytes available in the queue */

#define QueueGetSize(q) (QueueWrapped(q) ? \
                         ((q)->pbEnd - (q)->pbHead) + ((q)->pbTail - (q)->pbBuf) : \
                         (q)->pbTail - (q)->pbHead)

/* This function is to be called by tasks performing insertions on
 * the queue if QueueGetFreeContig() returns 0.
 */

#define QueueWrapPointersInsert(q) ((q)->pbTail == (q)->pbEnd ?  \
                                    ((q)->pbTail = (q)->pbBuf) : 0)

/* Get the number of contiguous bytes that could be inserted in the queue */

#define QueueGetFreeContig(q) (QueueWrapped(q) ?                \
                               (q)->pbHead - (q)->pbTail - 1  : \
                               (q)->pbEnd - (q)->pbTail - ((q)->pbHead == (q)->pbBuf))

/* Get the total number of bytes that could be inserted in the queue */

#define QueueGetFree(q)  (QueueWrapped(q) ?               \
                          (q)->pbHead - (q)->pbTail - 1 : \
                          ((q)->pbEnd - (q)->pbTail) + ((q)->pbHead - (q)->pbBuf))

/* Called to adjust pointes after inserting 'size' bytes at the pbTail
 */

#define QueueInsertContig(q, size) do {                          \
                                     (q)->pbTail += size;        \
                                     QueueWrapPointersInsert(q); \
                                   } while (0)

/* Called to adjust pointes after removing 'size' bytes from the pbHead
 */

#define QueueRemoveContig(q, size) do {                          \
                                     (q)->pbHead += size;        \
                                     QueueWrapPointersRemove(q); \
                                   } while (0)

#endif /* __COMQUEUE_H_DEFINED */

