LIB_DIRS  = unix btree slib squish mex prot comdll
PROG_DIRS = squish max mex util
DIRS 	  = $(LIB_DIRS) $(PROG_DIRS)

topmost:: header usage

include vars.mk
MAXIMUS=${PREFIX}/etc/max.prm

.PHONY: all clean install mkdirs squish max screwy install_libs install_binaries usage topmost build

header::
	@echo "Maximus-CBCS Master Makefile for UNIX Port"
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
	@echo "         build          build maximus and squish"
	@echo "         config_install install configuration files"
	@echo "         install        build and install everything"
	@echo "         squish         build squish"
	@echo "         squish_install build and install squish"
	@echo "         max            build maximus"
	@echo "         max_install    build and install maximus"
	@echo

mkdirs:
	[ -d "$(LIB)" ] || mkdir -p "$(LIB)"
	[ -d "$(BIN)" ] || mkdir -p "$(BIN)"

all:	mkdirs clean squish_install max_install

clean install_binaries install_libs:
	$(foreach DIR, $(DIRS), cd $(DIR) && $(MAKE) -k $@; cd ..; )

squish_install: mkdirs
	$(foreach DIR, btree slib squish unix, cd $(DIR) && $(MAKE) install_libs; cd ..; )
	cd squish && $(MAKE) install

max_install: mkdirs
	$(foreach DIR, $(LIB_DIRS), cd $(DIR) && $(MAKE) install_libs; cd ..; )
	cd util && $(MAKE)
	$(foreach DIR, $(PROG_DIRS), cd $(DIR) && $(MAKE) install; cd ..; )

squish:
	$(foreach DIR, btree slib unix squish, cd $(DIR) && $(MAKE); cd ..; )

max:
	$(foreach DIR, $(LIB_DIRS), cd $(DIR) && $(MAKE); cd ..; )
	cd util && $(MAKE)
	$(foreach DIR, $(PROG_DIRS), cd $(DIR) && $(MAKE); cd ..; )

config_install:
	$(warning config_install has never been tested!)
	[ ! -f ${PREFIX}/etc/max.ctl ] || echo "This is not a fresh install -- not copying install_tree directory"
	[ -f ${PREFIX}/etc/max.ctl ] || $(CP) -rp install_tree/* $(PREFIX)
	cd $(PREFIX)/etc && ../bin/silt max -x
	cd $(PREFIX)/etc && ../bin/maid -d english
	cd $(PREFIX)/m && $(foreach FILE, *.mex, ../bin/mex $(FILE:.mex=);)
	cd $(PREFIX) && bin/mecca $(shell find . -name \*.mec)
	[ ! -f ${PREFIX}/etc/user.bbs ] || echo "This is not a fresh install -- not creating new user.bbs"
	[ -f ${PREFIX}/etc/user.bbs ] || cd ${PREFIX}/etc && ../bin/max -c

install: mkdirs squish_install max_install config_install

build:	squish max

GPL gpl license::
	@[ -x /usr/bin/less ] && cat LICENSE | /usr/bin/less || /bin/true
	@[ ! -x /usr/bin/less ] && cat LICENSE | more || /bin/true
