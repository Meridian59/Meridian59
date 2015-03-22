# makefile for textures

TOPDIR=..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\textures

.SUFFIXES: .bmp

BGFS = \
	$(OUTDIR)\grd20121.bgf \
	$(OUTDIR)\grd20122.bgf \
	$(OUTDIR)\grd20124.bgf \
	$(OUTDIR)\grd20125.bgf \
	$(OUTDIR)\grd20126.bgf \
	$(OUTDIR)\grd20127.bgf \
	$(OUTDIR)\grd20128.bgf \
	$(OUTDIR)\grd20129.bgf \
	$(OUTDIR)\grd20130.bgf \

all: makedirs $(BGFS)

{$(SOURCEDIR)}.bmp.bgf:
	$(MAKEBGF) -o $@ -n "Unnamed $*" 1 $< 1 1 1

# bitmaps for walls, ceilings, and floors

$(OUTDIR)\grd20121.bgf:
	$(MAKEBGF) -o $@ -n "tosin-base-floor" -s 8 1 $** 1 1 1

$(OUTDIR)\grd20122.bgf:
	$(MAKEBGF) -o $@ -n "tosin-base-ceil" -s 1 1 $** 1 1 1

$(OUTDIR)\grd20124.bgf:
	$(MAKEBGF) -o $@ -n "tos inn h" -s 2 1 $** 1 1 1

$(OUTDIR)\grd20125.bgf:
	$(MAKEBGF) -o $@ -n "tos inn i floor" -s 1 1 $** 1 1 1

$(OUTDIR)\grd20126.bgf:
	$(MAKEBGF) -o $@ -n "tos-escape-tunnel-w" -s 4 1 $** 1 1 1

$(OUTDIR)\grd20127.bgf:
	$(MAKEBGF) -o $@ -n "tos-escape-tunnel-w2" -s 4 1 $** 1 1 1

$(OUTDIR)\grd20128.bgf:
	$(MAKEBGF) -o $@ -n "tos-escape-tunnel-door" -s 4 1 $** 1 1 1

$(OUTDIR)\grd20129.bgf:
	$(MAKEBGF) -o $@ -n "tos-escape-tunnel-w3" -s 4 1 $** 1 1 1

$(OUTDIR)\grd20130.bgf:
	$(MAKEBGF) -o $@ -n "mudroot" -s 4 1 $** 1 1 1


!include $(TOPDIR)\rules.mak
