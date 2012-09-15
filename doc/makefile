# makefile for documentation directory

TOPDIR = ..
!include $(TOPDIR)\common.mak

# Latex doesn't work if filename has backslashes

SOURCEDIR = .

.SUFFIXES: .tex

all: overview.dvi blakod.dvi

{$(SOURCEDIR)}.tex.dvi:
	$(LATEX) $(SOURCEDIR)/$(<F)

clean:
	$(RM) $(OUTDIR)\*
