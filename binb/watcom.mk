.IF $(COMP)==WC
  .IMPORT: WCCOMP WPCOMP WLLINK
.END

.IF $(COMP)==W3
  .IMPORT: WCCOMP WPCOMP WLLINK
.END

# WATCOM C (286)

WC_INCLUDE  :=  /i=
WC_DEBUG    :=  /oe1/od/d2
WC_DEBUGOFF :=  /d0
WC_MODEL    :=  /m
WC_OUTDIR   :=  /Fo=
WC_OVL_OBJ  :=
WC_STARTUP  :=
WC_LIBS      =  $(PVTLIB)

.IF $(MODE)==p
WC_CC       :=  $(WCCOMP) /dOS_2 /2
WC_PP       :=  $(WPCOMP) /dOS_2 /2
.ELSE
WC_CC       :=  $(WCCOMP)
WC_PP       :=  $(WPCOMP)
.END

#WC_STDOPS   :=  /d2/j
#WC_STDOPS_SIZ:= /d2/j
WC_STDOPS   :=  /j/zp1/oaxte100
WC_STDOPS_SIZ:= /oals/s/j/zp1
WC_SCC      :=
WC_CHKSTK   :=
WC_DEFINE   :=  /d
WC_SAVEREG  :=  /r
WC_EXCEPTION:=  /xs
WC_ISR      :=  /zu

.IF $(MODE)==p
# Multithreaded and DLL only applies to OS/2
WC_MT       := /zu /bm
WC_DLL      := /zu /ml /r /bd
.END

# No point in using /of for protected mode.

.IF $(MODE)==r
WC_DYNAM    :=  /of	#/wo
.END

WC_MSMALL   :=  s
WC_MMEDIUM  :=  m
WC_MCOMPACT :=  c
WC_MLARGE   :=  l
WC_MHUGE    :=  h
WC_MTINY    :=  t
WC_FMSMALL  :=  s
WC_FMMEDIUM :=  m
WC_FMCOMPACT:=  c
WC_FMLARGE  :=  l
WC_FMHUGE   :=  h
WC_FMTINY   :=  t
WC_MODS     :=  s m c l
WC_FLAT     :=  NO


# WATCOM C/386

W3_INCLUDE  :=  /i=
W3_DEBUG    :=  /oe1/od/d2
W3_DEBUGOFF :=  /d0
W3_MODEL    :=  /m
W3_OUTDIR   :=  /Fo=
W3_OVL_OBJ  :=
W3_STARTUP  :=
W3_LIBS      =  $(PVTLIB)
#W3_STDOPS   :=  /oaxte/j
W3_STDOPS   :=  /j/zp1/4/fp3/onmaxte100
W3_STDOPS_SIZ:= /oals/s/j
#W3_STDOPS_SIZ:= /d2/s/j
W3_SCC      :=
W3_CHKSTK   :=
W3_DEFINE   :=  /d
W3_DYNAM    :=  # No point in using /of for 386 mode
W3_EXCEPTION:=  /xs

.IF $(MODE)==p
# Multithreaded and DLL only applies to OS/2
W3_MT       :=  /bm
W3_DLL      :=  /bd
.ELSE
.IF $(MODE)==n
W3_MT       :=  /bm
W3_DLL      :=  /bd
.END
.END


.IF $(MODE)==n
W3_CC       :=  $(WCCOMP) /dNT -bt=NT
W3_PP       :=  $(WPCOMP) /dNT -bt=NT
.ELSE
.IF $(MODE)==p
W3_CC       :=  $(WCCOMP) /dOS_2
W3_PP       :=  $(WPCOMP) /dOS_2
.ELSE
W3_CC       :=  $(WCCOMP)
W3_PP       :=  $(WPCOMP)
.END
.END

# Only use the flat model for WCC386

W3_MSMALL   :=  f
W3_MMEDIUM  :=  f
W3_MCOMPACT :=  f
W3_MLARGE   :=  f
W3_MHUGE    :=  f
W3_MTINY    :=  f
W3_FMSMALL  :=  f
W3_FMMEDIUM :=  f
W3_FMCOMPACT:=  f
W3_FMLARGE  :=  f
W3_FMHUGE   :=  f
W3_FMTINY   :=  f
W3_MODS     :=  f
W3_FLAT     :=  YES


# WATCOM Linker

WL_STDOPS   :=  opt q
WL_NOI      :=  opt case
.IF $(COMP)==W3
WL_DEBUG    :=  deb al
.ELSE
WL_DEBUG    :=  deb al
.END
WL_STACK    :=  opt st=
WL_WARNDUPE :=

.IF $(MODE)==p
.IF $(COMP)==W3
WL_LINK      =  $(WLLINK) sys os2v2 $(PMCOMPAT) opt newf
.ELSE
WL_LINK      =  $(WLLINK) sys os2 $(PMCOMPAT) opt newf
.END
.ELSE
.IF $(MODE)==n
WL_LINK     :=  $(WLLINK) opt q sys nt
.ELSE
.IF $($(COMP)_FLAT)==YES
WL_LINK     :=  $(WLLINK) opt q sys dos4g
.ELSE
WL_LINK     :=  $(WLLINK) opt q sys dos
.END
.END
.END

.IF $(COMP) == W3
# The new WC 11.0 includes a "lib.exe" that tries to be like
# the Microsoft libber (except that it doesn't work very well).
# This means that we need to use the WC-specific libber.
USE_WLIB	:=	1
.END
