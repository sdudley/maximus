include ../vars.mk

LANGHDR     	:=  ../lang/english.lth
LANGSRC     	:=  ../lang/english.mad
LOADLIBES	+= -lmexvm -lmsgapi -lsmserial -lmax -lmaxbt -lmaxdb -lxfer -lcomm -lcompat
CFLAGS 		+= -USWAP -UOVERLAY_BUILD
CPPFLAGS	+= -I../lang

# Main maximus object

MAXOBJ := max.o

# Other run-time objects

MOBJS :=        uedit.obj       ued_cmds.obj    ued_disp.obj    \
display.obj     disp_dat.obj    disp_qu.obj     max_rip.obj     \
disp_max.obj    disp_mis.obj    med_add.obj     maxed.obj       \
med_scrn.obj    med_move.obj    med_del.obj     med_quot.obj    \
med_read.obj    med_misc.obj    f_area.obj      f_con.obj       \
f_intrin.obj    f_isval.obj     f_kill.obj      f_locate.obj    \
f_misc.obj      f_over.obj      f_raw.obj       f_tag.obj       \
f_titles.obj    f_type.obj      f_up.obj        f_logup.obj     \
f_xfer.obj      f_idx.obj       f_down.obj      f_hurl.obj      \
f_queue.obj                                                     \
tagapi.obj      m_area.obj      m_browse.obj    mb_list.obj     \
mb_qwk.obj      mb_qwkup.obj    m_change.obj    m_create.obj    \
m_editor.obj    m_editu.obj     m_enter.obj     m_for.obj       \
m_full.obj      m_header.obj    mh_graph.obj    mh_tty.obj      \
m_hurl.obj      m_inq.obj       m_intrin.obj    m_isval.obj     \
m_kill.obj      m_lread.obj     m_mdate.obj     m_node.obj      \
m_read.obj      m_reply.obj     m_save.obj      m_scan.obj      \
m_tag.obj       mb_novl.obj     m_upload.obj    m_xport.obj     \
me_misc.obj     m_edit.obj      m_misc.obj      m_restr.obj     \
mb_read.obj     m_util.obj      m_attach.obj    l_attach.obj    \
t_add.obj       t_disp.obj      t_kill.obj      max_clss.obj    \
t_menu.obj      t_misc.obj      t_qwk.obj       t_report.obj    \
static.obj      language.obj    api_brow.obj    mex.obj         \
mexint.obj      debug.obj       events.obj      max_in.obj      \
max_gets.obj    max_locl.obj    max_out.obj     max_outl.obj    \
max_outr.obj    max_wfc.obj     max_fman.obj    max_init.obj    \
max_mtsk.obj    max_inif.obj    max_args.obj    max_rest.obj    \
max_fini.obj    max_mcmd.obj    max_misc.obj    max_cust.obj    \
max_menu.obj    max_rmen.obj    max_runo.obj    max_main.obj    \
max_chng.obj    node.obj        max_log.obj     max_ocmd.obj    \
max_fbbs.obj    max_bor.obj     max_cmod.obj    max_xtrn.obj    \
max_fins.obj    max_chat.obj    max_cho.obj     max_bar.obj     \
fos.obj							log.obj         max_sq.obj      \
v7.obj          joho.obj        emsi.obj        fos_dos.obj     \
     max_prot.obj    fos_os2.obj     callinfo.obj    \
async.obj       medinit.obj     os2.obj         dos.obj         \
				thunk.obj       asyncnt.obj						\
max2priv.obj    mexintu.obj     mexstat.obj     mexfarea.obj    \
mexcall.obj     mexoutp.obj     mextime.obj     mexlang.obj     \
mexrip.obj      mexstr.obj      mexmarea.obj    mexfile.obj     \
mexclass.obj    mexuser.obj     mexinp.obj      mextag.obj      \
mexffind.obj    mexxtrn.obj     mexrtl.obj      max_accs.obj    \
m_input.obj     m_lines.obj     f_okay.obj      atexit.obj

#callfos.obj

# Stack overflow handler for WC
#MOBJS       += stk.obj md5.obj tune.obj uniqren.obj strbuf.obj arc_def.obj \
#               noise.obj

MOBJS := $(strip $(MOBJS:.obj=.o))

DEPEND_FILES = $(MOBJS:.o=.c) max.c date.c

# Protocol objects

#ZOBJS       :=  rbsb.obj        rz.obj          zm.obj          sz.obj  \
#                fsend.obj       frecv.obj       pdata.obj
#ZOBJS := $(ZOBJS:.obj=.o)

# Total object list

OBJS        :=  $(strip $(MAXOBJ) $(MOBJS) $(ZOBJS) cppmain.o)

.PHONY: all install install_binaries
all: max
install: max
	cp -f $^ $(BIN)

.INTERMEDIARY: date.o

max: $(OBJS) date.o

# Wes: strange build line forces g++ for linking with objects containing cppmain.o.
# This is needed for gcc 3; otherwise we'd need to figure out which targets to
# add libgcc_s.so to, and only only gcc 3. This should work for gcc 2 and gcc 3.

max:
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) -o $@

# max2 is the same as max, but most of the object files are
# moved into an solib, to allow use to track function calls
# with truss -ulibmax2
libmax2.so: LD=gcc -shared
max2:	$(MAXOBJ) cppmain.o date.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -lmax2 $(LOADLIBES) -o $@

libmax2.so: $(MOBJS) $(ZOBJS)

bldupd.o: bldupd.h

.PRECIOUS: ../util/maid $(LANGHDR)

../util/maid:
	cd ../util && $(MAKE) maid

$(LANGHDR): $(LANGSRC) ../util/maid
	touch $<
	../util/maid -d -p $<
#	cp -f $@ $(SLIBDIR)\$(@:f)

max.o: CPPFLAGS += -DINSTALL_PREFIX=\"$(PREFIX)\"

max.o: $(LANGHDR) all.h proto.h modem.h all.h

static.o max_init.o: max_v.h $(LANGHDR) max_con.h prm.h

date.o: bldupd.h max_vr.h

f_con.o: max_con.h

m_area.o: max_msg.h

f_up.o f_down.o: f_idx.h

max_gets.o max_log.o: emsi.h

max_init.o: colour.h

fsend.o frecv.o: xmodem.h

#rbsb.o zm.o rz.o sz.o: zsjd.h zmodem.h

language.o: $(LANGSRC)

$(ZOBJS): xmodem.h pdata.h


clean:
	-rm *.o *.so max
