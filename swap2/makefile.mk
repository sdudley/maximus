CFLAGS := /ml
AFLAGS += /DMODL=large

all: swap.lib test.exe

swap.lib: spawn.obj exec.obj checkpat.obj

test.exe: test.obj swap.lib
        wlink sys dos name test file test lib swap

clean:
        -del *.obj *.exe *.lib
