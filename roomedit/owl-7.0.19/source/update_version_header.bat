@echo off
setlocal enableextensions

rem: If you edit this file make sure that all the 'echo' statements for warnings and error messages are updated
rem: with the correct line numbers.

if not "%1" == "" if not "%2" == "" if not "%3" == "" goto Generate

echo.
echo update_version_header.bat ^<ROOTDIR^> ^<template-file^> ^<version-header^>
echo Updates the given version information header based on the given SubWCRev template.
echo Copyright (c) 2011-2023 Vidar Hasfjord
echo See OWLNext for license information (http://owlnext.sourceforge.net).
echo.
echo Parameters:
echo.
echo   ^<ROOTDIR^> - Specifies the path to the OWLNext root directory.
echo   ^<template-file^> - Specifies the source file to use as a template.
echo   ^<version-header^> - Specifies the destination file.
echo.
echo If SubWCRev is not available, and the destination file does not already exist,
echo a version header setting the build number to 0 is generated. If SubWCRev is not
echo available, and the destination file does exist, it is left unchanged.
exit /B 0

:Generate

rem: The ugly "for" command with the %%~$PATH:i syntax checks whether SubWCRev is in the executable path.

set OWL_VERSION_HEADER_TMP="%TEMP%\OWL_VERSION_HEADER_%RANDOM%.tmp"
del /F %OWL_VERSION_HEADER_TMP% >nul 2>nul
for %%i in (SubWCRev.exe) do if not "%%~$PATH:i" == "" (
  SubWCRev %1 %2 %OWL_VERSION_HEADER_TMP%
  if errorlevel 1 (
    echo %~nx0^(35^): warning: SubWCRev failed. E.g. the OWLNext root is not a Subversion working copy.
    call :CreateRudimentaryHeader %1 %2
  )
) else (
  if exist "%3" (
    echo %~nx0^(40^): warning: Could not find SubWCRev to update the version header with accurate build info.
    echo %~nx0^(41^): warning: The existing version header is left unchanged, and might contain inaccurate info.
    goto Finish
  ) else (
    echo %~nx0^(44^): warning: Could not find SubWCRev to create the version header with accurate build info. 
    call :CreateRudimentaryHeader %1 %2
  )
)
if not exist %OWL_VERSION_HEADER_TMP% goto ErrorGenerationFailed

rem: Compare the existing header with the generated one, and update if necessary.

"%systemroot%\system32\fc" %OWL_VERSION_HEADER_TMP% %3 >nul 2>nul
if errorlevel 1 (
  copy /Y %OWL_VERSION_HEADER_TMP% %3 >nul
  if errorlevel 1 goto ErrorUpdateFailed
  echo Updated version header %3.
) else (
  echo Version header is up to date.
)
del %OWL_VERSION_HEADER_TMP% >nul 2>nul

:Finish

exit /B 0

:CreateRudimentaryHeader

echo %~nx0^(68^): warning: A version header setting the build number to 0 is generated.

rem: Find-and-replace (thanks to "http://www.dostips.com/?t=batch.findandreplace")
for /f "tokens=1,* delims=]" %%a in ('"type %2 | "%systemroot%\system32\find" /n /v """') do (
    set "line=%%b"
    if defined line (
    call set "line=%%line:$WCREV$=0%%"
    call set "line=%%line:$WCDATEUTC$=%date% %time%%%"
    call set "line=%%line:$WCMIXED?1:0$=0%%"
    call set "line=%%line:$WCMODS?1:0$=1%%"
    for /f "delims=" %%x in ('"echo."%%line%%""') do (echo %%~x >> %OWL_VERSION_HEADER_TMP%)
    ) else echo. >> %OWL_VERSION_HEADER_TMP%
)
exit /B

:ErrorSubWCRevFailed

echo %~nx0^(85^): error: SubWCRev failed to generate version header.
exit /B 1

:ErrorGenerationFailed

echo %~nx0^(90^): error: Failed to generate version header.
exit /B 2

:ErrorUpdateFailed

echo %~nx0^(95^): error: Failed to update version header %3.
exit /B 3
