@echo off
rem set GNUPATH=F:/GNU/
rem set GNUPATHB=F:\GNU\
rem set CPLUS_INCLUDE_PATH=%GNUPATH%G++-INCLUDE;%GNUPATH%GCC-INCLUDE;%GNUPATH%OS2-INCLUDE;%GNUPATH%scott-include;
rem set C_INCLUDE_PATH=%GNUPATH%GCC-INCLUDE;%GNUPATH%OS2-INCLUDE;%GNUPATH%scott-include;
rem set INCLUDE=%GNUPATH%OS2-INCLUDE;f:/slib;%STDINCLUDE%
rem set LIB=%GNUPATHB%LIB;%DEVROOT%slib
rem rem set TMPDIR=%GNUPATH%TMP
rem set path=%GNUPATHB%bin;%basepath%;

set GCC2=f:/gcc2
rem echo set INCLUDE=f:\gcc2\gccincl;f:\gcc2\os2incl;%include% pipe sed sQ\\Q/Qg  redir __tmp.cmd
set INCLUDE=f:\gcc2\gccincl;f:\gcc2\os2incl;%stdinclude%
set C_INCLUDE_PATH=f:\gcc2\gccincl;f:\gcc2\os2incl;%stdinclude%
rem call __tmp.cmd
rem del __tmp.cmd

set TMPDIR=f:/gcc2/tmp
set path=f:/gcc2/bin;%basepath%

set COMP=GC
set LINK=IL
set MODE=u

