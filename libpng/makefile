# makefile for libpng

TOPDIR=..
!include $(TOPDIR)\common.mak

OBJS = \
        $(OUTDIR)\example.obj \
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
		$(OUTDIR)\pngtest.obj \
		$(OUTDIR)\pngtrans.obj \
		$(OUTDIR)\pngwio.obj \
		$(OUTDIR)\pngwrite.obj \
		$(OUTDIR)\pngwtran.obj \
		$(OUTDIR)\pngwutil.obj

SOURCEDIR = $(LIBPNGDIR)

LIBS = zdll.lib

all: makedirs $(OUTDIR)\libpng.lib

$(OUTDIR)\libpng.lib: $(OBJS)
	$(LINKST) /NOLOGO /MACHINE:X86 /SUBSYSTEM:CONSOLE",5.01" $** $(LIBS) -out:$@
	$(CP) $(OUTDIR)\libpng.lib $(TOPDIR)\lib\
!include $(TOPDIR)\rules.mak
