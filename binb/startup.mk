# Standard DMAKE prologue information

# Search for headers in the directory that DMAKE.EXE was found in


.IMPORT: BINROOT
.INCLUDEDIRS: $(BINROOT)\binb

.IMPORT: COMP LINK ASM LIBBER SLIBDIR MODE HOSTMODE TEMP DEVROOT
.IMPORT: LIBDIR MLIBDIR DLLDIR MAXDIR MAPIDIR TUIDIR MCPDIR

# Default to real mode

MODE        *:= p

.INCLUDE: <dmake.mk>

# Compiler-specific support

.INCLUDE: <ms.mk>
.IF $(COMP)==MC
.INCLUDE: <ms32.mk>
.END

.INCLUDE: <turbo.mk>

.IF $(COMP)==WC
.INCLUDE: <watcom.mk>
.ELIF $(COMP)==W3
.INCLUDE: <watcom.mk>
.END

.INCLUDE: <ibm.mk>

.IF $(COMP)==GC
.INCLUDE: <gnu.mk>
.END

# Now map these onto the standard options.  Also defines inference rules

.INCLUDE: <generic.mk>

