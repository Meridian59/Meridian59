@echo off
if not exist %1 goto done
blakdiff %1 %2\%1
if errorlevel 1 copy %1 %2 > NUL
:done
