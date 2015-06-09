@echo off
setlocal EnableDelayedExpansion

rem Create a shortcut that can connect to localhost.
call :makeshortcut

echo Post-Build started.
for /F "tokens=1-6 usebackq delims=:" %%a in ("%ProgramW6432%\Open Meridian\settings.txt") do (
   set string=%%a
   set string=!string: =!
   if !string! == "ClientFolder" (
      set m59path=%%b%%c%%d%%e%%f
      set m59path=!m59path:\\=\!
      set m59driveletter=!m59path:~2,1!
      set m59path=!m59path:~1,-2!
      set m59path=!m59path:~2,255!
      set m59path=!m59driveletter!:!m59path!

      rem These two filetypes are the important ones. We could check for the music,
      rem but I've heard more than one user say they deleted it from their resource
      rem folder for various reasons.
      if EXIST "!m59path!\resource\*.bgf" if EXIST "!m59path!\resource\*.bsf" (
         call :copying
         if %ERRORLEVEL% LSS 8 goto found
      )
      echo Copy failed from !m59path!, trying next location...
   )
)

echo No graphics found, Please download the client (and graphics) using the patcher from openmeridian.org.
exit /b 0

:copying
echo Copying live graphics from !m59path! to client folder.
rem The error check after this is currently redundant, but kept in case
rem further copying code is added.
robocopy "!m59path!\resource" ".\run\localclient\resource" *.bsf *.bgf *.wav *.mp3 /R:0 /MT /XO > postbuild.log
if %ERRORLEVEL% GTR 7 goto:eof
rem These extensions aren't used.
rem robocopy "!m59path!\resource" ".\run\localclient\resource" *.mid *.xmi /R:0 /MT /XO > nul
rem if %ERRORLEVEL% GTR 7 goto:eof
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
