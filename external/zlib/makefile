# makefile for zlib

TOPDIR=..\..
!include $(TOPDIR)\common.mak

# Turn off treating .c files as C++; zlib code is old-school C
CFLAGS = $(CFLAGS:-TP=-TC)

OBJS = \
    $(OUTDIR)\adler32.obj \
    $(OUTDIR)\compress.obj \
    $(OUTDIR)\crc32.obj \
    $(OUTDIR)\deflate.obj \
    $(OUTDIR)\gzclose.obj \
    $(OUTDIR)\gzlib.obj \
    $(OUTDIR)\gzread.obj \
    $(OUTDIR)\gzwrite.obj \
    $(OUTDIR)\infback.obj \
    $(OUTDIR)\inffast.obj \
    $(OUTDIR)\inflate.obj \
    $(OUTDIR)\inftrees.obj \
    $(OUTDIR)\trees.obj \
    $(OUTDIR)\uncompr.obj \
    $(OUTDIR)\zutil.obj

all: makedirs $(BLAKLIBDIR)\zlib.lib

$(BLAKLIBDIR)\zlib.lib: $(OBJS)
    $(LIBPRG) $** $(LIBS) -out:$@

!include $(TOPDIR)\rules.mak
