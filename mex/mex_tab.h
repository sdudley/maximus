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

typedef union  {
          IDTYPE *id;
          TYPEDESC *typedesc;
          ATTRIBUTES *attrdesc;
          DATAOBJ *dataobj;
          RANGE range;
          CONSTTYPE constant;
          TOKEN token;
          PATCH patch;
          ELSETYPE elsetype;
          FUNCARGS *arg;
          FUNCCALL fcall;
          WHILETYPE whil;
          OPTTYPE opt;
          FORTYPE fr;
          word size;
        } YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#define	YYACCEPT	return(0)
#define	YYABORT	return(1)
#define	YYERROR	goto yyerrlab
#define	T_ID	3
#define	T_CONSTBYTE	4
#define	T_CONSTWORD	5
#define	T_CONSTDWORD	6
#define	T_CONSTSTRING	7
#define	T_BYTE	8
#define	T_WORD	9
#define	T_DWORD	10
#define	T_STRING	11
#define	T_VOID	12
#define	T_BEGIN	13
#define	T_END	14
#define	T_IF	15
#define	T_THEN	16
#define	T_ELSE	17
#define	T_GOTO	18
#define	T_WHILE	19
#define	T_DO	20
#define	T_FOR	21
#define	T_STRUCT	22
#define	T_DOT	23
#define	T_ELLIPSIS	24
#define	T_LPAREN	25
#define	T_RPAREN	26
#define	T_LBRACKET	27
#define	T_RBRACKET	28
#define	T_REF	29
#define	T_RETURN	30
#define	T_COMMA	31
#define	T_SEMICOLON	32
#define	T_COLON	33
#define	T_ARRAY	34
#define	T_RANGE	35
#define	T_OF	36
#define	T_UNSIGNED	37
#define	T_SIGNED	38
#define	T_SIZEOF	39
#define	T_ASSIGN	40
#define	T_LAND	41
#define	T_LOR	42
#define	T_NOTEQUAL	43
#define	T_EQUAL	44
#define	T_LE	45
#define	T_LT	46
#define	T_GE	47
#define	T_GT	48
#define	T_SHL	49
#define	T_SHR	50
#define	T_BAND	51
#define	T_BOR	52
#define	T_BPLUS	53
#define	T_MINUS	54
#define	T_BMULTIPLY	55
#define	T_BDIVIDE	56
#define	T_BMODULUS	57

