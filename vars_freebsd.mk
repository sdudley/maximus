# FreeBSD with GNU Make, GNU C

EXTRA_CPPFLAGS	+= -DHAVE_TIMER_T -DBSD
OS_LIBS		= -pthread
MDFLAGS		+= -D__FreeBSD__

ifeq ($(PREFIX),)
PREFIX		= /var/max
endif

# -DBSD: I'm a BSD UNIX!
# MDFLAGS set -D__FreeBSD__ in case makedepend doesn't. We do FreeBSD-specific checking
# against the __FreeBSD__ flag now, not FREEBSD. Generic BSD checking is done against
# the BSD #define.

