# If nothing is specified, use TC, Tlink and Tasm by default.

.IMPORT: CVSROOT
.ESCAPE_PREFIX := `

.IF $(MODE)==p
OPTP := p
.ELSE
.IF $(MODE)==n
OPTP := n
.END
.END

.IF $(HOSTMODE)==p
HOSTOPTP := p
.ELSE
.IF $(HOSTMODE)==n
HOSTOPTP := n
.END
.END

COMP        *=  TC
LINK        *=  TL
ASM         *=  TA
CMODE       :=  $(COMP)$(MODE)#
STACK        =  5632

# Assume vio (PM-compatible)
PMCOMPAT    :=  pmc


# For 32-bit compilers

FLAT        := $($(COMP)_FLAT)

# Default memory model is small, if not specified on the cmd line

MODEL       *= $($(COMP)_MSMALL)

# Generic C options for the current compiler

C_CPP       :=  $($(COMP)_CPP)
C_SAVEREG   :=  $($(COMP)_SAVEREG)
C_INCLUDE   :=  $($(COMP)_INCLUDE)
C_DEBUG     :=  $($(COMP)_DEBUG)
C_DEBUGOFF  :=  $($(COMP)_DEBUGOFF)
C_MODEL     :=  $($(COMP)_MODEL)
C_OUTDIR    :=  $($(COMP)_OUTDIR)
C_SCC       :=  $($(COMP)_SCC)
C_CHKSTK    :=  $($(COMP)_CHKSTK)
C_STDOPS    :=  $($(COMP)_STDOPS)
C_STDOPS_SIZ:=  $($(COMP)_STDOPS_SIZ)
C_DEFINE    :=  $($(COMP)_DEFINE)
C_DYNAM     :=  $($(COMP)_DYNAM)
C_MT        :=  $($(COMP)_MT)
C_DLL       :=  $($(COMP)_DLL)
C_LARGEFN   :=  $($(COMP)_LARGEFN)
C_EXCEPTION :=  $($(COMP)_EXCEPTION)
C_ISR       :=  $($(COMP)_ISR)

.IF $(MODE)==p
C_STDOPS_OPT:=  $(C_STDOPS)
.ELSE
.IF $(FLAT)==YES
C_STDOPS_OPT:=  $(C_STDOPS)
.ELSE
C_STDOPS_OPT:=  $(C_STDOPS_SIZ)
.END
.END


STARTUP      =  $($(COMP)_STARTUP)
OVL_OBJ     :=  $($(COMP)_OVL_OBJ)
LIBS         =  $($(COMP)_LIBS)
CFLAGS       =  $(C_DEBUG) $($(COMP)_STDOPS_SIZ) $(C_MODEL)$(MODEL)
STDCFLAGS    =  $(CFLAGS)
CC          :=  $($(COMP)_CC)
PP          :=  $($(COMP)_PP)

MSMALL      := $($(COMP)_MSMALL)
MMEDIUM     := $($(COMP)_MMEDIUM)
MCOMPACT    := $($(COMP)_MCOMPACT)
MLARGE      := $($(COMP)_MLARGE)
MHUGE       := $($(COMP)_MHUGE)
MTINY       := $($(COMP)_MTINY)

FMSMALL     := $($(COMP)_FMSMALL)
FMMEDIUM    := $($(COMP)_FMMEDIUM)
FMCOMPACT   := $($(COMP)_FMCOMPACT)
FMLARGE     := $($(COMP)_FMLARGE)
FMHUGE      := $($(COMP)_FMHUGE)
FMTINY      := $($(COMP)_FMTINY)
MODS        := $($(COMP)_MODS)


# Standard libraries

.IF $(MODE)==r
SQUISHLIB    =  $()$(MLIBDIR)\$(CMODE)mapi$(MODEL).lib
.ELSE
SMSERIALLIB  =  $()$(LIBDIR)\smserial.lib
.IF $(FLAT)==YES
.IF $(MODE)==n
SQUISHLIB    =  $()$(MLIBDIR)\msgapint.lib
.ELSE
SQUISHLIB    =  $()$(MLIBDIR)\msgapi32.lib
.END
.ELSE
SQUISHLIB    =  $()$(MLIBDIR)\msgapi.lib
.END
.END

.IF $(MODE)==p
.IF $(FLAT)==YES
TRACKLIB  = $(MLIBDIR)\maxbt32.lib $(MLIBDIR)\maxdb32.lib
.ELSE
TRACKLIB  = $(MLIBDIR)\maxbt.lib $(MLIBDIR)\maxdb.lib
.END
.ELIF $(MODE)==n
TRACKLIB  = $(MLIBDIR)\maxbtnt.lib $(MLIBDIR)\maxdbnt.lib
.ELSE
TRACKLIB  = $(MLIBDIR)\$(COMP)$(MODE)tra$(MODEL).lib $(MLIBDIR)\$(COMP)$(MODE)btr$(MODEL).lib
.END

TUILIB    =  $(LIBDIR)\$(CMODE)tui$(MODEL).lib


# Generic link options for the current linker

L_NOI       :=  $($(LINK)_NOI)
L_WARNDUPE  :=  $($(LINK)_WARNDUPE)
L_DEBUG     :=  $($(LINK)_DEBUG)
L_STDOPS     =  $($(LINK)_STDOPS)
L_STACK      =  $($(LINK)_STACK)
LFLAGS       =  $(L_NOI) $(L_WARNDUPE) $(L_STDOPS)
LL           =  $($(LINK)_LINK)

.IF $(CC)==$(NULL)
  .IMPORT: invalid_comp_macro_defined__must_be_uppercase
.END

.IF $(LL)==$(NULL)
  .IMPORT: invalid_link_macro_defined__must_be_uppercase
.END

# Our private library file for this compiler

PVTLIB       =  $(LIBDIR)\$(CMODE)lib$(MODEL).lib

# Generic assembler options

AS          :=  $($(ASM)_AS)
A_CASE      :=  $($(ASM)_CASE)
A_DEBUG     :=  $($(ASM)_DEBUG)
A_INCLUDE   :=  $($(ASM)_INCLUDE)
A_DEFINE    :=  $($(ASM)_DEFINE)

AFLAGS      :=  $(A_CASE) $(A_DEBUG)

.IF $(MODE)==p
AFLAGS      +:=  $(A_DEFINE)OS_2
.ELSE
.IF $(MODE)==n
AFLAGS      +:=  $(A_DEFINE)NT
.ELSE
AFLAGS      +:=  $(A_DEFINE)__MSDOS__
.END
.END

YY          :=  $(CVSROOT)\bison
YFLAGS      :=  -t -v -d -l
Y_OUT       :=  -o


# Special rules for using the WC linker, since it's "different".

.IF $(LINK) == WL
%.exe:;         $(LL) op st=$(STACK) $(LFLAGS) @$(mktmp $(STARTUP) file $(&:t"`nfile ":s|/|\|)`nname $@`n$(null,$(LIBS) $(NULL) library) $(LIBS:t",")`n).
.ELSE
.IF $(LINK) != OL
SEMICOLON := ;
.END

