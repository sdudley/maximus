# Linux with GNU Make, GNU C

EXTRA_CPPFLAGS	+= -DLINUX
OS_LIBS		= -lpthread
CFLAGS          += -fPIC -ggdb
CXXFLAGS        += -fPIC -ggdb

ifeq ($(PREFIX),)
PREFIX		= /var/max
endif
