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


/*  A Bison parser, made from mex_tab.y  */

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



  #define MEX_PARSER

  #include <stdio.h>
  #include <stdlib.h>
  #include <mem.h>
  #include "alc.h"
  #include "prog.h"
  #include "mex.h"

  #ifdef __TURBOC__
  #pragma warn -cln
  #endif
    
  ATTRIBUTES *curfn=NULL;

  #pragma off(unreferenced)
  static char rcs_id[]="$Id: mex_tab.c,v 1.1 2002/10/01 17:53:56 sdudley Exp $";
  #pragma on(unreferenced)


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
#define YYDEBUG
#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		213
#define	YYFLAG		-32768
#define	YYNTBASE	58

#define YYTRANSLATE(x) (x)

static const short yyrline[] = {     0,
   244,   247,   248,   251,   252,   256,   259,   260,   263,   270,
   272,   276,   278,   287,   289,   291,   295,   301,   309,   311,
   316,   318,   329,   330,   333,   335,   337,   341,   345,   347,
   349,   351,   353,   355,   357,   359,   361,   363,   365,   367,
   369,   374,   389,   402,   404,   409,   410,   413,   414,   417,
   419,   421,   426,   428,   430,   432,   435,   435,   438,   440,
   441,   443,   464,   468,   473,   478,   482,   484,   486,   490,
   494,   497,   500,   502,   506,   508,   520,   535,   537,   539,
   541,   543,   545,   550,   552,   557,   561,   563,   567,   569,
   571,   573,   575,   577,   579,   581,   583,   585,   587,   589,
   591,   593,   595,   597,   599,   601,   606,   615,   619,   621,
   623,   625,   630,   632,   635,   637,   639,   643,   645,   647,
   651
};

static const char * const yytname[] = {     0,
"error","$illegal.","T_ID","T_CONSTBYTE","T_CONSTWORD","T_CONSTDWORD","T_CONSTSTRING","T_BYTE","T_WORD","T_DWORD",
"T_STRING","T_VOID","T_BEGIN","T_END","T_IF","T_THEN","T_ELSE","T_GOTO","T_WHILE","T_DO",
"T_FOR","T_STRUCT","T_DOT","T_ELLIPSIS","T_LPAREN","T_RPAREN","T_LBRACKET","T_RBRACKET","T_REF","T_RETURN",
"T_COMMA","T_SEMICOLON","T_COLON","T_ARRAY","T_RANGE","T_OF","T_UNSIGNED","T_SIGNED","T_SIZEOF","T_ASSIGN",
"T_LAND","T_LOR","T_NOTEQUAL","T_EQUAL","T_LE","T_LT","T_GE","T_GT","T_SHL","T_SHR",
"T_BAND","T_BOR","T_BPLUS","T_MINUS","T_BMULTIPLY","T_BDIVIDE","T_BMODULUS","program"
};

static const short yyr1[] = {     0,
    58,    59,    59,    60,    60,    62,    63,    64,    61,    65,
    65,    67,    66,    68,    68,    68,    68,    69,    70,    70,
    72,    71,    73,    73,    74,    75,    74,    76,    77,    77,
    77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
    77,    78,    78,    79,    79,    80,    80,    81,    81,    82,
    82,    82,    83,    82,    82,    84,    82,    85,    86,    82,
    87,    82,    88,    89,    90,    82,    82,    82,    82,    91,
    92,    91,    94,    93,    95,    95,    95,    96,    96,    96,
    96,    96,    96,    97,    97,    98,    99,    99,   100,   100,
   100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
   100,   100,   100,   100,   100,   100,   101,   101,   102,   102,
   102,   102,   103,   103,   104,   104,   104,   105,   105,   105,
   106
};

static const short yyr2[] = {     0,
     1,     0,     2,     1,     1,     0,     0,     0,     9,     1,
     1,     0,     5,     0,     1,     3,     1,     3,     0,     1,
     0,     5,     0,     2,     3,     0,     7,     2,     1,     1,
     1,     2,     2,     2,     2,     2,     2,     1,     1,     6,
     2,     3,     2,     3,     1,     0,     2,     0,     1,     1,
     2,     2,     0,     5,     3,     0,     4,     0,     0,     5,
     0,     6,     0,     0,     0,    12,     3,     2,     1,     0,
     0,     3,     0,     5,     0,     3,     1,     1,     4,     4,
     1,     1,     1,     0,     1,     3,     1,     1,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     2,     3,     1,     1,     1,
     1,     1,     1,     2,     1,     4,     3,     1,     4,     3,
     1
};

