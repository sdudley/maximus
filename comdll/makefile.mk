.SOURCE: $(COMP)$(MODE)\\s
.SOURCE.c: . ..\slib
.SOURCE.obj: $(COMP)$(MODE)
.SOURCE.lib: $(LIBDIR)
.SOURCE.dll: $(DLLDIR)
.SOURCE.exe: ..\exe\comdll

CFLAGS  = $(C_STDOPS_OPT) $(C_OUTDIR)$@ $(C_DLL)

all .PHONY: ntcomm.dll pdclient.exe

pdclient.exe: pdclient.obj

ntcomm.dll: ntcomm.obj ntcommdl.obj ntcomm.lnk
        $(WLLINK) @ntcomm
        wlib /q ..\lib\ntcomm -+..\dll\ntcomm.dll

#%.obj: %.c
#        wcc386 /bt=nt /s /bd /dNT /d2 /Fo=W3n\\$* $<

