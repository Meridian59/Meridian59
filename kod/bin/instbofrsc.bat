@echo off
rem This ugly substitution is needed because I couldn't find a way to do it
rem in the makefile (used to copy %RSCNAME% "%TOPDIR%\run\localclient\resource" >nul))
rem Arguments: <root dir> <server dir> <kod file that was built>
SET TOPDIR=%1
SET SERVERDIR=%2
SET RSCNAME=%3
SET BOFNAME=%3
SET RSCNAME=%RSCNAME:.kod=.rsc%
SET BOFNAME=%BOFNAME:.kod=.bof%
copy %BOFNAME% %SERVERDIR%\loadkod >NUL
if NOT exist %RSCNAME% goto:eof

%TOPDIR%\bin\blakdiff %RSCNAME% %TOPDIR%\run\server\rsc\%RSCNAME% >NUL
if errorlevel 1 (copy %RSCNAME% %TOPDIR%\run\server\rsc >nul)
