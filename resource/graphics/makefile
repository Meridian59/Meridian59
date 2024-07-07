# makefile for object bitmaps

TOPDIR=..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg .bmp

BGFS = \
	$(OUTDIR)\arrowfir.bgf \
	$(OUTDIR)\arrowsil.bgf \
	$(OUTDIR)\arrowner.bgf \
	$(OUTDIR)\mystswd.bgf \
	$(OUTDIR)\mystwdov.bgf \
	$(OUTDIR)\ring3.bgf \
	$(OUTDIR)\povmystsw.bgf \
	$(OUTDIR)\splash.bgf \
	$(OUTDIR)\moon.bgf \

all: makedirs $(BGFS)

{$(SOURCEDIR)}.bbg.bgf:
	$(MAKEBGF) -o $@ @$<

# How to make a BGF from a single bitmap file
{$(SOURCEDIR)}.bmp.bgf:
	$(MAKEBGF) -o $@ 1 $< 1 1 1

!include $(TOPDIR)\rules.mak