%.exe:;         $(LL) $(LFLAGS) @$(mktmp $(STARTUP) $(&:t"+`n":s|/|\|)`n$@`nnul`n$(LIBS))$(SEMICOLON)
.END

# Default inference rules for C -> OBJ, ASM -> OBJ, OBJ -> EXE,
# and Y -> C.

%.lib: %.dll
        implib $*.lib $*.dll

# If the makefile is empty, create a new one.  (LIB will
# ask "Create $*.lib?" if we don't create it first, which
# throws our mktmp command into spasms.
#
.IF $(COMP) == WC
USE_WLIB = 1
.END
.IF $(COMP) == W3
USE_WLIB = 1
.END

.IF $(USE_WLIB)==1
%.lib:
        wlib $@ @$(mktmp -+$(?:t" `n-+")`n).
.ELSE
%.lib:
        +if not exist $@ $(LIBBER) $@,
        $(LIBBER) @$(mktmp $@`n-+$(?:t" &`n-+":s|/|\|);`n).
.END

# Default rules for making model-dependent C object files

%.obj $(SWC): %.c ; $(CC) $(CFLAGS) $(<:s|\\|\|)
%.obj $(SWC): %.cc ; $(PP) $(CFLAGS) $(C_CPP) $(<:s|\\|\|)
%.obj $(SWC): %.cpp ; $(PP) $(CFLAGS) $(C_CPP) $(<:s|\\|\|)

%.c: %.y
        +if exist $*.c del $*.c
        $(YY) $(YFLAGS) $(Y_OUT) $*.c $*.y

%.res: %.rc
        rc -r $<

# Don't need other memory models for flat architechture
	
%.obf $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MSMALL) $(<:s|\\|\|)
%.obf $(SWC): %.cc; $(PP) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_CPP) $(<:s|\\|\|)
%.obf $(SWC): %.cpp; $(PP) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_CPP) $(<:s|\\|\|)
%.obs $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MSMALL) $(<:s|\\|\|)
%.obs $(SWC): %.cc; $(PP) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_CPP) $(<:s|\\|\|)
%.obs $(SWC): %.cpp; $(PP) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_CPP) $(<:s|\\|\|)
%.obS $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MSMALL) $(<:s|\\|\|)
%.obS $(SWC): %.cc; $(PP) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_CPP) $(<:s|\\|\|)
%.obS $(SWC): %.cpp; $(PP) $(CFLAGS) $(C_MODEL)$(MSMALL) $(C_CPP) $(<:s|\\|\|)


