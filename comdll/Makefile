WHICH=telnet

include ../vars.mk

ifeq ($(WHICH),telnet)
libcomm.so: telnetcomm.o
else
libcomm.so: modemcom.o
endif
	gcc -shared $^ -o $@

all: libcomm.so

install_libs install: libcomm.so
	cp -f $^ $(LIB)

telnetcomm.o: ipcomm.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -ggdb -DTELNET -c $^ -o $@

modemcom.o: modemcom.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -ggdb -c $^ -o $@


clean:
	-rm libcomm.so *.o
