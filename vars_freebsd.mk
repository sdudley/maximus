# FreeBSD with GNU Make, GNU C

EXTRA_CPPFLAGS	+= 
OS_LIBS		= -pthread

ifeq ($(PREFIX),)
PREFIX		= /var/max
endif

