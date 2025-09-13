# makefile for fmtlib

TOPDIR=..\..
!include $(TOPDIR)\common.mak

# Use only the traditional source files (no modules)
OBJS = \
    $(OUTDIR)\format.obj \
    $(OUTDIR)\os.obj

all: makedirs $(BLAKLIBDIR)\fmtlib.lib

$(BLAKLIBDIR)\fmtlib.lib: $(OBJS)
    $(LIBPRG) $** $(LIBS) -out:$@

# Compile rules for source files
$(OUTDIR)\format.obj: src\format.cc
    $(CC) $(CFLAGS) -I. -Iinclude -c $** -Fo$@

$(OUTDIR)\os.obj: src\os.cc
    $(CC) $(CFLAGS) -I. -Iinclude -c $** -Fo$@

!include $(TOPDIR)\rules.mak