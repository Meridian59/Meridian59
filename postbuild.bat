@echo off
setlocal EnableDelayedExpansion

rem Create a shortcut that can connect to localhost
call :makeshortcut

echo Post-Build started.
for /F "tokens=1-6 usebackq delims=:" %%a in ("%programfiles%\Open Meridian\settings.txt") do (
   set string=%%a
   set string=!string: =!
   if !string! == "ClientFolder" (
      set m59path=%%b%%c%%d%%e%%f
      set m59path=!m59path:\\=\!
      set m59driveletter=!m59path:~2,1!
      set m59path=!m59path:~1,-2!
      set m59path=!m59path:~2,255!
      set m59path=!m59driveletter!:!m59path!
      if EXIST "!m59path!\resource" (
         call :copying
         if !ERRORLEVEL! EQU 0 goto found
         echo Trying next location...
      )
   )
)

echo No graphics found! Please download the client (and graphics) using the patcher from openmeridian.org.
exit /b 1

:copying
echo Copying live graphics from !m59path! to client folder.
copy /y "!m59path!\resource\*.bgf" .\run\localclient\resource\ >nul
if %ERRORLEVEL% GTR 0 goto:eof
copy /y "!m59path!\resource\*.wav" .\run\localclient\resource\ >nul
if %ERRORLEVEL% GTR 0 goto:eof
rem copy /y "!m59path!\resource\*.xmi" .\run\localclient\resource >nul
rem if %ERRORLEVEL% GTR 0 goto:eof
copy /y "!m59path!\resource\*.mp3" .\run\localclient\resource\ >nul
if %ERRORLEVEL% GTR 0 goto:eof
copy /y "!m59path!\resource\*.bsf" .\run\localclient\resource\ >nul
if %ERRORLEVEL% GTR 0 goto:eof
rem copy /y "!m59path!\resource\*.mid" .\run\localclient\resource\ >nul
rem if %ERRORLEVEL% GTR 0 goto:eof
goto:eof

:found 
echo Post-Build Finished.
echo Please remember to update blakserv.cfg if server is being copied to production.
goto:eof

:makeshortcut
echo Creating meridian.exe shortcut to connect to localhost.
set CURPATH=%~dp0
set cSctVBS=CreateShortcut.vbs
(
  echo Set oWS = WScript.CreateObject^("WScript.Shell"^) 
  echo sLinkFile = oWS.ExpandEnvironmentStrings^("!CURPATH!run\localclient\meridian.exe - Shortcut.lnk"^)
  echo Set oLink = oWS.CreateShortcut^(sLinkFile^)
  echo oLink.Arguments = "/H:127.0.0.1 /P:5959"
  echo oLink.TargetPath = oWS.ExpandEnvironmentStrings^("!CURPATH!run\localclient\meridian.exe"^)
  echo oLink.WorkingDirectory = oWS.ExpandEnvironmentStrings^("!CURPATH!run\localclient"^)
  echo oLink.Save
)1>!cSctVBS!
cscript //nologo .\!cSctVBS!
DEL !cSctVBS! /f /q
