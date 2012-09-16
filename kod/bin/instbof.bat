@echo off
rem This ugly substitution is needed because I couldn't find a way to do it
rem in the makefile.
SET SERVERDIR=%1
SET BOFNAME=%2
SET BOFNAME=%BOFNAME:.kod=.bof%
copy %BOFNAME% %SERVERDIR%\loadkod >NUL
