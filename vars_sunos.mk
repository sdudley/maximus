# Solaris 2 or SunOS 4 with GNU Make, GNU C
# Tested with gcc 2.95.3 from www.sunfreeware.com

OS_RELEASE=$(strip $(shell $(UNAME) -r | $(SED) -e 's/\.//' -e 's/\(..\)\(.*\)/\1/'))

ifneq (,$(findstring $(OS_RELEASE) ,55 56 57 58 59 ))
ifneq (,$(findstring $(YACC),yacc))
YFLAGS		+= -Qy
endif
EXTRA_CPPFLAGS	+= -DSYSV -DSOLARIS -DHAVE_TIMER_T
OS_LIBS		+= -lnsl -lsocket -lpthread
BSD_INSTALL	:= /usr/ucb/install
CFLAGS		+= -fPIC
CXXFLAGS	+= -fPIC
OS_OK		= 1
endif

OS_OK=1

ifneq (,$(findstring $(OS_RELEASE) ,40 41 ))
OS_OK		= 1
CPPFLAGS	+= -DBSD -DSUNOS4
endif

ifneq ($(strip $(OS_OK)),1)
$(error Unknown SunOS version $(OS_RELEASE) -- fix vars_sunos.mk) 
endif

ifeq ($(PREFIX),)
PREFIX		= /opt/max
endif