static const short yydefact[] = {     2,
     1,    29,    30,    31,    39,    38,     0,     0,     0,     0,
     3,     4,     5,     0,     0,   121,    41,     0,    35,    36,
    37,    32,    33,    34,     0,    45,    28,     6,    26,     0,
     0,     0,    25,     0,    23,    43,     0,    44,     7,     0,
    42,     0,    19,     0,    24,     0,     0,    40,    15,    20,
     0,    17,     0,    27,    41,     8,    19,     0,     0,    16,
    18,    12,    11,     9,    10,    23,    46,     0,     0,   115,
   109,   110,   111,   113,    21,    13,     0,     0,    58,    61,
     0,     0,    84,    69,     0,     0,    50,    47,    81,   108,
    78,     0,    88,    87,    82,   112,    83,     0,     0,    68,
    23,     0,    53,     0,     0,     0,    84,     0,     0,    88,
    87,     0,     0,    85,     0,   115,   106,    83,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    52,    51,   114,     0,     0,
     0,    73,    56,    46,     0,    55,    59,     0,     0,     0,
    86,    67,     0,     0,     0,   100,   101,   103,   102,    94,
    95,   104,   105,    97,    96,    98,    99,    92,    93,    89,
    90,    91,   117,     0,   107,    75,     0,     0,    70,     0,
     0,    63,    79,    80,   117,     0,   116,     0,    77,    57,
    22,    71,    54,    60,     0,    84,   116,    74,    75,     0,
    62,     0,    76,    72,    64,    84,    65,     0,     0,    66,
     0,     0,     0
};

static const short yydefgoto[] = {   211,
     1,    11,    12,    34,    43,    59,    64,    65,    66,    51,
    52,    53,    87,   101,    40,    45,    35,    14,    46,    31,
    25,    68,    -1,    88,   145,   177,   105,   180,   106,   196,
   206,   208,   193,   200,    89,   176,   188,    90,   113,    91,
    92,   110,   111,    95,    96,    97,    98,   112
};

static const short yypact[] = {-32768,
   119,-32768,-32768,-32768,-32768,-32768,    25,    17,    67,    78,
-32768,-32768,-32768,    25,    20,-32768,    24,    21,-32768,-32768,
-32768,-32768,-32768,-32768,    28,-32768,-32768,-32768,-32768,     1,
    27,    25,-32768,    32,-32768,    44,    33,-32768,-32768,    86,
-32768,   385,     5,    36,-32768,    39,    25,-32768,-32768,-32768,
    47,    48,   385,-32768,-32768,-32768,     5,    25,     9,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   119,   200,    49,     2,
-32768,-32768,-32768,-32768,-32768,-32768,    59,    25,-32768,-32768,
    68,   321,     8,-32768,    76,    13,-32768,-32768,-32768,-32768,
-32768,   457,    72,    73,-32768,   100,    23,    69,    -1,-32768,
-32768,     8,-32768,    84,    59,   284,     8,    93,   335,-32768,
-32768,    96,    90,   457,   385,   101,-32768,    31,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,-32768,-32768,-32768,    25,     8,
     8,-32768,-32768,   119,   284,-32768,-32768,   106,   102,    13,
-32768,-32768,   109,    25,     8,   472,   472,   195,   195,   313,
   313,   313,   313,    10,    10,    10,    10,    34,    34,-32768,
-32768,-32768,   103,   383,   457,     8,   284,   254,   127,   284,
    59,-32768,-32768,-32768,-32768,   413,   105,   110,   440,-32768,
-32768,-32768,-32768,-32768,   116,     8,-32768,-32768,     8,   284,
-32768,   117,-32768,-32768,-32768,     8,-32768,   124,   284,-32768,
   151,   152,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    97,
-32768,-32768,-32768,-32768,   -45,   154,-32768,   107,     3,-32768,
-32768,    14,-32768,   -97,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   -38,   -80,  -104,   -75,
    57,   -67,   -63,-32768,-32768,   -76,-32768,    -7
};


#define	YYLAST		529


