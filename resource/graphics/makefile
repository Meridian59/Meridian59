# makefile for object bitmaps

TOPDIR=..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = \
	$(OUTDIR)\arrowfir.bgf \
	$(OUTDIR)\arrowsil.bgf \
	$(OUTDIR)\arrowner.bgf \
	$(OUTDIR)\mystswd.bgf \
	$(OUTDIR)\mystwdov.bgf \
	$(OUTDIR)\ring3.bgf \
	$(OUTDIR)\povmystsw.bgf \
	premadebgf\holdrings.bgf \
	premadebgf\holdproj.bgf \
	premadebgf\blindproj.bgf \
	premadebgf\dazzleproj.bgf \

all: makedirs $(BGFS)

{$(SOURCEDIR)}.bbg.bgf:
	$(MAKEBGF) -o $@ @$<

!include $(TOPDIR)\rules.mak
