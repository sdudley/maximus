include ../vars.mk

BTREE_OBJ :=    btree.obj       bt_open.obj     bt_look.obj     \
                bt_ins.obj      bt_rem.obj                      \
                                                                \
                btnode.obj      palist.obj      btreec.obj      \
                                                                \
                                                                \
                blkio.obj       bbuf.obj        blkiobuf.obj    \
                dllc.obj
#               bt_lookr.obj
#		share.obj

TRACK_OBJ :=    dbase.obj       dbasec.obj                      \
                track.obj       trackc.obj	dllc.obj

TRACK_OBJ := $(TRACK_OBJ:.obj=.o)
BTREE_OBJ := $(BTREE_OBJ:.obj=.o)

EXTRA_LOADLIBES = -lmaxdb -lmaxbt -lmsgapi -lstdc++
#LOADLIBES := -lmaxdb -lmaxbt $(LOADLIBES)

.PHONY:		all tracklib install install_libs install_bin

all:		tracklib trackexp trackimp bttest #audit

install_bin:	trackexp trackimp bttest
		@[ -d "$(BIN)" ] || mkdir -p "$(BIN)"
		cp -f $^ "$(BIN)"

install_libs:	libmaxdb.so libmaxbt.so
		@[ -d "$(LIB)" ] || mkdir -p "$(LIB)"
		cp -f $^ "$(LIB)"

install:	install_libs install_bin

tracklib:	libmaxbt.so libmaxdb.so

libmaxdb.so:	$(TRACK_OBJ)
		gcc -shared -nostartfiles $^ -o $@

libmaxbt.so:	$(BTREE_OBJ)
		gcc -shared -nostartfiles $^ -o $@

audit:		audit.o cppmain.o

ctbase:		ctbase.o

clean:
	-rm *.so *.o trackexp trackimp bttest