static const short yytable[] = {    17,
    93,   103,   149,    15,    94,   117,    26,    28,   148,   118,
    70,    71,    72,    73,    74,   116,    71,    72,    73,    74,
    67,    62,    16,   142,    38,    30,  -121,    16,    49,   147,
   -14,   143,    82,    50,  -121,    36,    29,    82,    93,    55,
    63,  -118,    94,    18,    48,   139,    85,   179,    41,   140,
    61,    85,    27,   154,    37,   144,    39,   155,    32,    33,
    99,    86,   131,   132,   133,   134,   135,    54,    42,   183,
   104,    27,    56,   118,    19,    20,    21,    93,    57,   190,
   100,    94,   194,   102,   108,    22,    23,    24,   133,   134,
   135,   202,   107,     2,     3,     4,     5,     6,    99,    44,
   115,   207,   204,   136,   137,   195,   138,     7,   141,    93,
    93,   210,    93,    94,    94,   146,    94,   153,   150,     8,
   142,   152,     9,    10,   181,  -121,     2,     3,     4,     5,
     6,   173,    93,   182,   184,   198,    94,    99,   109,   114,
     7,    93,  -120,   192,  -119,    94,   185,   201,   205,   209,
   212,   213,     8,    60,    13,     9,    10,   178,   109,    58,
   203,     0,     0,   114,     0,     0,     0,     0,     0,    99,
    99,     0,    99,     0,     0,   156,   157,   158,   159,   160,
   161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
   171,   172,    99,     0,     0,     0,   174,   175,     0,     0,
    69,    99,    70,    71,    72,    73,    74,     0,     0,     0,
     0,   186,    75,    76,    77,     0,     0,    78,    79,    80,
    81,     0,     0,     0,    82,     0,     0,     0,     0,    83,
     0,    84,   189,     0,     0,     0,     0,     0,    85,   123,
   124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
   134,   135,   114,    86,    69,   189,    70,    71,    72,    73,
    74,     0,   114,     0,     0,     0,    75,   191,    77,     0,
     0,    78,    79,    80,    81,     0,     0,     0,    82,     0,
     0,     0,     0,    83,    69,    84,    70,    71,    72,    73,
    74,     0,    85,     0,     0,     0,    75,     0,    77,     0,
     0,    78,    79,    80,    81,     0,     0,    86,    82,     0,
     0,     0,     0,    83,     0,    84,     0,     0,     0,     0,
     0,     0,    85,    70,    71,    72,    73,    74,     2,     3,
     4,     5,     6,     0,     0,     0,     0,    86,     0,     0,
     0,     0,    47,     0,     0,    82,     0,     0,     0,     0,
     0,     0,     0,     0,     8,     0,     0,     9,    10,    85,
   151,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     0,     0,     0,     0,    86,   119,   120,   121,   122,   123,
   124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
   134,   135,     2,     3,     4,     5,     6,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    47,     0,     0,     0,
   187,     0,     0,     0,     0,     0,     0,     0,     8,     0,
     0,     9,    10,   119,   120,   121,   122,   123,   124,   125,
   126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
   197,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   119,   120,   121,   122,   123,   124,   125,
   126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
   199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
   129,   130,   131,   132,   133,   134,   135,   119,   120,   121,
   122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
   132,   133,   134,   135,   121,   122,   123,   124,   125,   126,
   127,   128,   129,   130,   131,   132,   133,   134,   135
};

static const short yycheck[] = {     7,
    68,    77,   107,     1,    68,    86,    14,    15,   106,    86,
     3,     4,     5,     6,     7,     3,     4,     5,     6,     7,
    66,    13,     3,    25,    32,     5,    25,     3,    24,   105,
    26,    33,    25,    29,    33,    35,    13,    25,   106,    47,
    32,    40,   106,    27,    42,    23,    39,   145,     5,    27,
    58,    39,    33,    23,    28,   101,    25,    27,    31,    32,
    68,    54,    53,    54,    55,    56,    57,    32,    36,   150,
    78,    33,    26,   150,     8,     9,    10,   145,    31,   177,
    32,   145,   180,    25,    82,     8,     9,    10,    55,    56,
    57,   196,    25,     8,     9,    10,    11,    12,   106,    14,
    25,   206,   200,    32,    32,   181,     7,    22,    40,   177,
   178,   209,   180,   177,   178,    32,   180,   115,    26,    34,
    25,    32,    37,    38,    19,    25,     8,     9,    10,    11,
    12,   139,   200,    32,    26,    26,   200,   145,    82,    83,
    22,   209,    40,    17,    40,   209,   154,    32,    32,    26,
     0,     0,    34,    57,     1,    37,    38,   144,   102,    53,
   199,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,   177,
   178,    -1,   180,    -1,    -1,   119,   120,   121,   122,   123,
   124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
   134,   135,   200,    -1,    -1,    -1,   140,   141,    -1,    -1,
     1,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
    -1,   155,    13,    14,    15,    -1,    -1,    18,    19,    20,
    21,    -1,    -1,    -1,    25,    -1,    -1,    -1,    -1,    30,
    -1,    32,   176,    -1,    -1,    -1,    -1,    -1,    39,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,   196,    54,     1,   199,     3,     4,     5,     6,
     7,    -1,   206,    -1,    -1,    -1,    13,    14,    15,    -1,
    -1,    18,    19,    20,    21,    -1,    -1,    -1,    25,    -1,
    -1,    -1,    -1,    30,     1,    32,     3,     4,     5,     6,
     7,    -1,    39,    -1,    -1,    -1,    13,    -1,    15,    -1,
    -1,    18,    19,    20,    21,    -1,    -1,    54,    25,    -1,
    -1,    -1,    -1,    30,    -1,    32,    -1,    -1,    -1,    -1,
    -1,    -1,    39,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    -1,    -1,    -1,    -1,    54,    -1,    -1,
    -1,    -1,    22,    -1,    -1,    25,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    34,    -1,    -1,    37,    38,    39,
    26,    49,    50,    51,    52,    53,    54,    55,    56,    57,
    -1,    -1,    -1,    -1,    54,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,     8,     9,    10,    11,    12,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,
    -1,    37,    38,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    56,    57,    41,    42,    43,
    44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
    54,    55,    56,    57,    43,    44,    45,    46,    47,    48,
    49,    50,    51,    52,    53,    54,    55,    56,    57
};
#define YYPURE 1

