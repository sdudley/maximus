.SOURCE.exe: ..\exe\install
.SOURCE.h:   $(TUIDIR)


CFLAGS = $(C_STDOPS) $(C_MODEL)$(MODEL) $(C_OUTDIR)$@ $(C_DEBUG) /zt64


INSTOBJS := install.obj     cvt202.obj      wdearc.obj                  \
            wio.obj                         decode.obj      maketree.obj\
            maketbl.obj     huf.obj

.IF $(MAKETARGETS) == arc.exe

MODEL   := $(MLARGE)
CFLAGS  += $(C_STDOPS_OPT) $(C_OUTDIR)$@

# Separate out directory needed since uint is defined differently
# for compress versus decompress (!).  See ar.h.

.SOURCE.obj: $(CMODE)a

OBJS := arc.obj wio.obj encode.obj maketree.obj maketbl.obj huf.obj

arc.exe: $(OBJS)

.ELIF $(MAKETARGETS) == instr.exe

.SOURCE.obj: $(COMP)$(MODE)
MODEL   := $(MLARGE)
LIBS   +:= $(TUILIB)
OBJS    := $(INSTOBJS)
CFLAGS  += $(C_DEFINE)MAX_INSTALL

instr.exe: $(OBJS)
        wlink @instr

.ELIF $(MAKETARGETS) == install.exe

.SOURCE.obj: $(COMP)$(MODE)
MODEL   := $(MLARGE)
LIBS   +:= $(TUILIB)
OBJS    := $(INSTOBJS)
CFLAGS  += $(C_DEFINE)MAX_INSTALL

install.exe: $(OBJS) instr.exe
.IF $(FLAT)==NO
        wlink @install
.ELSE
        wlink @inst32
.END

.ELSE

all .PHONY:
.IF $(MODE)==r
        $(MAKE) instr.exe
.ELSE
        $(MAKE) arc.exe
        $(MAKE) install.exe
.END

.END

install.obj: install.c install.dlg



clean .PHONY:
        -del ..\exe\install\*.exe wcp wcr /n

.DIRCACHE=no

