.IMPORT: RC20EXE

.SOURCE:        $(SLIBDIR);.
.SOURCE.obj:    $(COMP)$(MODE)
.SOURCE.exe:    ..\exe\sm

MODEL := $(MLARGE)
CFLAGS += $(C_MT) $(C_DEBUG) $(C_OUTDIR)$@

RCCOMP := $(RC20EXE)

all .PHONY: sm.res sm.exe

OBJS := sm.obj          sm_view.obj             ezfont.obj              \
        pmdebug.obj     sm_th.obj               sm_msg.obj

sm.exe: $(OBJS)
        wlink @sm
        $(RCCOMP) sm.res ..\exe\sm\sm.exe

#ezfont.obj: ezfont.c
#        $(CC) $(CFLAGS) ..\..\slib\ezfont
#
#pmdebug.obj: pmdebug.c
#        $(CC) $(CFLAGS) ..\..\slib\pmdebug

sm.exe:: sm.res
        $(RCCOMP) sm.res ..\exe\sm\sm.exe

sm.ico: $(CVSROOT)\sm.ico
        +copy $(CVSROOT)\sm.ico sm.ico

sm.res: sm.rc sm.ico msg.dlg
        $(RCCOMP) -r sm

rcs .PHONY:
        rin *.c *.h *.dlg *.rc *.mk *.lnk *.ico

