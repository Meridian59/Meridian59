# Makefile for keybind app

TOPDIR=..
!include $(TOPDIR)\common.mak

MSBUILD = msbuild.exe
PROJECT = m59Bind.csproj
LIB = 

all: m59Bind

m59Bind:
!ifdef DEBUG
	$(MSBUILD) /p:configuration=Debug,platform=x86 /verbosity:quiet $(PROJECT)
	$(CP) bin\x86\debug\m59Bind.exe $(CLIENTRUNDIR)
!else
	$(MSBUILD) /p:configuration=Release,platform=x86 /verbosity:quiet $(PROJECT)
	$(CP) bin\x86\release\m59Bind.exe $(CLIENTRUNDIR)
!endif

clean:
	@$(RMDIR) /S /Q obj >nul 2>&1
	@$(RMDIR) /S /Q bin >nul 2>&1