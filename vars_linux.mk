# Linux with GNU Make, GNU C

EXTRA_CPPFLAGS	+= -DLINUX
OS_LIBS		= -lpthread

ifeq ($(PREFIX),)
PREFIX		= /var/max
endif