.IF $(FLAT)==NO
%.obm $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MMEDIUM) $(<:s|\\|\|)
%.obM $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MMEDIUM) $(<:s|\\|\|)
%.obc $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MCOMPACT) $(<:s|\\|\|)
%.obC $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MCOMPACT) $(<:s|\\|\|)
%.obl $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MLARGE) $(<:s|\\|\|)
%.obL $(SWC): %.c; $(CC) $(CFLAGS) $(C_MODEL)$(MLARGE) $(<:s|\\|\|)
.END

.IF $(FLAT)==YES
OPT32 := 32
.END

# Ditto, but for ASM files

ACMD = $(AS) $(AFLAGS)


# Don't need other memory models for flat architechture

.IF $(FLAT)==YES
%.obj: %.asm; $(ACMD) $(A_DEFINE)__FLAT__ $<, $@;
%.obf: %.asm; $(ACMD) $(A_DEFINE)__FLAT__ $<, $@;
.ELSE
%.obj: %.asm; $(ACMD) $<, $@;
%.obs: %.asm; $(ACMD) $(A_DEFINE)MSMALL   $<, $@;
%.obS: %.asm; $(ACMD) $(A_DEFINE)MSMALL   $<, $@;
%.obm: %.asm; $(ACMD) $(A_DEFINE)MMEDIUM  $<, $@;
%.obM: %.asm; $(ACMD) $(A_DEFINE)MMEDIUM  $<, $@;
%.obc: %.asm; $(ACMD) $(A_DEFINE)MCOMPACT $<, $@;
%.obC: %.asm; $(ACMD) $(A_DEFINE)MCOMPACT $<, $@;
%.obl: %.asm; $(ACMD) $(A_DEFINE)MLARGE   $<, $@;
%.obL: %.asm; $(ACMD) $(A_DEFINE)MLARGE   $<, $@;
.END

# Rules for making archives

%.lzh:
        +if exist $@ del $@
        lh a $(LHFLAGS) $@ @$(mktmp $(&:t" `n":s|/|\\|)`n)

%.zip:
        +if exist $@ del $@
        pkz -a $(ZIPFLAGS) $@ @$(mktmp $(&:t" `n":s|/|\\|)`n)

