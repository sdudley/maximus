@echo off
set WATCOM=c:\watcom
set WATCOM_LIBS=c:\dv\comp\wc15
set mode=p
set WATCOM_VERSION=11
if .%1==.   set WCCOMP=wcc
if .%1==.   set WPCOMP=wpp
if .%1==.   set COMP=WC
if .%1==.   set INCLUDE=%WATCOM%\H;%WATCOM_LIBS%\H\OS21x;%stdinclude%

if .%1==.32 set WCCOMP=wcc386
if .%1==.32 set WPCOMP=wpp386
if .%1==.32 set COMP=W3
if .%1==.32 set INCLUDE=%WATCOM%\H;%WATCOM_LIBS%\H\OS2;%stdinclude%
if .%1==.32 set MODE=p

if .%1==.nt set WPCOMP=wpp386
if .%1==.nt set WCCOMP=wcc386
if .%1==.nt set WATCOM=%WATCOM%
if .%1==.nt set COMP=W3
if .%1==.nt set INCLUDE=%WATCOM%\H;%WATCOM_LIBS%\H\NT;%stdinclude%
if .%1==.nt set MODE=n

SET EDPATH=%watcom%\eddat
SET PATH=%WATCOM%\binnt;%WATCOM%\binw;%BASEPATH%
SEt LINK=WL
SET WLLINK=wlink libpath %watcom_libs%\lib386\nt lib nt,kernel32,user32
rem * If you have less than 16Mb RAM, use this line instead:
rem * SET WCGMEMORY=2048
SET WCGMEMORY=8192
SET WCC=/d2/w4/zq/zp1/d__WCVER__#%WCVER%/wcd=1180/wcd=1181
SET WPP=/d2/w4/zq/zp1
SET WCC386=%WCC%
SET WPP386=%WPP%

SET LIB=%WATCOM%\lib286;%WATCOM%\lib286\dos;%WATCOM_LIBS%\lib286\dos
SET LIBDOS=%WATCOM%\lib286;%WATCOM%\lib286\dos;%WATCOM_LIBS%\lib286\dos
SET LIBOS2=%WATCOM%\lib286;%WATCOM%\lib286\os2;%WATCOM_LIBS%\lib286\os2;%TOOLKT13LIB%
SET LIBOS2FLAT=%WATCOM%\lib386;%WATCOM%\lib386\os2;%WATCOM_LIBS%\lib386\os2;%TOOLKT20LIB%
SET LIBNT=%WATCOM%\lib386;%WATCOM%\lib386\nt;%WATCOM_LIBS%\lib386\nt;%TOOLKT20LIB%