/* Skeleton output parser for bison,
   copyright (C) 1984 Bob Corbett and Richard Stallman

		       NO WARRANTY

  BECAUSE THIS PROGRAM IS LICENSED FREE OF CHARGE, WE PROVIDE ABSOLUTELY
NO WARRANTY, TO THE EXTENT PERMITTED BY APPLICABLE STATE LAW.  EXCEPT
WHEN OTHERWISE STATED IN WRITING, FREE SOFTWARE FOUNDATION, INC,
RICHARD M. STALLMAN AND/OR OTHER PARTIES PROVIDE THIS PROGRAM "AS IS"
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY
AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
CORRECTION.

 IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW WILL RICHARD M.
STALLMAN, THE FREE SOFTWARE FOUNDATION, INC., AND/OR ANY OTHER PARTY
WHO MAY MODIFY AND REDISTRIBUTE THIS PROGRAM AS PERMITTED BELOW, BE
LIABLE TO YOU FOR DAMAGES, INCLUDING ANY LOST PROFITS, LOST MONIES, OR
OTHER SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
USE OR INABILITY TO USE (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR
DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR
A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) THIS
PROGRAM, EVEN IF YOU HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES, OR FOR ANY CLAIM BY ANY OTHER PARTY.

		GENERAL PUBLIC LICENSE TO COPY

  1. You may copy and distribute verbatim copies of this source file
as you receive it, in any medium, provided that you conspicuously and
appropriately publish on each copy a valid copyright notice "Copyright
(C) 1985 Free Software Foundation, Inc."; and include following the
copyright notice a verbatim copy of the above disclaimer of warranty
and of this License.  You may charge a distribution fee for the
physical act of transferring a copy.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be licensed at no charge to all
    third parties on terms identical to those contained in this
    License Agreement (except that you may choose to grant more extensive
    warranty protection to some or all third parties, at your option).

    c) You may charge a distribution fee for the physical act of
    transferring a copy, and you may at your option offer warranty
    protection in exchange for a fee.

Mere aggregation of another unrelated program with this program (or its
derivative) on a volume of a storage or distribution medium does not bring
the other program under the scope of these terms.

  3. You may copy and distribute this program (or a portion or derivative
of it, under Paragraph 2) in object code or executable form under the terms
of Paragraphs 1 and 2 above provided that you also do one of the following:

    a) accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    b) accompany it with a written offer, valid for at least three
    years, to give any third party free (except for a nominal
    shipping charge) a complete machine-readable copy of the
    corresponding source code, to be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    c) accompany it with the information you received as to where the
    corresponding source code may be obtained.  (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form alone.)

For an executable file, complete source code means all the source code for
all modules it contains; but, as a special exception, it need not include
source code for modules which are standard libraries that accompany the
operating system on which the executable file runs.

  4. You may not copy, sublicense, distribute or transfer this program
except as expressly provided under this License Agreement.  Any attempt
otherwise to copy, sublicense, distribute or transfer this program is void and
your rights to use the program under this License agreement shall be
automatically terminated.  However, parties who have received computer
software programs from you with this License Agreement will not have
their licenses terminated so long as such parties remain in full compliance.

  5. If you wish to incorporate parts of this program into other free
programs whose distribution conditions are different, write to the Free
Software Foundation at 675 Mass Ave, Cambridge, MA 02139.  We have not yet
worked out a simple rule that can be stated here, but we will often permit
this.  We will be guided by the two goals of preserving the free status of
all derivatives of our free software and of promoting the sharing and reuse of
software.


In other words, you are welcome to use, share and improve this program.
You are forbidden to forbid anyone else to use, share and improve
what you give them.   Help stamp out software-hoarding!  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYFAIL		goto yyerrlab;

#define YYTERROR	1

#ifndef YYIMPURE
#define YYLEX		yylex()
#endif

#ifndef YYPURE
#define YYLEX		yylex(&yylval, &yylloc)
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYIMPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/

int yynerr;			/*  number of parse errors so far       */

