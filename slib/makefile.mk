.IMPORT: MAXCOMMPATH
.SOURCE: $(COMP)$(MODE)\\s
.SOURCE.obf: $(COMP)$(MODE)
.SOURCE.obs: $(COMP)$(MODE)\\s
.SOURCE.obS: $(COMP)$(MODE)\\s
.SOURCE.obm: $(COMP)$(MODE)\\m
.SOURCE.obM: $(COMP)$(MODE)\\m
.SOURCE.obc: $(COMP)$(MODE)\\c
.SOURCE.obC: $(COMP)$(MODE)\\c
.SOURCE.obl: $(COMP)$(MODE)\\l
.SOURCE.obL: $(COMP)$(MODE)\\l
.SOURCE.lib: $(LIBDIR)
.SOURCE.dll: $(DLLDIR)

CFLAGS  = $(C_DEBUG) $(C_STDOPS_OPT) $(C_OUTDIR)$@

OBJS :=      hmalloc.obJ  hmemmove.obJ hfmemmov.obJ     \
hread.obJ    hfread.obJ                                 \
areaapi.obJ  userapi.obJ  md5.obJ                       \
nopen.obJ    fd2n.obJ     isdevice.obJ                  \
brktrap.obJ  ncheck.obJ   timer.obJ    1stchar.obJ      \
getword.obJ  avatar.obJ   fncystr.obJ  months.obJ       \
strftim.obJ  fexist.obJ   stristr.obJ  putss.obJ        \
zfree.obJ    qksort.obJ   memstr.obJ   hprintf.obJ      \
tolower.obJ  toupper.obJ  savedir.obJ  xprintf.obJ      \
xputs.obJ    xputch.obJ   crep.obJ     strstrx.obJ      \
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
mtask.obJ    smalloc.obJ  strocpy.obJ                   \
cshopen.obJ  crc32.obJ    crc16.obJ    strrstr.obJ      \
crit.obJ     crit_asm.obJ cpu_asm.obJ  fpu_asm.obJ      \
skiplist.obJ dsmalloc.obJ acomp.obJ    arc_cmd.obJ      \
arcmatch.obJ bfile.obJ    bprintf.obJ  _ctype.obJ       \
setfsize.obJ cstrupr.obJ  strnncpy.obJ zeller.obJ       \
flat.obJ     getmax.obJ   prmapi.obJ   strbuf.obJ       \
$(COMP)_misc.obJ


# Make SMSERIAL.DLL for OS/2

.IF $(MODE)==p
.IF $(COMP)==WC
SMSERIAL := smserial.dll
MAXCOMM := maxcomm.lib
.END
.END


# Flat or segmented?

.IF $(FLAT)==YES

UAPIVER := 10
DVIDVER := 10
SLIBVER := 10

UAPIDLL := uapi$(UAPIVER)32.dll
DVIDDLL := dvid$(DVIDVER)32.dll
SLIBDLL := slib$(SLIBVER)32.dll

#DLLS := $(SLIBDLL) $(UAPIDLL) $(DVIDDLL)

all .PHONY: $(COMP)$(MODE)lib$(FMSMALL).lib $(DLLS)

MODS := $(FMSMALL)
.ELSE
all .PHONY: $(SMSERIAL) $(MAXCOMM) $(COMP)$(MODE)lib{s m c l}.lib

MODS := $(MSMALL) $(MMEDIUM) $(MCOMPACT) $(MLARGE)
.END


# Real or protected-mode?

.IF $(MODE)==n                                                  # Windows NT
OBJS += vio.obJ os2key.obJ os2file.obJ

.ELSE
.IF $(MODE)==r                                                  # MS-DOS

OBJS += flusha.obJ   getch.obJ    ffinda.obJ

OBJS += dv.obJ       dv_attr.obJ  dv_cleol.obJ dv_cls.obJ       \
        dv_getch.obJ dv_getxy.obJ dv_goxy.obJ  dv_gxyb.obJ      \
        dv_info.obJ  dv_infoa.obJ dv_mode.obJ  dv_page.obJ      \
        dv_putc.obJ  dv_putch.obJ dv_puts.obJ  dv_scrl.obJ      \
        dv_scrla.obJ dv_shad.obJ  dv_sync.obJ  dv_blitz.obJ     \
        share.obJ

