include ../vars.mk


#CFLAGS += -DTELNET
.PHONY: all install install_libs

all: libcomm.so

install_libs: libcomm.so
	@[ -d "$(LIB)" ] || mkdir -p "$(LIB)"
	cp -f $^ "$(LIB)"

install: install_libs

libcomm.so: ipcomm.o common.o modemcom.o
	$(CC) $(CFLAGS) -ggdb -shared $^ -lutil $(LDFLAGS) -o $@


clean:
	-rm *.o libcomm.so
