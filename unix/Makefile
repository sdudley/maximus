include ../vars.mk

.PHONY: all install install_libs

all: libcompat.so

install_libs: libcompat.so
	@[ -d "$(LIB)" ] || mkdir -p "$(LIB)"
	cp -f $^ "$(LIB)"

install: install_libs

libcompat.so: dosio.o winstr.o conio.o viocurses.o dossem.o dosproc.o process.o wincomm.o xpunix.o
	$(CC) $(CFLAGS) -shared $^ $(LDFLAGS) -o $@


clean:
	-rm *.o libcompat.so
