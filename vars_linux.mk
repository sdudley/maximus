# Linux with GNU Make, GNU C

EXTRA_CPPFLAGS	+= -DLINUX
OS_LIBS		= -lpthread
CFLAGS          += -fPIC
CXXFLAGS        += -fPIC

ifeq ($(PREFIX),)
PREFIX		= /var/max
endif