#ifdef YYDEBUG
int yydebug = 0;		/*  nonzero means print parse trace	*/
#endif

#endif  /* YYIMPURE */


/*  YYMAXDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYMAXDEPTH
#define YYMAXDEPTH 200
#endif

/*  YYMAXLIMIT is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#ifndef YYMAXLIMIT
#define YYMAXLIMIT 10000
#endif


int yyparse(void)
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  YYLTYPE *yylsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYMAXDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYMAXDEPTH];	/*  the semantic value stack		*/
  YYLTYPE yylsa[YYMAXDEPTH];	/*  the location stack			*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  short *p_yyss=NULL;
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */
  YYSTYPE *p_yyvs=NULL;
  YYLTYPE *yyls = yylsa;
  YYLTYPE *p_yyls=NULL;

  int yyss_alcd=0;
  int yyvs_alcd=0;
  int yyls_alcd=0;

  int yymaxdepth = YYMAXDEPTH;

#ifndef YYPURE
  int yychar;
  YYSTYPE yylval;
  YYLTYPE yylloc;
#endif

#ifdef YYDEBUG
  extern int yydebug;
#endif


  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

  (void)yyrline;
  (void)yytname;
  (void)yyss_alcd;
  (void)yyvs_alcd;
  (void)yyls_alcd;
  (void)p_yyss;
  (void)p_yyvs;
  (void)p_yyls;



#ifdef YYDEBUG
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerr = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
  yylsp = yyls;

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yymaxdepth - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      YYLTYPE *yyls1 = yyls;
      short *yyss1 = yyss;

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yymaxdepth);

      yyss = yyss1; yyvs = yyvs1; yyls = yyls1;
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yymaxdepth >= YYMAXLIMIT)
	yyerror("parser stack overflow");
      yymaxdepth *= 2;
      if (yymaxdepth > YYMAXLIMIT)
	yymaxdepth = YYMAXLIMIT;
      yyss = p_yyss = (short *) malloc (yymaxdepth * sizeof (*yyssp));
      yyss_alcd=1;

      bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = p_yyvs = (YYSTYPE *) malloc (yymaxdepth * sizeof (*yyvsp));
      yyvs_alcd=1;

      bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = p_yyls = (YYLTYPE *) malloc (yymaxdepth * sizeof (*yylsp));
      yyls_alcd=1;

      bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#else
  (void)yyls1;
  (void)yylsp;
#endif

#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#ifdef YYDEBUG
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yymaxdepth);
#endif

      if (yyssp >= yyss + yymaxdepth - 1)
	YYABORT;
    }

#ifdef YYDEBUG
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
yyresume:

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#ifdef YYDEBUG
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#ifdef YYDEBUG
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#ifdef YYDEBUG
      if (yydebug)
	fprintf(stderr, "Next token is %d (%s)\n", yychar, yytname[yychar1]);
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#ifdef YYDEBUG
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

#ifdef YYDEBUG
  if (yydebug)
    {
      if (yylen == 1)
	fprintf (stderr, "Reducing 1 value via line %d, ",
		 yyrline[yyn]);
      else
	fprintf (stderr, "Reducing %d values via line %d, ",
		 yylen, yyrline[yyn]);
    }
