# Defines for GNU C/C++

# GCC doesn't have memory models

GC_MSMALL   :=
GC_MMEDIUM  :=
GC_MCOMPACT :=
GC_MLARGE   :=
GC_MHUGE    :=
GC_MTINY    :=
GC_FMSMALL  :=f
GC_FMMEDIUM :=f
GC_FMCOMPACT:=f
GC_FMLARGE  :=f
GC_FMHUGE   :=f
GC_FMTINY   :=f
GC_MODS     :=f

GC_INCLUDE  := -I
GC_DEBUG    :=
GC_DEBUGOFF :=#
GC_MODEL    :=#
GC_OUTDIR   :=  -o #
GC_OVL_OBJ  :=#
GC_STARTUP  :=#
GC_LIBS      =  $(PVTLIB) libc libgpp os2
GC_CPP      :=  -x c++

GC_CC       :=  gcc -DUNIX

GC_STDOPS   :=  -c -Wall
GC_STDOPS_SIZ:= -c -Wall
GC_SCC      :=#
GC_CHKSTK   := -mstack-check
GC_DEFINE   := -D
GC_DYNAM    :=#
GC_MT       :=#
GC_DLL      :=#
GC_FLAT     :=  YES

