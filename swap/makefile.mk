
.SOURCE.c: .
.SOURCE.h: .

.IF $(COMP)==TC


.SOURCE.obj: tcr

TLINKDEFS := /m /s /c
TASMDEFS  := /w2 /D_Large /mx
TCDEFS    := -ml -c
MODEL     := $(MLARGE)

swaptest.exe:           swapl.obj \
                        swaptest.obj
            tlink $(TLINKDEFS) $(TC_STARTUP) tcr\swapl tcr\swaptest.obj,swaptest,nul,$(TLIBDIR)\cl

swaptest.obj:           swaptest.c \
                        swap.h
            tcc $(TCDEFS) -otcr\swaptest.obj swaptest.c


swapl.obj:              swap.asm
            tasm $(TASMDEFS) /D__TURBOC__ swap.asm, tcr\swapl.obj;

.ELIF $(COMP)==WC

.SOURCE.obj: wcr

WCDEFS = /ml
MADEFS= /w2 /D_Large /mx /dMASM51

swaptest.exe:   swapl.obj       swaptest.obj
        wlink sys dos n swaptest.exe f wcr\swapl,wcr\swaptest opt map=swaptest.map opt verb

swaptest.obj: swaptest.c swap.h
        wcc $(WCDEFS) /Fo=wcr\swaptest.obj swaptest.c

swapl.obj: swap.asm
        masm $(MADEFS) /D__WATCOMC__ swap.asm, wcr\swapl.obj;

.ELSE

.ERROR: invalid_compiler_setting
.END


