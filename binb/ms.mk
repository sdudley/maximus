.IF $(COMP)==MC
.IMPORT: MSCDIR LIBMSC COMBINED

.IF $(MODE)==p
MC_LARGEFN  :=  /B2 c2l
.END

MC_MSMALL   :=  S
MC_MMEDIUM  :=  M
MC_MCOMPACT :=  C
MC_MLARGE   :=  L
MC_MHUGE    :=  H
MC_MTINY    :=  T
MC_FMSMALL  :=  s
MC_FMMEDIUM :=  m
MC_FMCOMPACT:=  c
MC_FMLARGE  :=  l
MC_FMHUGE   :=  h
MC_FMTINY   :=  t
MC_MODS     :=  s m c l

MC_INCLUDE  :=  /I
MC_DEBUG    :=  /Zi
MC_DEBUGOFF :=
MC_MODEL    :=  /A
MC_OUTDIR   :=  /Fo
MC_OVL_OBJ  :=
MC_STARTUP  :=

MSLIBDIR    := $(MSCDIR)\lib

.IF $(COMBINED)==Y
MC_LIBS      =  $(PVTLIB) $(MSLIBDIR)\$(MODEL)libce$(MODE).lib
.ELSE
MC_LIBS      =  $(PVTLIB) $(MSLIBDIR)\$(MODEL)libc$(MODE).lib $(MSLIBDIR)\em.lib $(MSLIBDIR)\libh.lib $(MSLIBDIR)\$(MODEL)libfp.lib
.END

.IF $(MODE)==p
MC_LIBS     +=  doscalls os2
.END

.IF $(MODE)==p
MC_CC       :=  cl /DOS_2
.ELSE
MC_CC       :=  cl
.END

MC_STDOPS   :=  /nologo /c /Oaxzr /W4 /Gs /Gr # /Oxarz
MC_STDOPS_SIZ:= /nologo /c /Oaxzr /W4 /Gs /Gr # /Oxarz
MC_SCC      :=
MC_CHKSTK   :=  /Ge
MC_DEFINE   :=  /D
MC_DYNAM    :=

.IF $(MODE)==p
MC_MT       :=  /MT
MC_DLL      :=  /ML
.ELSE
MC_MT       :=
MC_DLL      :=
.END

MC_FLAT     :=  NO

.END

MA_AS       := masm /t
MA_CASE     := /Mx
MA_DEFINE   := /d
MA_DEBUG    := /Zi
MA_INCLUDE  := /I
MA_FLAGS    :=

ML_LINK     := link
