# Solaris 2 or SunOS 4 with GNU Make, GNU C
# Tested with gcc 2.95.3 from www.sunfreeware.com

OS_RELEASE=$(strip $(shell $(UNAME) -r | $(SED) -e 's/\.//' -e 's/\(..\)\(.*\)/\1/'))

#ifneq(,$(findstring $(OS_RELEASE) ,55 56 57 58 59 ))
PATH		+= /usr/ccs/bin
YACC		= yacc
YFLAGS		+= -Qy
CPPFLAGS	+= -DSYSV -DSOLARIS
OS_LIBS		+= -lnsl -lsocket -lpthread
BSD_INSTALL	:= /usr/ucb/install
OS_OK		= 1
#endif

#ifneq(,$(findstring $(OS_RELEASE) ,40 41 ))
#OS_OK		= 1
#CPPFLAGS	+= -DBSD -DSUNOS4
#endif

ifndef $(OS_OK)
$(error Unknown SunOS version -- fix vars_sunos.mk)
endif

ifeq ($(PREFIX),)
PREFIX		= /opt/max
endif
