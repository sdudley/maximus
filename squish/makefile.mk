OUTDIR      :=  $(COMP)$(MODE)

.IF $(FLAT)==YES
SQEXE       :=  sq386$(OPTP).exe
SQBASE      :=  sq386$(OPTP)
SQFIXEXE    :=  sqfix32$(OPTP).exe
.ELSE
SQEXE       :=  squish$(OPTP).exe
SQBASE      :=  squish$(OPTP)
SQFIXEXE    :=  sqfix$(OPTP).exe
.END

.IMPORT .IGNORE: DIST
.EXPORT: DIST

.IF $(DIST) != Y
  LFLAGS    +=  $(L_DEBUG)
.END

.IF $(COMP)==WC
LFLAGS += opt verb opt map=..\map\squish\$*.map
.END

.IF $(COMP)==W3
LFLAGS += opt st=16k opt verb opt map=..\map\squish\$*.map
.END


.SOURCE:        $(MAXDIR) $(OUTDIR) $(MAPIDIR)
.SOURCE.obs:    $(OUTDIR)\s
.SOURCE.obS:    $(OUTDIR)\s
.SOURCE.obm:    $(OUTDIR)\m
.SOURCE.obM:    $(OUTDIR)\M
.SOURCE.obc:    $(OUTDIR)\c
.SOURCE.obC:    $(OUTDIR)\C
.SOURCE.obl:    $(OUTDIR)\l
.SOURCE.obL:    $(OUTDIR)\L
.SOURCE.exe:    ..\exe\squish
.SOURCE.dll:    ..\dll
.SOURCE.obj:    $(OUTDIR)
.SOURCE.obf:    $(OUTDIR)

# Default memory model of small

MODEL       :=  $(MSMALL)

# Default C flags option

CFLAGS      +=  $(C_OUTDIR)$@


.IF $(MAKETARGETS) == $(NULL)

MAINTARGETS := squish sqfix
EXTRATARGETS:= sqpack sqconv sqinfo sqset sstat sqreidx

.IF $(FLAT) == YES
.IF $(MODE)==n
MAINTARGETS +:= $(EXTRATARGETS)
.ELSE
MAINTARGETS +:= $(EXTRATARGETS)
.END
.ELSE
MAINTARGETS +:= $(EXTRATARGETS)
.END

.IF $(MODE)==p
.IF $(FLAT)==YES
MAINTARGETS +=  killrc32 msgtra32
.ELSE
MAINTARGETS +=  killrcat msgtrack
.END
.END

all .PHONY: $(MAINTARGETS)

squish .PHONY: 
        $(MAKE) $(SQEXE)

killrcat .PHONY:
        $(MAKE) killrcat.dll

killrc32 .PHONY:
        $(MAKE) killrc32.dll

msgtrack .PHONY:
        $(MAKE) msgtrack.dll

msgtra32 .PHONY:
        $(MAKE) msgtra32.dll

sqpack .PHONY: 
        $(MAKE) sqpack$(OPTP).exe

sqconv .PHONY:
        $(MAKE) sqconv$(OPTP).exe

sqfix .PHONY: 
        $(MAKE) $(SQFIXEXE)

sqinfo .PHONY:
        $(MAKE) sqinfo$(OPTP).exe

sqset .PHONY:
        $(MAKE) sqset$(OPTP).exe

sstat .PHONY:
        $(MAKE) sstat$(OPTP).exe

sqreidx .PHONY:
        $(MAKE) sqreidx$(OPTP).exe

############################################################################
##                          SQUISH.EXE                                     #
############################################################################

.ELIF $(MAKETARGETS) == $(SQEXE)

MODEL       :=  $(MLARGE)

CFLAGS      :=  $(C_DEBUG) $(C_SCC) $(C_MODEL)$(MODEL)
CFLAGS      +=  $(C_OUTDIR)$@ $(SWAPDEF)
CFLAGS      +=  $(C_LARGEFN) $(C_STDOPS_OPT)

LFLAGS       =  $(L_WARNDUPE) $(L_STDOPS) $(L_DEBUG)

LIBS        +=  $(SQUISHLIB) $(SMSERIALLIB)

.IF $(COMP)==WC
LFLAGS += opt st=8192 deb all
.END

AOUTDIR     :=  # asm .obj files go to current directory

.IF $(MODE)==r
.IF $(COMP)==TC
  SWAPDEF   :=  $(C_DEFINE)SWAP
  SWAPFN    :=  ..\\swap\\tcr\\swap$(MODEL).obj
.ELIF $(COMP)==WC
  SWAPDEF   :=  $(C_DEFINE)SWAP
  SWAPFN    :=  ..\\swap\\wcr\\swap$(MODEL).obj
.END
.END

OBJS        :=  squish.obj   s_abbs.obj         s_config.obj    \
                s_scan.obj   s_toss.obj         s_pack.obj      \
                s_squash.obj s_match.obj        s_log.obj       \
                s_misc.obj   s_hole.obj         s_link.obj      \
                s_busy.obj   s_stat.obj         s_sflo.obj      \
                s_thunk.obj  s_dupe.obj

