@echo off

echo.
echo ===============================================
echo  OWLNext Build Script for Microsoft Visual C++
echo ===============================================
echo.

rem If the compiler is configured then try to identify it.
rem Otherwise search for a compiler environment to set up.
rem Try the newest known compiler first.
rem If all fails, quit with a message.

echo ---
echo --- Identifying the compiler
echo ---
echo.

if not defined VCINSTALLDIR goto try14
if not exist "%VCINSTALLDIR%\bin\cl.exe" goto error
goto build

:try14

if not exist "%VS140COMNTOOLS%VSVARS32.BAT" goto try12
call "%VS140COMNTOOLS%VSVARS32.BAT"
goto build

:try12

if not exist "%VS120COMNTOOLS%VSVARS32.BAT" goto try11
call "%VS120COMNTOOLS%VSVARS32.BAT"
goto build

:try11

if not exist "%VS110COMNTOOLS%VSVARS32.BAT" goto try10
call "%VS110COMNTOOLS%VSVARS32.BAT"
goto build

:try10

if not exist "%VS100COMNTOOLS%VSVARS32.BAT" goto try9
call "%VS100COMNTOOLS%VSVARS32.BAT"
goto build

:try9

if not exist "%VS90COMNTOOLS%VSVARS32.BAT" goto try8
call "%VS90COMNTOOLS%VSVARS32.BAT"
goto build

:try8

if not exist "%VS80COMNTOOLS%VSVARS32.BAT" goto try71
call "%VS80COMNTOOLS%VSVARS32.BAT"
goto build

:try71

if not exist "%VS71COMNTOOLS%VSVARS32.BAT" goto error
call "%VS71COMNTOOLS%VSVARS32.BAT"
goto build

:error

echo ---
echo --- Error: Unable to locate a supported compiler.
echo --- See the script source for supported versions.
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