.ELSE                                                           # OS/2
OBJS += vio.obJ os2key.obJ os2file.obJ os2com.obJ


maxcomm.lib: $(MAXCOMMPATH)
        wlib /q $@ +$<

.IF $(SMSERIAL) != $(NULL)

# Do the Squish serialization semaphore

$(SMSERIAL): smserial.ob$(FMLARGE)
        wlink @smserial
        wlib /q $(LIBDIR)\smserial.lib -+$(DLLDIR)\smserial.dll

smserial.obl: smserial.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MLARGE) $(C_DLL) $(<:s|\\|\|)

.END # .IF SMSERIAL

.END # .ELSE OS/2
.END # .ELSE DOS





# Microsoft or standard?

.IF $(COMP)==MC

.IF $(FLAT)==YES
$(COMP)$(MODE)lib$(MSMALL).lib: $(strip $(OBJS:s/J/F/))

.ELSE
$(COMP)$(MODE)libs.lib: $(strip $(OBJS:s/J/S/))
$(COMP)$(MODE)libm.lib: $(strip $(OBJS:s/J/M/))
$(COMP)$(MODE)libc.lib: $(strip $(OBJS:s/J/C/))
$(COMP)$(MODE)libl.lib: $(strip $(OBJS:s/J/L/))

.END

.ELSE                                                   # !MSC

.IF $(FLAT)==YES
$(COMP)$(MODE)lib$(FMSMALL).lib: $(strip $(OBJS:s/J/f/))

.ELSE
$(COMP)$(MODE)libs.lib: $(strip $(OBJS:s/J/s/))
$(COMP)$(MODE)libm.lib: $(strip $(OBJS:s/J/m/))
$(COMP)$(MODE)libc.lib: $(strip $(OBJS:s/J/c/))
$(COMP)$(MODE)libl.lib: $(strip $(OBJS:s/J/l/))

.END    # !FLAT

.END    # !MSC


nopen.ob$(FMCOMPACT): nopen.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MCOMPACT) $(C_SAVEREG) $(<:s|\\|\|)

.IF $(FLAT)==YES
$(SLIBDLL):
        wlink @slib32
        wlib ..\lib\slib32.lib -+$@

$(DVIDDLL):
        wlink @dvid32
        wlib ..\lib\dvid32.lib -+$@

$(UAPIDLL):
        wlink @uapi32
        wlib ..\lib\uapi32.lib -+$@
.END


.IF $(FLAT)==NO
nopen.ob$(FMLARGE): nopen.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MLARGE) $(C_SAVEREG) $(<:s|\\|\|)

brktrap.ob$(FMSMALL): brktrap.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_ISR) $(<:s|\\|\|)

brktrap.ob$(FMMEDIUM): brktrap.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MMEDIUM) $(C_ISR) $(<:s|\\|\|)

brktrap.ob$(FMCOMPACT): brktrap.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MCOMPACT) $(C_ISR) $(<:s|\\|\|)

brktrap.ob$(FMLARGE): brktrap.c
        $(CC) $(CFLAGS) $(C_MODEL)$(MLARGE) $(C_ISR) $(<:s|\\|\|)
.END

# Stuff that needs to have DMAKE swapped out when under DOS:

win.ob{$(MODS)} arc_def.ob{$(MODS)} priv.ob{$(MODS)} \
  parsenn.ob{$(MODS)} adj_user.ob{$(MODS)} strftim.ob{$(MODS)} \
  ffname.ob{$(MODS)} dmalloc.ob$(MLARGE) skiplist.ob$(MLARGE) \
  {s ds}malloc.ob$(MLARGE) dv.ob$(MLARGE) .SWAP:

rcs .PHONY:
        rin *.c *.h *.cc *.asm *.inc *.mac *.lnk *.mk

