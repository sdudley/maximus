WHICH=telnet

include ../vars.mk

ifeq ($(WHICH),telnet)
libcomm.so: telnetcomm.o
else
libcomm.so: stdcomm.o
endif
	gcc -shared $^ -o $@

all: libcomm.so

install_libs install: libcomm.so
	cp -f $^ $(LIB)

telnetcomm.o: ipcomm.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DTELNET -c $^ -o $@

stdcomm.o: stdcomm.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DTELNET -c $^ -o $@


clean:
	-rm libcomm.so *.o
