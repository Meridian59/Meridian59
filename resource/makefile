# makefile for bitmaps

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource
OUTDIR = $(SOURCEDIR)\bin

all:
	@for %i in (graphics modules music rooms sounds textures) do \
		(echo Building %i... & \
		cd %i & \
	 	$(MAKE) & \
		cd .. )