#endif


  switch (yyn) {

case 5:
{ /* nothing */ ;
    break;}
case 6:
{ yyval.attrdesc=curfn=function_begin(yyvsp[-1].typedesc, yyvsp[0].id); ;
    break;}
case 7:
{ yyval.size=offset; scope_open(); ;
    break;}
case 8:
{ function_args(yyvsp[-4].attrdesc, yyvsp[-1].arg); ;
    break;}
case 9:
{ VMADDR end_quad=this_quad;
                                  scope_close();
                                  function_end(yyvsp[-6].attrdesc, yyvsp[0].size, end_quad);
                                  offset=yyvsp[-4].size;
                                ;
    break;}
case 10:
{ yyval.size=TRUE; ;
    break;}
case 11:
{ yyval.size=FALSE; ;
    break;}
case 12:
{ yyval.size=offset; GenFuncStartQuad(curfn); ;
    break;}
case 13:
{
                                  /* Reset the value of the offset pointer  *
                                   * for the local activation record.       */

                                  offset=yyvsp[-3].size;
                                ;
    break;}
case 14:
{ yyval.arg=NULL; ;
    break;}
case 15:
{ yyval.arg=declare_ellipsis(); ;
    break;}
case 16:
{ if (yyvsp[-2].arg) yyvsp[-2].arg->next=yyvsp[0].arg;
                                  yyval.arg=yyvsp[-2].arg;
                                ;
    break;}
case 17:
{ if (yyvsp[0].arg) yyvsp[0].arg->next=NULL;
                                  yyval.arg=yyvsp[0].arg;
                                ;
    break;}
case 18:
{ yyval.arg=smalloc(sizeof(FUNCARGS));
                                  yyval.arg->type=yyvsp[-1].typedesc; yyval.arg->name=sstrdup(yyvsp[0].id);
                                  yyval.arg->next=NULL;
                                  yyval.arg->ref=yyvsp[-2].opt.bool;
                                ;
    break;}
case 19:
{ yyval.opt.bool=FALSE; ;
    break;}
case 20:
{ yyval.opt.bool=TRUE; ;
    break;}
case 21:
{ scope_open(); yyval.size=offset; ;
    break;}
case 22:
{
                                  /* Reset the value of the offset pointer  *
                                   * for the local activation record.       */

                                  offset=yyvsp[-3].size;
                                  scope_close();
                                ;
    break;}
case 25:
{ declare_vars(yyvsp[-2].typedesc,yyvsp[-1].attrdesc); ;
    break;}
case 26:
{ yyval.typedesc=define_struct_id(yyvsp[-1].id); ;
    break;}
case 27:
{ define_struct_body(yyvsp[-3].typedesc); ;
    break;}
case 28:
{ yyval.typedesc=yyvsp[-1].typedesc; /* default action */ ;
    break;}
case 29:
{ yyval.typedesc=&UnsignedByteType; ;
    break;}
case 30:
{ yyval.typedesc=&WordType; ;
    break;}
case 31:
{ yyval.typedesc=&DwordType; ;
    break;}
case 32:
{ yyval.typedesc=&ByteType; ;
    break;}
case 33:
{ yyval.typedesc=&WordType; ;
    break;}
case 34:
{ yyval.typedesc=&DwordType; ;
    break;}
case 35:
{ yyval.typedesc=&UnsignedByteType; ;
    break;}
case 36:
{ yyval.typedesc=&UnsignedWordType; ;
    break;}
case 37:
{ yyval.typedesc=&UnsignedDwordType; ;
    break;}
case 38:
{ yyval.typedesc=&VoidType; ;
    break;}
case 39:
{ yyval.typedesc=&StringType; ;
    break;}
case 40:
{ yyval.typedesc=array_descriptor(&yyvsp[-3].range,yyvsp[0].typedesc); ;
    break;}
case 41:
{ yyval.typedesc=declare_struct(yyvsp[0].id); ;
    break;}
case 42:
{
                                  yyval.range.lo=yyvsp[-2].constant.val;
                                  yyval.range.hi=yyvsp[0].constant.val;

                                  if (yyval.range.hi < yyval.range.lo ||
                                      yyval.range.hi > 0x7fff ||
                                      yyval.range.lo > 0x7fff)
                                  {
                                    error(MEXERR_INVALIDRANGE,
                                          yyval.range.lo,yyval.range.hi);

                                    yyval.range.hi=yyval.range.lo;
                                  }
                                ;
    break;}
case 43:
{
                                  yyval.range.lo = yyvsp[-1].constant.val;
                                  yyval.range.hi = (VMADDR)-1;

                                  if (yyval.range.lo > 0x7fff)
                                  {
                                    error(MEXERR_INVALIDRANGE,
                                          yyval.range.lo, -1);
                                  }
                                ;
    break;}
case 44:
{ yyval.attrdesc=var_list(yyvsp[0].id,yyvsp[-2].attrdesc); ;
    break;}
case 45:
{ yyval.attrdesc=var_list(yyvsp[0].id,NULL); ;
    break;}
case 50:
{ ;
    break;}
case 51:
{ MaybeFreeTemporary(yyvsp[-1].dataobj, TRUE); ;
    break;}
case 52:
{
                                  warn(MEXERR_WARN_MEANINGLESSEXPR);
                                  MaybeFreeTemporary(yyvsp[-1].dataobj, TRUE);
                                ;
    break;}
case 53:
{ yyval.patch=IfTest(yyvsp[0].dataobj); ;
    break;}
case 54:
{ IfEnd(& yyvsp[-2].patch, & yyvsp[0].elsetype); ;
    break;}
case 55:
{ ProcessGoto(yyvsp[-1].id); ;
    break;}
case 56:
{ DeclareLabel(yyvsp[-1].id); ;
    break;}
case 58:
{ yyval.whil.top_quad=this_quad; ;
    break;}
case 59:
{ WhileTest(&yyvsp[-1].whil, yyvsp[0].dataobj); ;
    break;}
case 60:
{ GenWhileOut(&yyvsp[-3].whil); ;
    break;}
case 61:
{ yyval.whil.top_quad=this_quad; ;
    break;}
case 62:
{ GenDoWhileOut(&yyvsp[-4].whil, yyvsp[-1].dataobj); ;
    break;}
case 63:
{ yyval.fr.vmTest = this_quad;
                                  MaybeFreeTemporary(yyvsp[-1].dataobj, TRUE);
                                ;
    break;}
case 64:
{ GenForTest(&yyvsp[-2].fr, yyvsp[-1].dataobj);
                                  yyvsp[-2].fr.vmPost = this_quad;
                                ;
    break;}
case 65:
{
                                  GenForJmpTest(&yyvsp[-4].fr);
                                  MaybeFreeTemporary(yyvsp[0].dataobj, TRUE);
                                  yyvsp[-4].fr.vmBody = this_quad;
                                ;
    break;}
case 66:
{
                                  GenForJmpPostAndCleanup(&yyvsp[-7].fr);
                                ;
    break;}
case 67:
{ GenFuncRet(yyvsp[-1].dataobj, curfn); ;
    break;}
case 68:
{ yyerrok; ;
    break;}
case 69:
{ /* null statement */ ;
    break;}
case 70:
{ yyval.elsetype.patchout=NULL;
                                  yyval.elsetype.else_label=this_quad;
                                ;
    break;}
case 71:
{ ElseHandler(&yyval.elsetype); ;
    break;}
case 72:
{ yyval.elsetype=yyvsp[-1].elsetype; ;
    break;}
case 73:
{ yyval.fcall=StartFuncCall(yyvsp[-1].id); ;
    break;}
case 74:
{ yyval.dataobj=EndFuncCall(&yyvsp[-2].fcall, yyvsp[-1].dataobj); ;
    break;}
case 75:
{ yyval.dataobj=NULL; ;
    break;}
case 76:
{
                                  if (!yyvsp[-2].dataobj)
                                  {
                                    yyvsp[-2].dataobj = NewDataObj();
                                    yyvsp[-2].dataobj->type = NULL;
                                    yyvsp[-2].dataobj->argtype = NULL;
                                  }

                                  yyvsp[-2].dataobj->next_arg=yyvsp[0].dataobj;
                                  yyval.dataobj=yyvsp[-2].dataobj;
                                ;
    break;}
case 77:
{
                                  if (yyvsp[0].dataobj)
                                    yyvsp[0].dataobj->next_arg=NULL;
                                  else
                                  {
                                    yyvsp[0].dataobj = NewDataObj();
                                    yyvsp[0].dataobj->type = NULL;
                                    yyvsp[0].dataobj->argtype = NULL;
                                  }

                                  yyval.dataobj=yyvsp[0].dataobj;
                                ;
    break;}
case 78:
{ yyval.dataobj=yyvsp[0].dataobj; ;
    break;}
case 79:
{ yyval.dataobj=TypeCast(yyvsp[0].dataobj, yyvsp[-2].typedesc); ;
    break;}
case 80:
{ yyval.dataobj=EvalSizeof(yyvsp[-1].typedesc); ;
    break;}
case 81:
{ yyval.dataobj=yyvsp[0].dataobj; ;
    break;}
case 82:
{ yyval.dataobj=yyvsp[0].dataobj; ;
    break;}
case 83:
{ yyval.dataobj=yyvsp[0].dataobj; ;
    break;}
case 84:
{ yyval.dataobj=NULL; ;
    break;}
case 85:
{ yyval.dataobj=yyvsp[0].dataobj; ;
    break;}
case 86:
{ yyval.dataobj=yyvsp[-1].dataobj; ;
    break;}
case 87:
{yyval.dataobj = yyvsp[0].dataobj; ;
    break;}
case 88:
{yyval.dataobj = yyvsp[0].dataobj; ;
    break;}
case 89:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_BMULTIPLY,yyvsp[0].dataobj); ;
    break;}
