.IF $(COMP)==TC
.IMPORT: TLIBDIR
.END

.IF $(COMP)==TP
.IMPORT: TLIBDIR
.END

# tOpaz C++ (aka BC++ for os/2)

.IF $(COMP)==OC
.IMPORT: TLIBDIR
.END

.IF $(COMP)==BC
.IMPORT: TLIBDIR
.END

# Turbo C 2.0

.IF $(COMP)==TC
TC_MSMALL   :=  s
TC_MMEDIUM  :=  m
TC_MCOMPACT :=  c
TC_MLARGE   :=  l
TC_MHUGE    :=  h
TC_MTINY    :=  t
TC_FMSMALL  :=  s
TC_FMMEDIUM :=  m
TC_FMCOMPACT:=  c
TC_FMLARGE  :=  l
TC_FMHUGE   :=  h
TC_FMTINY   :=  t


TC_INCLUDE  :=  -I
TC_DEBUG    :=  -v+
TC_DEBUGOFF :=  -v-
TC_MODEL    :=  -m
TC_OUTDIR   :=  -o
TC_SCC      :=  -p
TC_CHKSTK   :=  -N
TC_OVL_OBJ  :=  $(MAXDIR)\msc4
TC_STARTUP   =  $(TLIBDIR)\\novid\c0$(MODEL)
TC_LIBS      =  $(TLIBDIR)\emu $(TLIBDIR)\math$(MODEL) $(TLIBDIR)\c$(MODEL)
TC_LIBS     +=  $(PVTLIB)
TC_CC       :=  tcc
TC_STDOPS   :=  -d-
TC_STDOPS_SIZ:= -d- -G-
TC_DEFINE   :=  -D
TC_DYNAM    :=
TC_FLAT     :=  NO

.END

# Turbo C++ 1.0

.IF $(COMP)==TP

TP_MSMALL   :=  s
TP_MMEDIUM  :=  m
TP_MCOMPACT :=  c
TP_MLARGE   :=  l
TP_MHUGE    :=  h
TP_MTINY    :=  t
TP_FMSMALL  :=  s
TP_FMMEDIUM :=  m
TP_FMCOMPACT:=  c
TP_FMLARGE  :=  l
TP_FMHUGE   :=  h
TP_FMTINY   :=  t

TP_INCLUDE  :=  -I
TP_DEBUG    :=  -v
TP_DEBUGOFF :=  -v-
TP_MODEL    :=  -m
TP_OUTDIR   :=  -o
TP_SCC      :=  -p
TP_CHKSTK   :=  -N
TP_OVL_OBJ  :=  $(MAXDIR)\msc4
TP_STARTUP   =  $(TLIBDIR)\\c0$(MODEL)
TP_LIBS      =  $(TLIBDIR)\emu $(TLIBDIR)\math$(MODEL) $(TLIBDIR)\c$(MODEL)
TP_LIBS     +=  $(PVTLIB)
TP_CC       :=  tcc
TP_STDOPS   :=  -d-
TP_STDOPS_SIZ:= -d- -G-
TP_DEFINE   :=  -D
TP_DYNAM    :=  -Y
TP_FLAT     :=  NO

.END


# Borland C++ 3.0

.IF $(COMP)==BC

BC_MSMALL   :=  s
BC_MMEDIUM  :=  m
BC_MCOMPACT :=  c
BC_MLARGE   :=  l
BC_MHUGE    :=  h
BC_MTINY    :=  t
BC_FMSMALL  :=  s
BC_FMMEDIUM :=  m
BC_FMCOMPACT:=  c
BC_FMLARGE  :=  l
BC_FMHUGE   :=  h
BC_FMTINY   :=  t

BC_INCLUDE  :=  -I
BC_DEBUG    :=  -v
BC_DEBUGOFF :=  -v-
BC_MODEL    :=  -m
BC_OUTDIR   :=  -o
BC_SCC      :=  -p
BC_CHKSTK   :=  -N
BC_OVL_OBJ  :=  $(MAXDIR)\msc4
BC_STARTUP   =  $(TLIBDIR)\\c0$(MODEL)
BC_LIBS      =  $(TLIBDIR)\emu $(TLIBDIR)\math$(MODEL) $(TLIBDIR)\c$(MODEL)
BC_LIBS     +=  $(PVTLIB)
BC_CC       :=  bcc
BC_STDOPS   :=  -d-
BC_STDOPS_SIZ:= -d- -G-
BC_DEFINE   :=  -D
BC_DYNAM    :=  -Y
BC_FLAT     :=  NO

.END


.IF $(COMP)==OC

.IMPORT: OCDIR INCLUDE

OC_MSMALL   :=  s
OC_MMEDIUM  :=  s
OC_MCOMPACT :=  s
OC_MLARGE   :=  s
OC_MHUGE    :=  s
OC_MTINY    :=  s
OC_FMSMALL  :=  f
OC_FMMEDIUM :=  f
OC_FMCOMPACT:=  f
OC_FMLARGE  :=  f
OC_FMHUGE   :=  f
OC_FMTINY   :=  f


OC_INCLUDE  :=  -I
OC_DEBUG    :=  -v
OC_DEBUGOFF :=
OC_MODEL    :=  -m
OC_OUTDIR   :=  -o
OC_SCC      :=  -p
OC_CHKSTK   :=  -N
OC_OVL_OBJ  :=  $(MAXDIR)\msc4
OC_STARTUP   =  $(TLIBDIR)\c02
OC_LIBS      =  $(TLIBDIR)\c2 $(TLIBDIR)\os2
OC_LIBS     +=  $(PVTLIB)
OC_CC       :=  bcc -DOS_2
OC_STDOPS   :=  -I$(INCLUDE) -L$(OCDIR)LIB -c
OC_STDOPS_SIZ:= -I$(INCLUDE) -L$(OCDIR)LIB -c -d- -G-
OC_DEFINE   :=  -D
OC_DYNAM    :=
OC_FLAT     :=  YES

.END

.IF $(LINK)==OL

# tOpaz Linker (any version)

OL_WARNDUPE :=  /d
OL_NOI      :=  /c
OL_LINK     :=  tlink
OL_DEBUG    :=  /v
OL_STDOPS   :=
OL_STACK    :=

.END

# Turbo Linker (any version)

TL_WARNDUPE :=  /d
TL_NOI      :=  /c
TL_LINK     :=  tlink
TL_DEBUG    :=  /v
TL_STDOPS   :=
TL_STACK    :=

# Turbo Assembler

TA_AS       := tasm
TA_CASE     := /Mx
TA_DEFINE   := /d
TA_DEBUG    := /Zi
TA_INCLUDE  := /I
TA_FLAGS    := /jQUIRKS /jMASM51 /dTASM


