.SOURCE: $(LIBDIR);.;
.SOURCE.exe: ..\exe\mex
.SOURCE.dll: ..\dll
.SOURCE.lib: ..\lib

LFLAGS  := $(L_WARNDUPE) $(L_STDOPS) $(L_DEBUG) 
CFLAGS  += $(C_OUTDIR)$(MEXOUTPATH)\$(@:f) $(C_DEBUG)
MODEL   := $(MLARGE)

.IF $(COMP)==WC
LFLAGS += opt verb opt map=..\map\mex\$*.map
.END

.IF $(COMP)==W3
LFLAGS += opt verb opt map=..\map\mex\$*.map
.END

.IF $(FLAT)==YES
OPT32   := 32
.END

VMDLL   := mexvm$(OPT32).dll


.IF $(COMP)==WC
LFLAGS += $(L_STACK)16384
.ELIF $(COMP)==W3
LFLAGS += $(L_STACK)32768
.END

VMALL_OBJS :=   vm_run.obj      vm_heap.obj     vm_symt.obj             \
                vm_read.obj     vm_opcvt.obj    vm_opflo.obj            \
                vm_opfun.obj    vm_opmth.obj    vm_opstk.obj            \
                vm_opstr.obj


.IF $(MODE)==p
PTARGETS := mexvm
.ELSE
PTARGETS := vms
.END

all .PHONY: mex $(PTARGETS)

mex .PHONY:
        $(MAKE) mex$(OPTP).exe

vms .PHONY:
        $(MAKE) vms$(COMP)$(MODE).lib
#        $(MAKE) vms$(OPTP)$(OPT32).exe

#vmd .PHONY:
#        $(MAKE) vmd$(OPTP)$(OPT32).exe

mexvm .PHONY:
        $(MAKE) $(VMDLL)


.IF $(MAKETARGETS) == mex$(OPTP).exe

MEXOUTPATH := $(COMP)$(MODE)
.SOURCE.obj: $(COMP)$(MODE)

TABOBJ  :=      mex_tab.obj

SEMOBJS :=      sem_decl.obj sem_func.obj sem_scop.obj sem_expr.obj       \
                sem_flow.obj sem_goto.obj sem_gen.obj  sem_vm.obj

OBJS    :=      mex_main.obj mex_lex.obj  mex_symt.obj $(SEMOBJS)         \
                mex_misc.obj mex_err.obj

mex_tab.c: mex_tab.y
        +if exist $*.c del $*.c
        +echo Expect 1 shift/reduce conflict
        $(YY) $(YFLAGS) $(Y_OUT) $*.c $*.y

mex$(OPTP).exe: $(TABOBJ) $(OBJS)


.ELIF $(MAKETARGETS) == vms$(COMP)$(MODE).lib

#############################################################################
##                                   VMS.LIB                               ##
#############################################################################

MEXOUTPATH := $(COMP)$(MODE)
.SOURCE.obj: $(COMP)$(MODE)

CFLAGS +:= $(C_DYNAM)

vms$(COMP)$(MODE).lib: $(VMALL_OBJS)


#############################################################################
##                                  MEXVM                                  ##
#############################################################################

.ELIF $(MAKETARGETS) == mexvm$(OPT32).dll

MEXOUTPATH := $(COMP)$(MODE)d
.SOURCE.obj: $(COMP)$(MODE)d

CFLAGS     += $(C_DLL)

mexvm$(OPT32).dll: vm_dll.obj $(VMALL_OBJS)
        wlink @mexvm$(OPT32)
        wlib ..\lib\mexvm$(OPT32) -+..\dll\mexvm$(OPT32).dll

vm_dll.obj: vm_dll.c
        $(CC) $(CFLAGS) $(C_DLL) vm_dll

.END

rcs .PHONY:
        rin *.c *.h *.lnk *.mk

clean .PHONY:
        -del w3p\*.* w3pd\*.* *.err mex_tab.c mex_tab.h mex_tab.out /n

.DIRCACHE=no
