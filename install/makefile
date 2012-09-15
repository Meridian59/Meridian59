TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\install

all: makedirs $(OUTDIR)\meridian59-install.exe

$(OUTDIR)\meridian59-install.exe: $(SOURCEDIR)\install.nsi
	$(MAKENSIS) $**

!include $(TOPDIR)\rules.mak
