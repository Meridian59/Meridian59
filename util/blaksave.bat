@echo off
rem Press save button
btnpush blakserv "BlakSton Server" 40003
if errorlevel 1 goto exit

if "%1". == "NOEXIT". goto exit

rem Press exit button
btnpush blakserv "BlakSton Server" 40001
if errorlevel 1 goto exit

:waitconfirm

echo Waiting for dialog
btnpush #32770 "BlakSton Server" 6

if errorlevel 1 goto retry

goto exit

:retry
sleep 10
goto waitconfirm

:exit