.IF $(COMP)==WC
$(SQEXE):       $(SWAPFN) $(OBJS) bld.obj
        bldupd bld.h
        $(LL) @$(SQBASE)
.ELIF $(COMP)==W3

$(SQEXE):       $(SWAPFN) $(OBJS) bld.obj
        bldupd bld.h
        $(LL) @$(SQBASE)
.ELSE
$(SQEXE):       $(SWAPFN) $(OBJS) bld.obj
.END

.IF $(COMP)==WC
s_thunk.obj: s_thunk.c
        $(CC) /r $(CFLAGS) s_thunk
.END

bld.obj: bld.h sqver.h

$(OBJS):        apidebug.h

############################################################################
##                          SQPACK.EXE                                    ##
############################################################################

.ELIF $(MAKETARGETS) == sqpack$(OPTP).exe

MODEL       :=  $(MLARGE)
OBJS        :=  sqpack.obj
LIBS        +=  $(SQUISHLIB)

sqpack$(OPTP).exe:     $(OBJS)

############################################################################
##                            SQCONV.EXE                                  ##
############################################################################

.ELIF $(MAKETARGETS) == sqconv$(OPTP).exe

OBJS        :=  sqconv.obj
LIBS        +=  $(SQUISHLIB)

.IF $(COMP)==WC
LFLAGS      +=  opt nocase
.END

sqconv$(OPTP).exe:   $(OBJS)

############################################################################
##                          SQFIX.EXE                                     ##
############################################################################

.ELIF $(MAKETARGETS) == $(SQFIXEXE)

MODEL       :=  $(MLARGE)
OBJS        :=  sqfix.obj
LIBS        +=  $(SQUISHLIB)


$(SQFIXEXE):      $(OBJS)

############################################################################
##                          SQINFO.EXE                                    ##
############################################################################

.ELIF $(MAKETARGETS) == sqinfo$(OPTP).exe

OBJS        :=  sqinfo.obj

sqinfo$(OPTP).exe:     $(OBJS)

############################################################################
##                          SQSET.EXE                                     ##
############################################################################

.ELIF $(MAKETARGETS) == sqset$(OPTP).exe

OBJS        :=  sqset.obj

sqset$(OPTP).exe:     $(OBJS)

############################################################################
##                          SSTAT.EXE                                     ##
############################################################################

.ELIF $(MAKETARGETS) == sstat$(OPTP).exe

OBJS        :=  sstat.obj

sstat$(OPTP).exe:     $(OBJS)

############################################################################
##                       SQREIDX.EXE                                      ##
############################################################################

.ELIF $(MAKETARGETS) == sqreidx$(OPTP).exe

OBJS        :=  sqreidx.obj
LIBS        +=  $(SQUISHLIB)

sqreidx$(OPTP).exe:     $(OBJS)

############################################################################
##                       KILLRCAT.DLL                                     ##
############################################################################

.ELIF $(MAKETARGETS) == killrcat.dll

MODEL       :=  $(MLARGE)
CFLAGS      +=  $(C_DLL)

killrcat.dll: killrcat.obj
        wlink deb all f wcp\killrcat n ..\dll\killrcat.dll sys os2 dll initi \
                lib os2 opt many,prot,modname=killrcat          \
                opt verb,map=..\map\squish\killrcat.map

############################################################################
##                       KILLRC32.DLL                                     ##
############################################################################

.ELIF $(MAKETARGETS) == killrc32.dll

CFLAGS      +=  $(C_DLL)

killrc32.dll: killrc32.obj
        wlink deb all f w3p\killrc32 n ..\dll\killrc32.dll sys os2v2 dll initi \
                lib os2386 opt many,modname=killrc32            \
                opt verb,map=..\map\squish\killrc32.map

killrc32.obj: killrcat.c
        $(CC) $(CFLAGS) killrcat.c

############################################################################
##                       MSGTRACK.DLL                                     ##
############################################################################

.ELIF $(MAKETARGETS) == msgtrack.dll

MODEL       :=  $(MLARGE)
CFLAGS      +=  $(C_DLL)

msgtrack.dll: msgtrack.obj
        wlink deb all f wcp\msgtrack n ..\dll\msgtrack.dll sys os2 dll initi \
                lib os2 opt many,prot,modname=msgtrack          \
                opt verb,map=..\map\squish\msgtrack.map

############################################################################
##                       MSGTRA32.DLL                                     ##
############################################################################

.ELIF $(MAKETARGETS) == msgtra32.dll

CFLAGS      +=  $(C_DLL)

msgtra32.dll: msgtra32.obj
        wlink deb all f w3p\msgtra32 n ..\dll\msgtra32.dll sys os2v2 dll initi \
                lib os2386 opt many,modname=msgtra32            \
                opt verb,map=..\map\squish\msgtra32.map

msgtra32.obj: msgtrack.c
        $(CC) $(CFLAGS) msgtrack.c


.END

rcs .PHONY:
        rin *.c *.h *.mk *.lnk

.DIRCACHE=no

