#
#  SqaFix 0.99b8 port to Linux
# 
#  This makefile was taken from the GPL'ed Maximus code from the squish
#  directory and modified to compile sqafix with the Maximus source code
#  distributed under the GPL.
#
#  Copyright 2003  R. F. Jones and Bo Simonsen
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#  Notes:
#    Need to add installation of sqafix configuration files to the 
#    install portion of this script.
#
include ../vars.mk

MAINTARGETS := sqafix
EXTRATARGETS:= 
MAINTARGETS += $(EXTRATARGETS)
EXTRA_LOADLIBES += -lmax -lmsgapi

.PHONY: all install install_libs install_binaries

all: $(MAINTARGETS) 

SQUISH_OBJS :=	crc32.c		\
		lst-mngr.obj	sqaarc.obj	sqacfg.obj	\
		sqafix.obj	sqamsg.obj 	\
		sqastr.obj	sqaexe.obj	fnmerge.obj	\
		sqauti.obj
 
SQUISH_OBJS := $(SQUISH_OBJS:.obj=.o) 

$(SQUISH_OBJS):        sqafix.h
sqafix.o sqafix: CPPFLAGS += -DINSTALL_PREFIX=\"$(PREFIX)\" 
sqafix: $(SQUISH_OBJS)

install: install_libs install_binaries

install_binaries: $(MAINTARGETS)
	cp -f $^ $(BIN)

clean:
	-rm -f $(MAINTARGETS) *.o

