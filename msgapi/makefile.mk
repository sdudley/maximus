# $Id: makefile.mk,v 1.1.1.1 2002/10/01 17:54:24 sdudley Exp $

.IMPORT .IGNORE: DIST
.EXPORT: DIST

OUTDIR      :=  $(CMODE)

.SOURCE:        $(SLIBDIR) $(MAXDIR)
.SOURCE.obs:    $(OUTDIR)\s
.SOURCE.obS:    $(OUTDIR)\S
.SOURCE.obm:    $(OUTDIR)\m
.SOURCE.obM:    $(OUTDIR)\M
.SOURCE.obc:    $(OUTDIR)\c
.SOURCE.obC:    $(OUTDIR)\C
.SOURCE.obl:    $(OUTDIR)\l
.SOURCE.obL:    $(OUTDIR)\L
.SOURCE.obf:    $(OUTDIR)
.SOURCE.exe:    ..\exe
.SOURCE.dll:    ..\dll
.SOURCE.lib:    ..\lib

SQUISHLIB    =  $(CMODE)mapi$(MODEL).lib

MODEL       :=  $(MLARGE)

CFLAGS      :=  $(C_STDOPS_OPT) $(C_SCC)
CFLAGS      +=  $(C_OUTDIR)$@ $(SWAPDEF) $(C_DLL)

# **********
# CFLAGS += $(C_DEFINE)HCC
# **********

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
                api_sdm.obJ sqasm.obJ                                   \
                                                                        \
                bld.obJ

.IF $(MODE)==p
.IF $(FLAT)==NO
APIOBJS     +=  1stchar.obJ cvtdate.obJ date2bin.obJ dosdate.obJ fexist.obJ \
                ffind.obJ flush.obJ months.obJ $(COMP)_misc.obJ parsenn.obJ \
                qksort.obJ stristr.obJ strocpy.obJ trail.obJ _ctype.obJ     \
                nopen.obJ setfsize.obJ tdelay.obJ
.END
.END

# Use only large-model for OS/2
.IF $(MODE)==p
MODS :=     $(FMLARGE)
.ELSE
.IF $(FLAT)==YES
MODS :=     $(FMLARGE)
.ELSE
MODS :=     $(FMSMALL) $(FMMEDIUM) $(FMCOMPACT) $(FMLARGE)
.END
.END

.IF $(MODE)==r
SQLIBS      :=  $(CMODE)mapi{$(MODS)}.lib
.ELSE

.IF $(FLAT)==YES
.IF $(MODE)==n
SQLIBS      := msgapint.dll
DLLTARGET   := msgapint
.ELSE
SQLIBS      := msgapi32.dll
DLLTARGET   := msgapi32
.END
.ELSE
SQLIBS      := msgapi.dll
DLLTARGET   := msgapi
.END

.END

BLDOBJS     :=  bld.ob{$(MODS)}
MSGAPIOBJS  :=  msgapi.ob{$(MODS)}
API_SQOBJS  :=  sq_area.ob{$(MODS)} sq_misc.ob{$(MODS)}
API_SDMOBJS :=  api_sdm.ob{$(MODS)}

all .PHONY:            $(SQLIBS)

$(BLDOBJS):     bld.h
$(MSGAPIOBJS):  msgapi.h apidebug.h
$(API_SQOBJS):  msgapi.h apidebug.h
$(API_SDMOBJS): msgapi.h apidebug.h

# Build rule for the .DLL

msgapint.dll:       $(APIOBJS:s/J/f)
        bldupd bld.h
        $(WLLINK) @msgapint
        wlib -q ..\lib\msgapint.lib -+..\dll\msgapint.dll

msgapi.dll:       $(APIOBJS:s/J/l)
        bldupd bld.h
        $(WLLINK) @msgapi16
        wlib -q ..\lib\msgapi.lib -+..\dll\msgapi.dll

msgapi32.dll:       $(APIOBJS:s/J/f)
        bldupd bld.h
        $(WLLINK) @msgapi32
        wlib -q ..\lib\msgapi32.lib -+..\dll\msgapi32.dll


.IF $(FLAT)==YES
$(CMODE)mapi$(FMSMALL).lib:   $(APIOBJS:s/J/f/)
.ELSE
$(CMODE)mapi$(FMSMALL).lib:   $(APIOBJS:s/J/s/)

$(CMODE)mapi$(FMMEDIUM).lib:  $(APIOBJS:s/J/m/)

$(CMODE)mapi$(FMCOMPACT).lib: $(APIOBJS:s/J/c/)

$(CMODE)mapi$(FMLARGE).lib:   $(APIOBJS:s/J/l/)

.END

rcs .PHONY:
        rin *.c *.h *.mk *.asm *.lnk

.DIRCACHE=no

