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

/*# name=Maximus version number information
*/

#ifndef __MAX_VR_DEFINED
  #define __MAX_VR_DEFINED

  #ifdef MAX_INCL_VER /* if we need to include version info */

    #include "bldupd.h"

    #ifdef MAX_DEFINE_VERSION
      #define ver_extern
      #define VER_IS(x) = x
      #define VER_LEN(x) x
    #else
      #define ver_extern extern
      #define VER_IS(x)
      #define VER_LEN(x) x
    #endif

    #if defined(OS_2)
      #define SLASH_2 "/2"
    #elif defined(NT)
      #define SLASH_2 "/NT"
    #elif defined(UNIX)
      #define SLASH_2 "/UNIX"
    #else
      #define SLASH_2
    #endif

    ver_extern char *us_short VER_IS("Maximus" SLASH_2);
    ver_extern char *name VER_IS("MAXIMUS" SLASH_2);

    #define stringize2(x) #x
    #define stringize(x) stringize2(x)

    #define MAX_VER_INT 3
    #define VER_MAJ     "3"
    #define VER_MIN     "03"
    #define TEAR_TEST   "b"

    #define VER_CHECKSUM (~'3'+~'0'+~'3')
    #define NAME_CHEKSUM (~'M'+~'A'+~'X')

    #define VER         VER_MAJ "." VER_MIN

    #define VERSION     VER

    ver_extern char *version VER_IS(VERSION);
    ver_extern char *version_short VER_IS(VER_MAJ);
    ver_extern char *xfer_id VER_IS("Maximus " VERSION);
    ver_extern char test[VER_LEN(20)] VER_IS(TEAR_TEST "\0\0");
    ver_extern char tear_ver[VER_LEN(20)] VER_IS(VER TEAR_TEST);

    #undef SLASH_2
    #undef stringize
    #undef stringize2

  #endif /* MAX_INCL_VER */

#endif /* __MAX_VR_H_DEFINED */

/* Official Max 2.00 release history:

   alpha/1, alpha/2, beta/1, alpha/3, beta/2, alpha/4, johnny/5, deep/6,
   lucky/7, crazy/8, cloud/9, warp/10, xyyzy/11, plugh/12, stalag/14,
   final/15, final/15+, sweet/16, sweet/16+, omega/17, omega/17+
*/


