NO_DEPEND_RULE=TRUE

include ../vars.mk

LOADLIBES=$(OSLOADLIBES)

TARGETS=c_compiler stamp_combo endian

all: $(TARGETS)
clean:
	-rm $(TARGETS)

stamp_combo.o: stamp_combo.c ../slib/stamp.h ../slib/typedefs.h
stamp_combo.o: ../slib/compiler.h ../slib/compiler_details.h ../slib/compiler_unix.h
