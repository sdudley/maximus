@echo off
if .%1==.   goto normal
if .%1==.32 goto bit32
if .%1==.nt goto nt

:normal
set WCCOMP=wcc
set WPCOMP=wpp
set COMP=WC
set INCLUDE=%WATCOM%\H;%stdinclude%
set WCVER=1000
set MODE=r
goto done

:bit32
set WCCOMP=wcc386
set WPCOMP=wpp386
set COMP=W3
set INCLUDE=%WATCOM%\H;%stdinclude%
set WCVER=1000
set MODE=r
goto done

:nt
set WCCOMP=wcc386
set WPCOMP=wpp386
set COMP=W3
set INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;%stdinclude%
set WCVER=1000
set MODE=n
goto done

:done
if %MODE%==n     PATH %WATCOM%\binnt;%WATCOM%\binb;%BASEPATH%
if not %MODE%==n PATH %WATCOM%\bin;%WATCOM%\binb;%BASEPATH%

SEt LINK=WL
SET WLLINK=wlink
SET WCGMEMORY=8192
SET WCC=/d2/w4/zq/zp1/d__WCVER__#%WCVER%
SET WPP=/d2/w4/zq/zp1/d__WCVER__#%WCVER%
SET WCC386=%WCC%
SET WPP386=%WPP%

SET LIB=%DEVROOT%slib
rem SET LIBDOS=%WATCOM%\lib286;%WATCOM%\lib286\dos
rem SET LIBOS2=%WATCOM%\lib286;%WATCOM%\lib286\os2;%TOOLKT13LIB%
rem SET LIBOS2FLAT=%WATCOM%\lib386;%WATCOM%\lib386\os2;%TOOLKT20LIB%

