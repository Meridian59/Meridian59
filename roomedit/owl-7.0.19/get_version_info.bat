@echo off
setlocal enableextensions

set SHOULD_PAUSE=1
if "%1" == "/c" (set SHOULD_PAUSE=0 & goto GetInfo)
if "%1" == "" goto GetInfo

:ShowHelp

echo.
echo get_version_info.bat
echo Extracts and displays version information from "version.h".
echo Copyright (c) 2023 Vidar Hasfjord
echo See OWLNext for license information (https://sourceforge.net/projects/owlnext).
echo.
echo Options:
echo.
echo  /c - Do not pause.
exit /b 1

:GetInfo

set FIND="%systemroot%\system32\find.exe"
if not exist %FIND% goto ErrorFindCommandMissing

if "%SHOULD_PAUSE%" == "1" (cls & echo %~nx0:)
set VERSION_H=include\owl\version.h
set VERSION_H_TEMPLATE=source\owlcore\version.h
if exist %VERSION_H% (
  echo.
  echo Information from last build setup:
  %FIND% "define OWL_VERSION(v)" %VERSION_H%
  %FIND% "define OWL_PRERELEASE" %VERSION_H%
  %FIND% "define OWL_BUILD_REVISION" %VERSION_H%
  %FIND% "define OWL_BUILD_DIRTY" %VERSION_H%
) else if exist %VERSION_H_TEMPLATE% (
  %FIND% "define OWL_VERSION(v)" %VERSION_H_TEMPLATE%
  %FIND% "define OWL_PRERELEASE" %VERSION_H_TEMPLATE%
  echo.
  echo Perform build setup to generate "%VERSION_H%".
  echo The generated header will include build information.
) else (
  goto ErrorHeaderMissing
)
if "%SHOULD_PAUSE%" == "1" (echo. & pause)
exit /b 0

:ErrorFindCommandMissing

if "%SHOULD_PAUSE%" == "1" (cls & echo %~nx0:)
echo.
echo ERROR: The Windows FIND command is not present at the expected location ^(%FIND%^). 1>&2
if "%SHOULD_PAUSE%" == "1" (echo. & pause)
exit /b 2

:ErrorHeaderMissing

echo.
echo ERROR: Cannot locate the version header nor the template:
echo.
echo   Header: %VERSION_H%
echo   Template: %VERSION_H_TEMPLATE%
echo.
echo Update your installation.
if "%SHOULD_PAUSE%" == "1" (echo. & pause)
exit /b 2
