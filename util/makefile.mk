.IMPORT: UTILDIR SQUISHDIR PROTDIR SORTER

.IMPORT .IGNORE: DIST
.EXPORT: DIST

.SOURCE.c:   $(UTILDIR) $(MAXDIR)
.SOURCE.h:   $(UTILDIR) $(MAXDIR) $(MAPIDIR)
.SOURCE.asm: $(UTILDIR) $(MAXDIR)
.SOURCE.obj: $(COMP)$(MODE)
.SOURCE.exe: ..\exe\util
.SOURCE.lth:    $(SLIBDIR)

CFLAGS      += $(C_SCC) $(C_INCLUDE)$(MAXDIR) $(C_INCLUDE)$(PROTDIR)
CFLAGS      += $(C_OUTDIR)$(@:s|\\|\|)$() $(C_DEBUG)

MODEL   := $(MSMALL)

.IF $(DIST) != Y
  LFLAGS += $(L_DEBUG)
.ENDIF

.IF $(COMP)==WC
LFLAGS += opt verb opt map=..\map\util\$*.map
.END

.IF $(COMP)==W3
LFLAGS += opt verb opt map=..\map\util\$*.map
.END

.IF $(FLAT)==YES
STACK       :=  16384
.END

#.IF $(MAKETARGETS)==$(NULL)
#  .KEEP_STATE := makefile._$(COMP)
#.ELSE
#  .KEEP_STATE := $(MAKETARGETS:s/.exe//)._$(COMP)
#.ENDIF

##############################################################################
# MAID
##############################################################################

.IF $(MAKETARGETS) == maid$(OPTP).exe english.lth

.IMPORT: LANGDIR

OBJS        :=  maid.obj
MODEL       := $(MCOMPACT)

maid$(OPTP).exe: $(OBJS)

.IF $(MODE)==$(HOSTMODE)
english.lth: maid$(OPTP).exe
        ..\exe\util\maid$(OPTP) -d -p ..\lang\english
        +copy ..\lang\english.lth ..\slib
.END

maid.obj: prm.h

##############################################################################
# MECCA
##############################################################################

.ELIF $(MAKETARGETS) == mecca$(OPTP).exe

MODEL       := $(MCOMPACT)
OBJS        := mecca.obj        init.obj


mecca$(OPTP).exe: $(OBJS)

mecca.c init.c: mecca_vb.h
        LSORT S C C mecca_vb.h mecca_vb.~~~ V 1 99 C A 0 Y Y Y
        +if exist mecca_vb.~~~ copy mecca_vb.h mecca_vb.bak
        +if exist mecca_vb.~~~ del mecca_vb.h
        +if exist mecca_vb.~~~ ren mecca_vb.~~~ mecca_vb.h
        touch mecca.c
        touch init.c

mecca.obj init.obj: mecca.h mecca_vb.h


##############################################################################
# ACCEM
##############################################################################

.ELIF $(MAKETARGETS) == accem$(OPTP).exe

MODEL       := $(MCOMPACT)
OBJS        := accem.obj        init.obj

accem$(OPTP).exe: $(OBJS)

accem.c: mecca_vb.h
        +
#        $(SORTER) mecca_vb.h
#        touch accem.c

accem.obj: mecca.h mecca_vb.h


##############################################################################
# SCANBLD
##############################################################################

.ELIF $(MAKETARGETS) == scanbld$(OPTP).exe

MODEL := $(MLARGE)

LIBS        +=  $(SQUISHLIB) $(TRACKLIB)
OBJS        :=  cppmain.obj scanbld.obj

scanbld$(OPTP).exe: $(OBJS)


##############################################################################
# ANSI2BBS
##############################################################################

.ELIF $(MAKETARGETS) == ans2bbs$(OPTP).exe

OBJS        :=  ansi2bbs.obj
CFLAGS      +=  $(C_DEFINE)ANSI2BBS

ans2bbs$(OPTP).exe: $(OBJS)


##############################################################################
# ANSI2MEC
##############################################################################

.ELIF $(MAKETARGETS) == ans2mec$(OPTP).exe

OBJS        :=  ansi2mec.obj
CFLAGS      +=  $(C_DEFINE)ANSI2MEC

ans2mec$(OPTP).exe: $(OBJS)

ansi2mec.obj: ansi2bbs.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MODEL) $(<:s|\\|\|)

##############################################################################
# CVTUSR
##############################################################################

.ELIF $(MAKETARGETS) == cvtusr$(OPTP).exe

MODEL       :=  $(MLARGE)

OBJS        :=  cppmain.obj cvtusr.obj cvt_misc.obj

LIBS        +=  $(TRACKLIB)

cvtusr$(OPTP).exe: $(OBJS)


##############################################################################
# EDITCALL
##############################################################################

.ELIF $(MAKETARGETS) == editcal$(OPTP).exe

OBJS        :=  editcall.obj

editcal$(OPTP).exe: $(OBJS)

##############################################################################
# MR          
##############################################################################

.ELIF $(MAKETARGETS) == mr$(OPTP).exe

MODEL       :=  $(MLARGE)
OBJS        :=  mr.obj #copymsg.obj
LIBS        +=  $(SQUISHLIB)

mr$(OPTP).exe: $(OBJS)

##############################################################################
# MAXPIPE
##############################################################################

.ELIF $(MAKETARGETS) == maxpipe.exe

OBJS        :=  maxpipe.obj
LIBS        +=  $(MLIBDIR)\maxcomm.lib
CFLAGS      +=  $(C_MT) /j/s

