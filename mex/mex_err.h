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

/*************************** FATAL ERRORS *********************************/

#define MEXERR_FATAL_NOMEM              1000
#define MEXERR_FATAL_INTERNAL           1001

/****************************** ERRORS ***********************************/

/* General syntax errors */

#define MEXERR_SYNTAX                   2000
#define MEXERR_INVALIDIDENTTYPE         2001

/* Preprocessor stuff */

#define MEXERR_CANTOPENREAD             2100
#define MEXERR_INVALIDINCLUDEDIR        2101
#define MEXERR_INVALIDINCLUDEFILE       2102
#define MEXERR_TOOMANYNESTEDINCL        2103
#define MEXERR_INVALIDIFDEF             2104
#define MEXERR_TOOMANYNESTEDIFDEF       2105
#define MEXERR_UNMATCHEDIFDEF           2106
#define MEXERR_UNMATCHEDENDIF           2107
#define MEXERR_UNMATCHEDELSE            2108
#define MEXERR_UNMATCHEDELIFDEF         2109
#define MEXERR_INVALIDDEFINE            2110
#define MEXERR_MACROALREADYDEFINED      2111
#define MEXERR_ERRORDIRECTIVE           2112
#define MEXERR_UNKNOWNDIRECTIVE         2113

/* Lexer stuff */

#define MEXERR_UNTERMCHARCONST          2200
#define MEXERR_UNTERMSTRINGCONST        2201
#define MEXERR_INVALCHARCONST           2202
#define MEXERR_INVALIDHEX               2203
#define MEXERR_INVALIDCHAR              2204

/* Type stuff */

#define MEXERR_TYPEMUSTNOTBEVOID        2300
#define MEXERR_SYMISNOTSTRUCT           2301
#define MEXERR_INVALIDTYPEFORSTMT       2302
#define MEXERR_CANTUSEFUNCASVAR         2303
#define MEXERR_VOIDCANTHAVEVALUE        2304
#define MEXERR_INVALIDTYPECONV          2305
#define MEXERR_VARMUSTBEARRAY           2306
#define MEXERR_CANTINDEXNONARRAY        2307
#define MEXERR_INVALIDIDXTYPE           2308
#define MEXERR_OUTOFRANGESUBSCRIPT      2309
#define MEXERR_DOTOPERATORFORSTRUCTS    2310
#define MEXERR_FIELDNOTSTRUCTMEMBER     2311
#define MEXERR_NOTAGOTOLABEL            2312
#define MEXERR_LVALUEREQUIRED           2313
#define MEXERR_CANNOTAPPLYUNARY         2314
#define MEXERR_ARRAYMUSTHAVEBOUNDS      2315
#define MEXERR_SIZEOFBOUNDLESSARRAY     2316
#define MEXERR_CANTASSIGNSTRUCT         2317

/* Symbol table */

#define MEXERR_REDECLARATIONOF          2400
#define MEXERR_REDECLOFSTRUCT           2401
#define MEXERR_UNDEFSTRUCTNAME          2402
#define MEXERR_REDECLOFLABEL            2403
#define MEXERR_UNDEFLABEL               2404
#define MEXERR_UNDEFVARIABLE            2405
#define MEXERR_STRUCTMUSTBEDECL         2406
#define MEXERR_REDECLOFFUNC             2407
#define MEXERR_REDECLOFARG              2408
#define MEXERR_REDECLOFFUNCBODY         2409
#define MEXERR_INVALIDRANGE             2410

/* Function stuff */

#define MEXERR_ARGMISMATCH              2501
#define MEXERR_TOOMANYARGSINDECL        2502
#define MEXERR_TOOFEWARGSINDECL         2503
#define MEXERR_CALLTOFUNCWITHNOPROTO    2504
#define MEXERR_VARIABLENOTFUNCTION      2505
#define MEXERR_LVALUEREQDARG            2506
#define MEXERR_TOOFEWARGSINCALL         2507
#define MEXERR_TOOMANYARGSINCALL        2508
#define MEXERR_CANTRETURNVOID           2509
#define MEXERR_FUNCMUSTRETURNVALUE      2510

/***************************** WARNINGS ***********************************/

#define MEXERR_WARN_CONSTANTTRUNCATED   3000
#define MEXERR_WARN_IDENTTRUNCATED      3001
#define MEXERR_WARN_MEANINGLESSEXPR     3002

/***************************** DEBUGGING **********************************/

#define MEXERR_DEBUG                    4000


