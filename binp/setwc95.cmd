@echo off
set mode=p
if .%1==.   set WCCOMP=wcc
if .%1==.   set WPCOMP=wpp
if .%1==.   set WATCOM=%WC16DIR%
if .%1==.   set COMP=WC
if .%1==.   set INCLUDE=%WATCOM%\H;%TOOLKT13INC%;%stdinclude%
if .%1==.   set WCVER=950

if .%1==.32 set WCCOMP=wcc386
if .%1==.32 set WPCOMP=wpp386
if .%1==.32 set WATCOM=%WC32DIR%
if .%1==.32 set COMP=W3
if .%1==.32 set INCLUDE=%WATCOM%\H;%TOOLKT20INC%;%stdinclude%
if .%1==.32 set WCVER=950
if .%1==.32 set MODE=p

if .%1==.nt set WPCOMP=wpp386
if .%1==.nt set WCCOMP=wcc386
if .%1==.nt set WATCOM=%WC32DIR%
if .%1==.nt set COMP=W3
if .%1==.nt set INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;%stdinclude%
if .%1==.nt set WCVER=950
if .%1==.nt set MODE=n

SET PATH=%WATCOM%\binp;%WATCOM%\binb;%BASEPATH%
SEt LINK=WL
SET WLLINK=wlink
rem * If you have less than 16Mb RAM, use this line instead:
rem * SET WCGMEMORY=2048
SET WCGMEMORY=4096
SET WCC=/d2/w4/zq/zp1/d__WCVER__#%WCVER%
SET WPP=/d2/w4/zq/zp1
SET WCC386=%WCC%
SET WPP386=%WPP%

SET LIB=%DEVROOT%lib
SET LIBDOS=%WC16DIR%\lib286;%WC16DIR%\lib286\dos
SET LIBOS2=%WC16DIR%\lib286;%WC16DIR%\lib286\os2;%TOOLKT13LIB%
SET LIBOS2FLAT=%WC32DIR%\lib386;%WC32DIR%\lib386\os2;%TOOLKT20LIB%