maxpipe.exe: $(OBJS)

##############################################################################
# MXPIPE32
##############################################################################

.ELIF $(MAKETARGETS) == mxpipe32.exe

OBJS        :=  mxpipe32.obj
LIBS        +=  $(MLIBDIR)\maxcomm.lib
CFLAGS      +=  $(C_MT) /j/s
LFLAGS      +=  $(L_STACK)32768

mxpipe32.exe: $(OBJS)

##############################################################################
# PIPER
##############################################################################

#.ELIF $(MAKETARGETS) == piper$(OPTP).exe
#
#OBJS        := piper.obj
#
#piper$(OPTP).exe: $(OBJS)

##############################################################################
# FIXLR
##############################################################################

.ELIF $(MAKETARGETS) == fixlr$(OPTP).exe

OBJS         := fixlr.obj

fixlr$(OPTP).exe: $(OBJS)

##############################################################################
# SETLR
##############################################################################

.ELIF $(MAKETARGETS) == setlr$(OPTP).exe

LIBS         += $(SQUISHLIB)
OBJS         := setlr.obj
setlr$(OPTP).exe: $(OBJS)

##############################################################################
# FB
##############################################################################

.ELIF $(MAKETARGETS) == fb$(OPTP).exe

MODEL       :=  $(MLARGE)
OBJS        :=  fb.obj
LIBS        +=  $(SQUISHLIB)

fb$(OPTP).exe: $(OBJS)

$(OBJS): fb.h



##############################################################################
# ORACLE
##############################################################################

.ELIF $(MAKETARGETS) == oracle$(OPTP).exe

MODEL       :=  $(MLARGE)

CFLAGS      +=  $(C_DEFINE)ORACLE $(C_SCC)

OBJS        :=  cppmain.obj                                                  \
                max.obj         max_in.obj      max_outl.obj    max_outr.obj \
                display.obj     disp_dat.obj    disp_qu.obj     disp_max.obj \
                disp_mis.obj    max_misc.obj    max_init.obj    max_fini.obj \
                max_xtrn.obj    fos.obj         oracle.obj      callfos.obj  \
                max_out.obj     max_gets.obj    language.obj    max_ocmd.obj \
                fos_dos.obj     fos_os2.obj     max_clss.obj    atexit.obj

static.obj max_init.obj: max_v.h max_con.h prm.h

max.obj static.obj max_init.obj language.obj: english.lth
                
.IF $(LINK)==WL
LFLAGS      +=  opt map,verb
.END

.IF $(MODE)==p
OBJS        +=  async.obj
LIBS        +=  $(MLIBDIR)\maxcomm.lib
.END

.IF $(MODE)==n
OBJS        +=  asyncnt.obj
LIBS        += $(LIBDIR)\\ntcomm.lib
.END

#LIBS += $(TRACKLIB)

oracle$(OPTP).exe: $(OBJS)

##############################################################################
# SILT
##############################################################################

.ELIF $(MAKETARGETS) == silt$(OPTP).exe

MODEL       :=  $(MLARGE)

OBJS        :=  silt.obj        cppmain.obj     s_equip.obj     s_matrix.obj \
                s_menu.obj      s_misc.obj      s_parse.obj     s_sessio.obj \
                s_system.obj    s_reader.obj    s_lang.obj      s_colour.obj \
                s_proto.obj     s_marea.obj     s_farea.obj     s_heap.obj   \
                s_area.obj      s_access.obj    l_attach.obj    max2priv.obj

LIBS        +=  $(SQUISHLIB) $(TRACKLIB)

silt$(OPTP).exe: $(OBJS)

$(OBJS): silt.h prm.h msgapi.h

silt.obj s_menu.obj: option.h

silt.obj m_colour.obj: colour.h

##############################################################################
# ALL
##############################################################################
.ELSE

.IF $(MODE)==p
.IF $(FLAT)==YES
MAXPIPE := maxpipe
.END
.END

all .PHONY:     maid silt mecca accem ansi2bbs ansi2mec cvtusr editcall \
                fb mr fixlr setlr scanbld $(MAXPIPE) oracle

fb .PHONY:
        $(MAKE) fb$(OPTP).exe

fixlr .PHONY:
        $(MAKE) fixlr$(OPTP).exe

setlr .PHONY:
        $(MAKE) setlr$(OPTP).exe

maid .PHONY:
        $(MAKE) maid$(OPTP).exe english.lth

mecca .PHONY:
        $(MAKE) mecca$(OPTP).exe

accem .PHONY:
        $(MAKE) accem$(OPTP).exe

silt .PHONY:
        $(MAKE) silt$(OPTP).exe

scanbld .PHONY:
        $(MAKE) scanbld$(OPTP).exe

ansi2bbs .PHONY:
        $(MAKE) ans2bbs$(OPTP).exe

ansi2mec .PHONY:
        $(MAKE) ans2mec$(OPTP).exe

cvtusr .PHONY:
        $(MAKE) cvtusr$(OPTP).exe

editcall .PHONY:
        $(MAKE) editcal$(OPTP).exe

#piper .PHONY:
#        $(MAKE) piper$(OPTP).exe

maxpipe .PHONY:
        $(MAKE) maxpipe.exe

oracle .PHONY:
        $(MAKE) oracle$(OPTP).exe

mr .PHONY:
        $(MAKE) mr$(OPTP).exe


.END


rcs .PHONY:
        rin *.c *.h *.cc *.mk *.lnk

.DIRCACHE=no

