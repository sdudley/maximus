# Defines for IBM C Set/2

# CSet doesn't have memory models

IC_MSMALL   :=
IC_MMEDIUM  :=
IC_MCOMPACT :=
IC_MLARGE   :=
IC_MHUGE    :=
IC_MTINY    :=
IC_FMSMALL  :=f
IC_FMMEDIUM :=f
IC_FMCOMPACT:=f
IC_FMLARGE  :=f
IC_FMHUGE   :=f
IC_FMTINY   :=f
IC_MODS     :=f

IC_INCLUDE  :=  /I
IC_DEBUG    :=  /Ti
IC_DEBUGOFF :=  /Ti-
IC_MODEL    :=#
IC_OUTDIR   :=  /Fo
IC_OVL_OBJ  :=#
IC_STARTUP  :=#
.IF $(COMBINED)==Y
IC_LIBS      =  $(PVTLIB) $(LIBMSC)\$(MODEL)libce$(MODE).lib
.ELSE
IC_LIBS      =  $(PVTLIB) $(LIBMSC)\$(MODEL)libc$(MODE).lib $(LIBMSC)\em.lib $(LIBMSC)\libh.lib $(LIBMSC)\$(MODEL)libfp.lib
.END

.IF $(MODE)==p
IC_LIBS     +=  doscalls os2
.END

IC_CC       :=  icc /DOS_2

IC_STDOPS   :=  /Gs- /O- /C /Sm /Q /Sp1
IC_STDOPS_SIZ:= /Gs- /O- /C /Sm /Q /Sp1
IC_SCC      :=
IC_CHKSTK   :=  /Gs+
IC_DEFINE   :=  /D
IC_DYNAM    :=#
IC_MT       :=  /Gm+
IC_DLL      :=  /Ge-
IC_FLAT     :=  YES

IL_NOI      :=  /noi
IL_DEBUG    :=  /de
IL_WARNDUPE :=#
IL_STDOPS   :=  /nod/batch/packc/exep/st:4096
IL_STACK    :=  /st:
IL_LINK     :=  link386

