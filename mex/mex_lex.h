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

/*# name=Structures for the lexical analysis routines
*/


#ifndef __ETL_H_DEFINED
#define __ETL_H_DEFINED

/* Table of reserved words and corresponding tokens, for screening out      *
 * reserved words when processing identifiers.                              */

mex_extern struct _id
{
  char *name;
  word value;
} idlist[]
#ifdef MEX_INIT
=       {
                {"char",        T_BYTE},
                {"int",         T_WORD},
                {"long",        T_DWORD},
                {"void",        T_VOID},
                {"string",      T_STRING},
                {"if",          T_IF},
                {"then",        T_THEN},
                {"else",        T_ELSE},
                {"goto",        T_GOTO},
                {"array",       T_ARRAY},
                {"of",          T_OF},
                {"while",       T_WHILE},
                {"do",          T_DO},
                {"for",         T_FOR},
                {"ref",         T_REF},
                {"return",      T_RETURN},
                {"shl",         T_SHL},
                {"shr",         T_SHR},
                {"and",         T_LAND},
                {"or",          T_LOR},
                {"struct",      T_STRUCT},
                {"unsigned",    T_UNSIGNED},
                {"signed",      T_SIGNED},
                {"sizeof",      T_SIZEOF},
                {NULL,          0}
        }
#endif
        ;


/* A corresponding table of operators and their associated tokens */

mex_extern struct _id ops[]
#ifdef MEX_INIT
=       {
                {"{",           T_BEGIN},
                {"}",           T_END},
                {"..",          T_RANGE},
                {":",           T_COLON},
                {">=",          T_GE},
                {">",           T_GT},
                {"<=",          T_LE},
                {"<",           T_LT},
                {"=",           T_EQUAL},
                {"<>",          T_NOTEQUAL},
                {"+",           T_BPLUS},
                {"-",           T_MINUS},
                {"*",           T_BMULTIPLY},
                {"/",           T_BDIVIDE},
                {"%",           T_BMODULUS},
                {"(",           T_LPAREN},
                {")",           T_RPAREN},
                {"[",           T_LBRACKET},
                {"]",           T_RBRACKET},
                {",",           T_COMMA},
                {";",           T_SEMICOLON},
                {"&",           T_BAND},
                {"|",           T_BOR},
                {NULL,          0}
        }
#endif
        ;


#endif /* __ET_H_DEFINED */

