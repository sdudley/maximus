# $Id: makefile.mk,v 1.1.1.1 2002/10/01 17:51:16 sdudley Exp $

############################################################################
#                                                                          #
#                      Startup and directory information                   #
#                                                                          #
############################################################################

.IMPORT:        PROTDIR SQUISHDIR LANGDIR

.SOURCE.c:      $(MAXDIR) $(PROTDIR) $(SLIBDIR)
.SOURCE.cpp:    $(MAXDIR) $(PROTDIR)
.SOURCE.h:      $(MAXDIR) $(PROTDIR)
.SOURCE.exe:    ..\exe\max
.SOURCE.lth:    $(LANGDIR)
.SOURCE.mad:    $(LANGDIR)
#.SOURCE.lib:    $(LIBDIR)

MAXLINELENGTH=3500

#not supported with free wlink
#OVERLAY_BUILD=1

############################################################################
#                                                                          #
#                        Global Constants & Defines                        #
#                                                                          #
############################################################################

MODEL       :=  $(MLARGE)

LANGHDR     :=  english.lth
LANGSRC     :=  english.mad
LIBS       +:=  $(SQUISHLIB)

.IF $(COMP)==TC
LIBS        +:= $(LIBDIR)\\vms$(COMP)$(MODE).lib
.END

.IF $(MODE)==r
OPTSWAP     := $(C_DEFINE)SWAP
.END

.IF $(MODE)==p
LIBS       +:=  $(LIBDIR)\\maxcomm.lib
.END

############################################################################
#                                                                          #
#                        Standard C Compiler Flags                         #
#                                                                          #
############################################################################

CFLAG1      :=  $(C_MODEL)$(MODEL) $(C_STDOPS_SIZ) $(OPTSWAP)
CFLAG2       =  $(C_OUTDIR)$(@:s|\\|\|)$() $(C_DYNAM)
CFLAGS       =  $(CFLAG1) $(CFLAG2)

.IF $(OVERLAY_BUILD)==1
CFLAGS      +=  $(C_DEFINE)OVERLAY_BUILD
OPT_OVERLAY	:=	o
.ELSE
OPT_OVERLAY	:=
.END

.IF $(COMP)==W3
.IF $(MODE)==p
CFLAGS += /of
.END
.END

SOBJDIR     :=  $(SLIBDIR)\$(COMP)$(MODE)
#$(C_DEFINE)EMSI

.IF $(FLAT)==YES
OPTP32 := $(OPTP)32
.ELSE
OPTP32 := $(OPTP)
.END


# Main maximus object

MAXOBJ := max.obj

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
callfos.obj     max_prot.obj    fos_os2.obj     callinfo.obj    \
async.obj       medinit.obj     os2.obj         dos.obj         \
				thunk.obj       asyncnt.obj						\
max2priv.obj    mexintu.obj     mexstat.obj     mexfarea.obj    \
mexcall.obj     mexoutp.obj     mextime.obj     mexlang.obj     \
mexrip.obj      mexstr.obj      mexmarea.obj    mexfile.obj     \
mexclass.obj    mexuser.obj     mexinp.obj      mextag.obj      \
mexffind.obj    mexxtrn.obj     mexrtl.obj      max_accs.obj    \
m_input.obj     m_lines.obj     f_okay.obj      atexit.obj

.IF $(COMP)==WC
.IF $(MODE)==r
# Stack overflow handler for WC
MOBJS       += stk.obj md5.obj tune.obj uniqren.obj strbuf.obj arc_def.obj \
               noise.obj
.END
.END

.IF $(COMP)==W3
.IF $(MODE)==p
# Add WC 10.6 patch
MOBJS += main2o32.obj
.END
.END



# Protocol objects

ZOBJS       :=  rbsb.obj        rz.obj          zm.obj          sz.obj  \
                fsend.obj       frecv.obj       pdata.obj

# Total object list

OBJS        :=  $(strip $(MAXOBJ) $(MOBJS) $(ZOBJS) cppmain.obj)


############################################################################
#                                                                          #
#                        Object modules and default rule                   #
#                                                                          #
############################################################################

all .PHONY: max$(OPTP)d.exe max$(OPTP).exe

