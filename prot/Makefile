include ../vars.mk

OBJS = frecv.o   fsend.o   pdata.o   rbsb.o    rz.o      sz.o      zm.o

libxfer.so: $(OBJS)
	gcc -shared $^ -o $@

install: libxfer.so
	cp libxfer.so $(LIB)

clean:
	-rm $(OBJS) libxfer.so
