# makefile for zlib

TOPDIR=..
!include $(TOPDIR)\common.mak

# Must compile zlib as C code (-TC) not as C++ (-TP)
CCOMMONFLAGS = $(CCOMMONFLAGS) -TC

OBJS = \
        $(OUTDIR)\adler32.obj \
        $(OUTDIR)\compress.obj \
		$(OUTDIR)\crc32.obj \
        $(OUTDIR)\deflate.obj \
		$(OUTDIR)\gvmat32c.obj \
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

SOURCEDIR = $(ZLIBDIR)

all: makedirs $(BLAKLIBDIR)\zlib.lib

$(BLAKLIBDIR)\zlib.lib: $(OBJS)
	$(LINKST) $(LINKSTFLAGS) /SUBSYSTEM:CONSOLE",5.01" $** $(LIBS) -out:$@
	
	
!include $(TOPDIR)\rules.mak
