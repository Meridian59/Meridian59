# makefile for libpng

TOPDIR=..\..
!include $(TOPDIR)\common.mak

LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:CONSOLE",5.01"

OBJS = \
    $(OUTDIR)\png.obj \
    $(OUTDIR)\pngerror.obj \
    $(OUTDIR)\pngget.obj \
    $(OUTDIR)\pngmem.obj \
    $(OUTDIR)\pngpread.obj \
    $(OUTDIR)\pngread.obj \
    $(OUTDIR)\pngrio.obj \
    $(OUTDIR)\pngrtran.obj \
    $(OUTDIR)\pngrutil.obj \
    $(OUTDIR)\pngset.obj \
    $(OUTDIR)\pngtrans.obj \
    $(OUTDIR)\pngwio.obj \
    $(OUTDIR)\pngwrite.obj \
    $(OUTDIR)\pngwtran.obj \
    $(OUTDIR)\pngwutil.obj

all: makedirs $(BLAKLIBDIR)\libpng.lib

$(BLAKLIBDIR)\libpng.lib: $(OBJS)
	$(LIBPRG) $** $(LIBS) -out:$@

!include $(TOPDIR)\rules.mak
