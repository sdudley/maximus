/** @file align.c Helper functions to align structures (etc)
 *                which need to be aligned, but aren't.
 *
 *  @note This is NOT good programming style, but it is 
 *        necessary to do this to avoid changing large
 *        parts of Maximus to run on non-sloppy (e.g.
 *	  sparc) processors that demand proper alignment
 *        of certain types.
 */

#include <stdlib.h>
#include "compiler.h"

void NoMem();

#if !SLOPPY_ALIGNMENT_OKAY

#if defined(NEED_MEMALIGN)
/* Before enabling this, see if you have posix_memalign() */
void *memalign(size_t alignment, size_t size)
{
  char *ptr = malloc(alignment + size);

  ptr += (long)ptr % alignment;
  return ptr;
}
#endif

void *_aligndup(const void *unaligned, size_t alignment, size_t size)
{
  void *aligned;

  if ((long)unaligned % alignment == 0)
    return (void *)unaligned; /* It actually is aligned */

  aligned = memalign(alignment, size);
  if (!aligned)
    NoMem();
  memcpy(aligned, unaligned, size);

  return aligned;
}

void _unaligndup_free(void *unaligned, void *aligned, size_t size)
{
  if (unaligned == aligned)
    return; /* It was actually aligned before _aligndup() was called */

  memcpy(unaligned, aligned, size);
  return;
}

static void 	*alignedStaticRAM = NULL;
void *_alignStatic(const void *unaligned, size_t size)
{
  static size_t	alignedSize = 0;

  if ((long)unaligned % _MAX_ALIGNMENT == 0)
    return (void *)unaligned; /* It actually is aligned */

  if (alignedStaticRAM == NULL)
  {
    /* 2KB oughtta be enough for anybody */
    alignedStaticRAM = memalign(_MAX_ALIGNMENT, 2048);
    alignedSize = 2048;

  }

  if (alignedSize < size)
  {
    /* Just in case it isn't.. */
    alignedStaticRAM = realloc(alignedStaticRAM, size);
    alignedSize = size;
  }

  if (alignedStaticRAM == NULL)
    NoMem();

  memcpy(alignedStaticRAM, unaligned, size);
  return alignedStaticRAM;
}

void _unalign(void *unaligned, void *aligned, size_t size)
{
  if (unaligned != aligned)
    memcpy(unaligned, aligned, size);

  return;
}

#endif /* !SLOPPY_ALIGNMENT_OKAY */

