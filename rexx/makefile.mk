.SOURCE: . ..\slib
.SOURCE.obj: w3p
.SOURCE.dll: ..\dll

all .PHONY: maxuapi.dll

CFLAGS = $(C_STDOPS_OPT) $(C_DLL) $(C_OUTDIR)$@

SOBJS := maxuapi.obj expimp.obj rexx.obj misc.obj
LOBJS := userapi.obj strnncpy.obj setfsize.obj fexist.obj \
         ffind.obj

maxuapi.dll: $(SOBJS) $(LOBJS) maxuapi.lnk
        wlink @maxuapi

