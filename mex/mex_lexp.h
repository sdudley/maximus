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

/*# name=Private definiions for the lexical analysis module
*/

#ifndef LEXP_H_DEFINED__
#define LEXP_H_DEFINED__

#define MAX_INCLUDE 16      /* Max number of nested includes */
#define STR_BLOCK   32      /* Increment size for storing strings in heap */
#define PPLEN       256     /* Max size of a preprocessor directive */
#define MAX_IFBLKS  32      /* Maximum number of #ifdef/#endif blocks */

/* Internal constants used when evaluating a numeric expression */

#define TYPE_DEC 0x00
#define TYPE_HEX 0x01


typedef struct _filestack
{
  char *name;
  FILE *fp;
  long save_linenum;

} FILESTACK;

typedef struct _macdef
{
  char *szName;
  char *szText;
  int iLength;
  struct _macdef *next;
} *MACDEF;


/* Structure used for each macro being processed */

typedef struct _macro
{
  int fInMacro;
  int iPos;
  MACDEF md;
} MACRO;


/* A structure containing the translations we should perform on             *
 * character constants.                                                     */

typedef struct _xlt
{
  char from;
  char to;
} XLTABLE;

#endif /* LEXP_H_DEFINED__ */