case 90:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_BDIVIDE,yyvsp[0].dataobj); ;
    break;}
case 91:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_BMODULUS,yyvsp[0].dataobj); ;
    break;}
case 92:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_BPLUS,yyvsp[0].dataobj); ;
    break;}
case 93:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_MINUS,yyvsp[0].dataobj); ;
    break;}
case 94:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_LE,yyvsp[0].dataobj); ;
    break;}
case 95:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_LT,yyvsp[0].dataobj); ;
    break;}
case 96:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_SHR,yyvsp[0].dataobj); ;
    break;}
case 97:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_SHL,yyvsp[0].dataobj); ;
    break;}
case 98:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_BAND,yyvsp[0].dataobj); ;
    break;}
case 99:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_BOR,yyvsp[0].dataobj); ;
    break;}
case 100:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_LAND,yyvsp[0].dataobj); ;
    break;}
case 101:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_LOR,yyvsp[0].dataobj); ;
    break;}
case 102:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_EQUAL,yyvsp[0].dataobj); ;
    break;}
case 103:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_NOTEQUAL,yyvsp[0].dataobj); ;
    break;}
case 104:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_GE,yyvsp[0].dataobj); ;
    break;}
case 105:
{ yyval.dataobj=EvalBinary(yyvsp[-2].dataobj,T_GT,yyvsp[0].dataobj); ;
    break;}