############################################################################
#                                                                          #
#                        Make rule for MAXD.EXE                            #
#                                                                          #
############################################################################

.IF $(MAKETARGETS)==max$(OPTP)d.exe

# Constants for the debugging build

.SOURCE.obj:    $(MAXDIR)\$(COMP)$(MODE)d
CFLAGS +:= $(C_DEBUG)
%.obj: %.asm; $(ACMD) $<, $(COMP)$(MODE)d\$*;

.IF $(COMP)==WC

max$(OPTP)d.exe .PRECIOUS: $(OBJS) date.obj $(LANGHDR) $(LIBS)
        $(LL) @$(MAXDIR)\max$(MODE)d$(OPT_OVERLAY)
.ELIF $(COMP)==W3

max$(OPTP)d.exe .PRECIOUS: $(OBJS) date.obj $(LANGHDR) $(LIBS)
        $(LL) @$(MAXDIR)\max$(OPTP32)d
.ELSE
max$(OPTP)d.exe .PRECIOUS: $(OBJS) date.obj $(LIBS)
.END

.ELSE

max$(OPTP)d.exe .PHONY:
        $(MAKE) max$(OPTP)d.exe
.END

############################################################################
#                                                                          #
#                        Make rule for MAX.EXE                             #
#                                                                          #
############################################################################

.IF $(MAKETARGETS)==max$(OPTP).exe

.SOURCE.obj:    $(MAXDIR)\$(COMP)$(MODE)
%.obj: %.asm; $(ACMD) $<, $(COMP)$(MODE)\$*;

.IF $(COMP)==WC

max$(OPTP).exe .PRECIOUS: $(OBJS) date.obj $(LANGHDR) $(LIBS)
        bldupd bldupd.h
        $(LL) @$(MAXDIR)\max$(MODE)$(OPT_OVERLAY)
.ELIF $(COMP)==W3

max$(OPTP).exe .PRECIOUS: $(OBJS) date.obj $(LANGHDR) $(LIBS)
        bldupd bldupd.h
        $(LL) @$(MAXDIR)\max$(OPTP32)
.ELSE
max$(OPTP).exe .PRECIOUS: $(OBJS) date.obj $(PVTLIB)
.END

.ELSE
max$(OPTP).exe .PHONY:
        $(MAKE) max$(OPTP).exe
.END



############################################################################
#                                                                          #
#                        Linking rules and response files                  #
#                                                                          #
############################################################################

LFLAGS      :=  $(L_WARNDUPE) $(L_STDOPS) $(L_DEBUG)
STACK       :=  6144

.IF $(COMP) != TC
LFLAGS      +=  $(L_STACK)$(STACK)
.END


############################################################################
#                                                                          #
#                          Explicit dependencies                           #
#                                                                          #
############################################################################

DOSWAP := .SWAP

$(LANGHDR): $(LANGSRC)
        touch $<
        ..\exe\util\maid$(HOSTOPTP) -d -p $<
        +copy $@ $(SLIBDIR)\$(@:f)

.IF $(COMP)==WC
thunk.obj: thunk.c
        $(CC) $(CFLAGS) /zdp thunk
.END

$(MAXOBJ) $(DOSWAP): max.c $(LANGHDR) all.h

$(MAXOBJ) $(DOSWAP): proto.h modem.h all.h

static.obj max_init.obj $(DOSWAP): max_v.h $(LANGHDR) max_con.h prm.h

date.obj: bldupd.h max_vr.h

f_con.obj: max_con.h

m_area.obj $(DOSWAP): max_msg.h

f_up.obj f_down.obj: f_idx.h

max_gets.obj max_log.obj: emsi.h

max_init.obj: colour.h

fsend.obj frecv.obj: xmodem.h

rbsb.obj zm.obj rz.obj sz.obj: zsjd.h zmodem.h

language.obj: $(LANGSRC)

#
# Dependencies for the protocol code.
#

$(ZOBJS) $(DOSWAP): xmodem.h pdata.h

rcs .PHONY:
        rin *.c *.h *.cc *.asm *.mk *.lnk ..\lang\english.mad ..\prot\*.c ..\prot\*.h ..\m\*.m ..\m\*.mh

