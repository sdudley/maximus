@echo off
set mode=p
if .%1==.   set WCCOMP=wcc
if .%1==.   set WPCOMP=wpp
if .%1==.   set COMP=WC
if .%1==.   set INCLUDE=c:\dv\comp\wc10\H;%TOOLKT13INC%;%stdinclude%

if .%1==.32 set WCCOMP=wcc386
if .%1==.32 set WPCOMP=wpp386
if .%1==.32 set COMP=W3
if .%1==.32 set INCLUDE=c:\dv\comp\wc10\H;%TOOLKT20INC%;%stdinclude%
if .%1==.32 set MODE=p

if .%1==.nt set WPCOMP=wpp386
if .%1==.nt set WCCOMP=wcc386
if .%1==.nt set WATCOM=c:\dv\comp\wc10
if .%1==.nt set COMP=W3
if .%1==.nt set INCLUDE=c:\dv\comp\wc10\H;c:\dv\comp\wc10\H\NT;%stdinclude%
if .%1==.nt set MODE=n

SET EDPATH=c:\dv\comp\wc10\eddat
SET PATH=c:\dv\comp\wc10\binnt;c:\dv\comp\wc10\binw;c:\dv\comp\wc10\binb;%BASEPATH%
SEt LINK=WL
SET WLLINK=wlink
rem * If you have less than 16Mb RAM, use this line insteac:
rem * SET WCGMEMORY=2048
SET WCGMEMORY=8192
SET WCC=/d2/w4/zq/zp1/d__WCVER__#%WCVER%
SET WPP=/d2/w4/zq/zp1
SET WCC386=%WCC%
SET WPP386=%WPP%

SET LIB=c:\dv\comp\wc10\lib286;c:\dv\comp\wc10\lib286\dos
SET LIBDOS=c:\dv\comp\wc10\lib286;c:\dv\comp\wc10\lib286\dos
SET LIBOS2=c:\dv\comp\wc10\lib286;c:\dv\comp\wc10\lib286\os2;%TOOLKT13LIB%
SET LIBOS2FLAT=c:\dv\comp\wc10\lib386;c:\dv\comp\wc10\lib386\os2;%TOOLKT20LIB%
SET LIBNT=c:\dv\comp\wc10\lib386;c:\dv\comp\wc10\lib386\nt;%TOOLKT20LIB%

