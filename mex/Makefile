include ../vars.mk

EXTRA_CPPFLAGS += -DHEAP_SIGNATURE -DDEBUGVM

VMDLL   := libmexvm.so

VMALL_OBJS :=   vm_run.obj      vm_heap.obj     vm_symt.obj             \
                vm_read.obj     vm_opcvt.obj    vm_opflo.obj            \
                vm_opfun.obj    vm_opmth.obj    vm_opstk.obj            \
                vm_opstr.obj

VMALL_OBJS := $(VMALL_OBJS:.obj=.o)

LOADLIBES := $(LOADLIBES) -lmsgapi
#CPPFLAGS += -DWES_HACK

.PHONY: all

all: $(VMDLL) mex

TABOBJ  :=      mex_tab.obj

SEMOBJS :=      sem_decl.obj sem_func.obj sem_scop.obj sem_expr.obj       \
                sem_flow.obj sem_goto.obj sem_gen.obj  sem_vm.obj

OBJS    :=      mex_main.obj mex_lex.obj  mex_symt.obj $(SEMOBJS)         \
                mex_misc.obj mex_err.obj

OBJS := $(OBJS:.obj=.o)
TABOBJ := $(TABOBJ:.obj=.o)
SEMOBJS := $(SEMOBJS:.obj=.o)

mex.h: mex_tab.h
$(VMALL_OBJS) $(OBJS) $(TABOBJ): mex.h

# Expect 1 shift/reduce conflict
mex_tab.c: mex_tab.y 

mex: $(TABOBJ) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES)

$(VMDLL): $(VMALL_OBJS)
	gcc -shared $^ -o $@

vm_dll.obj: vm_dll.c
	$(CC) $(CFLAGS) $(C_DLL) vm_dll

clean:
	-rm mex_tab.c mex_tab.h *.o *.a *.so mex

.PHONY: install_libs install_binaries install clean

install_libs: $(VMDLL)
	cp -f $^ $(LIB)

install_binaries: mex
	cp -f $^ $(BIN)

install: install_libs install_binaries
	
