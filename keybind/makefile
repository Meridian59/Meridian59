# Makefile for keybind app

TOPDIR=..
!include $(TOPDIR)\common.mak

MSBUILD = msbuild.exe
PROJECT = m59Bind.csproj
LIB = 

all: m59Bind

m59Bind:
!ifdef DEBUG
	$(MSBUILD) /p:configuration=Debug /verbosity:quiet $(PROJECT)
	$(CP) bin\debug\m59Bind.exe $(CLIENTRUNDIR)
!else
	$(MSBUILD) /p:configuration=Release /verbosity:quiet $(PROJECT)
	$(CP) bin\release\m59Bind.exe $(CLIENTRUNDIR)
!endif
   
clean:
	$(RMDIR) /S /Q obj
	$(RMDIR) /S /Q bin