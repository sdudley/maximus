include ../vars.mk

# Assembly code which I ripped out of the object list:
#
# cpu_asm.asm   dv_cleol.asm  dv_infoa.asm  dv_putc.asm   dv_shad.asm   flat.asm      hfmemmov.asm
# share.asm     crit_asm.asm  dv_getch.asm  dv_mode.asm   dv_putch.asm  dv_sync.asm   flusha.asm    getch.asm
# hfread.asm    dv_blitz.asm  dv_gxyb.asm   dv_page.asm   dv_scrla.asm  ffinda.asm    fpu_asm.asm
# gsdate.asm    mtask.asm     xputch.asm

# Assembly code which wasn't in the objec list:
#
# get_cpu.asm   sleeper.asm   gsdate.asm

OBJS :=      hmalloc.obJ  hmemmove.obJ     \
hread.obJ                                   \
areaapi.obJ  userapi.obJ  md5.obJ                       \
nopen.obJ    fd2n.obJ     isdevice.obJ                  \
brktrap.obJ  ncheck.obJ   timer.obJ    1stchar.obJ      \
getword.obJ  avatar.obJ   fncystr.obJ  months.obJ       \
strftim.obJ  fexist.obJ   stristr.obJ  putss.obJ        \
zfree.obJ    qksort.obJ   memstr.obJ   hprintf.obJ      \
tolower.obJ  toupper.obJ  savedir.obJ  xprintf.obJ      \
xputs.obJ    crep.obJ     strstrx.obJ      \
aname.obJ    ffind.obJ    fdate.obJ    gedate.obJ       \
priv.obJ     getdisk.obJ  setdisk.obJ  weekday.obJ      \
get_fdt.obJ  set_fdt.obJ  c2s.obJ      makedir.obJ      \
sfopen.obJ   iqsort.obJ   shfopen.obJ  gwordq.obJ       \
ffname.obJ   sbs.obJ      isleap.obJ   tdelay.obJ       \
dmalloc.obJ  noise.obJ    tune.obJ     date2bin.obJ     \
dosdate.obJ  flush.obJ    canon.obJ    win.obJ          \
cvtdate.obJ  trail.obJ    parsenn.obJ  qslist.obJ       \
lcopy.obJ    fnsplit.obJ  uniqren.obJ  mktemp.obJ       \
soundex.obJ  address.obJ  mktime.obJ   cencode.obJ      \
ieee2lng.obJ ieee2msb.obJ lng2ieee.obJ msb2ieee.obJ     \
adj_user.obJ win_pick.obJ arc_def.obJ  coreleft.obJ     \
smalloc.obJ  strocpy.obJ                   \
cshopen.obJ  crc32.obJ    crc16.obJ    strrstr.obJ      \
crit.obJ     \
skiplist.obJ acomp.obJ    arc_cmd.obJ      \
arcmatch.obJ bfile.obJ    bprintf.obJ  _ctype.obJ       \
setfsize.obJ cstrupr.obJ  strnncpy.obJ zeller.obJ       \
getmax.obJ   prmapi.obJ   strbuf.obJ       \
$(COMP)_misc.obJ vio.obJ align.obJ

.PHONY: all install install_libs

all: libsmserial.so libmax.so

#OBJS += dv.obJ       dv_attr.obJ  dv_cls.obJ       \
#        dv_getxy.obJ dv_goxy.obJ        \
#        dv_info.obJ        \
#        dv_puts.obJ  dv_scrl.obJ      \
              
OBJS := $(OBJS:.obJ=.o) os2file.o os2key.o
DEPEND_FILES=$(OBJS:.o=.c)

libsmserial.so: smserial.o
	gcc -shared $^ -o $@

libmax.so: $(OBJS)
	gcc -shared $^ -o $@

install_libs: libsmserial.so libmax.so
	cp -f $^ "$(LIB)"

install:  install_libs

clean:
	-rm *.o *.so

