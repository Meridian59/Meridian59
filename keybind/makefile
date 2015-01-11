# Makefile for keybind app

TOPDIR=..
!include $(TOPDIR)\common.mak

MSBUILD = msbuild.exe
PROJECT = m59Bind.csproj
LIB = 

all: m59Bind

m59Bind:
	$(MSBUILD) $(PROJECT)
	$(CP) bin\debug\m59Bind.exe $(CLIENTRUNDIR)

clean:
	$(RMDIR) /S /Q obj
	$(RMDIR) /S /Q bin