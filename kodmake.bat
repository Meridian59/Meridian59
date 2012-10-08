@echo off
echo Starting Kodmake.
echo Checking for uncompiled KODs....
for /R .\kod\ %%k in (*.kod) do (
	if not exist "%%~dk%%~pk%%~nk.bof" (
		if not exist "%%~dk%%~pkmakefile" echo makefile not found in %%~dk%%~pk. Please create the makefile manually. Aborting... & goto:eof
	)
)

cd kod
nmake
cd ..

echo Clearing BOF files...
del .\run\server\memmap\*.bof 2>nul
echo Copying BOF files...
for /R .\kod\ %%f in (*.bof) do xcopy "%%f" .\run\server\loadkod\ /y >nul

echo Clearing RSC files...
del .\run\localclient\resource\*.rsc 2>nul
del .\run\server\rsc\*.rsc 2>nul
echo Copying RSC files....
for /R .\kod\ %%f in (*.rsc) do xcopy "%%f" .\run\server\rsc\ /y >nul
echo Building RSB file....
.\bin\rscmerge.exe -o .\run\localclient\resource\rsc0000.rsb .\run\server\rsc\*.rsc
if "%errorlevel%"=="0" echo RSB Success.

echo Kodmake Successful!