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

#ifndef __NOPEN_H_DEFINED
#define __NOPEN_H_DEFINED

#include "uni.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"

#ifdef __TURBOC__
#undef sopen
int _stdc sopen(const char *name, int mode, int shacc, ...);
#endif

int _stdc nsopen(const char *name, int mode, int shacc, ...);
int _stdc nopen(const char *name, int mode, ...);
int _stdc nread(int fd, char *buf, unsigned len);
int _stdc nwrite(int fd, char *buf, unsigned len);
long _stdc nlseek(int fd, long ofs, int pos);
long _stdc ntell(int fd);
int _stdc nclose(int fd);
int _stdc ndup(int fd);
int _stdc ndup2(int fd1, int fd2);

#endif /* __NOPEN_H_DEFINED */

