.SOURCE:        $(MCPDIR)
.SOURCE.exe:    ..\exe\mcp
.SOURCE.obj:    $(COMP)$(MODE)
.SOURCE.dll:    ..\dll
.SOURCE.lib:    ..\lib

MODEL := $(MLARGE)
CFLAGS += $(C_DEBUG) $(C_OUTDIR)$(COMP)$(MODE)\$()$(@:f)
LFLAGS += deb all

.IF $(FLAT)==YES

LFLAGS += opt verb opt map=..\map\mcp\$*.map
OPT32 := 32

all .PHONY:     dll normal

.ELSE

LFLAGS += opt verb opt map=..\map\mcp\$*.map
all .PHONY:     dll

.END


normal .PHONY:
        $(MAKE) normal_bld

dll .PHONY:
        $(MAKE) dll_bld

.IF $(MAKETARGETS)==normal_bld

CFLAGS += $(C_MT)
LFLAGS += opt st=32k

normal_bld .PHONY: mcp.exe maxshow.exe maxstart.exe maxend.exe

mcp.exe: mcp.obj mcp_max.obj mcp_cli.obj mcp_log.obj

maxshow.exe: maxshow.obj

maxstart.exe: maxstart.obj

maxend.exe: maxend.obj

.ELIF $(MAKETARGETS)==dll_bld

dll_bld .PHONY: mcp$(OPT32).dll

CFLAGS += $(C_DLL)

mcp$(OPT32).dll: mcp_dll.obj
        wlink $(LFLAGS) @mcp$(OPT32)
        wlib ..\lib\mcp$(OPT32) -+..\dll\mcp$(OPT32).dll

.END

rcs .PHONY:
        rin *.c *.h *.lnk *.mk

clean .PHONY:
        -del w3p\* wcp\* *.err ..\exe\mcp\* /n

.DIRCACHE=no

