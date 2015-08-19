# makefile for libpng

TOPDIR=..
!include $(TOPDIR)\common.mak

OBJS = \
        $(OUTDIR)\example.obj \
        $(OUTDIR)\png.obj \
		$(OUTDIR)\pngerr.obj \
        $(OUTDIR)\pngget.obj \
		$(OUTDIR)\pngmem.obj \
        $(OUTDIR)\pngpread.obj \
        $(OUTDIR)\pngread.obj \
        $(OUTDIR)\pngrio.obj \
		$(OUTDIR)\pngrtran.obj \
		$(OUTDIR)\pngrutil.obj \
		$(OUTDIR)\pngset.obj \
		$(OUTDIR)\pngtest.obj \
		$(OUTDIR)\pngtrans.obj \
		$(OUTDIR)\pngwio.obj \
		$(OUTDIR)\pngwrite.obj \
		$(OUTDIR)\pngwtran.obj \
		$(OUTDIR)\pngwutil.obj

SOURCEDIR = $(LIBPNGDIR)

LIBS = zdll.lib

all: makedirs $(BLAKLIBDIR)\libpng.lib

$(BLAKLIBDIR)\libpng.lib: $(OBJS)
	$(LINKST) $(LINKSTFLAGS) /SUBSYSTEM:CONSOLE",5.01" $** $(LIBS) -out:$@
!include $(TOPDIR)\rules.mak
