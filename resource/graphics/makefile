# makefile for object bitmaps

TOPDIR=..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = \
	arrowfir.bgf \
	arrowsil.bgf \
	arrowner.bgf \

all: makedirs $(BGFS)

{$(SOURCEDIR)}.bbg.bgf:
	$(MAKEBGF) -o $(OUTDIR)\$*.bgf @$(SOURCEDIR)\$*.bbg

!include $(TOPDIR)\rules.mak
