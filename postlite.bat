@echo off
echo Post-Build (Lite Edition) started.
echo Clearing client RSC files... 
del .\run\localclient\resource\*.rsc 
echo Clearing client RSB files... 
del .\run\localclient\resource\*.rsb
echo Building RSB file.... 
.\bin\rscmerge.exe -o .\run\localclient\resource\rsc0000.rsb .\run\server\rsc\*.rsc 
if "%errorlevel%"=="0" echo RSB Success! 
echo Running RSC Duplicate cheker....
echo *************************************************
.\run\server\rsc\RSCDuplicateTest2.exe
echo *************************************************
echo If there is anything between the asterisk lines, please resolve before continuing.
echo Deleting Logs...
del /F/S/Q .\run\server\channel\*.txt
echo Post-Build Finished.