include ../vars.mk

MAINTARGETS := squish sqfix
EXTRATARGETS:= sqpack sqconv sqinfo sqset sstat sqreidx
MAINTARGETS += $(EXTRATARGETS)
EXTRA_LOADLIBES += -lmsgapi -ldl

.PHONY: all install install_libs install_binaries

all: $(MAINTARGETS) libkillrcat.so libmsgtrack.so 

SQUISH_OBJS :=	squish.obj   s_abbs.obj         s_config.obj    \
                s_scan.obj   s_toss.obj        s_pack.obj      \
                s_squash.obj s_match.obj        s_log.obj       \
                s_misc.obj   s_hole.obj         s_link.obj      \
                s_busy.obj   s_stat.obj         s_sflo.obj      \
                s_thunk.obj  s_dupe.obj		

SQUISH_OBJS := $(SQUISH_OBJS:.obj=.o) bld.o
bld.o: bld.h sqver.h
bld.h:
	bldupd bld.h

$(SQUISH_OBJS):        apidebug.h
squish.o squish: CPPFLAGS += -DINSTALL_PREFIX=\"$(PREFIX)\"
squish: $(SQUISH_OBJS)

libkillrcat.so: killrcat.o
	gcc -shared $^ -o $@

libmsgtrack.so: msgtrack.o
	gcc -shared $^ -o $@

install: install_libs install_binaries

install_libs: libkillrcat.so libmsgtrack.so
	cp -f $^ $(LIB)

install_binaries: $(MAINTARGETS)
	cp -f $^ $(BIN)

clean:
	-rm -f $(MAINTARGETS) *.o *.so

