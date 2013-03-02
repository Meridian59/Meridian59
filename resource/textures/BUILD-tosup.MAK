# makefile for bitmaps tosupdate 2013 v1

!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\textures
OUTDIR = $(TOPDIR)\resource\bin

WALL = -r

.SUFFIXES: .bmp

{$(SOURCEDIR)}.bmp.bgf:
	makebgf -o $*.bgf -n "Unnamed $*" 1 $< 1 1 1

# currently fried ones
# none


BGFS = \
	grd20121.bgf \
	grd20122.bgf \
	grd20124.bgf \
	grd20125.bgf \
	grd20126.bgf \
	grd20127.bgf \
	grd20128.bgf \
	grd20129.bgf \
	grd20130.bgf \

all:  $(BGFS)

# bitmaps for walls, ceilings, and floors

grd20121.bgf:
	makebgf -o $*.bgf -r -n "tosin-base-floor" -s 8 1 $(SOURCEDIR)\grd20121.BMP 1 1 1

grd20122.bgf:
	makebgf -o $*.bgf -r -n "tosin-base-ceil" -s 1 1 $(SOURCEDIR)\grd20122.BMP 1 1 1

grd20124.bgf:
	makebgf -o $*.bgf -r -n "tos inn h" -s 2 1 $(SOURCEDIR)\grd20124.BMP 1 1 1

grd20125.bgf:
	makebgf -o $*.bgf -r -n "tos inn i floor" -s 1 1 $(SOURCEDIR)\grd20125.BMP 1 1 1
 
grd20126.bgf:
	makebgf -o $*.bgf -r -n "tos-escape-tunnel-w" -s 4 1 $(SOURCEDIR)\grd20126.BMP 1 1 1

grd20127.bgf:
	makebgf -o $*.bgf -r -n "tos-escape-tunnel-w2" -s 4 1 $(SOURCEDIR)\grd20127.BMP 1 1 1

grd20128.bgf:
	makebgf -o $*.bgf -r -n "tos-escape-tunnel-door" -s 4 1 $(SOURCEDIR)\grd20128.BMP 1 1 1

grd20129.bgf:
	makebgf -o $*.bgf -r -n "tos-escape-tunnel-w3" -s 4 1 $(SOURCEDIR)\grd20129.BMP 1 1 1
  
grd20130.bgf:
	makebgf -o $*.bgf -r -n "mudroot" -s 4 1 $(SOURCEDIR)\grd20130.BMP 1 1 1
          
clean: 
	$(RM) $(OUTDIR)\*

squeakyclean: clean

