# Source and target paths

# Standard output directory
OUTDIR := $(COMP)$(MODE)

.SOURCE.lib: ..\lib
.SOURCE.exe: ..\exe\\btree
.SOURCE.dll: ..\dll
.SOURCE.obj: $(OUTDIR)

DLL_FLAGS =

# Always use the large memory model

MODEL := $(MLARGE)

# Standard compile/link flags

CFLAGS += $(C_DEBUG) $(C_OUTDIR)$(@:s|\\|\|)$() $(C_EXCEPTION)
LFLAGS := $(L_NOI) $(L_WARNDUPE) $(L_STDOPS) $(L_DEBUG)

.IF $(COMP)==WC
LFLAGS += opt verb opt map=..\map\btree\$*.map
.END

.IF $(COMP)==W3
LFLAGS += opt verb opt map=..\map\btree\$*.map
.END


all .PHONY: tracklib trkexp$(OPTP).exe trkimp$(OPTP).exe bttest$(OPTP).exe #audit.exe

# If we're doing OS/2, make a .DLL.  Otherwise, just build a
# static .lib file.

.IF $(MODE)==p

.IF $(FLAT)==YES
OPT32 := 32
.END

DLL_FLAGS = $(C_DLL)

BTREELIB := ..\lib\maxbt$(OPT32).lib
TRACKLIB := ..\lib\maxdb$(OPT32).lib

tracklib .PHONY:
        $(MAKE) maxbt$(OPT32).dll
        $(MAKE) maxdb$(OPT32).dll

.ELSE
.IF $(MODE)==n

OPT32 := nt

DLL_FLAGS = $(C_MT)

BTREELIB := ..\lib\maxbt$(OPT32).lib
TRACKLIB := ..\lib\maxdb$(OPT32).lib

tracklib .PHONY:
        $(MAKE) maxbt$(OPT32).dll
        $(MAKE) maxdb$(OPT32).dll

.ELSE
BTREELIB := $(COMP)$(MODE)btr$(MODEL).lib
TRACKLIB := $(COMP)$(MODE)tra$(MODEL).lib
tracklib .PHONY: $(BTREELIB) $(TRACKLIB)
.END
.END

BTREE_OBJ :=    btree.obj       bt_open.obj     bt_look.obj     \
                bt_ins.obj      bt_rem.obj                      \
                                                                \
                btnode.obj      palist.obj      btreec.obj      \
                                                                \
                                                                \
                blkio.obj       bbuf.obj        blkiobuf.obj    \
                share.obj       
#               bt_lookr.obj

TRACK_OBJ :=    dbase.obj       dbasec.obj                      \
                track.obj       trackc.obj


# MAXDBASE.DLL builder

.IF $(MAKETARGETS)==maxdb$(OPT32).dll

CFLAGS +:= $(DLL_FLAGS)

TRACK_OBJ +:= dllc.obj

maxdb$(OPT32).dll: $(TRACK_OBJ)
        $(WLLINK) opt q @maxdb$(OPT32)
        wlib $(TRACKLIB) -+$@

.ELIF $(MAKETARGETS)==maxbt$(OPT32).dll

CFLAGS +:= $(DLL_FLAGS)

BTREE_OBJ +:= dllc.obj

maxbt$(OPT32).dll: $(BTREE_OBJ)
        $(WLLINK) opt q @maxbt$(OPT32)
        wlib $(BTREELIB) -+$@

.ELSE

audit.obj:      audit.c
        wcc /ml /fo=$@ audit

audit.exe:      audit.obj       $(TRACKLIB) $(BTREELIB) cppmain.obj

trkexp$(OPTP).exe:   trackexp.obj    $(TRACKLIB) $(BTREELIB)

trkimp$(OPTP).exe:   trackimp.obj    $(TRACKLIB) $(BTREELIB)

bttest$(OPTP).exe:     bttest.obj      $(TRACKLIB) $(BTREELIB)

ctbase.exe: ctbase.obj $(TRACKLIB) $(BTREELIB)
        $(WLLINK) @ctbase.lnk

$(TRACKLIB):    $(TRACK_OBJ)

$(BTREELIB):    $(BTREE_OBJ)

.END

.IF $(FLAT)==YES
dllc.obj: dllc.c
        wcc386 /dOS_2 /d2/oals/s/j/zp1/ml/d2 /Fo=$@ /zu/mf/r/bd dllc.c
.ELSE
dllc.obj: dllc.c
        wcc /dOS_2 /2 /d2/oals/s/j/zp1/ml/d2 /fo=$@ /zu/ml/r/bd dllc.c

.END

rcs .PHONY:
        rin *.c *.h *.cc *.lnk *.asm makefile.mk
