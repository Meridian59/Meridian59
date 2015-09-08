# makefile for zlib

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TC           Compile as C code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
CFLAGS = $(CFLAGS) /arch:SSE /TC 

# ----------------------------------------------------------------------
# Additional static linker flags (see common.mak)
# /SUBSYSTEM:CONSOLE",5.01" NO-UI Windows XP (5.01)
LINKSTFLAGS = $(LINKSTFLAGS) /SUBSYSTEM:CONSOLE",5.01"

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

all: makedirs $(BLAKLIBDIR)\zlib.lib

$(BLAKLIBDIR)\zlib.lib: $(OBJS)
    $(LINKST) $(LINKSTFLAGS) $** $(LIBS) -out:$@
		
!include $(TOPDIR)\rules.mak
