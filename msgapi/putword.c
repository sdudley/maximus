#ifndef __LITTLE_ENDIAN__

#include "putword.h"

/*
 *  put_dword
 *
 *  Writes a 4 byte word in little endian notation, independent of the local
 *  system architecture.
 */

void put_dword(byte * ptr, dword value)
{
    ptr[0] = (byte) (value & 0xff);
    ptr[1] = (byte) ((value >> 8) & 0xff);
    ptr[2] = (byte) ((value >> 16) & 0xff);
    ptr[3] = (byte) ((value >> 24) & 0xff);
}

/*
 *  put_word
 *
 *  Writes a 4 byte word in little endian notation, independent of the local
 *  system architecture.
 */

void put_word(byte * ptr, word value)
{
    ptr[0] = (byte) (value & 0xff);
    ptr[1] = (byte) ((value >> 8) & 0xff);
}

#endif
