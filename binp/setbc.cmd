@echo off
set comp=OC
set link=OL
set OCDIR=%COMPROOT%TOPAZ\
set LIBDIR=%OCDIR%LIB
set INCLUDE=%OCDIR%INCLUDE;%stdinclude%
set MODE=p
path %OCDIR%BIN;%basepath%