case 106:
{ yyval.dataobj=EvalUnary(yyvsp[0].dataobj,T_MINUS); ;
    break;}
case 107:
{
                                  /* The binary operator expects
                                   * assignments to be given
                                   * in the order "src -> dest";
                                   * hence the $3 $1 ordering.
                                   */
                                  yyval.dataobj=EvalBinary(yyvsp[0].dataobj,T_ASSIGN,yyvsp[-2].dataobj);
                                ;
    break;}
case 108:
{ yyval.dataobj=yyvsp[0].dataobj; ;
    break;}
case 109:
{ yyval.dataobj=byteref(&yyvsp[0].constant); ;
    break;}
case 110:
{ yyval.dataobj=wordref(&yyvsp[0].constant); ;
    break;}
case 111:
{ yyval.dataobj=dwordref(&yyvsp[0].constant); ;
    break;}
case 112:
{ yyval.dataobj=stringref(&yyvsp[0].constant); ;
    break;}
case 113:
{ yyval.constant=yyvsp[0].constant; ;
    break;}
case 114:
{ yyval.constant=string_merge(yyvsp[-1].constant, yyvsp[0].constant); ;
    break;}
case 115:
{ yyval.dataobj=idref(yyvsp[0].id); ;
    break;}
case 116:
{ yyval.dataobj=ProcessIndex(yyvsp[-3].dataobj, yyvsp[-1].dataobj, FALSE); ;
    break;}
case 117:
{ yyval.dataobj=ProcessStruct(yyvsp[-2].dataobj, yyvsp[0].id); ;
    break;}
case 118:
{ yyval.dataobj=idref(yyvsp[0].id); ;
    break;}
case 119:
{ yyval.dataobj=ProcessIndex(yyvsp[-3].dataobj, yyvsp[-1].dataobj, TRUE); ;
    break;}
case 120:
{ yyval.dataobj=ProcessStruct(yyvsp[-2].dataobj, yyvsp[0].id); ;
    break;}
case 121:
{ yyval.id=yyvsp[0].id; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#ifdef YYDEBUG
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerr;
      yyerror("parse error");
    }

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#ifdef YYDEBUG
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#ifdef YYDEBUG
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#ifdef YYDEBUG
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

#ifdef NEVER
  if (yyss_alcd && p_yyss)
        free(p_yyss);

  if (yyvs_alcd && p_yyvs)
        free(p_yyvs);

  if (yyls_alcd && p_yyls)
        free(p_yyls);
#endif
}





