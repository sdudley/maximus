include ../vars.mk

APIOBJS     :=  msgapi.obJ                                              \
                                                                        \
                sq_area.obJ                                             \
                sq_msg.obJ                                              \
                sq_read.obJ                                             \
                sq_write.obJ                                            \
                sq_kill.obJ                                             \
                sq_uid.obJ                                              \
                sq_lock.obJ                                             \
                sq_misc.obJ                                             \
                sq_help.obJ                                             \
                sq_idx.obJ                                              \
                                                                        \
                api_sdm.obJ                                    \
                                                                        \
                bld.obJ structrw.o
	

APIOBJS := $(APIOBJS:.obJ=.o)

# in libmax.so
#
#APIOBJS     +=  1stchar.obJ cvtdate.obJ date2bin.obJ dosdate.obJ fexist.obJ \
#                ffind.obJ flush.obJ months.obJ $(COMP)_misc.obJ parsenn.obJ \
#                qksort.obJ stristr.obJ strocpy.obJ trail.obJ _ctype.obJ     \
#                nopen.obJ setfsize.obJ tdelay.obJ

BLDOBJS     :=  bld.o
MSGAPIOBJS  :=  msgapi.o
API_SQOBJS  :=  sq_area.o sq_misc.o
API_SDMOBJS :=  api_sdm.o

.PHONY: all install

all: libmsgapi.so
install_libs install: libmsgapi.so
	cp -f $^ $(LIB)

$(BLDOBJS):     bld.h
$(MSGAPIOBJS):  msgapi.h apidebug.h
$(API_SQOBJS):  msgapi.h apidebug.h
$(API_SDMOBJS): msgapi.h apidebug.h

# Build rule for the .DLL

libmsgapi.so: $(APIOBJS)
	gcc -shared $^ -o $@

clean:
	-rm *.o *.so
