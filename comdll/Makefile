include ../vars.mk

libcomm.so: fdcomm.o
	gcc -shared $^ -o $@

install: libcomm.so
	cp -f $^ /var/max/lib

clean:
	-rm fdcomm.o libcomm.so
