@echo off
setlocal enableextensions

echo.
echo =====================================
echo  OWLNext Build Script for C++Builder
echo =====================================
echo.

if not defined BDS (
  echo ---
  echo --- Error: Unable to find the development tools.
  echo --- Define the BDS ^(product root^) environment variable.
  echo ---
  exit /b 1
)

set COMPROOT=%BDS%
if defined COMPVER goto parse_version

echo ---
echo --- Detecting the compiler version
echo ---
echo.

set FIND="%systemroot%\system32\find.exe"
if not exist %FIND% (
  echo ---
  echo --- Error: Unable to locate the FIND command.
  echo ---
  exit /b 2
)

set BCC32_BANNER="%TEMP%\OWL_BCC32_BANNER.tmp"
"%BDS%\bin\bcc32.exe" --version >%BCC32_BANNER% 2>&1
if errorlevel 1 (
  echo ---
  echo --- Error: Unable to generate compiler version banner for analysis.
  echo ---
  exit /b 3
)
if not exist %BCC32_BANNER% (
  echo ---
  echo --- Error: Failed to create temporary file %BCC32_BANNER%.
  echo ---
  exit /b 4
)

rem Try the latest known compiler first and proceed in reverse order.

%FIND% "C++ 7.20" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=720
  goto compiler_detected
)

%FIND% "C++ 7.10" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=710
  goto compiler_detected
)

%FIND% "C++ 7.00" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=19
  goto compiler_detected
)

%FIND% "C++ 6.90" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=18
  goto compiler_detected
)

%FIND% "C++ 6.80" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=17
  goto compiler_detected
)

%FIND% "C++ 6.70" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=16
  goto compiler_detected
)

%FIND% "C++ 6.60" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=15
  goto compiler_detected
)

%FIND% "C++ 6.50" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=14
  goto compiler_detected
)

%FIND% "C++ 6.40" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=13
  goto compiler_detected
)

%FIND% "C++ 6.30" <%BCC32_BANNER% >nul
if not errorlevel 1 (
  set COMPVER=12
  goto compiler_detected
)

:compiler_detection_error

echo ---
echo --- Error: Unable to detect the compiler version.
echo --- See "bc.mak" for acceptable values for COMPVER.
echo ---
%FIND% "COMPVER=" bc.mak 2>nul
exit /b 5

:compiler_detected

del %BCC32_BANNER% >nul 2>&1
echo COMPVER = %COMPVER%
echo.

:parse_version

echo ---
echo --- Parsing the OWLNext version number from sources
echo ---
echo.

call ..\parse_version.bat OWL version.h >"%TEMP%\OWL_VERSION.tmp"
for /f "tokens=1,2 delims==" %%i in (%TEMP%\OWL_VERSION.tmp) do set %%i=%%j
echo OWL_VERSION = %OWL_VERSION%
echo.
if "%OWL_VERSION%" == "" (
  echo ---
  echo --- Error: Unable to parse the version number.
  echo ---
  exit /b 6
)

set MAKE="%COMPROOT%\bin\make" -f bc.mak -a -c -l+ 
set COMMON=-DNOCOMPILE_ASM WIN32=1 OWLVER=%OWL_VERSION% BCBROOT="%COMPROOT%" %1 %2 %3 %4 %5 %6 %7 %8 %9
set DEBUGS=DEBUG=1 CODEGUARD=0

rem Multithreaded non-Unicode variants

echo ---
echo --- Building the OWLNext Static Release Library (multithreaded)
echo ---
echo.
%MAKE% -DMT -DOWLSECTION %COMMON%
echo.

echo ---
echo --- Building the OWLNext Release DLL (multithreaded)
echo ---
echo.
%MAKE% -DMT DLL=1 %COMMON%
echo.

echo ---
echo --- Building the OWLNext Static Debug Library (multithreaded)
echo ---
echo.
%MAKE% -DMT -DOWLSECTION %DEBUGS% %COMMON%
echo.

echo ---
echo --- Building the OWLNext Debug DLL (multithreaded)
echo ---
echo.
%MAKE% -DMT DLL=1 %DEBUGS% %COMMON%
echo.

rem Singlethreaded non-Unicode variants

echo ---
echo --- Building the OWLNext Static Release Library (singlethreaded)
echo ---
echo.
%MAKE% -DOWLSECTION %COMMON%
echo.

echo ---
echo --- Building the OWLNext Release DLL (singlethreaded)
echo ---
echo.
%MAKE% DLL=1 %COMMON%
echo.

echo ---
echo --- Building the OWLNext Static Debug Library (singlethreaded)
echo ---
echo.
%MAKE% -DOWLSECTION %DEBUGS% %COMMON%
echo.

echo ---
echo --- Building the OWLNext Debug DLL (singlethreaded)
echo ---
echo.
%MAKE% DLL=1 %DEBUGS% %COMMON%
echo.

rem Multithreaded Unicode variants

echo ---
echo --- Building the OWLNext Unicode Static Release Library (multithreaded)
echo ---
echo.
%MAKE% -DUNICODE -DMT -DOWLSECTION %COMMON%
echo.

echo ---
echo --- Building the OWLNext Unicode Release DLL (multithreaded)
echo ---
echo.
%MAKE% -DUNICODE -DMT DLL=1 %COMMON%
echo.

echo ---
echo --- Building the OWLNext Unicode Static Debug Library (multithreaded)
echo ---
echo.
%MAKE% -DUNICODE -DMT -DOWLSECTION %DEBUGS% %COMMON%
echo.

echo ---
echo --- Building the OWLNext Unicode Debug DLL (multithreaded)
echo ---
echo.
%MAKE% -DUNICODE -DMT DLL=1 %DEBUGS% %COMMON%
echo.

rem Singlethreaded Unicode variants

echo ---
echo --- Building the OWLNext Unicode Static Release Library (singlethreaded)
echo ---
echo.
%MAKE% -DUNICODE -DOWLSECTION %COMMON%
echo.

echo ---
echo --- Building the OWLNext Unicode Release DLL (singlethreaded)
echo ---
echo.
%MAKE% -DUNICODE DLL=1 %COMMON%
echo.

echo ---
echo --- Building the OWLNext Unicode Static Debug Library (singlethreaded)
echo ---
echo.
%MAKE% -DUNICODE -DOWLSECTION %DEBUGS% %COMMON%
echo.

echo ---
echo --- Building the OWLNext Unicode Debug DLL (singlethreaded)
echo ---
echo.
%MAKE% -DUNICODE DLL=1 %DEBUGS% %COMMON%
echo.

rem Report outputs

echo ---
echo --- Listing the target outputs after successful build
echo ---
echo.
dir ..\..\lib\owl-*.lib ..\..\bin\owl-*.dll

:exit
