include ../vars.mk

.PHONY: all install install_libs

all: libcompat.so

install_libs: libcompat.so
	@[ -d "$(LIB)" ] || mkdir -p "$(LIB)"
	cp -f $^ "$(LIB)"

install: install_libs

libcompat.so: adcase.o dosio.o winstr.o conio.o viocurses.o dossem.o dosproc.o wincomm.o xpunix.o process.o
	$(CC) $(CFLAGS) -ggdb -shared $^ -lutil $(LDFLAGS) -o $@


clean:
	-rm *.o libcompat.so
