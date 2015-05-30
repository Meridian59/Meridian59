@echo off
echo Post-Build started.
echo Copying Official Graphics to client folder...
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.bgf" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.wav" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.xmi" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.mp3" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.bsf" .\run\localclient\resource\
copy /y "%programfiles%\Open Meridian\Meridian 103\resource\*.mid" .\run\localclient\resource\
echo Please remember to update blakserv.cfg if server is being copied to production.
echo Post-Build Finished.