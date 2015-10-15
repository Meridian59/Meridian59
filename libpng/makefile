# makefile for libpng

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP           Compile as C++ code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
CFLAGS = $(CFLAGS) /arch:SSE /TP /D_USING_V110_SDK71_

# ----------------------------------------------------------------------
# Additional static linker flags (see common.mak)
# /SUBSYSTEM:CONSOLE",5.01" NO-UI Windows XP (5.01)
LINKSTFLAGS = $(LINKSTFLAGS) /SUBSYSTEM:CONSOLE",5.01"

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

all: makedirs $(BLAKLIBDIR)\libpng.lib

$(BLAKLIBDIR)\libpng.lib: $(OBJS)
	$(LINKST) $(LINKSTFLAGS) $** $(LIBS) -out:$@

!include $(TOPDIR)\rules.mak
