include ../vars.mk

PROGS := 	maid mecca accem ansi2bbs scanbld ansi2mec cvtusr \
		editcall mr fixlr setlr fb silt #maxpipe mxpipe32 piper

all: $(PROGS)
#install: $(PROGS)
install: maid mecca accem silt fb ansi2bbs
	cp -f $^ $(BIN)

maid: EXTRA_LD_LIBRARY_PATH=$(SRC)/slib $(SRC)/unix
maid: maid.o
mecca: mecca.o init.o

mecca.c init.c: mecca_vb.h

mecca_vb.h: mecca_vb.h.in
	sort < $< > $@

mecca.h: mecca_vb.h
mecca.o init.o accem.o: mecca.h
fixlr setlr fb mr editcall mecca cvtusr ansi2mec ansi2bbs: EXTRA_LOADLIBES += -lmsgapi
ansi2bbs.o: ansi2bbs.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DANSI2BBS -o $@ -c $<

ansi2mec.o: ansi2bbs.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DANSI2MEC -o $@ -c $<

cvtusr:		cvtusr.o cppmain.o cvt_misc.o
scanbld accem: EXTRA_LOADLIBES += -lmsgapi
accem:		accem.o init.o
scanbld:	cppmain.o scanbld.o


SILT_OBJS   :=  silt.obj        cppmain.obj     s_equip.obj     s_matrix.obj \
                s_menu.obj      s_misc.obj      s_parse.obj     s_sessio.obj \
                s_system.obj    s_reader.obj    s_lang.obj      s_colour.obj \
                s_proto.obj     s_marea.obj     s_farea.obj     s_heap.obj   \
                s_area.obj      s_access.obj    l_attach.obj    max2priv.obj

SILT_OBJS := $(SILT_OBJS:.obj=.o)

silt: $(SILT_OBJS)

#silt.o s_menu.o: option.h

#silt.o m_colour.o: colour.h
l_attach.c max2priv.c:
	ln -s ../max/$@ $@


clean:
	rm *.o $(PROGS)

max2priv.o:	CPPFLAGS += -I../lang
silt: EXTRA_LOADLIBES += -lmaxdb -lmaxbt -lmsgapi
