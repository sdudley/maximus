@echo off
set comp=TC
set link=TL

set TLIBDIR=s:\lib
path s:\bin;%basepath%

set QC=s:\bin\turboc.cfg
echo >%QC% -Is:\Include;q:\Slib;q:\max;q:\squish;q:\prot;q:\msgapi;q:\ui;q:\mex;q:\btree
echo >>%QC% -Lr:\Tc\Lib
echo >>%QC% -d-
echo >>%QC% -f
echo >>%QC% -u
echo >>%QC% -r
echo >>%QC% -C
echo >>%QC% -c
echo >>%QC% -Er:\Tc\Bin\Tasm.Exe
echo >>%QC% -w
echo >>%QC% -w-ucp
echo >>%QC% -O
echo >>%QC% -Z
echo >>%QC% -G
echo >>%QC% -v
set QC=
