@echo off

echo.
echo ===============================================
echo  OWLNext Build Script for Microsoft Visual C++
echo ===============================================
echo.

rem Check that the compiler is configured.

if defined VISUALSTUDIOVERSION goto build
echo ---
echo --- Error: VISUALSTUDIOVERSION is undefined.
echo --- Run the environment setup script for the compiler (e.g. vcvars64.bat).
echo ---
goto exit

:build

echo.
set _MAKE=nmake -fvc.mak
set _COMMON=%1 %2 %3 %4 %5 %6 %7 %8 %9 

rem Non-Unicode variants

echo ---
echo --- Building the OWLNext Static Debug Library
echo ---
echo.
echo %_MAKE% DEBUG=1 %_COMMON%
%_MAKE% DEBUG=1 %_COMMON%
if errorlevel 1 goto exit
echo.

echo ---
echo --- Building the OWLNext Static Release Library
echo ---
echo.
echo %_MAKE% %_COMMON%
%_MAKE% %_COMMON%
if errorlevel 1 goto exit
echo.

echo ---
echo --- Building the OWLNext Debug DLL
echo ---
echo.
echo %_MAKE% DLL=1 DEBUG=1 %_COMMON% 
%_MAKE% DLL=1 DEBUG=1 %_COMMON% 
if errorlevel 1 goto exit
echo.

echo ---
echo --- Building the OWLNext Release DLL
echo ---
echo.
echo %_MAKE% DLL=1 %_COMMON% 
%_MAKE% DLL=1 %_COMMON% 
if errorlevel 1 goto exit
echo.

rem Unicode variants

echo ---
echo --- Building the OWLNext Unicode Static Debug Library
echo ---
echo.
echo %_MAKE% UNICODE=1 DEBUG=1 %_COMMON%
%_MAKE% UNICODE=1 DEBUG=1 %_COMMON%
if errorlevel 1 goto exit
echo.

echo ---
echo --- Building the OWLNext Unicode Static Release Library
echo ---
echo.
echo %_MAKE% UNICODE=1 %_COMMON%
%_MAKE% UNICODE=1 %_COMMON%
if errorlevel 1 goto exit
echo.

echo ---
echo --- Building the OWLNext Unicode Debug DLL
echo ---
echo.
echo %_MAKE% UNICODE=1 DLL=1 DEBUG=1 %_COMMON% 
%_MAKE% UNICODE=1 DLL=1 DEBUG=1 %_COMMON% 
if errorlevel 1 goto exit
echo.

echo ---
echo --- Building the OWLNext Unicode Release DLL
echo ---
echo.
echo %_MAKE% UNICODE=1 DLL=1 %_COMMON% 
%_MAKE% UNICODE=1 DLL=1 %_COMMON% 
if errorlevel 1 goto exit
echo.

rem Report outputs

echo ---
echo --- Listing the target outputs
echo ---
echo.
echo dir ..\..\lib\owl-*.lib ..\..\bin\owl-*.dll
echo.
dir ..\..\lib\owl-*.lib ..\..\bin\owl-*.dll

:exit

rem Clear local symbols.

set _MAKE=
set _COMMON=
