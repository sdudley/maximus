# @file 	Makefile	Master Makefile for Makefile 3.03 onward
# @author			Wes Garland
# @date				May 13th, 2003
#
# $Id: Makefile,v 1.9 2004/01/19 23:37:01 paltas Exp $
# $Log: Makefile,v $
# Revision 1.9  2004/01/19 23:37:01  paltas
# Added some to get freebsd work, and improved maxcomm a bit..
#
# Revision 1.8  2003/10/05 01:56:37  rfj
# Updated master Makefile to not build SqaFix when compiling just Maximus
# code.
#
# Revision 1.7  2003/08/15 19:57:52  rfj
# Master makefile updated to support SqaFix source code as part of the Maximus
# SourceForge project.  SqaFix program is now under GPL.
#
# Revision 1.6  2003/06/29 20:38:51  wesgarland
# Cosmetic change
#
# Revision 1.5  2003/06/12 03:26:43  wesgarland
# Corrected PREFIX-passing between master Makefile and copy_install_ree.sh
#
# Revision 1.4  2003/06/12 02:50:52  wesgarland
# Modified to better support non-standard PREFIX
#
# Revision 1.3  2003/06/11 19:23:53  wesgarland
# Successfully performs a "make distclean; ./configure; make build; make install"
#
#

SQUISH_LIB_DIRS = btree slib unix msgapi squish
SQAFIX_LIB_DIRS = msgapi sqafix
MAX_LIB_DIRS	= slib unix msgapi mex prot comdll 
LIB_DIRS	= $(SQUISH_LIB_DIRS) $(SQAFIX_LIB_DIRS) $(MAX_LIB_DIRS)
PROG_DIRS	= squish max mex util 
DIRS		= $(LIB_DIRS) $(PROG_DIRS) sqafix
NO_DEPEND_RULE	:= TRUE

topmost:: header usage

include vars.mk
MAXIMUS=$(PREFIX)/etc/max.prm

.PHONY: all depend clean install mkdirs squish max install_libs install_binaries \
	usage topmost build config_install configure reconfig sqafix

header::
	@echo "Maximus-CBCS Master Makefile"
	@echo 

usage::
	@echo "Maximus was written by Scott Dudley (Lanius Corporation),"
	@echo "Peter Fitzsimmions, and David Nugent, and released in 2002"
	@echo "under the GPL (GNU Public Licence). The UNIX port is by"
	@echo 'Wes Garland. Type "make gpl" to view the text of the'
	@echo "licence."
	@echo
	@echo "Paths:                  (edit vars.mk to change)"
	@echo "         prefix         $(PREFIX)"
	@echo "         libraries      $(LIB)"
	@echo "         binaries       $(BIN)"
	@echo      
	@echo "Targets:"
	@echo "         build          build maximus, squish and SqaFix"
	@echo "         config_install install configuration files"
	@echo "         install        build and install everything"
	@echo "         squish         build squish"
	@echo "         squish_install build and install squish"
	@echo "         sqafix         build SqaFix"
	@echo "         sqafix_install build and install SqaFix"
	@echo "         max            build maximus"
	@echo "         max_install    build and install maximus"
	@echo

mkdirs:
	[ -d "$(LIB)" ] || mkdir -p "$(LIB)"
	[ -d "$(BIN)" ] || mkdir -p "$(BIN)"

all:	mkdirs clean squish_install max_install sqafix_install

clean:  
	$(foreach DIR, $(DIRS) configuration-tests, cd $(DIR) && $(MAKE) -k $@; cd ..; )
	-rm depend.mk.bak depend.mk
	-rm */depend.mk.bak */depend.mk

dist-clean distclean: clean
	-rm slib/compiler_details.h
	-rm vars.mk vars_local.mk

depend install_binaries install_libs:
	$(foreach DIR, $(DIRS), cd $(DIR) && $(MAKE) -k $@; cd ..; )

squish_install: mkdirs
	$(foreach DIR, $(SQUISH_LIB_DIRS), cd $(DIR) && $(MAKE) install_libs; cd ..; )
	cd squish && $(MAKE) install

sqafix_install: mkdirs
	$(foreach DIR, $(SQAFIX_LIB_DIRS), cd $(DIR) && $(MAKE) install_libs; cd ..; )
	cd sqafix && $(MAKE) install

max_install: mkdirs
	$(foreach DIR, $(MAX_LIB_DIRS), cd $(DIR) && $(MAKE) install_libs; cd ..; )
	cd util && $(MAKE)
	$(foreach DIR, $(PROG_DIRS), cd $(DIR) && $(MAKE) install; cd ..; )

squish:
	$(foreach DIR, $(SQUISH_LIB_DIRS), cd $(DIR) && $(MAKE); cd ..; )
	cd squish && $(MAKE)

sqafix:
	$(foreach DIR, $(SQAFIX_LIB_DIRS), cd $(DIR) && $(MAKE); cd ..; )
	cd sqafix && $(MAKE)

max:
	$(foreach DIR, $(MAX_LIB_DIRS), cd $(DIR) && $(MAKE); cd ..; )
	cd util && $(MAKE)
	$(foreach DIR, $(PROG_DIRS), cd $(DIR) && $(MAKE); cd ..; )

configure:
	./configure "--prefix=$(PREFIX)"

config_install:
	@export PREFIX
	@scripts/copy_install_tree.sh "$(PREFIX)"

	@$(MAKE) reconfig

	@[ ! -f ${PREFIX}/etc/user.bbs ] || echo "This is not a fresh install -- not creating new user.bbs"
	@[ -f ${PREFIX}/etc/user.bbs ] || echo "Creating user.bbs"
	@[ -f ${PREFIX}/etc/user.bbs ] || (cd ${PREFIX} && bin/max etc/max -c || /bin/true)
	@echo
	@echo "Configuration complete."

reconfig:
	@echo "Pass one"
	@echo " - Compiling english.mad (bootstrap)"
	@cd $(PREFIX)/etc/lang && $(PREFIX)/bin/maid english -p

	@echo " - Compiling MECCA help files"
	@cd $(PREFIX) && bin/mecca etc/help/\*.mec

	@echo " - Compiling misc MECCA files"
	@cd $(PREFIX) && bin/mecca etc/misc/\*.mec

	@echo " - Compiling max.ctl"
	@cd $(PREFIX) && bin/silt etc/max -x

	@echo " - Compiling MEX files"
	@cd $(PREFIX)/m && $(foreach FILE, *.mex, ../bin/mex $(FILE:.mex=);)

	@echo
	@echo "Pass two"
	@echo " - Re-Compiling english.mad "
	@cd $(PREFIX)/etc/lang && $(PREFIX)/bin/maid english -d -s -p$(PREFIX)/etc/max
	@echo " - Re-Compiling max.ctl"
	@sleep 2 # Quell Warnings in max
	@cd $(PREFIX) && bin/silt etc/max -p

install: mkdirs squish_install sqafix_install max_install config_install

build:	squish sqafix max
	@echo "Build Complete; edit your control files and 'make install'"

GPL gpl license::
	@[ -x /usr/bin/less ] && cat LICENSE | /usr/bin/less || /bin/true
	@[ ! -x /usr/bin/less ] && cat LICENSE | more || /bin/true
