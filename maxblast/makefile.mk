.SOURCE.obj: $(CMODE)
.SOURCE.dll: $(DLLDIR)
.SOURCE.lib: $(LIBDIR)

MODEL           := $(MLARGE)
LFLAGS          += opt map,verb

.IF $(FLAT)==YES
foo: error_cannot_compile_in_flat_mode
.END

.IF $(MODE)==p
OPTP            := p
MAXBLASTDLL     := maxblast.dll
MAXBLAS2DLL     := maxblas2.dll
BLASTOBJ        := maxblast.lib
.ELSE
BLASTOBJ        := maxblast.obj
.END

all .PHONY:     $(MAXBLASTDLL) $(MAXBLAS2DLL) play$(OPTP).exe

OBJS            := play.obj voc_file.obj voc_snd.obj $(BLASTOBJ)


play$(OPTP).exe:   $(OBJS)

$(OBJS):           voc_data.h voc_snd.h voc_file.h

.IF $(MODE)==p

$(MAXBLASTDLL):     maxblast.obj blastos2.obj
        wlink @maxblast
        wlib ..\lib\maxblast.lib -+$@

$(BLASTOBJ): $(DLLDIR)\maxblast.dll
        wlib ..\lib\maxblast.lib -+$(DLLDIR)\maxblast.dll

$(MAXBLAS2DLL):     maxblas2.obj blastos2.obj
        wlink @maxblas2

maxblast.obj: maxblast.asm
        masm /mx /dOS_2 maxblast,$(CMODE)\\maxblast;

blastos2.obj: blastos2.asm
        masm /mx /dOS_2 blastos2,$(CMODE)\\blastos2;
.END

CFLAGS += $(C_OUTDIR)$(CMODE)\$(@:f) $(C_DEBUG)
LFLAGS += $(L_DEBUG)

