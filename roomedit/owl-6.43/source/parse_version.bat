@echo off
setlocal enableextensions

if not "%1" == "" if not "%2" == "" goto Parse

echo.
echo parse_version.bat ^<macro-prefix^> ^<file^> 
echo Parses the version number from the given source file.
echo Copyright © 2010-2012 Vidar Hasfjord
echo See OWLNext for license information (http://owlnext.sourceforge.net).
echo.
echo Parameters:
echo.
echo   ^<macro-prefix^> - Specifies the macro prefix for the library.
echo   ^<file^> - Specifies the source file to parse, e.g. "version.h".
echo.
echo Return value:
echo.
echo   0 on success, otherwise 1 or greater
echo.
echo Assuming the macro prefix is set to OWL, the source file must contain a 
echo single definition of the version number formatted as follows:
echo.
echo   ^#define OWL_VERSION(v) v(^<major^>,^<minor^>,^<release^>,^<build^>)
echo.
echo and a prerelease definition as follows:
echo.
echo   ^#define OWL_PRERELEASE ^<0^|1^>
echo. 
echo Assuming the macro prefix is set to OWL, then the parsed version number 
echo and prerelease state are written to the standard output stream as follows:
echo.
echo   OWL_VERSION=^<version-number^>
echo   OWL_PRERELEASE=^<0^|1^>
echo.
echo The formatting of the version number depends on the prerelease state:
echo.
echo   Release: ^<major^>.^<minor^>
echo   Prerelease: ^<major^>.^<minor^>.^<release^>
echo.
echo PRIMARY USAGE: A makefile can redirect the output of this script to a 
echo temporary file, which is then included to determine the version number 
echo and prerelease state of the library.
exit /b

:Parse

rem Check that the given input file exists. If not, return error code 404.

if not exist %2 (
  echo Error: The given input file does not exist ^(%2^).
  exit /b 404
)

rem Assign a variable to the full path of the Windows FIND command.
rem Using the full path avoids conflicts with other similarly named software, e.g. Cygwin.
rem We use the FOR command to convert the path to the short version to eliminate any spaces.
rem If we cannot find the FIND command where we expect, return error code 405.

for %%i in (%systemroot%\system32\find.exe) do (set FIND=%%~si)
if not exist %FIND% (
  echo Error: The Windows FIND command is not present at the expected location ^(%FIND%^).
  exit /b 405
)

rem Perform the parsing

for /F "tokens=3 delims= " %%i in ('%FIND% "define %1_PRERELEASE" %2') do (
  set _PRERELEASE=%%~i
)
for /F "tokens=5-8 delims=(,) " %%i in ('%FIND% "define %1_VERSION(v) v(" %2') do (
  if "%_PRERELEASE%" == "1" (
    rem The prerelease version number is formatted as "major.minor.release".
    echo %1_VERSION=%%i.%%j.%%k
  ) else (
    rem The stable release version number is formatted as "major.minor".
    echo %1_VERSION=%%i.%%j
  )
  echo %1_PRERELEASE=%_PRERELEASE%
)
