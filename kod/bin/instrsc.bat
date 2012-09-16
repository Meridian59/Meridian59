@echo off
rem This ugly substitution is needed because I couldn't find a way to do it
rem in the makefile.
rem Arguments: <root dir> <kod file that was built>
SET TOPDIR=%1
SET RSCNAME=%2
SET RSCNAME=%RSCNAME:.kod=.rsc%
if not exist %RSCNAME% goto done
%TOPDIR%\bin\blakdiff %RSCNAME% %TOPDIR%\run\server\rsc\%RSCNAME% >NUL
if errorlevel 1 (copy %RSCNAME% %TOPDIR%\run\server\rsc >nul & copy %RSCNAME% "%TOPDIR%\run\localclient\resource" >nul)
:done
