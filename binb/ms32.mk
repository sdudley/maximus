M3_MSMALL   :=
M3_MMEDIUM  :=
M3_MCOMPACT :=
M3_MLARGE   :=
M3_MHUGE    :=
M3_MTINY    :=
M3_FMSMALL  :=  f
M3_FMMEDIUM :=  f
M3_FMCOMPACT:=  f
M3_FMLARGE  :=  f
M3_FMHUGE   :=  f
M3_FMTINY   :=  f
M3_MODS     :=  f

M3_INCLUDE  :=  /I
M3_DEBUG    :=  /Zi
M3_DEBUGOFF :=
M3_MODEL    :=
M3_OUTDIR   :=  /Fo
M3_OVL_OBJ  :=
M3_STARTUP  :=

MSLIBDIR    := $(MSCDIR)\lib

M3_LIBS      =  $(PVTLIB) $(MSLIBDIR)\libc.lib $(MSLIBDIR)\ntdll.lib $(MSLIBDIR)\kernel32.lib
M3_CC       :=  cl386 /DNT /D_X86_
#M3_STDOPS   :=  /nologo /c /Oaxt /W3 /Gs /Gr # /Oxarz
#M3_STDOPS_SIZ:= /nologo /c /Oaxt /W3 /Gs /Gr # /Oxarz
M3_STDOPS   :=  /nologo /c /W3 /Gs /Gr /Od
M3_STDOPS_SIZ:= /nologo /c /W3 /Gs /Gr /Od
M3_SCC      :=
M3_CHKSTK   :=  /Ge
M3_DEFINE   :=  /D
M3_DYNAM    :=
M3_MT       :=  /MT
M3_DLL      :=  /ML
M3_FLAT     :=  YES

NL_NOI      =
NL_DEBUG    =   -debugtype:cv
NL_WARNDUPE =
NL_STDOPS   =   -subsystem:console -stack:4096
NL_STACK    =   -stack:
NL_LINK     =   coff -link

