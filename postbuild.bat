@echo off
echo Post-Build started.
echo Clearing client RSC files... 
del .\run\localclient\resource\*.rsc 
echo Clearing client RSB files... 
del .\run\localclient\resource\*.rsb
echo Copying kodbase to server
copy /y .\kod\kodbase.txt .\run\server\kodbase.txt
echo Copying rooms to server
mkdir .\run\server\rooms\
copy /y .\resource\rooms\*.roo .\run\server\rooms\
echo Copying RSC files.... 
for /R .\kod\ %%f in (*.rsc) do copy "%%f" .\run\server\rsc\ 
echo Copying Constant files...
copy /y .\kod\include\*.khd .\run\server\
echo Building RSB file.... 
.\bin\rscmerge.exe -o .\run\localclient\resource\rsc0000.rsb .\run\server\rsc\*.rsc 
if "%errorlevel%"=="0" echo RSB Success! 
echo Copying Room files to client folder.
copy /y .\resource\rooms\*.roo .\run\localclient\resource\
echo Copying .dll files to client folder.
copy /y .\module\chess\release\chess.dll .\run\localclient\resource\
copy /y .\module\mailnews\release\mailnews.dll .\run\localclient\resource\
copy /y .\module\merintr\release\merintr.dll .\run\localclient\resource\
copy /y .\module\char\release\char.dll .\run\localclient\resource\
copy /y .\module\intro\release\intro.dll .\run\localclient\resource\
copy /y .\module\dm\release\dm.dll .\run\localclient\resource\
copy /y .\module\admin\release\admin.dll .\run\localclient\resource\
echo Copying Official Graphics to client folder...
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.bgf" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.wav" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.xmi" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.mp3" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.bsf" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.mid" .\run\localclient\resource\
echo Please remember to update blakserv.cfg if server is being copied to production.
echo Post-Build Finished.