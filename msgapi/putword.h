/*
 *  XMSGAPI; eXtended MsgAPI
 *
 *  Please refer to the file named LICENCE for copyright information.
 */

#ifndef __PUTWORD_H__
#define __PUTWORD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "typedefs.h"

/*
 *  get_dword
 *
 *  Reads in a 4 byte word that is stored in little endian (Intel) notation
 *  and converts it to the local representation n an architecture-
 *  independent manner
 *
 *  get_word
 *
 *  Reads in a 2 byte word that is stored in little endian (Intel) notation
 *  and converts it to the local representation in an architecture-
 *  independent manner
 */

#ifdef __LITTLE_ENDIAN__

#define put_dword(ptr, val)  (*(dword *)(ptr) = (val))
#define put_word(ptr, val)   (*(word *)(ptr) = (val))
#define get_dword(ptr)       (*(dword *)(ptr))
#define get_word(ptr)        (*(word *)(ptr))

#else

void put_word(byte * ptr, word value);
void put_dword(byte * ptr, dword value);

#define get_dword(ptr) \
  ((dword)((byte)(ptr)[0]) | \
  (((dword)((byte)(ptr)[1])) << 8) | \
  (((dword)((byte)(ptr)[2])) << 16) | \
  (((dword)((byte)(ptr)[3])) << 24))

#define get_word(ptr) \
  (word) ((word)((byte)(ptr)[0]) | \
  (((word)((byte)(ptr)[1])) << 8 ))

#endif

#ifdef __cplusplus
}
#endif

#endif
