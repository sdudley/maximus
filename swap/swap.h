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

/* This conditional extern statement is required when compiling with
   Turbo C++
 */
#ifdef __cplusplus
extern "C" {
#endif

int cdecl swap (unsigned char *program_name,
          unsigned char *command_line,
          unsigned char *exec_return,
          unsigned char *swap_fname);

int cdecl ems4_installed (void);
int cdecl xms_installed (void);

#ifdef __cplusplus
}
#endif

/* The return code from swap() will be one of the following.  Codes other    */
/* than SWAP_OK (0) indicate that an error occurred, and thus the program    */
/* has NOT been swapped, and the new program has NOT been executed.          */

#define SWAP_OK         (0)         /* Swapped OK and returned               */
#define SWAP_NO_SHRINK  (1)         /* Could not shrink DOS memory size      */
#define SWAP_NO_SAVE    (2)         /* Could not save program to XMS/EMS/disk*/
#define SWAP_NO_EXEC    (3)         /* Could not execute new program         */


/* If swap() returns 3, SWAP_NO_EXEC, the byte/char pointed to by the        */
/* parameter exec_return will be one of the following standard DOS error     */
/* codes, as specified in the DOS technical reference manuals.               */

#define BAD_FUNC        (0x01)   /* Bad DOS function number--unlikely          */
#define FILE_NOT_FOUND  (0x02)   /* File not found--couldn't find program_name */
#define ACCESS_DENIED   (0x05)   /* Access denied--couldn't open program_name  */
#define NO_MEMORY       (0x08)   /* Insufficient memory to run program_name    */
#define BAD_ENVIRON     (0x0A)   /* Invalid environment segment--unlikely      */
#define BAD_FORMAT      (0x0B)   /* Format invalid--unlikely                   */

