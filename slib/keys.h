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

/*# name=Keyboard keys definition
*/

#ifndef _KEYS_H
#define _KEYS_H

#define K_ESC   27

/********************/

#ifndef UNIX
# define K_ONEMORE 	0
#define K_F1    59
#define K_F2    60
#define K_F3    61
#define K_F4    62
#define K_F5    63
#define K_F6    64
#define K_F7    65
#define K_F8    66
#define K_F9    67
#define K_F10   68

#define K_SF1   84
#define K_SF2   85
#define K_SF3   86
#define K_SF4   87
#define K_SF5   88
#define K_SF6   89
#define K_SF7   90
#define K_SF8   91
#define K_SF9   92
#define K_SF10  93

#define K_CF1   94
#define K_CF2   95
#define K_CF3   96
#define K_CF4   97
#define K_CF5   98
#define K_CF6   99
#define K_CF7  100
#define K_CF8  101
#define K_CF9  102
#define K_CF10 103

#define K_AF1  104
#define K_AF2  105
#define K_AF3  106
#define K_AF4  107
#define K_AF5  108
#define K_AF6  109
#define K_AF7  110
#define K_AF8  111
#define K_AF9  112
#define K_AF10 113

#define K_ALT1 120
#define K_ALT2 121
#define K_ALT3 122
#define K_ALT4 123
#define K_ALT5 124
#define K_ALT6 125
#define K_ALT7 126
#define K_ALT8 127
#define K_ALT9 128
#define K_ALT0 129


#define K_ALTQ  16
#define K_ALTW  17
#define K_ALTE  18
#define K_ALTR  19
#define K_ALTT  20
#define K_ALTY  21
#define K_ALTU  22
#define K_ALTI  23
#define K_ALTO  24
#define K_ALTP  25

#define K_ALTA  30
#define K_ALTS  31
#define K_ALTD  32
#define K_ALTF  33
#define K_ALTG  34
#define K_ALTH  35
#define K_ALTJ  36
#define K_ALTK  37
#define K_ALTL  38

#define K_ALTZ  44
#define K_ALTX  45
#define K_ALTC  46
#define K_ALTV  47
#define K_ALTB  48
#define K_ALTN  49
#define K_ALTM  50

#define K_HOME  71
#define K_UP    72
#define K_PGUP  73
#define K_LEFT  75
#define K_CTR   76
#define K_RIGHT 77
#define K_END   79
#define K_DOWN  80
#define K_PGDN  81

#define K_INS   82
#define K_DEL   83
#define K_ALTBS 14
#define K_STAB  15
#define K_BS    8

#define K_CLEFT  115
#define K_CRIGHT 116
#define K_CEND   117
#define K_CPGDN  118
#define K_CHOME  119
#define K_CPGUP  132

/* Override a few of the really handy console
 * keys by fudging the scan codes. Here's how
 * we fudge them -- IIRC (from 1990!!!) under DOS,
 * you get ALT-A (for example) as scan code zero,
 * then another scancode (presumably K_ALTA) from
 * the keyboard. 
 *
 * So, I'm going to replace 'scan code 0' in the
 * code K_ONEMORE. Except, under UNIX, it'll be
 * 'ESC'.
 *
 * This means we can fall back to the mainframe-
 * style PF keys, where we send "ESC 1" for PF1.
 * And I'll turn ESC-control-k into ALT-K, and a few
 * others. I don't want to do cursor keys and stuff.
 */

#else /* UNIX */
# define K_ONEMORE 	K_ESC
# define K_F1		'1'
# define K_F2		'2'
# define K_F3		'3'
# define K_F4		'4'
# define K_F5		'5'
# define K_F6		'6'
# define K_F7		'7'
# define K_F8		'8'
# define K_F9		'9'
# define K_F10		'0'
# define K_F11		'-'
# define K_F12		'='
# define K_SF1		'!'
# define K_SF2		'@'
# define K_SF3		'#'
# define K_SF4		'$'
# define K_SF5		'%'
# define K_SF6		'^'
# define K_SF7		'&'
# define K_SF8		'*'
# define K_SF9		'('
# define K_SF10		')'
# define K_SF11		'_'
# define K_SF12		'+'
# define K_ALTA		'\001'
# define K_ALTB		'\002'
# define K_ALTC		'\003'
# define K_ALTD		'\004'
# define K_ALTE		'\005'
# define K_ALTF		'\006'
# define K_ALTG		'\007'
# define K_ALTH		'\010'
# define K_ALTI		'\011'
# define K_ALTJ		'\012'
# define K_ALTK		'\013'
# define K_ALTL		'\014'
# define K_ALTM		'\015'
# define K_ALTN		'\016'
# define K_ALTO		'\017'
# define K_ALTP		'\020'
# define K_ALTQ		'\021'
# define K_ALTR		'\022'
# define K_ALTS		'\023'
# define K_ALTT		'\024'
# define K_ALTU		'\025'
# define K_ALTV		'\026'
# define K_ALTW		'\027'
# define K_ALTX		'\030'
# define K_ALTY		'\031'
# define K_ALTZ		'\032'

/* Like VI, but ESC instead of control */
# define K_UP		'k'
# define K_LEFT		'h'
# define K_RIGHT	'l'
# define K_DOWN		'n'

/* I don't want these under UNIX (?) */
# define K_PGUP		'^'
# define K_PGDN		'v'
# define K_END		'>'
# define K_HOME		'<'
# define K_DEL		8
# define K_BS		0x7f
# define K_STAB		'['
# define K_CLEFT	'{'
# define K_CRIGHT	'}'
#endif /* UNIX */

/* These are common */
#define K_CTRLA   1
#define K_CTRLB   2
#define K_CTRLC   3
#define K_CTRLD   4
#define K_CTRLE   5
#define K_CTRLF   6
#define K_CTRLG   7
#define K_CTRLH   8
#define K_CTRLI   9
#define K_TAB     9
#define K_CTRLJ  10
#define K_CTRLK  11
#define K_CTRLL  12
#define K_CTRLM  13
#define K_RETURN 13
#define K_ENTER  13
#define K_CR     13
#define K_CTRLN  14
#define K_CTRLO  15
#define K_CTRLP  16
#define K_CTRLQ  17 
#define K_CTRLR  18
#define K_CTRLS  19
#define K_CTRLT  20
#define K_CTRLU  21
#define K_CTRLV  22
#define K_CTRLW  23
#define K_CTRLX  24
#define K_CTRLY  25
#define K_CTRLZ  26

#define K_VTDEL  8

#endif /* _KEYS_H */
