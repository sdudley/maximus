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

/*# CRC-32 macros
*/

#define CRC32POLY 0xedb88320L

#define CRC16CCITT    0x1021
#define CRC16CCITTREV 0x8408
#define CRC16         0x8005
#define CRC16REV      0xa001

/*#define xcrc32(c,crc) (cr3tab[((int) crc ^ c) & 0xff] ^ ((crc >> 8) & 0x00FFFFFFL))*/

#define xcrc32(c,crc) (cr3tab[((word) crc ^ (byte)c) & 0xff] ^ (dword)(((crc >> 8) & 0x00FFFFFFL))) /*SJD Mon  07-06-1992  01:15:30 */
#define updcrc32(c,crc) xcrc32(c,crc)

#define updcrc16(cp, crc) ( crctab[((crc >> 8) & 255) ^ (byte)cp] ^ (crc << 8))


