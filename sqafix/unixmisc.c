/****************************************************************/
/*								*/
/* File: unixmisc.c 						*/
/* This file is made by Bo Simonsen and distributed by GPL	*/
/*								*/
/*
 * SqaFix 0.99b8
 * Copyright 2003 Bo Simonsen
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/****************************************************************/


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int spawnlp(char* cmd, ...)
{
    va_list var_args;
    char * tmp;

    char * syscmd = NULL;

    va_start(var_args, cmd);
    syscmd = (char*) malloc(1024);
    
    strcpy(syscmd, cmd);
    
    while((tmp = va_arg(var_args, char*)) != NULL)
    {
	strcat(syscmd, " ");	
	strcat(syscmd, tmp);
    }
    return system(syscmd);
}


