@echo off

SET PATH=%mscdir%\binp;%mscdir%\binb;%basepath%
SET COMP=MC
SET LINK=ML
SET INCLUDE=%mscdir%\INCLUDE;%TOOLKT13INC%;%stdinclude%
SET LIBMSC=%DEVROOT%slib;%mscdir%\lib;%TOOLKT13LIB%
SET LIB=%LIBMSC%
